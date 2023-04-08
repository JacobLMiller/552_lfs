#include "global.h"

#define TOT_SECTORS 64

typedef struct block_addr {
    char              *buf;
    unsigned int       segment;
    unsigned int       block;
} block_addr;

typedef enum ftype{
    FILE_TYPE, 
    DIR_TYPE, 
    LINK_TYPE
} ftype;

typedef struct meta{
    ftype        type;
    unsigned int size;
    unsigned int num_blocks;
    char   *name;
} meta;


typedef struct inode {
    block_addr    addrs[8];
    meta         *meta;
    unsigned int  ino;
    struct inode *next;
} i_node;

typedef struct disk_data{
    unsigned int blocksize;
    unsigned int segsize;
    unsigned int disksize;
    unsigned int wearlimit;
    unsigned int cur_sector;
    unsigned int cur_block;
    unsigned int cur_segment;
    unsigned int table_size;
    char        *fname;
}disk_data;

typedef struct seg_data{
    bool         written;
    unsigned int segno;
    unsigned int fill_blocks;
    unsigned int *arr_num;
    i_node      **ino_arr;
}seg_data;