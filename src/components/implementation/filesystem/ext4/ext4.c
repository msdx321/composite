#include <cos_component.h>
#include <llprint.h>
#include <ext4.h>
#include <ext4_mkfs.h>
#include <ps_ns.h>

extern struct ext4_blockdev *ext4_blockdev_get(void);
static struct ext4_blockdev *bd;

static struct ext4_fs fs;
static int fs_type = F_SET_EXT4;
static struct ext4_mkfs_info info = {
    .block_size = 4096,
    .journal = true,
};
static struct ext4_bcache *bc;

struct ps_ns *ns;

PS_NSSLAB_CREATE(fd, sizeof(ext4_file), 3, 9 ,7);

word_t fs_fopen(const char *path, const char *flags) {
	ps_desc_t fd;
	ext4_file *file;

	file = ps_nsptr_alloc_fd(ns, &fd);
	ext4_fopen(file, path, flags);

	return (word_t)fd;
}

int fs_fclose(word_t fd) {
	return ext4_fclose(ps_nsptr_lkup_fd(ns, fd));
}

int fs_ftruncate(word_t fd, unsigned long size) {
	return ext4_ftruncate(ps_nsptr_lkup_fd(ns, fd), size);
}

size_t fs_fread(word_t fd, void *buf, size_t size) {
	size_t rcnt;

	ext4_fread(ps_nsptr_lkup_fd(ns, fd), buf, size, &rcnt);

	return rcnt;
}

size_t fs_fwrite(word_t fd, void *buf, size_t size) {
	size_t wcnt;

	ext4_fwrite(ps_nsptr_lkup_fd(ns, fd), buf, size, &wcnt);

	return wcnt;
}

int fs_fseek(word_t fd, long offset, unsigned long origin) {
	return ext4_fseek(ps_nsptr_lkup_fd(ns,fd), offset, origin);
}

unsigned long fs_ftell(word_t fd) {
	return ext4_ftell(ps_nsptr_lkup_fd(ns, fd));
}

unsigned long fs_fsize(word_t fd) {
	return ext4_fsize(ps_nsptr_lkup_fd(ns, fd));
}

void
cos_init(void)
{
	ns = ps_nsptr_create_slab_fd();

	bd = ext4_blockdev_get();
	ext4_dmask_set(DEBUG_ALL);

	ext4_mkfs(&fs, bd, &info, fs_type);
	ext4_device_register(bd, "ext4_fs");
	ext4_mount("ext4_fs", "/", false);

	printc("Hello world!\n");
	while (1) ;
}
