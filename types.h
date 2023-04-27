#include "global.h"

#define TOT_SECTORS 64
#define u_char unsigned char
#define DIR_BLKS 12

typedef enum ftype {
    NOFILE,
    FILE_TYPE,
    DIR_TYPE,
    SYM_LINK,
    SPECIAL,
    RESERVED,
    DEAD
}ftype;

typedef struct disk_header{
    char        magic[8];
    int         version;
    int         blocksize;
    int         segsize;
    int         crc;
}disk_header;


typedef struct checkpoint{
    char        magic[4];
    int         padding;
    long        seq_num;
    long        block_address;
    int         crc;
}checkpoint;

typedef struct segment_header{
    char        magic[4];
    u_char      pad[4];
    long        seq_num;
    int         crc;
}segment_header;

typedef struct Segment {
    long        inum;
    long        block_num;
}Segment;

typedef struct segment_data{
    segment_header       header;
    u_char               pad[4];
    u_char               reserved[8];
    int                  crc;
    u_char               pad2[4];
    Segment              MetaData[];
    // char                 *padding;
}segment_data;

typedef struct inod{
    char        magic[4];
    ftype       type;
    long        mode;
    long        num_links;
    int         uid;
    int         gid;
    long        size;
    long        direct_addr[DIR_BLKS];
    long        first_level;
    long        second_level;
    long        third_level;
    long        fourth_level;
}inod;

typedef struct dir_entry{
    int         inum;
    char        name[252];
}dir_entry;