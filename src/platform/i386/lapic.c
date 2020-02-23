#include "chal/chal_config.h"
#include "kernel.h"
#include "chal_cpu.h"
#include "isr.h"
#include "apic_cntl.h"

#define LAPIC_MAX NUM_CPU

int ncpus = 1;
int apicids[NUM_CPU];
u32_t logical_apicids[NUM_CPU];

#define CMOS_PORT    0x70

#define LAPIC_ID_REG             0x020 /* APIC id */
#define LAPIC_VERSION_REG        0x030 /* version */
#define LAPIC_TP_REG             0x080 /* Task Priority Register */

#define LAPIC_LDR_REG            0x0D0 /* Logical destination register */
#define LAPIC_SIV_REG            0x0F0 /* spurious interrupt vector */
#define LAPIC_SIV_ENABLE         (1 << 8) /* enable bit in the SIV */
#define LAPIC_EOI_REG            0x0B0 /* ack, or end-of-interrupt */
#define LAPIC_ESR                0x280 /* error status register */
#define LAPIC_ICR                0x300 /* interrupt control register */
#define LAPIC_PCINT              0x340 /* Performance counter interrupts */
#define LAPIC_LINT0              0x350 /* Local interrupt/vector table 0 */
#define LAPIC_LINT1              0x360 /* Local interrupt/vector table 1 */
#define LAPIC_INT_MASKED         (1<<17)
#define LAPIC_ERROR              0x370 /* Local interrupt/vector table error */

#define LAPIC_TIMER_LVT_REG 0x320
#define LAPIC_TIMER_MASKED (1<<16)
#define LAPIC_DIV_CONF_REG 0x3e0
#define LAPIC_INIT_COUNT_REG 0x380
#define LAPIC_CURR_COUNT_REG 0x390

#define LAPIC_PERIODIC_MODE (0x01 << 17)
#define LAPIC_ONESHOT_MODE (0x00 << 17)
#define LAPIC_TSCDEADLINE_MODE (0x02 << 17)

#define LAPIC_TIMER_CALIB_VAL 0xffffffff

/* flags for the interrupt control register ICR */
#define LAPIC_ICR_LEVEL          (1 << 15) /* level vs edge mode */
#define LAPIC_ICR_ASSERT         (1 << 14) /* assert */
#define LAPIC_ICR_LOGICAL        (1 << 11) /* logical destination */
#define LAPIC_ICR_STATUS         (1 << 12)
#define LAPIC_ICR_INIT           0x500     /* INIT */
#define LAPIC_ICR_SIPI           0x600     /* Startup IPI */
#define LAPIC_ICR_FIXED          0x000     /* fixed IPI */
#define LAPIC_IPI_ASND_VEC       HW_LAPIC_IPI_ASND /* interrupt vec for asnd ipi */

#define IA32_MSR_TSC_DEADLINE 0x000006e0

#define LAPIC_TIMER_MIN (1 << 12)
#define LAPIC_COUNTER_MIN (1 << 3)

#define LAPIC_ONESHOT_THRESH (1 << 12)
#define LAPIC_TSCDEADLINE_THRESH 0

#define LAPIC_LDR_OFFSET 24
#define LAPIC_LDR_MAST (0xfful << LAPIC_LDR_OFFSET)

extern int timer_process(struct pt_regs *regs);

enum lapic_timer_type
{
	LAPIC_ONESHOT = 0,
	LAPIC_PERIODIC,
	LAPIC_TSC_DEADLINE,
};

enum lapic_timer_div_by_config
{
	LAPIC_DIV_BY_2 = 0,
	LAPIC_DIV_BY_4,
	LAPIC_DIV_BY_8,
	LAPIC_DIV_BY_16,
	LAPIC_DIV_BY_32 = 8,
	LAPIC_DIV_BY_64,
	LAPIC_DIV_BY_128,
	LAPIC_DIV_BY_1,
};

static volatile void *lapic             = (void *)APIC_DEFAULT_PHYS;
static unsigned int   lapic_timer_mode  = LAPIC_TSC_DEADLINE;
static unsigned int   lapic_is_disabled[NUM_CPU] CACHE_ALIGNED;

