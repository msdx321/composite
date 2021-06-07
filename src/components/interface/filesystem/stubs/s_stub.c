#include <cos_stubs.h>
#include <filesystem.h>

vaddr_t *__fs_s_smem_pool[64];

COS_SERVER_3RET_STUB(word_t, fs_fopen) {
    compid_t token;
    token = (compid_t)cos_inv_token();

    return fs_fopen((const char *)__fs_s_smem_pool[token], (const char *)__fs_s_smem_pool[token] + 4096);
}

COS_SERVER_3RET_STUB(int, fs_fread) {
    compid_t token;
    size_t ret;

    token = (compid_t)cos_inv_token();

    ret = fs_fread(p0, __fs_s_smem_pool[token], p2);
    fs_fseek(p0, 4096, 1);

    return ret;
}

COS_SERVER_3RET_STUB(int, fs_fwrite) {
    compid_t token;
    size_t ret;

    token = (compid_t)cos_inv_token();

    ret = fs_fwrite(p0, __fs_s_smem_pool[token], p2);
    fs_fseek(p0, 4096, 1);

    return ret;
}

int
__fs_s_smem_init(cbuf_t cid)
{
	compid_t token;
	token = (compid_t)cos_inv_token();

	return memmgr_shared_page_map(cid, (vaddr_t *)&__fs_s_smem_pool[token]);
}