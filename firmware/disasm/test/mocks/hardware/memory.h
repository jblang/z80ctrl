#ifndef MOCK_MEMORY_H
#define MOCK_MEMORY_H

/* Mock memory.h for host compilation of Z80 disassembler tests
 * Provides simple memory simulation for testing
 */

#include <stdint.h>

// Simplified mock - only implements what disassembler needs
void mem_read(uint32_t addr, void* buf, uint32_t len);

// Test helper to set up memory contents
void test_mem_init(void);
void test_mem_write(uint32_t addr, const uint8_t* data, uint32_t len);

#endif // MOCK_MEMORY_H
