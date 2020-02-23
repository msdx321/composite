#include <cos_component.h>
#include <cos_kernel_api.h>
#include <llprint.h>
#include <pong.h>
#include <cos_debug.h>
#include <cos_types.h>
#include <hypercall.h>
#include <cos_rdtsc.h>

int
call(void)
{
	PRINTLOG(PRINT_DEBUG, "In call() in pong interface, client:%lu\n", cos_inv_token());
	return 0;
}

int
call_two(void)
{
	PRINTLOG(PRINT_DEBUG, "In call_two() in pong interface, client:%lu\n", cos_inv_token());
	return 2;
}

int
call_three(void)
{
	PRINTLOG(PRINT_DEBUG, "In call_three() in pong interface, client:%lu\n", cos_inv_token());
	return 3;
}

int
call_four(void)
{
	PRINTLOG(PRINT_DEBUG, "In call_four() in pong interface, client:%lu\n", cos_inv_token());
	return 4;
}

int
call_arg(int p1)
{
//	PRINTLOG(PRINT_DEBUG, "In call_arg() in pong interface, client:%lu. arg: %d\n", cos_inv_token(), p1);
//	return p1;
	return 0;
}

int
call_args(int p1, int p2, int p3, int p4)
{
//	PRINTLOG(PRINT_DEBUG, "In call_args() in pong interface, client:%lu. args: p1:%d p2:%d p3:%d p4:%d\n", cos_inv_token(), p1, p2, p3, p4);
//	return p1;
	return 0;
}

int
call_3rets(int *r2, int *r3, int p1, int p2, int p3, int p4)
{
	if (p1 == 1) {
		cycles_t now;

		//rdtscll(now);
		cos_rdtscp(now);
		*r2 = (unsigned long)(now >> 32);
		*r3 = (unsigned long)((now << 32) >> 32);
	}
//	PRINTLOG(PRINT_DEBUG, "In call_3rets() in pong interface, client:%lu. args: p1:%d p2:%d p3:%d p4:%d\n", cos_inv_token(), p1, p2, p3, p4);
//	*r2 = p1 + p2 + p3 + p4;
//	*r3 = p1 - p2 - p3 - p4;
	return 0;
}

void
cos_init(void)
{
	int ret;

	PRINTLOG(PRINT_DEBUG, "Welcome to the pong component\n");

	hypercall_comp_init_done();

	PRINTLOG(PRINT_ERROR, "Cannot reach here!\n");
	assert(0);
}
