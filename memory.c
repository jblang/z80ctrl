#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "memory.h"
#include "bus.h"

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

// Write specified number of bytes to external memory from a buffer
void write_mem_P(uint16_t addr, uint8_t *buf, uint16_t len)
{
    bus_master();
    DATA_OUTPUT;
    MREQ_LO;
    SET_ADDR(addr);
    uint16_t i;
    for (i = 0; i < len; i++) {
        SET_DATA(pgm_read_byte(&buf[i]));
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
int verify_mem(uint16_t start, uint16_t end, uint8_t *src, uint8_t log)
{
    uint8_t buf[256];
    int errors = 0;
    uint16_t j = 0, buflen = 256;
    uint32_t i = start;

    while (i <= end) {
        if (end - i + 1 < buflen)
            buflen = end - i + 1;
        read_mem(start, buf, buflen);
        for (j = 0; j < buflen; i++, j++) {
            if (buf[j] != src[i]) {
                if (log)
                    printf("%04x: expected %02x but read %02x\n", start+i, src[i], buf[j]);
                errors++;
            }
        }
    }
    return errors;
}

// Hex dump memory block to console
void dump_mem(uint16_t start, uint16_t end)
{
    uint8_t buf[16];
    uint8_t j;
    uint8_t buflen = 16;
    uint32_t i = start;

    while (i <= end) {
        printf("%04X: ", i);
        read_mem(i, buf, buflen);
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

void fill_mem(uint16_t start, uint16_t end, uint8_t value)
{
    uint8_t buf[256];
    uint16_t i;
    for (i = 0; i < 256; i++)
        buf[i] = value;
    for (;;) {
        if (end - start > 256) {
            write_mem(start, buf, 256);
            start += 256;
        } else {
            write_mem(start, buf, end - start + 1);
            break;
        }
    }
}