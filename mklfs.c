#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35 

#define u_int unsigned int

#define DEBUG 0

#define DEF_WEAR_LIM 1000 
//Size of disk in segments
#define DEF_DISK_SIZE 100
//Size of blocks in sectors (defined in flash)
#define DEF_BLOCK_SIZE 2
//Size of segments in blocks
#define DEF_SEG_SIZE 32


#include <unistd.h>
// #include <malloc.h>
#include "flash.h"
#include "types.h"


void fill_device(char *fname,u_int n_segments, u_int block_size, u_int seg_size){
    u_int blocks;
    Flash mydevice = Flash_Open(fname,0,&blocks);

    u_int buf[FLASH_BLOCK_SIZE];
    buf[0] = n_segments;
    buf[1] = block_size;
    buf[2] = seg_size;

    int fail = Flash_Write(mydevice,0,FLASH_SECTORS_PER_BLOCK,buf);
    if (fail){
        printf("Failed to write header.\n");
    }


    u_int buf2[FLASH_BLOCK_SIZE];
    printf("%d\n",FLASH_BLOCK_SIZE);
    for (int i = 0; i < 2; i++){
        buf2[i] = i*2;
    }
    fail = Flash_Write(mydevice,0,FLASH_SECTORS_PER_BLOCK,buf2);
    if (fail){
        printf("Oops %d\n",fail);
    }

}

int main(int argc, char **argv){

    //Init LFS parameters
    u_int n_segments = DEF_DISK_SIZE, block_size = DEF_BLOCK_SIZE, seg_size = DEF_SEG_SIZE;
    u_int wearlim = DEF_WEAR_LIM, blocks = n_segments / seg_size;
    blocks = n_segments % seg_size == 0 ? blocks : blocks+1;

    //Quit if no file handle
    if (argc < 2){
        printf("Please provide name of flash device to be created\n");
        return 1;
    }

    //Check that size is valid 
    if (seg_size < 3 || seg_size % FLASH_SECTORS_PER_BLOCK != 0){
        printf("Invalid segment size; Must >=3 and int multiple of %d",FLASH_SECTORS_PER_BLOCK);
    }

    int flash_device = Flash_Create(argv[1],wearlim,blocks);

    if (DEBUG){
        printf("Device creation was %s\n", flash_device == 0 ? "successful" : "unsuccessful");
        printf("Device name is %s\n",argv[1]);
    }

    fill_device(argv[1],n_segments,block_size,seg_size);

    return flash_device;
}