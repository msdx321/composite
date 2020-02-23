#include <thd.h>
#include <inv.h>
#include <hw.h>

#include "isr.h"
#include "kernel.h"
#include "chal/cpuid.h"

/*
 * These addressess are specified as offsets from the base HPET
 * pointer, which is a 1024-byte region of memory-mapped
 * registers. The reason we use offsets rather than a struct or
 * bitfields is that ALL accesses, both read and write, must be
 * aligned at 32- or 64-bit boundaries and must read or write an
 * entire 32- or 64-bit value at a time. Packed structs cause GCC to
 * produce code which attempts to operate on the single byte level,
 * which fails.
 */

#define HPET_OFFSET(n) ((unsigned char *)hpet + n)

#define HPET_CAPABILITIES (0x0)
#define HPET_CONFIGURATION (0x10)
#define HPET_INTERRUPT (0x20)
#define HPET_COUNTER (*(u64_t *)(HPET_OFFSET(0xf0)))

#define HPET_T0_CONFIG (0x100)
#define HPET_Tn_CONFIG(n) HPET_OFFSET(HPET_T0_CONFIG + (0x20 * n))

#define HPET_T0_COMPARATOR (0x108)
#define HPET_Tn_COMPARATOR(n) HPET_OFFSET(HPET_T0_COMPARATOR + (0x20 * n))

#define HPET_T0_INTERRUPT (0x110)
#define HPET_Tn_INTERRUPT(n) HPET_OFFSET(HPET_T0_INTERRUPT + (0x20 * n))

#define HPET_ENABLE_CNF (1ll)
#define HPET_LEG_RT_CNF (1ll << 1)

#define HPET_TAB_LENGTH (0x4)
#define HPET_TAB_ADDRESS (0x2c)

/* Bits in HPET_Tn_CONFIG */
/* 1 << 0 is reserved */
#define HPET_TN_INT_TYPE_CNF (1ll << 1) /* 0 = edge trigger, 1 = level trigger */
#define HPET_TN_INT_ENB_CNF (1ll << 2)  /* 0 = no interrupt, 1 = interrupt */
#define HPET_TN_TYPE_CNF (1ll << 3)     /* 0 = one-shot, 1 = periodic */
#define HPET_TN_PER_INT_CAP (1ll << 4)  /* read only, 1 = periodic supported */
#define HPET_TN_SIZE_CAP (1ll << 5)     /* 0 = 32-bit, 1 = 64-bit */
#define HPET_TN_VAL_SET_CNF (1ll << 6)  /* set to allow directly setting accumulator */
/* 1 << 7 is reserved */
#define HPET_TN_32MODE_CNF (1ll << 8)           /* 1 = force 32-bit access to 64-bit timer */
#define HPET_TN_INT_ROUTE_CNF (9) 	/* routing for interrupt */
#define HPET_TN_FSB_EN_CNF (1ll << 14)          /* 1 = deliver interrupts via FSB instead of APIC */
#define HPET_TN_FSB_INT_DEL_CAP (1ll << 15)     /* read only, 1 = FSB delivery available */

#define HPET_INT_ENABLE(n) (*hpet_interrupt = (0x1 << n)) /* Clears the INT n for level-triggered mode. */

#define __USECS_CEIL__(n, m) (n+(m-(n%m)))

static volatile u32_t *hpet_capabilities;
static volatile u64_t *hpet_config;
static volatile u64_t *hpet_interrupt;
static void           *hpet;

volatile struct hpet_timer {
	u64_t config;
	u64_t compare;
	u64_t interrupt;
	u64_t reserved;
} __attribute__((packed)) *hpet_timers;

/*
 * When determining how many CPU cycles are in a HPET tick, we must
 * execute a number of periodic ticks (HPET_CALIBRATION_ITER) at a
 * controlled interval, and use the HPET tick granularity to compute
 * how many CPU cycles per HPET tick there are.  Unfortunately, this
 * can be quite low (e.g. HPET tick of 10ns, CPU tick of 2ns) leading
 * to rounding error that is a significant fraction of the conversion
 * factor.
 *
 * Practically, this will lead to the divisor in the conversion being
 * smaller than it should be, thus causing timers to go off _later_
 * than they should.  Thus we use a multiplicative factor
 * (HPET_ERROR_BOUND_FACTOR) to lessen the rounding error.
 *
 * All of the hardware is documented in the HPET specification @
 * http://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf
 */

