#include <cos_component.h>
#include <llprint.h>
#include <ext4.h>
#include <ext4_mkfs.h>
#include <ps_ns.h>

extern struct ext4_blockdev *ext4_blockdev_get(void);
static struct ext4_blockdev *bd;

static struct ext4_fs        fs;
static int                   fs_type = F_SET_EXT4;
static struct ext4_mkfs_info info    = {
  .block_size = 4096,
  .journal    = true,
};
static struct ext4_bcache *bc;

struct ps_ns *ns;

PS_NSSLAB_CREATE(fd, sizeof(struct ext4_file), 2, 9, 6);

word_t
fs_fopen(const char *path, const char *flags)
{
	ps_desc_t  fd;
	ext4_file *file;

	fd = -1;
	file = ps_nsptr_alloc_fd(ns, &fd);
	int ret = ext4_fopen(file, path, flags);
	printc("ret %d\n", ret);

	printc("mp %p\n", file->mp);

	return (word_t)fd;
}

int
fs_fclose(word_t fd)
{
	return ext4_fclose(ps_nsptr_lkup_fd(ns, fd));
}

int
fs_ftruncate(word_t fd, unsigned long size)
{
	return ext4_ftruncate(ps_nsptr_lkup_fd(ns, fd), size);
}

size_t
fs_fread(word_t fd, void *buf, size_t size)
{
	size_t rcnt;

	int ret = ext4_fread(ps_nsptr_lkup_fd(ns, fd), buf, size, &rcnt);
	printc("fread ret %d\n", ret);

	return rcnt;
}

size_t
fs_fwrite(word_t fd, void *buf, size_t size)
{
	size_t wcnt;

	int ret = ext4_fwrite(ps_nsptr_lkup_fd(ns, fd), buf, size, &wcnt);
	printc("fwrite ret %d\n", ret);

	return wcnt;
}

int
fs_fseek(word_t fd, long offset, unsigned long origin)
{
	return ext4_fseek(ps_nsptr_lkup_fd(ns, fd), offset, origin);
}

unsigned long
fs_ftell(word_t fd)
{
	return ext4_ftell(ps_nsptr_lkup_fd(ns, fd));
}

unsigned long
fs_fsize(word_t fd)
{
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

	word_t fd, fd2;

	char buf[16];
	strcpy(buf, "TEST");

	fd = fs_fopen("/test.txt", "w+");
	printc("fs size: %ld\n", fs_fsize(fd));

	fs_fwrite(fd, buf, 16);
	printc("fs size: %ld\n", fs_fsize(fd));
	fs_fseek(fd, 0, 0);
	memset(buf, 0, 16);
	fs_fread(fd, buf, 16);
	printc("%s\n", buf);
}
