#ifndef RK_ACOM_LIB_H
#define RK_ACOM_LIB_H

#include <cos_types.h>
#include <cos_kernel_api.h>
#include <cos_defkernel_api.h>
#include <llprint.h>
#include <string.h>
#include <memmgr.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <rk_types.h>
#include <sinv_async.h>
#include <rk_socketcall_types.h>
#include <fcntl.h>
#include <errno.h>

#ifndef RK_MAX_PAGES
#define RK_MAX_PAGES 32 /* per thread, max number of pages */
#define RK_MAX_SZ (RK_MAX_PAGES * PAGE_SIZE)
#endif

/* call acom_client_init() on rk_sinv_info using RK_CLIENT(instance) as key and make sure there is a stubcomp which listens to requests on "rk" stub side */
/* call acom_client_thread_init() for every thread that is going to communicate with RK.. because this is ACOM, threads are expected to be AEPS and pass the rcv aep key.. To be unique and consistent use RK_RKEY() api for the key in AEP creation */
/* link to -lacom_client library!! */

static inline int
rk_socket_acom(struct sinv_async_info *rk_sinv_info, int domain, int type, int protocol, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_SOCKET, domain, type, protocol, r, p);
}

static inline int
rk_connect_acom(struct sinv_async_info *rk_sinv_info, int fd, int shmid, socklen_t addrlen, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_CONNECT, fd, shmid, addrlen, r, p);
}

static inline int
rk_bind_acom(struct sinv_async_info *rk_sinv_info, int socketfd, int shdmem_id, unsigned addrlen, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_BIND, socketfd, shdmem_id, addrlen, r, p);
}

static inline ssize_t
rk_recvfrom_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_RECVFROM, arg1, arg2, arg3, r, p);
}

static inline ssize_t
rk_sendto_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_SENDTO, arg1, arg2, arg3, r, p);
}

static inline int
rk_setsockopt_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_SETSOCKOPT, arg1, arg2, arg3, r, p);
}

static inline int
rk_getsockopt_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_GETSOCKOPT, arg1, arg2, arg3, r, p);
}

static inline void *
rk_mmap_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return (void *)acom_client_request(rk_sinv_info, RK_MMAP, arg1, arg2, arg3, r, p);
}

static inline long
rk_write_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_WRITE, arg1, arg2, arg3, r, p);
}

static inline long
rk_writev_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_WRITEV, arg1, arg2, arg3, r, p);
}

static inline long
rk_read_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_READ, arg1, arg2, arg3, r, p);
}

static inline int
rk_listen_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_LISTEN, arg1, arg2, 0, r, p);
}

static inline int
rk_clock_gettime_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_CLOCK_GETTIME, arg1, arg2, 0, r, p);
}

static inline int
rk_select_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_SELECT, arg1, arg2, 0, r, p);
}

static inline int
rk_accept_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_ACCEPT, arg1, arg2, 0, r, p);
}

static inline int
rk_open_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_OPEN, arg1, arg2, arg3, r, p);
}

static inline int
rk_close_acom(struct sinv_async_info *rk_sinv_info, int fd, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_CLOSE, fd, 0, 0, r, p);
}

static inline int
rk_fcntl_acom(struct sinv_async_info *rk_sinv_info, int fd, int cmd, int arg3, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_FCNTL, fd, cmd, arg3, r, p);
}

static inline int
rk_unlink_acom(struct sinv_async_info *rk_sinv_info, int arg1, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_UNLINK, arg1, 0, 0, r, p);
}

static inline int
rk_ftruncate_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_FTRUNCATE, arg1, arg2, 0, r, p);
}

static inline int
rk_getsockname_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_GETSOCKNAME, arg1, arg2, 0, r, p);
}

static inline int
rk_getpeername_acom(struct sinv_async_info *rk_sinv_info, int arg1, int arg2, tcap_res_t r, tcap_prio_t p)
{
	return acom_client_request(rk_sinv_info, RK_GETPEERNAME, arg1, arg2, 0, r, p);
}