#define ULONG_MAX 4294967295UL
#define HPET_PICO_PER_MICRO 1000000UL
#define HPET_FEMPTO_PER_PICO 1000UL
#define HPET_CALIBRATION_ITER 256
#define HPET_ERROR_BOUND_FACTOR 256
#define HPET_DEFAULT_PERIOD_US 1000 /* US = microseconds */

#define HPET_NUMBER_TIMERS 2
static int           hpet_calibration_init   = 0;
static unsigned long hpet_cpucyc_per_hpetcyc = HPET_ERROR_BOUND_FACTOR;
static unsigned long hpet_cpucyc_per_tick;
static unsigned long hpet_hpetcyc_per_tick;
static unsigned long hpet_periodicity_curr[HPET_NUMBER_TIMERS] = { 0 };
static cycles_t hpet_first_hpet_period = 0; /* for timer 0 = HPET_PERIODIC */
extern u32_t chal_msr_mhz;

static inline u64_t
hpet_cpu2hpet_cycles(u64_t cycles)
{
	unsigned long cyc;

	/* demote precision to enable word-sized math */
	cyc = (unsigned long)cycles;
	if (unlikely((u64_t)cyc < cycles)) cyc = ULONG_MAX;
	/* convert from CPU cycles to HPET cycles */
	cyc = (cyc / hpet_cpucyc_per_hpetcyc) * HPET_ERROR_BOUND_FACTOR;
	/* promote the precision to interact with the hardware correctly */
	cycles = cyc;

	return cycles;
}

static void
hpet_disable(hpet_type_t timer_type)
{
	/* Disable timer interrupts */
	*hpet_config &= ~HPET_ENABLE_CNF;

	/* Disable timer interrupt of timer_type */
	hpet_timers[timer_type].config  = 0;
	hpet_timers[timer_type].compare = 0;

	/* Enable timer interrupts */
	*hpet_config |= HPET_ENABLE_CNF;
}

static void
hpet_calibration(void)
{
	static int   cnt       = 0;
	static u64_t cycle     = 0, tot = 0, prev;
	static u32_t apic_curr = 0, apic_tot = 0, apic_prev;

	/* calibration only on BSP */
	assert(get_cpuid() == INIT_CORE);

	prev      = cycle;
	apic_prev = apic_curr;
	rdtscll(cycle);
	apic_curr = lapic_get_ccr();

	if (cnt) {
		tot += cycle - prev;
		apic_tot += (apic_prev - apic_curr);
	}
	if (cnt >= HPET_CALIBRATION_ITER) {
		assert(hpet_hpetcyc_per_tick);
		hpet_calibration_init = 0;
		hpet_cpucyc_per_tick  = (unsigned long)(tot / HPET_CALIBRATION_ITER);
		assert(hpet_cpucyc_per_tick > hpet_hpetcyc_per_tick);

		if (lapic_timer_calib_init) {
			u32_t cycs_to_apic_ratio = 0, apic_cycs_per_tick = 0;

			apic_cycs_per_tick = apic_tot / HPET_CALIBRATION_ITER;
			assert(apic_cycs_per_tick);

			cycs_to_apic_ratio = hpet_cpucyc_per_tick / apic_cycs_per_tick;
			lapic_timer_calibration(cycs_to_apic_ratio);
		}

		/* Possibly significant rounding error here.  Bound by the factor */
		hpet_cpucyc_per_hpetcyc = (HPET_ERROR_BOUND_FACTOR * hpet_cpucyc_per_tick) / hpet_hpetcyc_per_tick;
		printk("Timer calibrated:\n\tCPU cycles per HPET tick: %ld\n\tHPET ticks in %d us: %ld\n",
		       hpet_cpucyc_per_hpetcyc / HPET_ERROR_BOUND_FACTOR, HPET_DEFAULT_PERIOD_US,
		       hpet_hpetcyc_per_tick);

		hpet_disable(HPET_PERIODIC);
		hpet_disable(HPET_PERIODIC);
		chal_irq_disable(HW_HPET_PERIODIC, 0);
	}
	cnt++;
}

