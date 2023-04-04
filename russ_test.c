#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35 

#define u_int unsigned int


#include <stdio.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
// #include <malloc.h>
#include <stdlib.h>
#include "flash.h"


extern int Flash_Create(char *file, u_int wearLimit, u_int blocks);


typedef struct LFS_OpenFile {
    int  ino;    // Inode number.  This will be important in LFS but is just a dummy number here.
    int  is_the_writable_file;
} LFS_OpenFile;



struct {
    // of course, in LFS, you will need to replace this with a file stored
    // on disk!  But this simply exists to test write/read functionality.
    char buf[1024];
    int  len;
} writable_file_data = { .len = 0 };



// https://libfuse.github.io/doxygen/fuse_8h.html
// https://libfuse.github.io/doxygen/hello_8c.html
// https://libfuse.github.io/doxygen/structfuse__operations.html

// https://www.cs.hmc.edu/~geoff/classes/hmc.cs137.201801/homework/fuse/fuse_doc.html

// https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/



static int lfs_getattr(const char *path, struct stat *st);

static int lfs_readdir(const char *path, void *buffer,
                       fuse_fill_dir_t filler, off_t offset,
                       struct fuse_file_info *fi);

static int lfs_readlink(const char *path, char *buf, size_t len);

static int lfs_open(const char *path, struct fuse_file_info *fi);

static int lfs_release(const char *path, struct fuse_file_info *fi);

static int lfs_read(const char *path,
                    char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi);

static int lfs_write(const char *path,
                     const char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi);

static int lfs_truncate(const char *path, off_t size);



static struct fuse_operations ops = {
    .getattr  = lfs_getattr,
    .readdir  = lfs_readdir,
    .readlink = lfs_readlink,
    .open     = lfs_open,
    .release  = lfs_release,
    .read     = lfs_read,
    .write    = lfs_write,
    .truncate = lfs_truncate,
};



int main(int argc, char **argv)
{
    int test = Flash_Create("mfile", 2, 15);
    printf("%d\n",test);
    printf("Hello I am in main.\n");
    return fuse_main(argc,argv, &ops, NULL);
}



static int lfs_getattr(const char *path, struct stat *st)
{
    printf("RUSS: getattr(%s)\n", path);

    st->st_uid = getuid();
    st->st_gid = getgid();

    st->st_atime = time(NULL);     // time: now
    st->st_mtime = time(NULL);

    if (strcmp(path, "/"   ) == 0 ||
        strcmp(path, "/bar") == 0 ||
        strcmp(path, "/baz") == 0)
    {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
    }
    else if (strcmp(path, "/link") == 0)
    {
        st->st_mode = S_IFLNK | 0644;
        st->st_nlink = 1;
    }
    else if (strcmp(path, "/dest") == 0)
    {
        st->st_mode = S_IFREG | 0644;
        st->st_nlink = 1;
        st->st_size = 4;
    }
    else if (strcmp(path, "/writable") == 0)
    {
        st->st_mode = S_IFREG | 0755;
        st->st_nlink = 1;
        st->st_size = writable_file_data.len;
    }
    else
    {
        return -ENOENT;
    }

    return 0;
}



static int lfs_readdir(const char *path, void *buffer,
                       fuse_fill_dir_t filler, off_t offset,
                       struct fuse_file_info *fi)
{
    printf("RUSS: readdir(%s)\n", path);

    filler(buffer, "."   , NULL, 0);
    filler(buffer, ".."  , NULL, 0);
    filler(buffer, "bar" , NULL, 0);
    filler(buffer, "baz" , NULL, 0);
    filler(buffer, "link", NULL, 0);
    filler(buffer, "dest", NULL, 0);
    return 0;
}



static int lfs_readlink(const char *path, char *buf, size_t len)
{
    printf("RUSS: readlink(%s)\n", path);

    if (strcmp(path, "/link") == 0)
    {
        const char *dest = "dest";
        if (len < strlen(dest)+1)
            return -EINVAL;          // this is wrong, according to http://libfuse.github.io/doxygen/structfuse__operations.html#a0a7fe4e7fe5db55e96ef6adaf64c09e2

        strcpy(buf, dest);
    }
    else
    {
        return -ENOENT;
    }

    return 0;
}



// http://libfuse.github.io/doxygen/structfuse__operations.html#a08a085fceedd8770e3290a80aa9645ac
static int lfs_open(const char *path, struct fuse_file_info *fi)
{
    printf("RUSS: open(%s)    fi->flags = 0x%x\n", path, fi->flags);

    if ((fi->flags & O_ACCMODE) != O_RDONLY && strcmp(path, "/writable") != 0)
        return -EACCES;     // TODO: add support for writable files

    if (strcmp(path, "/dest") != 0 && strcmp(path, "/writable") != 0)
        return -ENOENT;     // TODO: support directory lookup

    LFS_OpenFile *file_data = calloc(1, sizeof(LFS_OpenFile));
    if (file_data == NULL)
        return -ENOMEM;

    /* This is a dummy value.  In LFS, we would do a directory lookup */
    static int next_ino = 1234;
    file_data->ino = next_ino;
    next_ino++;

    if (strcmp(path, "/writable") == 0)
        file_data->is_the_writable_file = 1;

    printf("   Buffer address: %lld 0x%p\n", (long long)file_data, file_data);
    fi->fh = (long long)file_data;

    return 0;
}



