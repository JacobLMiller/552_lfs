#include "types.h"
#include "flash.h"
#include "log.h"



disk_data *data;
static Flash FD;

// static u_int fin_segment;
// static u_int fin_block;

u_int get_cur_segment(){
    if(data->cur_block >= data->segsize){
        data->cur_block = 0;
        data->cur_segment ++;
    }
    return data->cur_segment;
}

u_int get_cur_block(){
    return data->cur_block++;
}

int log_write(char *arr){
    u_int blocks;
    FD = Flash_Open(data->fname, FLASH_ASYNC | FLASH_SILENT, &blocks);

    u_int cur_segment = data->cur_segment;
    u_int offset_blocks = (cur_segment * data->segsize);

    if( Flash_Erase(FD,offset_blocks, data->segsize) ){
        printf("Couldn't erase\n");
        return 1;
    }
    u_int write_sector = data->blocksize * offset_blocks;
    u_int sectors_per_seg = data->segsize * data->blocksize;
    if( Flash_Write(FD,write_sector,sectors_per_seg , (void *)arr ) ){
        printf("Couldn't write\n");
        return 1;
    }

    Flash_Close(FD);
    return 0;
}

int make_cp(){
    

    return 0;
}
