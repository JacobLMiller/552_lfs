#include <fuse.h>
#include "flash.h"
// #include "types.h"

#define u_int unsigned int

/**********Declaration of functions**********/

/**************** FUSE callbacks  ******************/
static int lfs_getattr(const char *path, struct stat *st);
static int lfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int lfs_read(const char *path,char *buf, size_t size, off_t offset,struct fuse_file_info *fi);
static int lfs_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi);
static int lfs_open(const char *path, struct fuse_file_info *fi);
static int lfs_create(const char *path, mode_t mt, struct fuse_file_info *fi);
static int lfs_time(const char *path, const struct timespec *tv);
static int lfs_release(const char *path, struct fuse_file_info *fi);
static int lfs_truncate(const char *path, off_t size);
static int lfs_readlink(const char *path, char *buf, size_t size);
/***************************************************/

