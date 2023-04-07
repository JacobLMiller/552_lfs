#include "global.h"
#include "types.h"

//Appends a file inode to directory
void append_file(i_node *curdir, i_node *file){
    i_node *end = curdir;
    if (end->next == NULL){
        end->next = file;
    }
    else{
        while (true){
            if (end->next == NULL){
                end->next = file;
                break;
            }
            end = end->next;
        }
    }
}