// http://libfuse.github.io/doxygen/structfuse__operations.html#a4a6f1b50c583774125b5003811ecebce
static int lfs_release(const char *path, struct fuse_file_info *fi)
{
    LFS_OpenFile *file_data = (LFS_OpenFile*)fi->fh;
    printf("RUSS: release(%s) file_data: { ino:%d }\n", path, file_data->ino);

    free(file_data);
    return 0;
}



// http://libfuse.github.io/doxygen/structfuse__operations.html#a272960bfd96a0100cbadc4e5a8886038
static int lfs_read(const char *path,
                    char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
    /* WARNING WARNING WARNING
     *
     * If you don't set st_size in getattr() properly, then you will see read()
     * operations hit your FUSE code but they won't actually deliver anything
     * to the user.  Seems like if st_size==0, you get a dummy buffer size 4K,
     * and if it's something reasonable, you get the file length as the buffer
     * size.  Not extensively tested, though.
     *
     * WARNING WARNING WARNING
     *
     * Read the spec for read() carefully, it has a different behavior than the
     * read() syscall!!!  The syscall is happy to return short (non-zero length)
     * buffers; for instance, if you read from a socket or from the keyboard,
     * these are quite common.  In that environment, read() returns zero on EOF
     * and anything else means "try again later, maybe I have more, and maybe
     * not."  But on the FUSE side, if you ever return short (even if non-zero)
     * it is treated as EOF, and will never ask you again.  My guess: the
     * "low-level" interface probably supports more complex reads.
     *
     *    -- Russ, Spring 23
     */

    LFS_OpenFile *file_data = (LFS_OpenFile*)fi->fh;
    printf("RUSS: read(%s, size=%ld, offset=%ld) file_data: { ino:%d }\n", path, size, offset, file_data->ino);

    char *file_buf;
    int   file_len;
    if (file_data->is_the_writable_file == 0)
    {
        file_buf = "dest";
        file_len = 4;
    }
    else
    {
        file_buf = writable_file_data.buf;
        file_len = writable_file_data.len;
    }

    if (offset >= file_len)
        return 0;    // EOF

    if (offset == 0)
    {
        if (size < file_len)
            return -EINVAL;     // THIS IS WRONG!  Implement me, handle short reads!!!

        memcpy(buf, file_buf, file_len);
        return file_len;
    }

    // any other offset...
    return -EINVAL;     // THIS IS WRONG!  Implement me, handle short reads!!!
}



// http://libfuse.github.io/doxygen/structfuse__operations.html#a1fdc611027324dd68a550f9662db1fac
static int lfs_write(const char *path,
                     const char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi)
{
    LFS_OpenFile *file_data = (LFS_OpenFile*)fi->fh;
    printf("RUSS: write(%s, size=%ld, offset=%ld) file_data: { ino:%d }\n", path, size, offset, file_data->ino);

    if (file_data->is_the_writable_file == 0)
        return -EINVAL;     // it's illegal to attempt to write to a readonly file

    if (offset != 0){
        printf("hello there");
    }
        // return -EINVAL;     // THIS IS WRONG!  Implement me, handle writes in the middle

    if (size > sizeof(writable_file_data.buf))
        return -EINVAL;     // THIS IS WRONG!  Implement me, handle long writes to files

    memcpy(writable_file_data.buf, buf, size);
    writable_file_data.len = size;
    return size;
}



// http://libfuse.github.io/doxygen/structfuse__operations.html#a73ddfa101255e902cb0ca25b40785be8
static int lfs_truncate(const char *path, off_t size)
{
    /* this is called to change the size of a file.  If you try to redirect
     * into a file, like this:
     *     command > output
     * then a truncate() will happen on the file before you see any new writes.
     * More completely, the sequence, if the file "output" already existed,
     * will be:
     *     getattr     <- sees if the file exists or not
     *     open
     *     truncate
     *     ...
     *
     * TODO: what is the sequence when we write to a file that never existed?
     *
     * TODO: I believe that it's legal (though rare) for truncate to actually
     *       *EXTEND* a file.
     */

    printf("RUSS: truncate(%s, size=%ld)\n", path,size);

    if (strcmp(path, "/writable") != 0)
        return -EINVAL;    // can't change the length of a non-writable file

    if (size > writable_file_data.len)
        return -EINVAL;    // TODO: implement me

    writable_file_data.len = size;
    return 0;
}

