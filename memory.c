#include "memory.h"
#include "bus.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Read specified number of bytes from external memory to a buffer
void read_mem(uint16_t addr, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    bus_master();
    DATA_INPUT;
    MREQ_LO;
    RD_LO;
    SET_ADDR(addr);
    for (i = 0; i < len; i++) {
        buf[i] = GET_DATA;
        addr++;
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr);
        } else {
            SET_ADDRLO(addr & 0xFF);
        }
    }
    RD_HI;
    MREQ_HI;
    bus_slave();
}

// Write specified number of bytes to external memory from a buffer
void write_mem(uint16_t addr, uint8_t *buf, uint16_t len)
{
    bus_master();
    DATA_OUTPUT;
    MREQ_LO;
    SET_ADDR(addr);
    uint16_t i;
    for (i = 0; i < len; i++) {
        SET_DATA(buf[i]);
        WR_LO;
        WR_HI;
        addr++;
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr);
        } else {
            SET_ADDRLO(addr & 0xFF);
        }
    }
    MREQ_HI;
    DATA_INPUT;
    bus_slave();
}

// Verify specified number of bytes from external memory against a buffer
int verify_mem(uint16_t start, uint8_t *src, uint16_t len, uint8_t log)
{
    uint8_t buf[256];
    int errors = 0;
    int i = 0, j = 0, buflen = 256;

    while (i < len) {
        if (len - i < buflen)
            buflen = len - i;
        read_mem(start+i, buf, buflen);
        for (j = 0; j < buflen; i++, j++) {
            if (buf[j] != src[i]) {
                if (log)
                    printf("Mismatch at %04x: expected %02x but read %02x\n", start+i, src[i], buf[j]);
                errors++;
            }
        }
    }
    return errors;
}

// Hex dump memory block to console
void dump_mem(uint16_t addr, uint16_t len)
{
    uint8_t buf[16];
    int i = 0, j = 0, buflen = 16;

    while (i < len) {
        printf("%04X: ", addr + i);
        read_mem(addr + i, buf, buflen);
        for (j = 0; j < buflen; j++) {
            printf("%02X ", buf[j]);
        }
        printf("  ");
        for (j = 0; j < buflen; j++, i++) {
            if (0x20 <= buf[j] && buf[j] <= 0x7e)
                printf("%c", buf[j]);
            else
                printf(".");
        }
        printf("\n");
    }
}