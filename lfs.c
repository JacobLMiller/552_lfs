#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35

#define u_int unsigned int

#include <math.h>
#include "flash.h"
#include "types.h"
#include "lfs.h"

#define ALLOC_SIZE FLASH_SECTORS_PER_BLOCK*FLASH_SECTOR_SIZE

extern int getuid();
extern int getgid();

/*Exposed from dir.c*/
extern inod *lookup(const char *path, int inum);
extern void init_inode_tab();
/********************************************/

/*Exposed from init.c*/
extern void load_lfs(char *fname);
extern void read_file(inod *ino, char *buf, int num_blocks);
extern int bsize_bytes;
/*********************/

static struct fuse_operations ops = {
    .getattr   =     lfs_getattr,
    .readdir   =     lfs_readdir,
    .read      =     lfs_read,
    .write     =     lfs_write,
    .open      =     lfs_open,
    .create    =     lfs_create,
    .utimens   =     lfs_time,
    .release   =     lfs_release,
    .truncate  =     lfs_truncate,
    .readlink  =     lfs_readlink,
    .getxattr  =     lfs_getxattr,
};

static int lfs_getattr(const char *path, struct stat *st){
    if (DEBUG)
        printf("Called getattr on: %s\n", path);

    inod *ino = lookup(path, 1);
    if (ino == NULL){
        return -ENOENT;
    }

    st->st_uid = getuid();
    st->st_gid = getgid();

    st->st_atime = time(NULL);
    st->st_mtime = time(NULL);

    st->st_nlink = ino->num_links;

    switch (ino->type){
    case DIR_TYPE:
        st->st_mode = S_IFDIR | ino->mode;
        break;
    case FILE_TYPE:
        st->st_mode = S_IFREG | ino->mode;
        st->st_size = ino->size;
        break;
    case SYM_LINK:
        st->st_mode = S_IFLNK | ino->mode;
        break;
    case NO_USE:
    case SPECIAL:
        st->st_mode = S_IFREG | ino->mode;
        break;
    case DEAD:
    default:
        printf("I don't think I should ever be here\n");
        return -ENOENT;
        break;
    }

    return 0;
}

static int lfs_readdir(const char *path, void *buffer, 
    fuse_fill_dir_t filler, off_t offset, 
    struct fuse_file_info *fi)
{
    if (DEBUG)
        printf("Called readdir on: %s\n", path);

    inod *dir = lookup(path,1);
    assert(dir->type == DIR_TYPE);


    int num_entries = dir->size / sizeof(dir_entry);
    int num_blocks = (dir->size / bsize_bytes) + 1;
    char *dir_buf = malloc(num_blocks * bsize_bytes);
    read_file(dir,dir_buf,num_blocks);
    dir_entry *children = (dir_entry *)dir_buf;

    for (int i=0; i<num_entries;i++){
        if(children[i].name[0] != '\0')
            filler(buffer,children[i].name,NULL,0);
    }
    free(children);

    return 0;
}

static int lfs_readlink(const char *path, char *buf, size_t size){
    if(DEBUG)
        printf("Called readlink on %s\n",path);
    
    inod *ino = lookup(path,1);

    int num_blocks = (ino->size / bsize_bytes) + 1;
    char *fbuf = malloc(num_blocks * bsize_bytes);
    read_file(ino,fbuf,num_blocks);
    memcpy(buf,fbuf,ino->size);
    free(fbuf);


    return 0;
}

static int lfs_read(const char *path,char *buf, size_t size, off_t offset,struct fuse_file_info *fi){
    if (DEBUG)
        printf("Called read on %s\n",path);

    inod *ino = lookup(path,1);

    int num_blocks = (ino->size / bsize_bytes) + 1;
    char *fbuf = malloc(num_blocks * bsize_bytes);
    read_file(ino,fbuf,num_blocks);
    memcpy(buf,fbuf,ino->size);
    free(fbuf);

    return ino->size;
}

static int lfs_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi){
    if(DEBUG)
        printf("write was called on %s\n",path);
    return 0;
}

static int lfs_open(const char *path, struct fuse_file_info *fi){
    if(DEBUG)
        printf("open was called on %s\n",path);
    return 0;
}

static int lfs_create(const char *path, mode_t mt, struct fuse_file_info *fi){
    if(DEBUG)
        printf("create was called on %s\n",path);
    return 0;
}

static int lfs_time(const char *path, const struct timespec *tv){
    return 0;
}

static int lfs_release(const char *path, struct fuse_file_info *fi){
    if(DEBUG)
        printf("release was called on %s\n",path);
    return 0;
}

static int lfs_truncate(const char *path, off_t size){
    if(DEBUG)
        printf("truncate was called on %s\n",path);
    return 0;
}

static int lfs_getxattr(const char *path, const char *buf, char *str, size_t size){
    if(DEBUG)
        printf("getxattr was called on %s\n",path);
    return 0;
}



int main(int argc, char **argv){

    //Quit if no file handle
    if (argc < 3){
        printf("Please provide name of flash device to be mounted\n");
        return 1;
    }

    char *devicename = argv[argc-2];
    char *mntpnt     = argv[argc-1];

    init_inode_tab();
    load_lfs(devicename);

    #define N_ARGS 4
    char *fuseargs[N_ARGS] = {
        "fuse_sys",
        mntpnt,
        "-f",
        "-s",
        // "-o auto_cache"
        // "-d"
    };

    return fuse_main(N_ARGS,fuseargs, &ops, NULL);

}