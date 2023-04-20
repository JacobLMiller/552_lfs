#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35 

#define u_int unsigned int

#define DEBUG 1

#define DEF_CP_INTERVAL 4
#define DEF_CLEAN_START 4
#define DEF_CLEAN_STOP  8


#include "flash.h"
#include "types.h"
#include "log.h"
#include "lfs.h"

extern int getuid();
extern int getgid();

/*Exposed from inode-tab.c*/
extern void init_inode_tab();
extern void init_seg_tab();
extern void i_node_insert(const char *str, i_node *node);
extern i_node *i_node_lookup(const char *str);
extern i_node *create_inode(char *name, ftype type);
extern void flush_to_log();
///////////////////////////

/*Exposed from dir.c*/
extern void append_file(i_node *root, i_node *new_node);
////////////////////////

/*Exposed from log.c*/
extern int log_write(char *arr);
///////////////////////

extern int BUGGY_crc(void *buf_void, int len);

/********************************************/

static Flash FD;
extern disk_data *data;

static u_int ops_since_flush = 0;

static void inc_ops(){
    ops_since_flush ++;
    if (ops_since_flush > DEF_CP_INTERVAL){
        // printf("Need to flush\n");
        flush_to_log();
        ops_since_flush = 0;
    }
}

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

static int lfs_getattr(const char *path, struct stat *st)
{
    if (DEBUG)
        printf("Called getattr on: %s\n", path);

    i_node *cur = i_node_lookup(path);
    if (cur == NULL){
        return -ENOENT;
    }

    st->st_uid = getuid();
    st->st_gid = getgid();

    st->st_atime = time(NULL);
    st->st_mtime = time(NULL);

    switch (cur->meta->type){
    case DIR_TYPE:
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 1;
        break;
    case FILE_TYPE:
        st->st_mode = S_IFREG | 0644;
        // st->st_mode = S_IFREG | 0755;
        st->st_nlink = 1;
        st->st_size = cur->meta->size;
        break;
    case LINK_TYPE: //Unfinished; does tot handle links
        break;
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
    if(DEBUG)
        printf("Called readdir on  %s\n",path);

    i_node *curdir = i_node_lookup(path);
    char * mystr, *splitstr;
    assert(curdir->meta->type == DIR_TYPE);

    i_node *curfile = curdir->next;
    while (curfile){
        mystr = (char *)malloc(sizeof(char) *1024);
        strcpy(mystr,curfile->meta->name);
        splitstr = strtok(mystr, "/"); //Will need to fix for arbitrary directory;
        filler(buffer,splitstr,NULL,0);
        free(mystr);
        curfile = curfile->next;
    }

    return 0;
}


// http://libfuse.github.io/doxygen/structfuse__operations.html#a272960bfd96a0100cbadc4e5a8886038
static int lfs_read(const char *path,
                    char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
    if(DEBUG)
        printf("Called read on  %s\n",path);

    i_node *file = i_node_lookup(path);
    int fsize = file->meta->size;
    if(fsize > 0 && fsize <= 1024){
        memcpy(buf,file->addrs[0].buf,fsize);
        return fsize;
    }
    else if (fsize > 1024){
        printf("big file\n");
        return -2;
    }
    

    return 0;

}

static int lfs_write(const char *path,
                     const char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi)
{   
    if(DEBUG)
        printf("Called write on %s\n", path);

    i_node *file = i_node_lookup(path);
    if (size <= 1024){
        memcpy(file->addrs[0].buf,buf,size);
        file->meta->size = size;
        // for(int i = size+1; i < 1024; i++){
        //     file->addrs[0].buf[i] = 'a';
        // }
        return size;
    }
    else{
        printf("too big: %ld\n", size);
    }
    
    inc_ops();
    return 0;
}

static int lfs_open(const char *path, struct fuse_file_info *fi)
{
    if(DEBUG)
        printf("Called open on %s\n", path);

    return 0;
}

static int lfs_create(const char *path, mode_t mt, struct fuse_file_info *fi){
    if(DEBUG)
        printf("Called create on %s\n", path);

    char *name = malloc(sizeof(char) * 25);
    strcpy(name,path);
    i_node *new_node = create_inode(name,FILE_TYPE);

    i_node_insert(path,new_node);

    i_node *root = i_node_lookup("/"); //Will need to fix for arbitrary directory; Get from string splitting.
    append_file(root,new_node);

    inc_ops();

    return 0;
}

static int lfs_time(const char *path, const struct timespec *tv){
    return 0;
}

static int lfs_release(const char *path, struct fuse_file_info *fi){
    if(DEBUG)
        printf("Called release on %s\n", path);
    return 0;
}

static int lfs_truncate(const char *path, off_t size){
    if(DEBUG)
        printf("Called truncate on %s\n", path);

    i_node *ino = i_node_lookup(path);
    ino->meta->size = size;

    inc_ops();
    return 0;
}


typedef struct russ_header{
    char        magic[8];
    int         version;
    int         blocksize;
    int         segsize;
    int         crc;
}russ_header;

static Flash load_device(char *fname){
    u_int blocks;
    Flash FD = Flash_Open(fname, 0, &blocks);

    if (DEBUG){
        printf("I have %d blocks\n",blocks);
    }
    char *head = malloc(TOT_SECTORS * FLASH_SECTOR_SIZE);
    Flash_Read(FD, 0,TOT_SECTORS,head);

    russ_header *data = (russ_header *)head;
    // data->fname = fname;

    if (DEBUG){
        printf("Magic string is %s\n",data->magic);
        printf("Version is %d\n",data->version);
        printf("Block size is %d\n", data->blocksize);
        printf("Segment size is %d\n",data->segsize);
        printf("crc is %d\n",data->crc);
        printf("Running buggy_crc gets us %d\n",BUGGY_crc());
        // printf("Our current segment is %d\n",data->cur_sector);
        // printf("Our current block is %d\n",data->cur_block);
    }

    // init_seg_tab();


    return FD;

}

static void init_root(){
    init_inode_tab();
    i_node *root = create_inode("/",DIR_TYPE);
    i_node_insert("/", root);

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

    FD = load_device(devicename);
    Flash_Close(FD);
    // init_root();


    #define N_ARGS 4
    char *fuseargs[N_ARGS] = {
        "fuse_sys",
        mntpnt,
        "-f",
        "-s",
        // "-o auto_cache"
        // "-d"
    };

    // fuse_main(N_ARGS,fuseargs, &ops, NULL);

    return 0;
}