#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cos_component.h>
#include <memmgr.h>

word_t fs_fopen(const char *path, const char *flags);
word_t COS_STUB_DECL(fs_fopen)(const char *path, const char *flags);

int fs_fclose(word_t fd);
int COS_STUB_DECL(fs_fclose)(word_t fd);

int fs_ftruncate(word_t fd, unsigned long size);
int COS_STUB_DECL(fs_ftruncate)(word_t fd, unsigned long size);

size_t fs_fread(word_t fd, void *buf, size_t size);
size_t COS_STUB_DECL(fs_fread)(word_t fd, void *buf, size_t size);

size_t fs_fwrite(word_t fd, void *buf, size_t size);
size_t COS_STUB_DECL(fs_fwrite)(word_t fd, void *buf, size_t size);

int fs_fseek(word_t fd, long offset, unsigned long origin);
int COS_STUB_DECL(fs_fseek)(word_t fd, long offset, unsigned long origin);

unsigned long fs_ftell(word_t fd);
unsigned long COS_STUB_DECL(fs_ftell)(word_t fd);

unsigned long fs_fsize(word_t fd);
unsigned long COS_STUB_DECL(fs_fsize)(word_t fd);

/*
int fs_dir_rm(const char *path);
int COS_STUB_DECL(fs_dir_rm)(const char *path);

int fs_dir_mv(const char *path, const char *new_path);
int COS_STUB_DECL(fs_dir_mv)(const char *path, const char *new_path);

int fs_dir_mk(const char *path);
int COS_STUB_DECL(fs_dir_mk)(const char *path);

int fs_dir_open(void *dir, const char *path);
int COS_STUB_DECL(fs_dir_open)(void *dir, const char *path);

int fs_dir_close(void *dir);
int COS_STUB_DECL(fs_dir_close)(void *dir);
*/

CCTOR int __fs_c_smem_init();

int __fs_s_smem_init(cbuf_t cid);
int COS_STUB_DECL(__fs_s_smem_init)(cbuf_t cid);


#endif /* FILESYSTEM_H */
