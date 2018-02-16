#ifndef LLBOOT_H
#define LLBOOT_H

#include <cos_kernel_api.h>

/*
 * strong assumption:
 *  - capid_t though is unsigned long, only assuming it occupies 16bits for now
 */
#define LLBOOT_ERROR 0xFFFF

enum llboot_cntl {
	LLBOOT_COMP_INIT_DONE = 0,
	LLBOOT_COMP_INFO_GET, /* packed! <retval>, <pgtbl, captbl>, <compcap, parent_spdid> */
	LLBOOT_COMP_INFO_NEXT, /* iterator to get comp_info */
	LLBOOT_COMP_FRONTIER_GET, /* get current cap frontier & vaddr frontier of spdid comp */
};

static inline int
llboot_comp_init_done(void)
{
	word_t ret = 0;

	/* DEPRICATED it seems, but not in my code base yet!*/
	ret = cos_sinv(BOOT_CAPTBL_SINV_CAP, LLBOOT_COMP_INIT_DONE, 0, 0, 0);

	return (int)(ret);
}

static inline int
llboot_comp_info_get(spdid_t spdid, pgtblcap_t *pgc, captblcap_t *capc, compcap_t *cc, spdid_t *psid, capid_t *rsfr)
{
	word_t r1 = 0, r2 = 0, r3 = 0;

	/*
	 * guess, llbooter must know it's coming from resmgr and allow only 
	 * that component special privilege to access anything.
	 * for all others, it should return error!
	 */
	r1    = cos_sinv_3rets(BOOT_CAPTBL_SINV_CAP, LLBOOT_COMP_INFO_GET, spdid, 0, 0, &r2, &r3);
	*rsfr = ((r1 << 16) >> 16);
	if (*rsfr == LLBOOT_ERROR) return -1;

	*pgc  = (r2 >> 16);
	*capc = ((r2 << 16) >> 16);
	*cc   = (r3 >> 16);
	*psid = ((r3 << 16) >> 16);

	return -1;
}

static inline int
llboot_comp_info_next(spdid_t *sid, pgtblcap_t *pgc, captblcap_t *capc, compcap_t *cc, spdid_t *psid, capid_t *rsfr)
{
	word_t r1 = 0, r2 = 0, r3 = 0;

	/*
	 * guess, llbooter must know it's coming from resmgr and allow only 
	 * that component special privilege to access anything.
	 * for all others, it should return error!
	 */
	r1    = cos_sinv_3rets(BOOT_CAPTBL_SINV_CAP, LLBOOT_COMP_INFO_NEXT, 0, 0, 0, &r2, &r3);
	*rsfr = (int)((r1 << 16) >> 16);

	if (*rsfr == LLBOOT_ERROR) return -1;

	*sid  = (r1 >> 16);
	*pgc  = (r2 >> 16);
	*capc = ((r2 << 16) >> 16);
	*cc   = (r3 >> 16);
	*psid = ((r3 << 16) >> 16);

	return 0;
}

static inline int
llboot_comp_frontier_get(spdid_t spdid, vaddr_t *vasfr, capid_t *capfr)
{
	int ret = 0;
	word_t r1 = 0, r2 = 0, r3 = 0;

	/*
	 * guess, llbooter must know it's coming from resmgr and allow only 
	 * that component special privilege to access anything.
	 * for all others, it should return error!
	 */
	r1  = cos_sinv_3rets(BOOT_CAPTBL_SINV_CAP, LLBOOT_COMP_FRONTIER_GET, spdid, 0, 0, &r2, &r3);
	if (r1) return -1;

	*capfr = ((r2 << 16) >> 16);
	*vasfr = (vaddr_t)r3;

	return ret;
}

#endif /* LLBOOT_H */
