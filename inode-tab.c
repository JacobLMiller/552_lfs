#include "types.h"

#define NUM_BUCKETS 256

typedef struct inode_container{
    i_node                  *ino;
    struct inode_container  *next;
}inocon;

static inocon *ITAB[NUM_BUCKETS];

static u_int avail_inum = 0;

static int hash(const char *s){
    int i = 0;
    for( ; s != NULL && *s != '\0'; s++)
        i += *s;
    
    return i % NUM_BUCKETS;
}

void init_inode_tab(){
    for (int i = 0; i < NUM_BUCKETS; i++)
        ITAB[i] = NULL;    
}


static void set_block_addr(block_addr *ba){
    
}

i_node *create_inode(char *name, ftype type){
    block_addr *ino_addr = malloc(sizeof(block_addr));
    meta       *ino_meta = malloc(sizeof(meta));
    i_node     *ino      = malloc(sizeof(i_node));

    set_block_addr(ino_addr);

    ino_meta->type     = type;
    ino_meta->size     = 0;
    ino_meta->last_mod = time(NULL);
    ino_meta->created  = time(NULL);
    ino_meta->name     = name;

    ino->ba   = ino_addr;
    ino->meta = ino_meta;
    ino->next = NULL;
    ino->ino  = avail_inum;
    avail_inum++;

    return ino;
}

/*
* Look up the string associated with an inode
* If found, return the inode, else return NULL.
*/

i_node *i_node_lookup(const char *str){
    assert(str);

    int hval = hash(str);
    inocon *cur = ITAB[hval];
    while (cur){
        if (strcmp(str, cur->ino->meta->name) == 0){
            return cur->ino;
        }
        cur = cur->next;
    }

    return NULL;
}

void i_node_insert(const char *str, i_node *node){
    int hval = hash(str);


    inocon *node_con = malloc(sizeof(inocon));
    node_con->ino = node;
    node_con->next = NULL;    

    if (ITAB[hval] == NULL){
        ITAB[hval] = node_con;
    }
    else{
        inocon *cur = ITAB[hval];

        while (true){
            if (! cur->next){
                cur->next = node_con;
                break;
            }
            cur = cur->next;
        }
    }

}