#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BLOCK_SIZE 4096
#define SEGMENT_SIZE (32 * BLOCK_SIZE)

typedef struct segment_header {
    char magic[4];
    uint32_t sequence_number;
    uint32_t crc;
} segment_header_t;

typedef struct segment_metadata_entry {
    uint32_t block_address;
    uint32_t flags;
    uint32_t crc;
} segment_metadata_entry_t;

int main() {
    // Allocate memory for the segment header and metadata array
    char* segment_data = (char*) malloc(SEGMENT_SIZE);
    if (segment_data == NULL) {
        printf("Failed to allocate memory for segment\n");
        return 1;
    }

    // Initialize the segment header
    segment_header_t* header = (segment_header_t*) segment_data;
    header->magic[0] = 'S';
    header->magic[1] = 'G';
    header->magic[2] = 'M';
    header->magic[3] = 'T';
    header->sequence_number = 1;
    header->crc = 0; // TODO: calculate CRC

    // Initialize the segment metadata array
    segment_metadata_entry_t* metadata_array = (segment_metadata_entry_t*) (segment_data + BLOCK_SIZE);
    for (int i = 0; i < 32; i++) {
        segment_metadata_entry_t* entry = metadata_array + i;
        entry->block_address = 32 + i;
        entry->flags = 0;
        entry->crc = 0; // TODO: calculate CRC
    }

    // Write the segment data to disk
    // TODO: replace with actual write operation
    printf("Segment data:\n");
    for (int i = 0; i < SEGMENT_SIZE; i++) {
        printf("%02x", segment_data[i]);
    }
    printf("\n");

    // Free the memory
    free(segment_data);

    return 0;
}