int
chal_cyc_usec(void)
{
	if (unlikely(lapic_timer_calib_init || hpet_calibration_init)) return 0;

	if (likely(hpet_cpucyc_per_tick)) return hpet_cpucyc_per_tick / HPET_DEFAULT_PERIOD_US;

	return 0;
}

int
hpet_periodic_handler(struct pt_regs *regs)
{
	int preempt = 1;

	lapic_ack();
	if (unlikely(hpet_calibration_init)) hpet_calibration();
	if (unlikely(hpet_periodicity_curr[HPET_PERIODIC] && !hpet_first_hpet_period)) rdtscll(hpet_first_hpet_period);

	preempt = cap_hw_asnd(&hw_asnd_caps[get_cpuid()][HW_HPET_PERIODIC], regs);
	HPET_INT_ENABLE(HPET_PERIODIC);

	return preempt;
}

int
hpet_oneshot_handler(struct pt_regs *regs)
{
	int preempt = 1;

	assert(!hpet_calibration_init && hpet_periodicity_curr[HPET_ONESHOT]);

	lapic_ack();
	preempt = cap_hw_asnd(&hw_asnd_caps[get_cpuid()][HW_HPET_ONESHOT], regs);
	HPET_INT_ENABLE(HPET_ONESHOT);

	return preempt;
}

void
hpet_set(hpet_type_t timer_type, u64_t cycles)
{
	u64_t outconfig = HPET_TN_INT_TYPE_CNF | HPET_TN_INT_ENB_CNF;

	/* Disable timer interrupts */
	*hpet_config &= ~HPET_ENABLE_CNF;

	/* TODO: doesn't work.. at least in legacy mode.. it just fires ONESHOT! */
	/* using hpet type HPET_ONESHOT for periodic timer */
	/* Reset main counter */
//	if (timer_type == HPET_ONESHOT) {
//		cycles = hpet_cpu2hpet_cycles(cycles);
//
//		/* Set a static value to count up to */
//		hpet_timers[timer_type].config = outconfig;
////		hpet_timers[timer_type].config |= (HW_HPET_ONESHOT - HW_IRQ_START) << HPET_TN_INT_ROUTE_CNF;
//		cycles += HPET_COUNTER;
//	} else {
		/* Set a periodic value */
		hpet_timers[timer_type].config = outconfig | HPET_TN_TYPE_CNF | HPET_TN_VAL_SET_CNF;
		/* Set the interrupt vector for periodic timer */
//		hpet_timers[timer_type].config |= (HW_HPET_PERIODIC - HW_IRQ_START) << HPET_TN_INT_ROUTE_CNF;
		/* Reset main counter */
		/* TODO: should you reset counter for each timer? */
		HPET_COUNTER = 0x00;
//	}
	hpet_timers[timer_type].compare = cycles;

	/* Enable timer interrupts */
	*hpet_config |= HPET_ENABLE_CNF;
}

u64_t
hpet_find(void *timer)
{
	u32_t          i;
	unsigned char  sum      = 0;
	unsigned char *hpetaddr = timer;
	u32_t          length   = *(u32_t *)(hpetaddr + HPET_TAB_LENGTH);

	printk("Initializing HPET @ %p\n", hpetaddr);

	for (i = 0; i < length; i++) {
		sum += hpetaddr[i];
	}

	if (sum == 0) {
		u64_t addr = *(u64_t *)(hpetaddr + HPET_TAB_ADDRESS);
		printk("\tChecksum is OK\n");
		printk("\tAddr: %016llx\n", addr);
		hpet = (void *)((u32_t)(addr & 0xffffffff));
		printk("\thpet: %p\n", hpet);
		return addr;
	}

	printk("\tInvalid checksum (%d)\n", sum);
	return 0;
}

