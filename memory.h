#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void read_mem(uint16_t addr, uint8_t * buf, uint16_t len);
void write_mem(uint16_t addr, uint8_t * buf, uint16_t len);

int verify_mem(uint16_t start, uint8_t *src, uint16_t len, uint8_t log);
void dump_mem(uint16_t addr, uint16_t len);

enum ihex_rc {
    IHEX_FORMAT = -128, 
    IHEX_COUNT,
    IHEX_CKSUM,
    IHEX_RECTYPE,
    IHEX_EOF
};

int write_ihex_rec(char *record);
char *read_ihex_rec(uint16_t addr, uint8_t len);

#endif