static inline vaddr_t
rk_get_shm_callvaddr_acom(cbuf_t *shmid)
{
	static cbuf_t id_calldata[MAX_NUM_THREADS] = { 0 };
	static vaddr_t addr_calldata[MAX_NUM_THREADS] = { 0 };

	if (unlikely(id_calldata[cos_thdid()] == 0)) id_calldata[cos_thdid()] = memmgr_shared_page_allocn(RK_MAX_PAGES, &addr_calldata[cos_thdid()]);

	assert(id_calldata[cos_thdid()] && addr_calldata[cos_thdid()]);
	*shmid = id_calldata[cos_thdid()];

	return addr_calldata[cos_thdid()];
}

static inline void *
rk_inv_mmap_acom(struct sinv_async_info *rk_sinv_info, void *addr, size_t len, int prot, int flags, int fd, off_t off, tcap_res_t r, tcap_prio_t p)
{
	void *ret=0;
	printc("%s\n", __func__);

	if (addr != NULL) {
		printc("parameter void *addr is not supported!\n");
		errno = ENOTSUP;
		return MAP_FAILED;
	}
	if (fd != -1) {
		printc("WARNING, file mapping is not supported, ignoring file\n");
	}

	printc("getting %d number of pages\n", len / PAGE_SIZE);
	addr = (void *)memmgr_heap_page_allocn(len / PAGE_SIZE);
	printc("addr: %p\n", addr);
	if (!addr){
		ret = (void *) -1;
	} else {
		ret = addr;
	}

	if (ret == (void *)-1) {  /* return value comes from man page */
		printc("mmap() failed!\n");
		/* This is a best guess about what went wrong */
		errno = ENOMEM;
	}
	return ret;
}

static inline ssize_t
rk_inv_write_acom(struct sinv_async_info *rk_sinv_info, int fd, const void *buf, size_t nbyte, tcap_res_t r, tcap_prio_t p)
{
	cbuf_t shmid = 0;
	vaddr_t shmaddr = NULL;

	shmaddr = rk_get_shm_callvaddr_acom(&shmid);

	memcpy((void *)shmaddr, buf, nbyte);

	return rk_write_acom(rk_sinv_info, fd, shmid, nbyte, r, p);
}

static inline int
rk_inv_fcntl_acom(struct sinv_async_info *rk_sinv_info, int fd, int cmd, void *arg, tcap_res_t r, tcap_prio_t p)
{
	if (cmd != F_SETFL) assert(0);

	return rk_fcntl_acom(rk_sinv_info, fd, cmd, (int)arg, r, p);
}

static inline ssize_t
rk_inv_writev_acom(struct sinv_async_info *rk_sinv_info, int fd, const struct iovec *iov, int iovcnt, tcap_res_t r, tcap_prio_t p)
{
	cbuf_t shmid = 0;
	vaddr_t shmaddr = NULL;
	struct iovec *tmpiov = NULL;
	int i = 0, off = 0;
	ssize_t ret, total = 0;
	void *tmpbuf = NULL;

	shmaddr = rk_get_shm_callvaddr_acom(&shmid);
	tmpiov = (struct iovec *)shmaddr;
	off = sizeof(struct iovec) * iovcnt;
	memcpy(tmpiov, iov, off);
	total += off;
	tmpbuf = (void *)(shmaddr + off);

	for (i = 0; i < iovcnt; i++) {
		if (i) tmpbuf += (tmpiov[i-1].iov_len);
		total += tmpiov[i].iov_len;

		assert(total <= RK_MAX_SZ);
		memcpy(tmpbuf, tmpiov[i].iov_base, tmpiov[i].iov_len);
	}

	assert(total <= RK_MAX_SZ);
	ret = rk_writev_acom(rk_sinv_info, fd, iovcnt, shmid, r, p);

	return ret;
}

static inline int
rk_inv_unlink_acom(struct sinv_async_info *rk_sinv_info, const char *path, tcap_res_t r, tcap_prio_t p)
{
	cbuf_t shmid = 0;
	vaddr_t shmaddr = NULL;

	shmaddr = rk_get_shm_callvaddr_acom(&shmid);

	assert(strlen(path) < 100);
	memcpy((void *)shmaddr, path, strlen(path) + 1);

	return rk_unlink_acom(rk_sinv_info, shmid, r, p);
}

static inline int
rk_inv_ftruncate_acom(struct sinv_async_info *rk_sinv_info, int fd, off_t length, tcap_res_t r, tcap_prio_t p)
{ return rk_ftruncate_acom(rk_sinv_info, fd, length, r, p); }

