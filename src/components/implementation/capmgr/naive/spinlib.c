#include "spinlib.h"

#define SPINLIB_CALIB 512

static u64_t spinlib_cycs_per_spin_iters = 0;
static u64_t spinlib_usecs_per_spin_iters = 0;
static u64_t spinlib_cycs_per_iter = 0;
static u64_t spinlib_rdtscp_avg_ovhd = 0;
unsigned int spinlib_cycs_per_us = 0;
static unsigned int spinlib_init = 0;
static volatile unsigned int spinlib_std_tmp_iters = 0;

void spinlib_calib(unsigned int cycs_per_us) __attribute__((optimize("O0")));
void spinlib_rdtscp_calib(void) __attribute__((optimize("O0")));
void spinlib_usecs(cycles_t usecs) __attribute__((optimize("O0")));
void spinlib_niters(unsigned long long niters) __attribute__((optimize("O0")));
unsigned long long spinlib_usecs_intern(cycles_t usecs) __attribute__((optimize("O0")));
void spinlib_cycles(cycles_t cycs) __attribute__((optimize("O0")));
void spinlib_std_iters(void) __attribute__((optimize("O0")));

#define SPINLIB_USE_STDITERS

#define rdtscpll(var) do { \
    u32_t low, high; \
    asm volatile( \
        "movl $0, %%eax \n" \
        "movl $0, %%ecx \n" \
        "cpuid \n" \
        "rdtsc \n" \
        "movl %%edx, %0 \n" \
        "movl %%eax, %1 \n" \
        "movl $0, %%eax \n" \
        "movl $0, %%ecx \n" \
        "cpuid \n" \
        : \
         "=r"(high), \
         "=r"(low) \
        : \
        : "eax", "ebx", "ecx", "edx" \
    ); \
    (var) = (((u64_t)high) << 32ull) | ((u64_t)low); \
} while(0)

static void
spinlib_calib_test(void)
{
	microsec_t test_us[] = { 10, 20, 30, 40, 50 };
	int i, sz = sizeof(test_us)/sizeof(test_us[0]);

	for (i = 0; i < sz; i++) {
		cycles_t st, end, elapsed_cycs, elus = 0, spincycs = 0, spiniters = 0;
		unsigned long long niters = 0;

		rdtscpll(st);
		niters = spinlib_usecs_intern(test_us[i]);
		rdtscpll(end);
		elapsed_cycs = end - st - spinlib_rdtscp_avg_ovhd;
		elus = elapsed_cycs / spinlib_cycs_per_us;

		PRINTC("SPIN %lluus => elapsed :%llucycs %lluus %lluiters\n", test_us[i], elapsed_cycs, elapsed_cycs / spinlib_cycs_per_us, niters);

		spiniters = (niters / elus) * test_us[i];
		rdtscpll(st);
		spinlib_niters(spiniters);
		rdtscpll(end);
		elapsed_cycs = end - st - spinlib_rdtscp_avg_ovhd;
		elus = elapsed_cycs / spinlib_cycs_per_us;

		PRINTC("SPIN %lluus => elapsed :%llucycs %lluus %lluiters\n", test_us[i], elapsed_cycs, elapsed_cycs / spinlib_cycs_per_us, spiniters);

		spincycs = (elapsed_cycs / elus) * test_us[i];
		rdtscpll(st);
		spinlib_cycles(spincycs);
		rdtscpll(end);
		elapsed_cycs = end - st - spinlib_rdtscp_avg_ovhd;

		PRINTC("SPIN %llucycs => elapsed :%llucycs %lluus\n", spincycs, elapsed_cycs, elapsed_cycs / spinlib_cycs_per_us);
	}
}

void
spinlib_niters(unsigned long long niters)
{
	unsigned long long i;

	for (i = 0 ; i < niters ; i++) {
		__asm__ __volatile__("nop": : :"memory");
	}
}

void
spinlib_std_iters(void)
{
	unsigned int i;

	for (i = 0 ; i < SPINLIB_ITERS_SPIN ; i++) {
		__asm__ __volatile__("nop": : :"memory");
	}

	spinlib_std_tmp_iters = i;
}

#define SPINLIB_RDTSCP_CALIB_ITERS 100000

