#include "types.h"

#define NUM_BUCKETS 256

static i_node *ITAB[NUM_BUCKETS];

static int h(const char *s){
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

i_node *i_node_lookup(const char *str){
    assert(str);

    int hval = h(str);

    return ITAB[hval];
}

void i_node_insert(const char *str, i_node *node){
    int hval = h(str);

    if(ITAB[hval] != NULL){
        printf("Collision!\n");
        assert(NULL);
    }

    ITAB[hval] = node;
    printf("inode %s is inserted at index %d\n", str, hval);
}