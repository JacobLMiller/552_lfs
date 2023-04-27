#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define MAGIC_STRING "RUSS_LFS"
#define LFS_VERSION 1
#define uint32_t unsigned int

typedef struct DiskHeader{
  char magic[8];
  uint32_t version;
  uint32_t block_size;
  uint32_t segment_size;
  uint32_t crc;
} DiskHeader;

typedef struct Checkpoint{
  char magic[4];
  char pad[4];
  uint32_t seq_num;
  uint32_t block_address;
  uint32_t crc;
} Checkpoint;

uint32_t buggy_crc(char* data, int size) {
  // implementation of buggy_crc() function
}

int main() {
  // example usage
  DiskHeader header;
  Checkpoint checkpoint;

  // read Disk Header from disk
  // assume that data is read into a buffer called 'buffer'
  char *buffer = malloc(sizeof(DiskHeader));
  memcpy(&header, buffer, sizeof(DiskHeader));

  // check if magic string matches
  if (strcmp(header.magic, MAGIC_STRING) != 0) {
    printf("Invalid disk header: magic string doesn't match\n");
    return 1;
  }

  // check if LFS version is correct
  if (header.version != LFS_VERSION) {
    printf("Invalid disk header: unsupported LFS version\n");
    return 1;
  }

  // calculate CRC of the Disk Header
  uint32_t crc = buggy_crc(buffer, sizeof(DiskHeader) - sizeof(uint32_t));
  if (crc != header.crc) {
    printf("Invalid disk header: CRC check failed\n");
    return 1;
  }

  // read Checkpoint from disk
  // assume that data is read into a buffer called 'buffer'
  memcpy(&checkpoint, buffer, sizeof(Checkpoint));

  // check if magic string matches
  if (strcmp(checkpoint.magic, "CKPT") != 0) {
    printf("Invalid checkpoint: magic string doesn't match\n");
    return 1;
  }

  // check if pad bytes are zero
  for (int i = 0; i < sizeof(checkpoint.pad); i++) {
    if (checkpoint.pad[i] != 0) {
      printf("Invalid checkpoint: pad bytes are not zero\n");
      return 1;
    }
  }

  // calculate CRC of the Checkpoint
  crc = buggy_crc(buffer, sizeof(Checkpoint) - sizeof(uint32_t));
  if (crc != checkpoint.crc) {
    printf("Invalid checkpoint: CRC check failed\n");
    return 1;
  }

  // print sequence number and block address of the checkpoint
  printf("Sequence Number: %u\n", checkpoint.seq_num);
  printf("Block Address: %u\n", checkpoint.block_address);

  return 0;
}