static atomic_t lapic_asnd_ipi_snd_counter[NUM_CPU] CACHE_ALIGNED;
static atomic_t lapic_asnd_ipi_rcv_counter[NUM_CPU] CACHE_ALIGNED;

static volatile unsigned int lapic_cycs_thresh        = 0;
static volatile u32_t        lapic_cpu_to_timer_ratio = 0;
volatile u32_t               lapic_timer_calib_init   = 0;

static void
lapic_write_reg(u32_t off, u32_t val)
{
	assert(lapic);

	*(volatile u32_t *)(lapic + off) = val;
}

void
lapic_ack(void)
{
	lapic_write_reg(LAPIC_EOI_REG, 0);
}

static u32_t
lapic_read_reg(u32_t off)
{
	assert(lapic);

	return *(volatile u32_t *)(lapic + off);
}

static int
lapic_tscdeadline_supported(void)
{
	u32_t a, b, c, d;

	chal_cpuid(6, &a, &b, &c, &d);
	if (a & (1 << 1)) printk("LAPIC Timer runs at Constant Rate!!\n");

	chal_cpuid(1, &a, &b, &c, &d);
	if (c & (1 << 24)) return 1;

	return 0;
}

static inline u32_t
lapic_cycles_to_timer(u32_t cycles)
{
	assert(lapic_cpu_to_timer_ratio);

	/* convert from (relative) CPU cycles to APIC counter */
	cycles = (cycles / lapic_cpu_to_timer_ratio);
	if (cycles == 0) cycles= (LAPIC_TIMER_MIN / lapic_cpu_to_timer_ratio);

	return cycles;
}

static int
lapic_apicid(void)
{
	u32_t a, b, c, d;

	chal_cpuid(1, &a, &b, &c, &d);

	return (int)(b >> 24); /* Vol 2, 3-205: high byte is apicid */
}

void
lapic_iter(struct lapic_cntl *l)
{
	static int off = 1;

	assert(l->header.len == sizeof(struct lapic_cntl));
	printk("\tLAPIC found: coreid %d, apicid %d\n", l->proc_id, l->apic_id);

	if (l->apic_id != apicids[INIT_CORE] && l->flags && ncpus < NUM_CPU && NUM_CPU > 1) {
		apicids[off++] = l->apic_id;
		ncpus++;
	}
}

u32_t
lapic_find_localaddr(void *l)
{
	u32_t          i;
	unsigned char  sum       = 0;
	unsigned char *lapicaddr = l;
	u32_t          length    = *(u32_t *)(lapicaddr + APIC_HDR_LEN_OFF);
	u32_t 	       addr, apic_flags, hi, lo;

	printk("Initializing LAPIC @ %p\n", lapicaddr);

	apicids[INIT_CORE] = lapic_apicid();
	for (i = 0; i < length; i++) {
		sum += lapicaddr[i];
	}

	if (sum != 0) {
		printk("\tInvalid checksum (%d)\n", sum);
		return 0;
	}

	addr       = *(u32_t *)(lapicaddr + APIC_CNTRLR_ADDR_OFF);
	apic_flags = *(u32_t *)(lapicaddr + APIC_CNTRLR_FLAGS_OFF);
	assert(apic_flags == 1); /* we're assuming the PIC exists */
	acpi_madt_intsrc_iter(lapicaddr);

	printk("\tChecksum is OK\n");
	lapic = (void *)(addr);
	printk("\tlapic: %p\n", lapic);

	readmsr(MSR_APIC_BASE, &lo, &hi);
	assert(lo & (1 << 8)); 	/* assume we are the BSP */
	/* instead of using bit 11 ("enable"), we use the SIV LAPIC control register */

	return addr;
}

static u32_t
cons_logical_id(const u32_t id)
{
	/*
	 * FIXME: xAPIC only support 8 bits bitmap for logical destination,
	 * So we will configure the logical id of cores with id larger than 7
	 * to 0 which means we should find out a way(x2APIC) to fix this when we
	 * have more than 8 cores in ioapic.
	 */

	if (id > 7) return 0;

	return (1ul << id) << LAPIC_LDR_OFFSET;
}

