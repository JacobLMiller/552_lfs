#include "global.h"
#include "types.h"

extern inod *inode_tab;
extern int bsize_bytes;
extern void read_file(inod *ino, char *buf, int num_blocks);

dir_entry *read_dir(inod *dir,int *num_children){
    *num_children = dir->size / sizeof(dir_entry);
    int num_blocks = (dir->size / bsize_bytes) + 1;
    char *dir_buf = malloc(num_blocks * bsize_bytes);
    read_file(dir,dir_buf,num_blocks);
    dir_entry *children = (dir_entry *)dir_buf;
    return children;
}

// static char *extractSubstring(char* inputString,char *remainder) {
//     char* start = strchr(inputString, '/');
//     if (start == NULL) {
//         return NULL;
//     }
//     char* end = strchr(start + 1, '/'); 
//     if (end == NULL) {
//         end = inputString + strlen(inputString); 
//     }
//     size_t length = end - start - 1; 
//     size_t remlen = strlen(inputString) - length;
//     char* substring = malloc(length + 1); 
//     if (substring == NULL) {
//         return NULL; 
//     }
//     memcpy(substring, start + 1, length);
//     memcpy(remainder,end,remlen);
//     substring[length] = '\0'; 
//     return substring;
// }


inod *lookup(const char *path, int inum){
    if(strcmp(path, "/") == 0){
        return &inode_tab[1];
    }
    // else if(strcmp(path, "") == 0){
    //     return &inode_tab[inum];
    // }

    inod *dir = &inode_tab[inum];
    assert(dir->type == DIR_TYPE);

    char* token;
    char* rest = (char *)path;
 
    while ((token = strtok_r(rest, "/", &rest))){
        if(strcmp(token, ".Trash-1000") == 0 || strcmp(token, ".Trash") == 0){
            return NULL;
        }
        int num_children;
        dir_entry *children = read_dir(dir,&num_children);
        for(int i=0; i<num_children;i++){
            if(strcmp(token,children[i].name) == 0){
                dir = &inode_tab[children[i].inum];
                // printf("Found and name is %s\n",children[i].name);
                break;
            }
        }
        free(children);
    }

    return dir;
}
