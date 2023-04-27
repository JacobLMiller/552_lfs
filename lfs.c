#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35

#define u_int unsigned int

#include <math.h>
#include "flash.h"
#include "types.h"
#include "log.h"
#include "lfs.h"

#define DEF_CP_INTERVAL 4
#define DEF_CLEAN_START 4
#define DEF_CLEAN_STOP  8

#define ALLOC_SIZE FLASH_SECTORS_PER_BLOCK*FLASH_SECTOR_SIZE

extern int getuid();
extern int getgid();

/*Exposed from dir.c*/
extern inod *lookup(const char *path, int inum);
/********************************************/

/*Exposed from init.c*/
extern void load_lfs(char *fname);
extern void read_file(inod *ino, char *buf,int num_blocks);
extern Flash FD; 
extern disk_header *data;
extern inod *inode_tab;
extern int tab_size;
extern int bsize_bytes;
extern int K;
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

    switch (ino->type){
    case DIR_TYPE:
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 1;
        break;
    case FILE_TYPE:
        st->st_mode = S_IFREG | 0644;
        // st->st_mode = S_IFREG | 0755;
        st->st_nlink = 1;
        st->st_size = ino->size;
        break;
    case SYM_LINK: //Unfinished; does tot handle links
        break;
    default:
        printf("I don't think I should ever be here\n");
        return -ENOENT;
        break;
    }

    return 0;
}

static int lfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
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
        filler(buffer,children[i].name,NULL,0);
    }
    free(children);

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
    printf("write was called\n");
    return 0;
}

static int lfs_open(const char *path, struct fuse_file_info *fi){
    printf("open was called on %s\n",path);
    return 0;
}

static int lfs_create(const char *path, mode_t mt, struct fuse_file_info *fi){
    printf("create was called\n");
    return 0;
}

static int lfs_time(const char *path, const struct timespec *tv){
    return 0;
}

static int lfs_release(const char *path, struct fuse_file_info *fi){
    printf("release was called\n");
    return 0;
}

static int lfs_truncate(const char *path, off_t size){
    printf("truncate was called\n");
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

    for(int i = 0; i < argc-1; i++){
        if(strcmp(argv[i],"-i") == 0){
            //cpnt_interval = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i],"-c") == 0){
            //cln_thrshld = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i],"-C") == 0){
            //stp_thrshld = (argv[i+1]);
        }
    }    

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

    fuse_main(N_ARGS,fuseargs, &ops, NULL);

    return 0;
}