static u32_t
lapic_set_ldr(const u32_t id)
{
	u32_t lid = cons_logical_id(id);

	lapic_write_reg(LAPIC_LDR_REG, lid | ~LAPIC_LDR_MAST);
	return lid >> LAPIC_LDR_OFFSET;
}

void
lapic_init(void)
{
	u32_t version;

	assert(lapic);

	/* setup LDR for logic destination before init lapic */
	logical_apicids[get_cpuid()] = lapic_set_ldr(get_cpuid());

	lapic_write_reg(LAPIC_SIV_REG, LAPIC_SIV_ENABLE | HW_LAPIC_SPURIOUS);

	version = lapic_read_reg(LAPIC_VERSION_REG);
	/* don't want to deal with booting up using CMOS */
	/* assert((version & 0xFF) >= 0x10); */
	/* lapic_write_reg(LAPIC_LINT0, LAPIC_INT_MASKED); */
	/* lapic_write_reg(LAPIC_LINT1, LAPIC_INT_MASKED); */
	/* if ((version >> 16) >= 4) lapic_write_reg(LAPIC_PCINT, LAPIC_INT_MASKED); */
	lapic_timer_init();

	lapic_write_reg(LAPIC_ESR, 0);
	lapic_write_reg(LAPIC_ESR, 0);
	lapic_ack();

	lapic_write_reg(LAPIC_TP_REG, 0);
}

void
lapic_disable_timer(int timer_type)
{
	assert(lapic_timer_calib_init == 0);
	if (lapic_is_disabled[get_cpuid()]) return;

	if (timer_type == LAPIC_ONESHOT) {
		lapic_write_reg(LAPIC_INIT_COUNT_REG, 0);
	} else if (timer_type == LAPIC_TSC_DEADLINE) {
		writemsr(IA32_MSR_TSC_DEADLINE, 0, 0);
	} else {
		printk("Mode (%d) not supported\n", timer_type);
		assert(0);
	}

	lapic_is_disabled[get_cpuid()] = 1;
}

void
lapic_set_timer(int timer_type, cycles_t deadline)
{
	u64_t now;

	assert(lapic_timer_calib_init == 0);
	rdtscll(now);
	if (deadline < now || (deadline - now) < LAPIC_TIMER_MIN) deadline = now + LAPIC_TIMER_MIN;

	if (timer_type == LAPIC_ONESHOT) {
		u32_t counter;

		counter = lapic_cycles_to_timer((u32_t)(deadline - now));
		if (counter == 0) counter= LAPIC_COUNTER_MIN;

		lapic_write_reg(LAPIC_INIT_COUNT_REG, counter);
	} else if (timer_type == LAPIC_TSC_DEADLINE) {
		writemsr(IA32_MSR_TSC_DEADLINE, (u32_t)((deadline << 32) >> 32), (u32_t)(deadline >> 32));
	} else {
		printk("Mode (%d) not supported\n", timer_type);
		assert(0);
	}

	lapic_is_disabled[get_cpuid()] = 0;
}

void
lapic_set_page(u32_t page)
{
	lapic = (void *)(page * (1 << 22) | ((u32_t)lapic & ((1 << 22) - 1)));

	printk("\tSet LAPIC @ %p\n", lapic);
}

u32_t
lapic_get_ccr(void)
{
	return lapic_read_reg(LAPIC_CURR_COUNT_REG);
}

void
lapic_timer_calibration(u32_t ratio)
{
	assert(ratio && lapic_timer_calib_init);

	lapic_timer_calib_init   = 0;
	lapic_cpu_to_timer_ratio = ratio;

	/* reset INIT counter, and unmask timer */
	lapic_write_reg(LAPIC_INIT_COUNT_REG, 0);
	lapic_write_reg(LAPIC_TIMER_LVT_REG, lapic_read_reg(LAPIC_TIMER_LVT_REG) & ~LAPIC_TIMER_MASKED);
	lapic_is_disabled[get_cpuid()] = 1;

	printk("LAPIC: Timer calibrated - CPU Cycles to APIC Timer Ratio is %u\n", lapic_cpu_to_timer_ratio);
}

void
chal_timer_set(cycles_t cycles)
{
	lapic_set_timer(lapic_timer_mode, cycles);
}

