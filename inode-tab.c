#include "types.h"
#include "flash.h"

#define NUM_BUCKETS 256

extern inod *inode_tab;

typedef struct inode_container{
    inod                    *ino;
    struct inode_container  *next;
    char                     name[255]; 
}inocon;

static inocon *ITAB[NUM_BUCKETS];

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


/*
* Look up the string associated with an inode
* If found, return the inode, else return NULL.
*/

inod *i_node_lookup(const char *str){
    assert(str);
    printf("Looking up %s\n",str);

    int hval = hash(str);
    inocon *cur = ITAB[hval];
    printf("I am %s\n", str);
    while (cur){
        if (strcmp(str, cur->name) == 0){
            return cur->ino;
        }
        cur = cur->next;
    }

    return NULL;
}

void i_node_insert(const char *str, inod *node){
    int hval = hash(str);

    inocon *node_con = malloc(sizeof(inocon));
    node_con->ino = node;
    memcpy(node_con->name,str,strlen(str));
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

