#include <stdio.h>
#include <assert.h>



/* this function attempts to generate a CRC from a buffer, but I'm pretty sure
 * that it doesn't work correctly.  One of these days, I will fix it up and
 * write a version which is correct.  But this gives a "pretty good" result,
 * and is sufficent for prototyping.
 *                         - Russ Lewis   10 Apr 2023
 */

int BUGGY_crc(void *buf_void, int len)
{
    unsigned char *buf = buf_void;

    assert(buf != NULL);
    assert(len > 1);

#define DIVISOR 0x137   // 0b1_0011_0111

    int cur = buf[0];
    int read_pos = 1;

    while (read_pos <= len)
    {
        int next_byte    = (read_pos < len) ? buf[read_pos] : 0;
        int nb_bits_left = 8;
        read_pos++;

        while (nb_bits_left > 0)
        {
//            printf("cur 0x%02x read_pos %d - next_byte 0x%02x bits_left %d\n", cur,read_pos, next_byte,nb_bits_left);

            assert(cur < 0x100);
            cur <<= 1;

            int high_bit = (next_byte >> 7) & 0x1;
            cur |= high_bit;

            next_byte <<= 1;
            next_byte  &= 0x0ff;
            nb_bits_left--;

//            printf("    SHIFT   new-cur 0x%02x new-nb 0x%02x\n", cur,next_byte);

            if (cur & 0x100)
            {
                cur ^= DIVISOR;
//                printf("    XOR     new-cur 0x%02x new-nb 0x%02x\n", cur,next_byte);
            }
        }
    }

    return cur;
}