void
chal_timer_disable(void)
{
	lapic_disable_timer(lapic_timer_mode);
}

unsigned int
chal_core_ipi_snd_get(cpuid_t cpu)
{
	if (cpu >= NUM_CPU) return -1;

	return lapic_asnd_ipi_snd_counter[cpu].counter;
}

unsigned int
chal_core_ipi_rcv_get(cpuid_t cpu)
{
	if (cpu >= NUM_CPU) return -1;

	return lapic_asnd_ipi_rcv_counter[cpu].counter;
}

unsigned int
chal_cyc_thresh(void)
{
	return lapic_cycs_thresh;
}

void
lapic_timer_init(void)
{
	u32_t low, high;
	u32_t a, b, c, d;

	chal_cpuid(1, &a, &b, &c, &d);
	if (c & (1 << 21)) printk("LAPIC:  processor supports x2APIC, IGNORED.\n");

	if (!lapic_tscdeadline_supported()) {
		printk("LAPIC: TSC-Deadline Mode not supported! Configuring Oneshot Mode!\n");

		/* Set the mode and vector */
		lapic_write_reg(LAPIC_TIMER_LVT_REG, HW_LAPIC_TIMER | LAPIC_ONESHOT_MODE);

		/* Set the timer and mask it, so timer interrupt is not fired - for timer calibration through HPET */
		lapic_write_reg(LAPIC_INIT_COUNT_REG, LAPIC_TIMER_CALIB_VAL);
		lapic_write_reg(LAPIC_TIMER_LVT_REG, lapic_read_reg(LAPIC_TIMER_LVT_REG) | LAPIC_TIMER_MASKED);
		if (get_cpuid() == INIT_CORE) {
			lapic_timer_mode       = LAPIC_ONESHOT;
			lapic_timer_calib_init = 1;
			lapic_cycs_thresh      = LAPIC_ONESHOT_THRESH;
		}
	} else {
		printk("LAPIC: Configuring TSC-Deadline Mode!\n");

		/* Set the mode and vector */
		lapic_write_reg(LAPIC_TIMER_LVT_REG, HW_LAPIC_TIMER | LAPIC_TSCDEADLINE_MODE);
		if (get_cpuid() == INIT_CORE) {
			lapic_timer_mode  = LAPIC_TSC_DEADLINE;
			lapic_cycs_thresh = LAPIC_TSCDEADLINE_THRESH;
		}
	}

	/* Set the divisor */
	lapic_write_reg(LAPIC_DIV_CONF_REG, LAPIC_DIV_BY_1);

	if (get_cpuid() != INIT_CORE) {
		/* reset INIT counter, and unmask timer */
		lapic_write_reg(LAPIC_INIT_COUNT_REG, 0);
		lapic_write_reg(LAPIC_TIMER_LVT_REG, lapic_read_reg(LAPIC_TIMER_LVT_REG) & ~LAPIC_TIMER_MASKED);
	}

	lapic_is_disabled[get_cpuid()] = 1;
}

static int
lapic_ipi_send(u32_t dest, u32_t vect_flags)
{
	lapic_write_reg(LAPIC_ICR + 0x10, dest << 24);
	lapic_read_reg(LAPIC_ICR + 0x10);

	lapic_write_reg(LAPIC_ICR, vect_flags);
	lapic_read_reg(LAPIC_ICR);

	return 0;
}
void
lapic_asnd_ipi_send(const cpuid_t cpu_id)
{
	assert(ncpus > 1 && cpu_id >= 0 && cpu_id < ncpus);

	/* how many sent by this CPU */
	atomic_inc(&lapic_asnd_ipi_snd_counter[get_cpuid()]);
	lapic_ipi_send(apicids[cpu_id], LAPIC_ICR_FIXED | LAPIC_IPI_ASND_VEC);

	return;
}

int
lapic_spurious_handler(struct pt_regs *regs)
{
	return 1;
}

int
lapic_ipi_asnd_handler(struct pt_regs *regs)
{
	int preempt = 1;

	/* how many recvd by this CPU */
	atomic_inc(&lapic_asnd_ipi_rcv_counter[get_cpuid()]);
	preempt = cap_ipi_process(regs);

	lapic_ack();

	return preempt;
}

