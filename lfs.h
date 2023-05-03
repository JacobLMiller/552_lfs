#include <fuse.h>
#include "flash.h"
// #include "types.h"

#define u_int unsigned int

/**********Declaration of functions**********/

/**************** FUSE callbacks  ******************/

/* 
 * lfs_getattr - get file attributes
 * 
 * This function is called by FUSE to get the attributes of a file or directory. 
 * It should fill in a struct stat with the appropriate values for the given path.
 */
static int lfs_getattr(const char *path, struct stat *st);

/*
 * lfs_readdir - read directory contents
 *
 * This function is called by FUSE to read the contents of a directory. It should 
 * call the filler function with the appropriate values for each file or directory 
 * in the given directory path.
 */
static int lfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

/* 
 * lfs_read - read data from a file
 * 
 * This function is called by FUSE to read data from a file. It should read `size`
 * bytes of data starting at `offset` from the file specified by `path`, and 
 * store the data in the buffer pointed to by `buf`.
 */
static int lfs_read(const char *path,char *buf, size_t size, off_t offset,struct fuse_file_info *fi);

/* 
 * lfs_write - write data to a file
 * 
 * This function is called by FUSE to write data to a file. It should write `size`
 * bytes of data starting at `offset` to the file specified by `path`, using the 
 * data stored in the buffer pointed to by `buf`.
 */
static int lfs_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi);

/* 
 * lfs_open - open a file
 * 
 * This function is called by FUSE to open a file. It should return 0 if the file 
 * was successfully opened, or an error code if the file could not be opened.
 */
static int lfs_open(const char *path, struct fuse_file_info *fi);

/* 
 * lfs_create - create a new file
 * 
 * This function is called by FUSE to create a new file. It should create a new 
 * file at the given path with the given mode, and return 0 if the file was 
 * successfully created, or an error code if the file could not be created.
 */
static int lfs_create(const char *path, mode_t mt, struct fuse_file_info *fi);

/* 
 * lfs_time - change file timestamps
 * 
 * This function is called by FUSE to change the timestamps (access time and 
 * modification time) of a file. It should return 0 if the timestamps were 
 * successfully changed, or an error code if the timestamps could not be changed.
 */
static int lfs_time(const char *path, const struct timespec *tv);

/*
 *This function is called when all file descriptors associated with a file are closed. 
 *The path argument is the path of the file and the fi argument contains information about the file. 
 *This function should release any resources associated with the file that were allocated in lfs_open.
*/
static int lfs_release(const char *path, struct fuse_file_info *fi);

/*
*This function is called to truncate a file to a specified size. 
*The 'path' argument is the path of the file and size is the new size of the file. 
*This function should update the size of the file in the file's inode and release any blocks that are no longer needed.
*/
static int lfs_truncate(const char *path, off_t size);

/*
*This function is called to read the target of a symbolic link. 
The path argument is the path of the symbolic link and buf is the buffer to write the target to. 
'size' is the size of the buffer.
*/
static int lfs_readlink(const char *path, char *buf, size_t size);

/*
*This function is called to retrieve the value of an extended attribute associated with a file. 
*The 'path' argument is the path of the file, 'buf' is the name of the attribute, 'str' is the buffer to write the attribute value to, 
*and 'size' is the size of the buffer.
*/
static int lfs_getxattr(const char *path, const char *buf, char *str, size_t size);
/***************************************************/


