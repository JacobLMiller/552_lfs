#include "types.h"
#include "flash.h"
#include "log.h"

#define NUM_BUCKETS 256

typedef struct inode_container{
    i_node                  *ino;
    struct inode_container  *next;
}inocon;

static inocon *ITAB[NUM_BUCKETS];
static seg_data **SEGTAB;

extern disk_data *data;

static u_int avail_inum = 0;

extern u_int get_cur_segment();
extern u_int get_cur_block();
extern int log_write(char *arr);

static int hash(const char *s){
    int i = 0;
    for( ; s != NULL && *s != '\0'; s++)
        i += *s;
    
    return i % NUM_BUCKETS;
}

static seg_data *allocate_segment(int segno){
    SEGTAB[segno] = malloc(sizeof(seg_data));
    seg_data *segment    = SEGTAB[segno];
    segment->segno       = segno;
    segment->fill_blocks = 0;
    segment->written     = false;
    segment->arr_num     = malloc(sizeof(unsigned int) * data->segsize);
    segment->ino_arr     = malloc(sizeof(i_node *) * data->segsize);

    return segment;
}

static void add_ino_to_segdata(i_node *ino, int off_num){
    seg_data *segment;
    int segno = ino->addrs[off_num].segment;
    if(SEGTAB[segno] == NULL){
        segment = allocate_segment(segno);
    }else{
        segment = SEGTAB[segno];
    }

    int block_num = ino->addrs[off_num].block;
    segment->arr_num[block_num] = off_num;
    segment->ino_arr[block_num] = ino;
    segment->fill_blocks++;

}

void init_seg_tab(){
    int n_segs = data->disksize;
    SEGTAB = malloc(sizeof(*SEGTAB) * n_segs);
    for(int i = 0; i < n_segs; i++){
        SEGTAB[i] = NULL;
    }

    seg_data *segment = allocate_segment(0);
    segment->fill_blocks = 32;
}


void flush_to_log(){
    for (int j = 1; j < data->disksize; j++){
        if(SEGTAB[j] != NULL && SEGTAB[j]->fill_blocks >= data->segsize && SEGTAB[j]->written == false){
            seg_data *segment = SEGTAB[j];
            char *disk_buf = malloc(sizeof(char) * data->segsize * data->blocksize * FLASH_SECTOR_SIZE);

            for (int i = 0; i < data->segsize; i++){
                i_node *ino = segment->ino_arr[i];
                int offset = segment->arr_num[i];
                strcat(disk_buf, ino->addrs[offset].buf);
            }

            log_write(disk_buf);

            free(disk_buf);
            segment->written = true;
        }
    }
}


void init_inode_tab(){
    for (int i = 0; i < NUM_BUCKETS; i++)
        ITAB[i] = NULL;    
}


static void set_block_addr(block_addr *ba){
    ba->segment = get_cur_segment();    
    ba->block = get_cur_block();

    int bsize_bytes = data->blocksize * FLASH_SECTOR_SIZE;
    ba->buf = malloc(sizeof(char) * bsize_bytes);
    for (int i = 0; i < bsize_bytes; i++)
        ba->buf[i] = 'a';
}

i_node *create_inode(char *name, ftype type){
    block_addr *ino_addr = malloc(sizeof(block_addr));
    meta       *ino_meta = malloc(sizeof(meta));
    i_node     *ino      = malloc(sizeof(i_node));

    set_block_addr(ino_addr);

    ino_meta->type       = type;
    ino_meta->size       = 0;
    ino_meta->num_blocks = 1;
    ino_meta->name       = name;

    ino->addrs[0] = *ino_addr;
    ino->meta = ino_meta;
    ino->next = NULL;
    ino->ino  = avail_inum;
    avail_inum++;

    add_ino_to_segdata(ino,0);

    return ino;
}

/*
* Look up the string associated with an inode
* If found, return the inode, else return NULL.
*/

i_node *i_node_lookup(const char *str){
    assert(str);

    int hval = hash(str);
    inocon *cur = ITAB[hval];
    while (cur){
        if (strcmp(str, cur->ino->meta->name) == 0){
            return cur->ino;
        }
        cur = cur->next;
    }

    return NULL;
}

void i_node_insert(const char *str, i_node *node){
    int hval = hash(str);


    inocon *node_con = malloc(sizeof(inocon));
    node_con->ino = node;
    node_con->next = NULL;    

    if (ITAB[hval] == NULL){
        ITAB[hval] = node_con;
    }
    else{
        inocon *cur = ITAB[hval];

        while (true){
            if (! cur->next){
                cur->next = node_con;
                break;
            }
            cur = cur->next;
        }
    }

}

