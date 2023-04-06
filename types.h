#include "global.h"

#define TOT_SECTORS 64

typedef struct block_addr {
    bool         is_null;
    unsigned int segment;
    unsigned int block;
    unsigned int sector;
    unsigned int offset;
} block_addr;

typedef enum ftype{
    FILE_TYPE, 
    DIR_TYPE, 
    LINK_TYPE
} ftype;

typedef struct meta{
    ftype        type;
    unsigned int size;
    unsigned int last_mod;
    unsigned int created;
    char   *name;
} meta;


typedef struct inode {
    char          buf[1024];
    block_addr   *ba;
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