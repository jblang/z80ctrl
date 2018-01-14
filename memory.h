#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void read_mem(uint16_t addr, uint8_t * buf, uint16_t len);
void write_mem(uint16_t addr, uint8_t * buf, uint16_t len);

int verify_mem(uint16_t start, uint8_t *src, uint16_t len, uint8_t log);
void dump_mem(uint16_t addr, uint16_t len);

enum ihex_rectype {
    IHEX_DATA = 0,
    IHEX_EOF,
    IHEX_ESA,
    IHEX_SSA,
    IHEX_ELA,
    IHEX_SLA
};

enum ihex_rc {
    IHEX_OK = 0,
    IHEX_FORMAT, 
    IHEX_COUNT,
    IHEX_CKSUM,
    IHEX_RECTYPE
};

typedef struct _ihex_res {
    uint8_t count;
    uint16_t addr;
    uint8_t type;
    uint8_t rc;
} ihex_res;

ihex_res write_ihex_rec(char *record);
char *read_ihex_rec(uint16_t addr, uint8_t len);

#endif