void
spinlib_calib_rdtscp(void)
{
	int i;
	volatile cycles_t total = 0;
	volatile cycles_t st, en;

	for (i = 0; i < SPINLIB_RDTSCP_CALIB_ITERS; i++) {
		rdtscpll(st);
		rdtscpll(en);
		total += en - st;
	}

	spinlib_rdtscp_avg_ovhd = total / SPINLIB_RDTSCP_CALIB_ITERS;
}

/* time taken in that loop */
void
spinlib_calib(unsigned int cycs_per_us)
{
	cycles_t total_cycs = 0;
	unsigned int iters = 0;

	if (spinlib_init) return;
	spinlib_cycs_per_us = cycs_per_us;
	spinlib_calib_rdtscp();

	while (iters < SPINLIB_CALIB) {
		cycles_t start, end;

		rdtscpll(start);
		spinlib_std_iters();
		rdtscpll(end);

		total_cycs += (end - start - (spinlib_rdtscp_avg_ovhd * 2));
		iters ++;
	}

	spinlib_cycs_per_spin_iters  = total_cycs / SPINLIB_CALIB;
	spinlib_usecs_per_spin_iters = spinlib_cycs_per_spin_iters / spinlib_cycs_per_us;
	spinlib_cycs_per_iter        = spinlib_cycs_per_spin_iters / SPINLIB_ITERS_SPIN;

	spinlib_init = 0;
	printc("RDTSCP OVHD: %llu, Spin calibration: ITERS:%u Cycs/ITERS:%llu usecs/ITERS:%llu cycs/I:%llu\n",
	       spinlib_rdtscp_avg_ovhd,
	       SPINLIB_ITERS_SPIN, spinlib_cycs_per_spin_iters, spinlib_usecs_per_spin_iters,
	       spinlib_cycs_per_iter);

	spinlib_calib_test();
}

void
spinlib_cycles(cycles_t cycs)
{
#ifdef SPINLIB_USE_STDITERS
	unsigned int i = 0;
	unsigned int iters = cycs / spinlib_cycs_per_spin_iters;
	unsigned int left = cycs % spinlib_cycs_per_spin_iters;

	assert(cycs >= spinlib_cycs_per_spin_iters);

	/* round off to next cycs/spin */
	if (left >= (spinlib_cycs_per_spin_iters / 2)) iters ++;

	while (i < iters) {
		spinlib_std_iters();
		i ++;
	}
#else
	unsigned long long niters = cycs / spinlib_cycs_per_iter;

	spinlib_niters(niters);
#endif
}

unsigned long long
spinlib_usecs_intern(cycles_t usecs)
{
#ifndef SPINLIB_USE_STDITERS
	cycles_t cycs_us = usecs * spinlib_cycs_per_us;
	unsigned long long niters = cycs_us / spinlib_cycs_per_iter;

	spinlib_niters(niters);

	return niters;
#else
	unsigned int i = 0;
	unsigned int iters = usecs / spinlib_usecs_per_spin_iters;
	unsigned int left = usecs % spinlib_usecs_per_spin_iters;
	unsigned long long niters_total = 0;

	assert(usecs >= spinlib_usecs_per_spin_iters);

	/* round off to next usec */
	if (left >= (spinlib_usecs_per_spin_iters / 2)) iters ++;

	while (i < iters) {
		spinlib_std_iters();
		niters_total += spinlib_std_tmp_iters;
		i ++;
	}

	return niters_total;
#endif
}

void
spinlib_usecs(cycles_t usecs)
{
#ifndef SPINLIB_USE_STDITERS
	cycles_t cycs_us = usecs * spinlib_cycs_per_us;
	unsigned long long niters = cycs_us / spinlib_cycs_per_iter;

	spinlib_niters(niters);
#else
	unsigned int i = 0;
	unsigned int iters = usecs / spinlib_usecs_per_spin_iters;
	unsigned int left = usecs % spinlib_usecs_per_spin_iters;

	assert(usecs >= spinlib_usecs_per_spin_iters);

	/* round off to next usec */
	if (left >= (spinlib_usecs_per_spin_iters / 2)) iters ++;

	while (i < iters) {
		spinlib_std_iters();
		i ++;
	}
#endif
}
