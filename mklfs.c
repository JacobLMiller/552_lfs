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

int totalsectors = 64;

void fill_device(char *fname,u_int n_segments, u_int block_size, u_int seg_size,u_int wearlimit){
    u_int blocks;
    Flash mydevice = Flash_Open(fname,FLASH_ASYNC,&blocks);

    if (! mydevice){
        printf("Could not open file\n");
        return;
    }
    
    disk_data *data = malloc(sizeof(disk_data));
    data->blocksize   = block_size;
    data->segsize     = seg_size;
    data->disksize    = n_segments;
    data->wearlimit   = wearlimit;
    data->cur_sector  = seg_size * block_size;
    data->cur_block   = 0;
    data->cur_segment = 1;
    data->table_size  = 0;

    int fail = Flash_Write(mydevice,0,totalsectors,(void*)data);
    if (fail){
        printf("Failed to write header.\n");
    }
    else 
        printf("Succesfully wrote to file %s\n",fname);

    Flash_Close(mydevice);

}

int main(int argc, char **argv){

    //Init LFS parameters
    u_int n_segments = DEF_DISK_SIZE, block_size = DEF_BLOCK_SIZE, seg_size = DEF_SEG_SIZE;
    u_int wearlim = DEF_WEAR_LIM;
    u_int blocks;

    //Quit if no file handle
    if (argc < 2){
        printf("Please provide name of flash device to be created\n");
        return 1;
    }

    //Check that size is valid 
    if (seg_size < 3 || seg_size % FLASH_SECTORS_PER_BLOCK != 0){
        printf("Invalid segment size; Must >=3 and int multiple of %d",FLASH_SECTORS_PER_BLOCK);
    }

    blocks = (block_size*seg_size*n_segments) / FLASH_SECTORS_PER_BLOCK;
    int flash_device = Flash_Create(argv[1],wearlim,blocks);

    if (DEBUG){
        printf("Device creation was %s\n", flash_device == 0 ? "successful" : "unsuccessful");
        printf("Device name is %s\n",argv[1]);
    }

    fill_device(argv[1],n_segments,block_size,seg_size,wearlim);

    return flash_device;
}