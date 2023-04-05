#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35 

#define u_int unsigned int

#define DEBUG 1

#define DEF_CP_INTERVAL 4
#define DEF_CLEAN_START 4
#define DEF_CLEAN_STOP  8


#include "flash.h"
#include "types.h"
#include "lfs.h"

extern int getuid();
extern int getgid();

/*Exposed from inode-tab.c*/
extern void init_inode_tab();
extern void i_node_insert(const char *str, i_node *node);
extern i_node *i_node_lookup(const char *str);
///////////////////////////

/*Exposed from dir.c*/
extern void append_file(i_node *root, i_node *new_node);
////////////////////////

/********************************************/

static Flash FD;
static disk_data *data;
static u_int ops_since_flush = 0;



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
        // printf("File not found\n");
        return -ENOENT;
    }

    st->st_uid = getuid();
    st->st_gid = getgid();

    st->st_atime = time(NULL);     // time: now
    st->st_mtime = time(NULL);

    switch (cur->meta->type){
    case DIR_TYPE:
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 1;
        break;
    case FILE_TYPE:
        // st->st_mode = S_IFREG | 0644;
        st->st_mode = S_IFREG | 0755;
        st->st_nlink = 1;
        st->st_size = cur->meta->size;
        break;
    case LINK_TYPE:
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
        mystr = (char *)malloc(sizeof(char) *25);
        strcpy(mystr,curfile->meta->name);
        splitstr = strtok(mystr, "/");
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
    if(fsize > 0){
        memcpy(buf,file->buf,fsize);
        return fsize;
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
        memcpy(file->buf,buf,size+1);
        for (int i = 0; i < size; i++){
            printf("%c",buf[i]);
        }
        file->meta->size = size;
        return size;
    }
    else{
        printf("too big: %ld\n", size);
    }
    

    return 0;
}

static int lfs_open(const char *path, struct fuse_file_info *fi)
{
    if(DEBUG)
        printf("Called open on %s\n", path);
    // i_node *ino = i_node_lookup(path);

    return 0;
}

static int lfs_create(const char *path, mode_t mt, struct fuse_file_info *fi){
    if(DEBUG)
        printf("Called create on %s\n", path);

    block_addr *new_addr; meta *new_meta; i_node *new_node;
    new_addr = malloc(sizeof(block_addr));

    new_meta = malloc(sizeof(meta));
    new_meta->type = FILE_TYPE;
    new_meta->size = 0;
    set_name(new_meta,path);

    new_node = malloc(sizeof(i_node));
    new_node->ba   = new_addr;
    new_node->meta = new_meta;
    new_node->next = NULL;

    i_node_insert(path,new_node);

    i_node *root = i_node_lookup("/");
    append_file(root,new_node);

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
    return 0;
}




static void set_name(meta *data, const char *path){
    data->name = malloc(sizeof(char) * 25);
    strcpy(data->name, path);
}

static Flash load_device(char *fname){
    u_int blocks;
    Flash FD = Flash_Open(fname, 0, &blocks);

    if (DEBUG){
        printf("I have %d blocks\n",blocks);
    }
    char head[TOT_SECTORS*512];
    Flash_Read(FD, 0,64,head);

    data = (disk_data *)head;

    if (DEBUG){
        printf("Block size is %d\n", data->blocksize);
        printf("Segment size is %d\n",data->segsize);
        printf("Our current segment is %d\n",data->cur_sector);
        printf("Our current block is %d\n",data->cur_block);
    }

    return FD;

}

static void init_root(){
    init_inode_tab();
    block_addr *root_addr = malloc(sizeof(block_addr));
    root_addr->block = 0;
    root_addr->segment = 0;
    root_addr->is_null = false;

    meta *root_meta = malloc(sizeof(meta));
    root_meta->type = DIR_TYPE;
    root_meta->size = 12345;
    root_meta->last_mod = 12;
    root_meta->created = time(NULL);

    char *str = malloc(sizeof(char) * 5);
    strcpy(str,"/");
    root_meta->name = str;

    i_node *root = malloc(sizeof(i_node));
    root->ba = root_addr;
    root->meta = root_meta;
    root->next = NULL;

    i_node_insert(str, root);

}

int main(int argc, char **argv){

    //Quit if no file handle
    if (argc < 3){
        printf("Please provide name of flash device to be mounted\n");
        return 1;
    }

    FD = load_device(argv[1]);
    init_root();

    char *fuseargs[4] = {
        "Jacob program",
        argv[2],
        "-f",
        "-s"
    };
    // fuseargs[0] = "Jacob program";
    // fuseargs[1] = argv[2];
    // fuseargs[2] = "-f";

    fuse_main(4,fuseargs, &ops, NULL);


    

    return 0;
}