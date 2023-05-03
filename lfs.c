// Define feature test macro for X/Open source compatibility and FUSE version
#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35

// Define u_int as an alias for unsigned int
#define u_int unsigned int

// Include necessary headers
#include <math.h>
#include "flash.h"
#include "types.h"
#include "lfs.h"

// Define the allocation size of a block
#define ALLOC_SIZE FLASH_SECTORS_PER_BLOCK*FLASH_SECTOR_SIZE

extern int getuid();
extern int getgid();

// Declare functions that are exposed from dir.c and init.c
/*Exposed from dir.c*/
extern inod *lookup(const char *path, int inum);
extern void init_inode_tab();
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

// Declare the function pointers for the FUSE operations
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

// Implementation of the lfs_getattr FUSE operation
static int lfs_getattr(const char *path, struct stat *st){
    if (DEBUG)
        printf("Called getattr on: %s\n", path);

    // Look up the inode for the given path
    inod *ino = lookup(path, 1);
    if (ino == NULL){
        return -ENOENT;
    }

    // Set the file attributes based on the inode information
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
    case NO_USE:
    case SPECIAL:
        st->st_mode = S_IFREG | ino->mode;
        // st->st_mode = S_IFREG | 0755;
        st->st_size = ino->size;
        break;
    case SYM_LINK:
        st->st_mode = S_IFLNK | 0644;
        break;
    case DEAD:
    default:
        printf("I don't think I should ever be here\n");
        return -ENOENT;
        break;
    }

    return 0;
}

// Implementation of the lfs_readdir FUSE operation
static int lfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){

    // Print a debug message if the DEBUG flag is set
    if (DEBUG)
        printf("Called readdir on: %s\n", path);

    // Look up the directory with the given path.
    inod *dir = lookup(path,1);
    assert(dir->type == DIR_TYPE);

    // Calculate the number of directory entries and the number of blocks in the directory.
    int num_entries = dir->size / sizeof(dir_entry);
    int num_blocks = (dir->size / bsize_bytes) + 1;

    // Allocate a buffer to read the directory contents into.
    char *dir_buf = malloc(num_blocks * bsize_bytes);
    read_file(dir,dir_buf,num_blocks);

        // Cast the buffer to an array of directory entries.
    dir_entry *children = (dir_entry *)dir_buf;

    // Loop over the directory entries and add them to the buffer for FUSE.
    for (int i=0; i<num_entries;i++){
        if(children[i].name[0] != '\0')
            filler(buffer,children[i].name,NULL,0);
    }
    
    // Free the directory entry buffer.
    free(children);

    return 0;
}

// This function is called when FUSE is reading a symbolic link.
// It will read the contents of the symbolic link with the given path.
static int lfs_readlink(const char *path, char *buf, size_t size){
    if(DEBUG)
        printf("Called readlink on %s\n",path);
        
    // Look up the inode of the symbolic link with the given path.
    inod *ino = lookup(path,1);

    // Calculate the number of blocks needed to read the contents of the symbolic link.
    int num_blocks = (ino->size / bsize_bytes) + 1;
    char *fbuf = malloc(num_blocks * bsize_bytes);
    read_file(ino,fbuf,num_blocks);
    memcpy(buf,fbuf,ino->size);
    free(fbuf);


    return 0;
}

//Function to implement the 'read' command
static int lfs_read(const char *path,char *buf, size_t size, off_t offset,struct fuse_file_info *fi){
    if (DEBUG)
        printf("Called read on %s\n",path);

    inod *ino = lookup(path,1);

    // Calculate the number of blocks required to read the file
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

    // Get the name of the flash device and mount point from the command-line arguments

    char *devicename = argv[argc-2];
    char *mntpnt     = argv[argc-1];

    // Initialize the inode table
    init_inode_tab();

    // Load the LFS from the flash device
    load_lfs(devicename);

    // Set up the arguments for the FUSE library
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
