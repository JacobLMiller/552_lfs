typedef struct new_header{
    char        magic[8];
    int         version;
    int         blocksize;
    int         segsize;
    int         crc;
}new_header;


typedef struct checkpoint{
    char        magic[4];
    int         padding;
    long        seq_num;
    long        block_address;
    int         crc;
}checkpoint;

typedef struct segment_header{
    char        magic[4];
    long        seq_num;
    int         crc;
}segment_header;



typedef struct seg_meta {

}seg_meta;

typedef struct segment_data{
    struct segment_header       header;
    int                         reserved;
    int                         crc;
    seg_meta                    *table;
    char                        *padding;
}segment_data;