static inline ssize_t
rk_inv_read_acom(struct sinv_async_info *rk_sinv_info, int fd, void *buf, size_t nbyte, tcap_res_t r, tcap_prio_t p)
{
	long ret;
	cbuf_t shmid = 0;
	vaddr_t shmaddr = NULL;

	shmaddr = rk_get_shm_callvaddr_acom(&shmid);

	assert(nbyte <= RK_MAX_SZ);
	ret = rk_read_acom(rk_sinv_info, fd, shmid, nbyte, r, p);

	assert(ret <= RK_MAX_SZ);
	memcpy((void *)buf, (void *)shmaddr, ret);

	return ret;
}

static inline int
rk_inv_open_acom(struct sinv_async_info *rk_sinv_info, const char *path, int flags, mode_t mode, tcap_res_t r, tcap_prio_t p)
{
	cbuf_t shmid = 0;
	vaddr_t shmaddr = NULL;

	shmaddr = rk_get_shm_callvaddr_acom(&shmid);
	assert(strlen(path) < 100);
	memcpy((void *)shmaddr, path, strlen(path) + 1);

	printc("path: %s\n", path);
	return rk_open_acom(rk_sinv_info, shmid, flags, mode, r, p);
}


static inline int
rk_inv_close_acom(struct sinv_async_info *rk_sinv_info, int fd, tcap_res_t r, tcap_prio_t p)
{
	return rk_close_acom(rk_sinv_info, fd, r, p);
}

static inline int
rk_inv_clock_gettime_acom(struct sinv_async_info *rk_sinv_info, clockid_t clock_id, struct timespec *tp, tcap_res_t r, tcap_prio_t p)
{
	int ret;
	cbuf_t shmid = 0;
	vaddr_t shmaddr = NULL;

	shmaddr = rk_get_shm_callvaddr_acom(&shmid);

	memcpy((void * __restrict__)shmaddr, tp, sizeof(struct timespec));

	ret = rk_clock_gettime_acom(rk_sinv_info, clock_id, shmid, r, p);
	assert(!ret);

	/* Copy shdmem back into tp */
	memcpy(tp, (void *)shmaddr, sizeof(struct timespec));

	return ret;
}

static void
__set_valid(int *null_array)
{
	int i, num = 4; /* Because select takes 4 pointers */
	for (i = 0 ; i < num ; i++) *(null_array + i) = 1;
}

static inline int
rk_inv_select_acom(struct sinv_async_info *rk_sinv_info, int nd, fd_set *in, fd_set *ou, fd_set *ex, struct timeval *tv, tcap_res_t r, tcap_prio_t p)
{
	cbuf_t shmid = 0;
	vaddr_t shmaddr = NULL;
	int ret;
	int *null_array;
	vaddr_t tmp;

	shmaddr = rk_get_shm_callvaddr_acom(&shmid);

	/* XXX This is A LOT of copying. Optimize me*/

	/*
	 * Select can pass in null to the pointer arguments, because we assign the pointers
	 * to shared memory we need something to keep track of which of the values were null
	 */

	null_array = (void *)shmaddr;
	__set_valid(null_array);
	tmp = shmaddr + (sizeof(int) * 4);

	if (in) memcpy((void *)tmp, in, sizeof(fd_set));
	else null_array[0] = 0;
	tmp += sizeof(fd_set);

	if (ou) memcpy((void *)tmp, ou, sizeof(fd_set));
	else null_array[1] = 0;
	tmp += sizeof(fd_set);

	if (ex) memcpy((void *)tmp, ex, sizeof(fd_set));
	else null_array[2] = 0;
	tmp += sizeof(fd_set);

	if (tv) memcpy((void *)tmp, tv, sizeof(struct timeval));
	else null_array[3] = 0;

	ret = rk_select_acom(rk_sinv_info, nd, shmid, r, p);

	tmp = shmaddr + (sizeof(int) * 4);
	if(in) memcpy(in, (void *)tmp, sizeof(fd_set));
	tmp += sizeof(fd_set);
	if(ou) memcpy(ou, (void *)tmp, sizeof(fd_set));
	tmp += sizeof(fd_set);
	if(ex) memcpy(ex, (void *)tmp, sizeof(fd_set));
	tmp += sizeof(fd_set);
	if(tv) memcpy(tv, (void *)tmp, sizeof(struct timeval));

	return ret;
}

