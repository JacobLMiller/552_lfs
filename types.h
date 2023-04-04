#include "global.h"

typedef struct block_addr {
    bool         is_null;
    unsigned int page;
    unsigned int block;
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
    const char   *name;
} meta;


typedef struct inode {
    block_addr   *ba;
    meta         *meta;
    struct inode *next;
} i_node;