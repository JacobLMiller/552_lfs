#include <zlib.h> // Include the zlib library for CRC calculations

// Define a function for calculating the CRC over the Disk Header
unsigned int calculate_crc(const unsigned int *disk_header) {
    // Initialize the CRC to zero
    unsigned int crc = crc32(0L, Z_NULL, 0);

    // Calculate the CRC over the entire Disk Header, except for the CRC field itself
    crc = crc32(crc, disk_header, 20);

    // Return the calculated CRC value
    return crc;
}
