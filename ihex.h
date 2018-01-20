#ifndef IHEX_H
#define IHEX_H

#include <stdint.h>

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

ihex_res ihex_to_bin(char *ihex, uint8_t *bin);
void bin_to_ihex(uint8_t *bin, char *ihex, uint16_t addr, uint8_t count);


#endif