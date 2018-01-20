#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void read_mem(uint16_t addr, uint8_t * buf, uint16_t len);
void write_mem(uint16_t addr, uint8_t * buf, uint16_t len);

void fill_mem(uint16_t start, uint16_t end, uint8_t value);
void dump_mem(uint16_t start, uint16_t end);
int verify_mem(uint16_t start, uint16_t end, uint8_t *src, uint8_t log);

#endif