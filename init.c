#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35

#define u_int unsigned int


#include <math.h>
#include "flash.h"
#include "types.h"

#define ALLOC_SIZE FLASH_SECTORS_PER_BLOCK*FLASH_SECTOR_SIZE

/*Globals**********/
Flash FD; 
disk_header *data;
inod *inode_tab;
int tab_size;
int bsize_bytes;
int K;
/*****************/


void mem_cpy_offset(char *dest, char *src, int start, int end);
void load_lfs(char *fname);
void read_file(inod *ino, char *buf, int num_blocks);

static void load_from_cpt(checkpoint *cpt);
static Flash load_device(char *fname);

/**************************************************************/
void mem_cpy_offset(char *dest, char *src, int start, int end){
    for (int i = 0; (i+start)<end; i++){
        dest[i+start] = src[i];
    }
}

void read_file(inod *ino, char *buf,int num_blocks){
    int start, end, remain;
    char *tmp_buf = malloc(data->blocksize * FLASH_SECTOR_SIZE);

    remain = num_blocks;
    long addrs[num_blocks];
    for(int i = 0; i<DIR_BLKS && i<num_blocks;i++){
        addrs[i] = ino->direct_addr[i];
        remain--;
    }
    if(remain > 0){
        long *addrbuf;
        char *charbuf = malloc(bsize_bytes);
        Flash_Read(FD,data->blocksize * ino->first_level, data->blocksize,charbuf);
        addrbuf = (long *)charbuf;
        for(int i=0; i+DIR_BLKS<num_blocks && remain > 0; i++){
            addrs[i+DIR_BLKS] = addrbuf[i];
            remain--;
        }
        free(charbuf);
    }

    if(ino->second_level > 0){
        printf("There are more blocks to read.\n");
    }

    for(int i = 0; i < num_blocks; i++){
        Flash_Read(FD,data->blocksize * addrs[i], data->blocksize, tmp_buf);

        start = i * data->blocksize * FLASH_SECTOR_SIZE;
        end = start + (data->blocksize * FLASH_SECTOR_SIZE);
        mem_cpy_offset(buf, tmp_buf, start,end);

    }
    

    free(tmp_buf);
}


static void load_from_cpt(checkpoint *cpt){
    if (DEBUG)
        printf("Block 0 can be found at address %ld\n",cpt->block_address);

    //Read in inode0
    char *buf = malloc(bsize_bytes);
    Flash_Read(FD,data->blocksize * cpt->block_address,data->blocksize,buf);
    inod *ino = (inod *)buf;
    ino->direct_addr[0] = cpt->block_address;

    //Allocate enough memory to inode_tab for all inodes
    int num_blocks = ino->size / bsize_bytes;
    if (ino->size % bsize_bytes != 0)
        num_blocks++;
    tab_size = ino->size/sizeof(inod);
    inode_tab = malloc(num_blocks * bsize_bytes);

    char *inod_buf = malloc(data->blocksize * FLASH_SECTOR_SIZE * num_blocks);
    read_file(ino,inod_buf,num_blocks);


    inode_tab = (inod *)inod_buf;

    if(DEBUG){
        for(int i=0; i<tab_size; i++)
            printf("I have size %ld\n",inode_tab[i].size);
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

    bsize_bytes = data->blocksize * FLASH_SECTOR_SIZE;
    K = bsize_bytes / sizeof(long);
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


void load_lfs(char *fname){
    FD = load_device(fname);
    
}