int
lapic_timer_handler(struct pt_regs *regs)
{
	int preempt = 1;

	lapic_ack();

	preempt = timer_process(regs);

	return preempt;
}

/* HACK: assume that the HZ of the processor is equivalent to that on the computer used for compilation. */
static void
delay_us(u32_t us)
{
	unsigned long long hz = CPU_GHZ, hz_per_us = hz * 1000;
	unsigned long long end;
	volatile unsigned long long tsc;

	rdtscll(tsc);
	end = tsc + (hz_per_us * us);
	while (1) {
		rdtscll(tsc);
		if (tsc >= end) return;
		asm("pause");
	}
}

/* The SMP boot patchcode from loader.S */
extern char smppatchstart, smppatchend, smpstack, stack;

void
smp_boot_all_ap(volatile int *cores_ready)
{
	int i;
	u32_t ret;
	char **stackpatch;

	/*
	 * Set up the processor boot-up code.  Use SMC to create the
	 * smp loader code with the stack address inlined into it at
	 * an address that real-mode 16-bit code can execute
	 */
	memcpy((char *)chal_pa2va(SMP_BOOT_PATCH_ADDR), &smppatchstart, &smppatchend - &smppatchstart);
	stackpatch  = (char **)chal_pa2va(SMP_BOOT_PATCH_ADDR + (&smpstack - &smppatchstart));

	for (i = 1; i < ncpus; i++) {
		struct cos_cpu_local_info *cli;
		int 	j;
		u16_t   *warm_reset_vec;

		/* init shutdown code */
		outb(CMOS_PORT, 0xF);
		outb(CMOS_PORT+1, 0x0A);
		/* Warm reset vector */
		warm_reset_vec = (u16_t *)chal_pa2va((0x40 << 4 | 0x67));
		warm_reset_vec[0] = 0;
		warm_reset_vec[1] = SMP_BOOT_PATCH_ADDR >> 4;

		ret = lapic_read_reg(LAPIC_ESR);
		if (ret) printk("SMP Bootup: LAPIC error status register is %x\n", ret);
		lapic_write_reg(LAPIC_ESR, 0);
		lapic_read_reg(LAPIC_ESR);

		printk("\nBooting AP %d with apic_id %d\n", i, apicids[i]);
		/* Application Processor (AP) startup sequence: */
		/* ...make sure that we pass this core's stack */
		*stackpatch = &stack + ((PAGE_SIZE * i) + (PAGE_SIZE - STK_INFO_OFF));
		/* ...initialize the coreid of the new processor */
		cli         = (struct cos_cpu_local_info *)*stackpatch;
		cli->cpuid  = i; /* the rest is initialized during the bootup process */

		/* Now the IPI coordination process to boot the AP: first send init ipi... */
		lapic_ipi_send(apicids[i], LAPIC_ICR_LEVEL | LAPIC_ICR_ASSERT | LAPIC_ICR_INIT);
		delay_us(200);
		/* ...deassert it... */
		lapic_ipi_send(apicids[i], LAPIC_ICR_LEVEL | LAPIC_ICR_INIT);
		/* ...wait for 10 ms... */
		delay_us(10000);
		for (j = 0; j < 2; j++) {
			/* ...send startup IPIs... */
			assert(!(SMP_BOOT_PATCH_ADDR >> 12 & ~0xFF)); /* some address validation */
			lapic_ipi_send(apicids[i], LAPIC_ICR_SIPI | (SMP_BOOT_PATCH_ADDR >> 12));
			/* ...wait for 200 us... */
			delay_us(200);
		}
		/* waiting for AP's booting */
		while(*(volatile int *)(cores_ready + i) == 0) ;
	}
	ret = lapic_read_reg(LAPIC_ESR);
	if (ret) printk("SMP Bootup: LAPIC error status register is %x\n", ret);
	lapic_write_reg(LAPIC_ESR, 0);
	lapic_read_reg(LAPIC_ESR);
}

void
smp_init(volatile int *cores_ready)
{
	smp_boot_all_ap(cores_ready);
}
