#include "types.h"
#include "flash.h"

#define NUM_BUCKETS 256

// Declare an array of pointers to the inocon struct (inode container)
extern inod *inode_tab;

// Define the inode container struct, which contains the inod struct,
// a pointer to the next inode container, and the name of the inode.
typedef struct inode_container{
    inod                    *ino;
    struct inode_container  *next;
    char                     name[255]; 
}inocon;

// Declare an array of pointers to inode containers
static inocon *ITAB[NUM_BUCKETS];

// Define a hash function that takes a string and returns an integer
static int hash(const char *s){
    int i = 0;
    for( ; s != NULL && *s != '\0'; s++)
        i += *s;
    
    return i % NUM_BUCKETS;
}

// Initialize the inode table
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

    int hval = hash(str);       // Compute the hash value of the string
    inocon *cur = ITAB[hval];   // Traverse the linked list of inode containers at the hash bucket
    while (cur){
        if (strcmp(str, cur->name) == 0){
            return cur->ino;
        }
        cur = cur->next;
    }

    return NULL;
}

// Insert an inode into the inode table, given a string and an inode struct pointer
void i_node_insert(const char *str, inod *node){
    
    int hval = hash(str);
    
    // Allocate memory for a new inode container
    inocon *node_con = malloc(sizeof(inocon));
    
    // Initialize the new inode container with the inode struct pointer and the name
    node_con->ino = node;
    memcpy(node_con->name,str,strlen(str));
    node_con->next = NULL;    

    // Insert the new inode container into the hash bucket
    if (ITAB[hval] == NULL){
        ITAB[hval] = node_con;
    } else {
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


