// Include necessary header files
#include "global.h"
#include "types.h"

// Declare global variables and functions from other files

/*Exposed from init.c*/
extern inod *inode_tab;
extern int bsize_bytes;
extern void read_file(inod *ino, char *buf, int num_blocks);
/*********************/

/*Exposed from inode-tab.c*/
extern inod *i_node_lookup(const char *str);
extern void i_node_insert(const char *str, inod *node);
/**************************/

// Define a function to read the contents of a directory inode
dir_entry *read_dir(inod *dir,int *num_children){
    *num_children = dir->size / sizeof(dir_entry);
    int num_blocks = (dir->size / bsize_bytes) + 1;
    char *dir_buf = malloc(num_blocks * bsize_bytes);
    read_file(dir,dir_buf,num_blocks);
    dir_entry *children = (dir_entry *)dir_buf;
    return children;
}

// Define a function to look up an inode based on its path
inod *lookup(const char *path, int inum){

    // First, try to find the inode in the lookup table
    inod *target = i_node_lookup(path);
    if(target != NULL){
        return target;
    }

    // If the inode is not found in the lookup table, check if the path is the root directory
    if(strcmp(path, "/") == 0){
        i_node_insert(path,&inode_tab[1]);
        return &inode_tab[1];
    }    
    

    inod *dir = &inode_tab[inum];
    assert(dir->type == DIR_TYPE);

    char* token;
    char* rest = (char *)path;
 
     // Split the path into individual directory names and traverse the directory tree
    while ((token = strtok_r(rest, "/", &rest))){
        
        // Check if the directory name is a special directory to ignore
        if(strcmp(token, ".Trash-1000") == 0 || strcmp(token, ".Trash") == 0){
            return NULL;
        }

        int num_children;
        dir_entry *children = read_dir(dir,&num_children);
        for(int i=0; i<num_children;i++){
            // Check if the current directory contains a child directory with the current name 
            if(strcmp(token,children[i].name) == 0){
                dir = &inode_tab[children[i].inum];
                break;
            }
        }

        // free the memory used by the directory contents
        free(children);
        
    }

    i_node_insert(path,dir);

    return dir;
}