static inline int
rk_inv_socketcall_acom(struct sinv_async_info *rk_sinv_info, int call, unsigned long *args, tcap_res_t r, tcap_prio_t p)
{
	int ret = -1;
	cbuf_t shmid = 0;
	vaddr_t shmaddr = NULL;

	shmaddr = rk_get_shm_callvaddr_acom(&shmid);

	switch (call) {
		case SOCKETCALL_SOCKET: { /* socket */
			int domain, type, protocol;

			domain     = *args;
			type       = *(args + 1);
			protocol   = *(args + 2);

			ret = rk_socket_acom(rk_sinv_info, domain, type, protocol, r, p);

			break;
		}
		case SOCKETCALL_CONNECT: {
			int sockfd = *args;
			struct sockaddr *addr = (struct sockaddr *)*(args + 1), *saddr = (struct sockaddr *)shmaddr;
			socklen_t addrlen = (socklen_t)*(args + 2);

			memcpy(saddr, addr, sizeof(struct sockaddr));

			ret = rk_connect_acom(rk_sinv_info, sockfd, shmid, addrlen, r, p);

			break;
		}
		case SOCKETCALL_BIND: { /* bind */
			int sockfd;
			void *addr;
			u32_t addrlen;

			sockfd  = *args;
			addr    = (void *)*(args + 1);
			addrlen = *(args + 2);

			memcpy((void * __restrict__)shmaddr, addr, addrlen);
			ret = rk_bind_acom(rk_sinv_info, sockfd, shmid, addrlen, r, p);

			break;
		}
		case SOCKETCALL_LISTEN: { /* listen */
			int s, backlog;

			s       = *args;
			backlog = *(args + 1);

			ret = rk_listen_acom(rk_sinv_info, s, backlog, r, p);

			break;
		}
		case SOCKETCALL_ACCEPT: { /* accept */
			int s;
			struct sockaddr *addr;
			socklen_t *addrlen;
			vaddr_t tmp;

			s       = *args;
			addr    = (struct sockaddr *)*(args + 1);
			addrlen = (socklen_t *)*(args + 2);

			/* Copy into shdmem */
			tmp = shmaddr;
			memcpy((void *)tmp, addr, sizeof(struct sockaddr));
			tmp += sizeof(struct sockaddr);
			memcpy((void *)tmp, addrlen, sizeof(vaddr_t));

			ret = rk_accept_acom(rk_sinv_info, s, shmid, r, p);

			/* Copy out of shdmem */
			tmp = shmaddr;
			memcpy(addr, (void *)tmp, sizeof(struct sockaddr));
			tmp += sizeof(struct sockaddr);
			memcpy(addrlen, (void *)tmp, sizeof(vaddr_t));

			break;
		}
		case SOCKETCALL_GETSOCKNAME: { /* getsockname */
			int fdes;
			struct sockaddr *asa;
			socklen_t *alen;
			vaddr_t tmp;

			fdes = *args;
			asa  = (struct sockaddr *)*(args + 1);
			alen = (socklen_t *)*(args + 2);

			/* Copy into shdmem */
			tmp = shmaddr;
			memcpy((void *)tmp, asa, sizeof(struct sockaddr));
			tmp += sizeof(struct sockaddr);
			memcpy((void *)tmp, alen, sizeof(socklen_t));

			ret = rk_getsockname_acom(rk_sinv_info, fdes, shmid, r, p);

			/* Copy out of shdmem */
			tmp = shmaddr;
			memcpy(asa, (void *)tmp, sizeof(struct sockaddr));
			tmp += sizeof(struct sockaddr);
			memcpy(alen, (void *)tmp, sizeof(socklen_t));

			break;
		}
		case SOCKETCALL_GETPEERNAME: { /* getpeername */
			int fdes;
			struct sockaddr *asa;
			socklen_t *alen;
			vaddr_t tmp;

			fdes = *args;
			asa  = (struct sockaddr *)*(args + 1);
			alen = (socklen_t *)*(args + 2);

			/* Copy into shdmem */
			tmp = shmaddr;
			memcpy((void *)tmp, asa, sizeof(struct sockaddr));
			tmp += sizeof(struct sockaddr);
			memcpy((void *)tmp, alen, sizeof(socklen_t));

			ret = rk_getpeername_acom(rk_sinv_info, fdes, shmid, r, p);

			/* Copy out of shdmem */
			tmp = shmaddr;
			memcpy(asa, (void *)tmp, sizeof(struct sockaddr));
			tmp += sizeof(struct sockaddr);
			memcpy(alen, (void *)tmp, sizeof(socklen_t));

			break;
		}
		case SOCKETCALL_SENDTO: { /* sendto */
			int sockfd = *args;
			void *outptr = (void *)*(args + 1);
			size_t len = (size_t)*(args + 2);
			int flags = (int)*(args + 3);
			struct sockaddr *dest_addr = (struct sockaddr *)*(args + 4);
			socklen_t addrlen = (socklen_t)*(args + 5);

			*(int *)shmaddr = flags;
			*(socklen_t *)(shmaddr + 4) = addrlen;
			memcpy((void *)(shmaddr + 8), dest_addr, sizeof(struct sockaddr_storage));
			assert(len <= (RK_MAX_SZ - 8 - sizeof(struct sockaddr_storage)));
			memcpy((void *)(shmaddr + 8 + sizeof(struct sockaddr_storage)), outptr, len);
			ret = rk_sendto_acom(rk_sinv_info, sockfd, len, shmid, r, p);
			break;
		}
		case SOCKETCALL_RECVFROM: { /* recvfrom */
			int sockfd = (int)*args;
			void *inptr = (void *)*(args + 1), *shmbuf = NULL;
			size_t len = (size_t)*(args + 2);
			int flags = (int)*(args + 3);
			struct sockaddr *src_addr = (struct sockaddr *)*(args + 4);
			socklen_t *addrlenptr = (socklen_t *)*(args + 5);

			*(int *)shmaddr = flags;
			if (src_addr == NULL) {
				*(int *)(shmaddr + 4) = 0;
				*(socklen_t *)(shmaddr + 8) = 0;
			} else {
				*(int *)(shmaddr + 4) = 1;
				*(socklen_t *)(shmaddr + 8) = *addrlenptr;
			}
			memset((void *)(shmaddr + 12), 0, sizeof(struct sockaddr_storage));
			shmbuf = (void *)(shmaddr + 12 + sizeof(struct sockaddr_storage));

			assert(len <= (RK_MAX_SZ - (12 + sizeof(struct sockaddr_storage))));

			ret = rk_recvfrom_acom(rk_sinv_info, sockfd, len, shmid, r, p);
			if (ret < 0) break;

			if (src_addr) {
				*addrlenptr = *(socklen_t *)(shmaddr + 8);
				memcpy(src_addr, (void *)(shmaddr + 12), *addrlenptr);
			}

			if (ret == 0) break;
			memcpy(inptr, shmbuf, ret);

			break;
		}
		case SOCKETCALL_SETSOCKOPT: { /* setsockopt */
			int sockfd, level, optname;
			const void *optval;
			socklen_t optlen;

			sockfd            = (int)*args;
			level		  = (int)*(args + 1);
			optname           = (int)*(args + 2);
			optval            = (const void *)*(args + 3);
			optlen            = (socklen_t)*(args + 4);

			memcpy((void *)shmaddr, optval, optlen);

			ret = rk_setsockopt_acom(rk_sinv_info, (sockfd << 16) | level, (optname << 16) | shmid, optlen, r, p);

			memcpy((void *)optval, (void *)shmaddr, optlen);

			break;
		}
		case SOCKETCALL_GETSOCKOPT: {
			/* int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen); */
			int sockfd = (int)*args, level = (int)*(args + 1), optname = (int)*(args + 2);
			void *optval = (void *)*(args + 3), *roptval = NULL;
			socklen_t *optlen = (socklen_t *)*(args + 4), *roptlen = NULL;

			/* output/return values on shared memory */
			roptlen = (socklen_t *)shmaddr;
			roptval = (void *)(shmaddr + 4);

			assert(sockfd < (1<<16) && shmid < (1<<16));
			ret = rk_getsockopt_acom(rk_sinv_info, sockfd << 16 | shmid, level, optname, r, p);
			/* copy from shared memory to user passed addresses */
			*optlen = *roptlen;
			memcpy(optval, roptval, *optlen);

			break;
		}
		default: {
			printc("%s, ERROR, unimplemented socket call: %d\n", __func__, call);
			assert(0);
		}
	}

	return ret;
}

#endif /* RK_ACOM_LIB_H */
