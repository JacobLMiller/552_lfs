#include <stdbool.h> 

typedef struct block_addr {
    bool         is_null;
    unsigned int page;
    unsigned int block;
} block_addr_t;

typedef enum ftype{
    FILE_TYPE, 
    DIR_TYPE, 
    LINK_TYPE
} ftype_t;

typedef struct meta{
    ftype_t        type;
    unsigned int size;
    unsigned int last_mod;
    unsigned int created;
} meta_t;


typedef struct inode {
    block_addr_t ba;
    meta_t       meta;
} inode_t;