/* Mock memory implementation for Z80 disassembler tests */

#include "memory.h"
#include <string.h>
#include <stdlib.h>

// Simple 64K memory buffer for testing
static uint8_t test_memory[65536];

void test_mem_init(void)
{
    memset(test_memory, 0, sizeof(test_memory));
}

void test_mem_write(uint32_t addr, const uint8_t* data, uint32_t len)
{
    if (addr + len > sizeof(test_memory)) {
        len = sizeof(test_memory) - addr;
    }
    memcpy(&test_memory[addr], data, len);
}

void mem_read(uint32_t addr, void* buf, uint32_t len)
{
    if (addr + len > sizeof(test_memory)) {
        len = sizeof(test_memory) - addr;
    }
    memcpy(buf, &test_memory[addr], len);
}
