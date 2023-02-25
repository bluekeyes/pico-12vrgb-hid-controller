#include <stdio.h>

#include "debug.h"

#define DUMP_BYTES_PER_LINE 16

void dump_buffer(void *buf, size_t len, bool wrap)
{
    size_t count = 0;
    for (void *ptr = buf; ptr < buf + len; ptr++) {
        if (wrap && count % DUMP_BYTES_PER_LINE == 0) {
            if (count > 0) {
                putchar('\n');
            }
            printf("%4d: ", count);
        }
        printf("%02x ", *((uint8_t *) ptr));
        count++;
    }
    if (count > 0) {
        putchar('\n');
    }
}
