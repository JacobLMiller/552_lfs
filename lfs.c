#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35

#define u_int unsigned int

#include <math.h>
#include "flash.h"
#include "types.h"
#include "log.h"
#include "lfs.h"

#define DEBUG 1

#define DEF_CP_INTERVAL 4
#define DEF_CLEAN_START 4
#define DEF_CLEAN_STOP  8

#define ALLOC_SIZE FLASH_SECTORS_PER_BLOCK*FLASH_SECTOR_SIZE

extern int getuid();
extern int getgid();

extern int BUGGY_crc(void *buf_void, int len);

/********************************************/

static Flash FD; 
static disk_header *data;
static inod *inode_tab;
static int tab_size;
static int bsize_bytes;

static void read_file(inod *ino, u_int size, char *buf){

}

static void read_inod_tab(long addr){
    char *buf = malloc(data->blocksize * FLASH_SECTOR_SIZE);
    Flash_Read(FD,data->blocksize * addr, data->blocksize, buf);
    
}

static void load_from_cpt(checkpoint *cpt){
    printf("Block 0 can be found at address %ld\n",cpt->block_address);
    //Read in inode0
    char *buf = malloc(data->blocksize * FLASH_SECTOR_SIZE);
    Flash_Read(FD,data->blocksize * cpt->block_address,data->blocksize,buf);
    inod *ino = (inod *)buf;
    ino->direct_addr[0] = cpt->block_address;

    //Allocate enough memory to inode_tab for all inodes
    int num_blocks = ino->size / bsize_bytes;
    if (ino->size % bsize_bytes != 0)
        num_blocks++;
    int num_inod_per_block = bsize_bytes/sizeof(inod);
    tab_size = num_inod_per_block * num_blocks;
    inode_tab = malloc(tab_size * sizeof(inod));

    for(int i = 0; i < 12 || i < num_blocks; i++){
        read_inod_tab(ino->direct_addr[i]);
    }
}

static Flash load_device(char *fname){
    u_int blocks;
    FD = Flash_Open(fname, 1, &blocks);

    if (DEBUG){
        printf("I have %d blocks\n",blocks);
    }
    char *head = malloc(ALLOC_SIZE);
    Flash_Read(FD, 0,FLASH_SECTORS_PER_BLOCK,head);

    data = (disk_header *)head;
    if (DEBUG){
        printf("Magic string is %s\n",data->magic);
        printf("Version is %d\n",data->version);
        printf("Block size is %d\n", data->blocksize);
        printf("Segment size is %d\n",data->segsize);
        printf("crc is %d\n",data->crc);
    }

    bsize_bytes = data->blocksize * ALLOC_SIZE;
    // free(head);
    
    char *buf1 = malloc(ALLOC_SIZE);
    char *buf2 = malloc(ALLOC_SIZE);
    Flash_Read(FD,FLASH_SECTORS_PER_BLOCK,FLASH_SECTORS_PER_BLOCK,buf1);
    Flash_Read(FD,2*FLASH_SECTORS_PER_BLOCK,FLASH_SECTORS_PER_BLOCK,buf2);
    checkpoint *cp1 = (checkpoint *)buf1;
    checkpoint *cp2 = (checkpoint *)buf2;

    if(cp1->seq_num > cp2->seq_num){
        load_from_cpt(cp1);
    }
    else{
        load_from_cpt(cp2);
    }
    free(buf1);
    free(buf2);
    

    return FD;

}

void read_segment(int seqnum){
    char *buf = malloc(sizeof(segment_header));
    Flash_Read(FD,seqnum * data->blocksize * data->segsize ,FLASH_SECTORS_PER_BLOCK,buf);
    segment_header *head = (segment_header *)buf;
    printf("Segment is %s\n",head->magic);    

}

int main(int argc, char **argv){

    //Quit if no file handle
    if (argc < 3){
        printf("Please provide name of flash device to be mounted\n");
        return 1;
    }

    char *devicename = argv[argc-2];
    char *mntpnt     = argv[argc-1];

    for(int i = 0; i < argc-1; i++){
        if(strcmp(argv[i],"-i") == 0){
            //cpnt_interval = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i],"-c") == 0){
            //cln_thrshld = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i],"-C") == 0){
            //stp_thrshld = (argv[i+1]);
        }
    }    

    FD = load_device(devicename);
    read_segment(1);
    Flash_Close(FD);


    // #define N_ARGS 4
    // char *fuseargs[N_ARGS] = {
    //     "fuse_sys",
    //     mntpnt,
    //     "-f",
    //     "-s",
    //     // "-o auto_cache"
    //     // "-d"
    // };

    // fuse_main(N_ARGS,fuseargs, &ops, NULL);

    return 0;
}