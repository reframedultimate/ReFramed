#include "uh/crc32.h"
#include <string.h>

static uint32_t crc32_table[256];

/* ------------------------------------------------------------------------- */
void crc32_init(void)
{
    for (int i = 0; i < 256; i++) {
        uint32_t rem = i;  /* remainder from polynomial division */
        for (int j = 0; j < 8; j++) {
            if (rem & 1) {
                rem >>= 1;
                rem ^= 0xedb88320;
            } else
                rem >>= 1;
        }
        crc32_table[i] = rem;
    }
}

/* ------------------------------------------------------------------------- */
uint32_t crc32_buf(const void* buf, uintptr_t len, uint32_t crc)
{
    crc = ~crc;
    const uint8_t* q = (const uint8_t*)buf + len;
    for (const uint8_t* p = (const uint8_t*)buf; p < q; p++) {
        uint8_t octet = *p;  /* Cast to unsigned octet. */
        crc = (crc >> 8) ^ crc32_table[(crc & 0xff) ^ octet];
    }
    return ~crc;
}

/* ------------------------------------------------------------------------- */
uint32_t crc32_str(const char* str, uint32_t crc)
{
    return crc32_buf((const void*)str, strlen(str), crc);
}
