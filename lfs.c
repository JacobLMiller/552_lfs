#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35 

#define u_int unsigned int

#define DEBUG 1

#define DEF_CP_INTERVAL 4
#define DEF_CLEAN_START 4
#define DEF_CLEAN_STOP  8


#include <stdio.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
// #include <malloc.h>
#include <stdlib.h>
#include "flash.h"
#include "types.h"

typedef struct sys_props {
    u_int n_segments;
    u_int block_size;
    u_int seg_size;
}sys_props;

static sys_props LFS_props;

Flash load_device(char *fname){
    u_int blocks;
    Flash FD = Flash_Open(fname, 0, &blocks);

    if (DEBUG)
        printf("I have %d blocks\n",blocks);
    
    u_int head[FLASH_BLOCK_SIZE];
    Flash_Read(FD,0,1,head);
    LFS_props.n_segments = head[0];
    LFS_props.block_size = head[1];
    LFS_props.seg_size   = head[2];

    return FD;

}

int main(int argc, char **argv){

    //Quit if no file handle
    if (argc < 3){
        printf("Please provide name of flash device to be mounted\n");
        return 1;
    }

    Flash FD = load_device(argv[1]);

    return 0;
}