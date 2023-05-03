// Include necessary headers

#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35

#define u_int unsigned int

#include <math.h>
#include "flash.h"
#include "types.h"

// Define the allocation size of blocks
#define ALLOC_SIZE FLASH_SECTORS_PER_BLOCK*FLASH_SECTOR_SIZE

/*Globals********************************************************/
Flash FD;           // Flash device
disk_header *data;  // pointer to disk header
inod *inode_tab;    // pointer to inodes table
int tab_size;       // number of inodes
int bsize_bytes;    // block size in bytes
int K;              // used for indexing blocks in the inodes
/**************************************************************/

// Declare function prototypes
void mem_cpy_offset(char *dest, char *src, int start, int end);
void load_lfs(char *fname);
void read_file(inod *ino, char *buf, int num_blocks);

static void load_from_cpt(checkpoint *cpt);
static Flash load_device(char *fname);

/**************************************************************/

// Copy the content of src to dest buffer from index start to index end
void mem_cpy_offset(char *dest, char *src, int start, int end){
    for (int i = 0; (i+start)<end; i++){
        dest[i+start] = src[i];
    }
}

// Read the file from the device and write its content into the buffer buf
void read_file(inod *ino, char *buf,int num_blocks){
    int start, end, remain;
    char *tmp_buf = malloc(data->blocksize * FLASH_SECTOR_SIZE);

    // Calculate the number of blocks to read and their addresses
    remain = num_blocks;
    long addrs[num_blocks];
    memset(addrs, 0, num_blocks * sizeof(long));
    for(int i = 0; i<DIR_BLKS && i<num_blocks;i++){
        addrs[i] = ino->direct_addr[i];
        remain--;
    }
    if(remain > 0){
        long *addrbuf;
        char *charbuf = malloc(bsize_bytes);
        Flash_Read(FD,data->blocksize * ino->first_level, data->blocksize,charbuf);
        addrbuf = (long *)charbuf;
        for(int i=0; i < K && remain > 0; i++){
            addrs[i+DIR_BLKS] = addrbuf[i];
            remain--;
        }
        free(charbuf);
    }

    if(remain > 0){
        long *addrbuf2;
        char *charbuf2 = malloc(bsize_bytes);
        Flash_Read(FD,data->blocksize * ino->second_level, data->blocksize, charbuf2);
        addrbuf2 = (long *)charbuf2;
        for (int j = 0; j*K < K*K && remain > 0; j++){
            long *addrbuf; 
            char *charbuf = malloc(bsize_bytes);
            Flash_Read(FD,data->blocksize * addrbuf2[j],data->blocksize,charbuf);
            addrbuf = (long *)charbuf;
            for(int i=0; i < K && remain > 0; i++){
                addrs[(j*K) + i + DIR_BLKS + K] = addrbuf[i];
                remain --;
            }
            free(charbuf);
        }
        free(charbuf2);
    }

    if(remain > 0){
        printf("Third and Fourth address not implemented\n");
    }

    // Read the blocks into the buffer
    for(int i = 0; i < num_blocks; i++){
        if(addrs[i] > 0)
            Flash_Read(FD,data->blocksize * addrs[i], data->blocksize, tmp_buf);
        else 
            memset(tmp_buf, 0, data->blocksize * FLASH_SECTOR_SIZE);

        start = i * data->blocksize * FLASH_SECTOR_SIZE;
        end = start + (data->blocksize * FLASH_SECTOR_SIZE);
        mem_cpy_offset(buf, tmp_buf, start,end);

    }
    

    free(tmp_buf);
}

//This function loads the inode table from the specified checkpoint
// Parameters:
//  cpt: pointer to the checkpoint structure to load from
static void load_from_cpt(checkpoint *cpt){
    if (DEBUG)
        printf("Block 0 can be found at address %ld\n",cpt->block_address);

    // Read in inode0 from the specified block
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
        printf("There are %d inodes\n",tab_size);        
        for(int i=0; i<tab_size; i++)
            printf("I have size %ld\n",inode_tab[i].size);
    }
    
}

// This function loads a block device and sets it as the current device.
static Flash load_device(char *fname){
    
    // Open the block device and get its file descriptor.    
    u_int blocks;
    FD = Flash_Open(fname, 1, &blocks);

    if (DEBUG)
        printf("I have %d blocks\n",blocks);
    
    // Read the first block, which contains the disk header, and store it in head.
    char *head = malloc(ALLOC_SIZE);
    Flash_Read(FD, 0,FLASH_SECTORS_PER_BLOCK,head);

    // Cast the memory pointed to by head to a disk_header struct.
    data = (disk_header *)head;

    // If the DEBUG flag is set, print some information from the disk header.
    if (DEBUG){
        printf("Magic string is %s\n",data->magic);
        printf("Version is %d\n",data->version);
        printf("Block size is %d\n", data->blocksize);
        printf("Segment size is %d\n",data->segsize);
        printf("crc is %d\n",data->crc);
    }

    // Compute the block size in bytes and the number of longs per block.
    bsize_bytes = data->blocksize * FLASH_SECTOR_SIZE;
    K = bsize_bytes / sizeof(long);
    
    // Allocate memory to read the checkpoints.
    char *buf1 = malloc(ALLOC_SIZE);
    char *buf2 = malloc(ALLOC_SIZE);

    // Read the first & second checkpoint block and store it in buf1 & buf2 respectively.
    Flash_Read(FD,FLASH_SECTORS_PER_BLOCK,FLASH_SECTORS_PER_BLOCK,buf1);
    Flash_Read(FD,2*FLASH_SECTORS_PER_BLOCK,FLASH_SECTORS_PER_BLOCK,buf2);

    // Cast the memory pointed to by buf1 and buf2 to checkpoint structs.
    checkpoint *cp1 = (checkpoint *)buf1;
    checkpoint *cp2 = (checkpoint *)buf2;

    // Choose the most recent checkpoint and load the inode table from it.
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

// This function loads a LFS from a file.
void load_lfs(char *fname){
    // Load the block device from the file and set it as the current device.
    FD = load_device(fname);
}
