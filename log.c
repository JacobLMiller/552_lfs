#include "log.h"
#include "types.h"
#include "flash.h"

disk_data *data;
static Flash FD;

int log_write(i_node *ino){
    u_int blocks;
    FD = Flash_Open(data->fname, FLASH_ASYNC | FLASH_SILENT, &blocks);

    // u_int cur_segment = data->cur_segment;
    // u_int cur_block   = data->cur_block;
    u_int cur_sector  = data->cur_sector;

    if( !Flash_Erase(FD,cur_sector,data->blocksize)){

        int scalar = FLASH_SECTORS_PER_BLOCK;
        if(Flash_Write(FD,cur_sector*scalar,(data->blocksize)*scalar,(void *)ino->buf)){
            printf("Could not erase\n");
        }
    }else{
        printf("Could not erase\n");
    }

    Flash_Close(FD);
    return 0;
}