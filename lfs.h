#include <fuse.h>
#include "flash.h"
// #include "types.h"

#define u_int unsigned int

/**********************************/
 //          struct stat {
        //        dev_t     st_dev;         /* ID of device containing file */
        //        ino_t     st_ino;         /* Inode number */
        //        mode_t    st_mode;        /* File type and mode */
        //        nlink_t   st_nlink;       /* Number of hard links */
        //        uid_t     st_uid;         /* User ID of owner */
        //        gid_t     st_gid;         /* Group ID of owner */
        //        dev_t     st_rdev;        /* Device ID (if special file) */
        //        off_t     st_size;        /* Total size, in bytes */
        //        blksize_t st_blksize;     /* Block size for filesystem I/O */
        //        blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */

        //        /* Since Linux 2.6, the kernel supports nanosecond
        //           precision for the following timestamp fields.
        //           For the details before Linux 2.6, see NOTES. */

        //        struct timespec st_atim;  /* Time of last access */
        //        struct timespec st_mtim;  /* Time of last modification */
        //        struct timespec st_ctim;  /* Time of last status change */

        //    #define st_atime st_atim.tv_sec      /* Backward compatibility */
        //    #define st_mtime st_mtim.tv_sec
        //    #define st_ctime st_ctim.tv_sec
        //    };


typedef struct sys_props {
    u_int n_segments;
    u_int block_size;
    u_int seg_size;
}sys_props;


/**********Declaration of functions**********/

/**************** FUSE callbacks  ******************/
static int lfs_getattr(const char *path, struct stat *st);
static int lfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int lfs_read(const char *path,char *buf, size_t size, off_t offset,struct fuse_file_info *fi);
static int lfs_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi);
static int lfs_open(const char *path, struct fuse_file_info *fi);
static int lfs_create(const char *path, mode_t mt, struct fuse_file_info *fi);
static int lfs_time(const char *path, const struct timespec tv[2], struct fuse_file_info *fi);
static int lfs_release(const char *path, struct fuse_file_info *fi);
/***************************************************/


static Flash load_device(char *fname);
static void set_name(meta *data, const char *path);
static void init_root();