void
chal_hpet_periodic_set(hwid_t hwid, unsigned long usecs_period)
{
	hpet_type_t type = 0;

	assert(hwid == HW_HPET_PERIODIC || hwid == HW_HPET_ONESHOT);
	type = (hwid == HW_HPET_PERIODIC ? HPET_PERIODIC : HPET_ONESHOT);

	if (hpet_periodicity_curr[type] != usecs_period) {
		hpet_disable(type);
		hpet_disable(type);

		hpet_periodicity_curr[type] = 0;
	}

	if (hpet_periodicity_curr[type] == 0) {
		unsigned long tick_multiple = 0;
		cycles_t hpetcyc_per_period = 0;

		assert(hpet_calibration_init == 0);
		assert((usecs_period >= HPET_DEFAULT_PERIOD_US) && (usecs_period % HPET_DEFAULT_PERIOD_US == 0));

		tick_multiple = usecs_period / HPET_DEFAULT_PERIOD_US;
		hpetcyc_per_period = (cycles_t)hpet_hpetcyc_per_tick * (cycles_t)tick_multiple;
		hpet_periodicity_curr[type] = usecs_period;
		if (type == HPET_PERIODIC) hpet_first_hpet_period = 0;
		hpet_set(type, hpetcyc_per_period);
		printk("Setting HPET [%u:%u] Periodicity:%lu hpetcyc_per_period:%llu\n", hwid, type, usecs_period, hpetcyc_per_period);
	}
}

cycles_t
chal_hpet_first_period(void)
{
	return hpet_first_hpet_period;
}

void
chal_hpet_disable(hwid_t hwid)
{
	printk("Disabling HPET %u\n", hwid);
	hpet_type_t type = (hwid == HW_HPET_PERIODIC ? HPET_PERIODIC : HPET_ONESHOT);

	hpet_disable(type);
	hpet_disable(type);
}

void
hpet_set_page(u32_t page)
{
	hpet              = (void *)(page * (1 << 22) | ((u32_t)hpet & ((1 << 22) - 1)));
	hpet_capabilities = (u32_t *)((unsigned char *)hpet + HPET_CAPABILITIES);
	hpet_config       = (u64_t *)((unsigned char *)hpet + HPET_CONFIGURATION);
	hpet_interrupt    = (u64_t *)((unsigned char *)hpet + HPET_INTERRUPT);
	hpet_timers       = (struct hpet_timer *)((unsigned char *)hpet + HPET_T0_CONFIG);

	printk("\tSet HPET @ %p\n", hpet);
}

void
hpet_init(void)
{
	unsigned long pico_per_hpetcyc;

	assert(hpet_capabilities);
	/* bits 32-63 are # of femptoseconds per HPET clock tick */
	pico_per_hpetcyc      = hpet_capabilities[1] / HPET_FEMPTO_PER_PICO;
	hpet_hpetcyc_per_tick = (HPET_DEFAULT_PERIOD_US * HPET_PICO_PER_MICRO) / pico_per_hpetcyc;

	printk("Enabling timer @ %p with tick granularity %ld picoseconds\n", hpet, pico_per_hpetcyc);

	/*
	 * FIXME: For some reason, setting to non-legacy mode isn't working well.
	 * Periodicity of the HPET fired is wrong and any interval configuration
	 * is still producing the same wrong interval timing.
	 *
	 * So, Enable legacy interrupt routing like we had before!
	 */
	*hpet_config |= HPET_LEG_RT_CNF;

	/*
	 * Set the timer as specified.  This assumes that the cycle
	 * specification is in hpet cycles (not cpu cycles).
	 */
	if (chal_msr_mhz && !lapic_timer_calib_init) {
		hpet_cpucyc_per_tick    = chal_msr_mhz * HPET_DEFAULT_PERIOD_US;
		hpet_cpucyc_per_hpetcyc = hpet_cpucyc_per_tick / hpet_hpetcyc_per_tick;
		printk("Timer not calibrated, instead computed using MSR frequency value\n");

		return;
	}

	hpet_calibration_init = 1;
	hpet_set(HPET_PERIODIC, hpet_hpetcyc_per_tick);
	chal_irq_enable(HW_HPET_PERIODIC, 0);
	chal_irq_disable(HW_HPET_ONESHOT, 0);
}
