#ifndef MOCK_PGMSPACE_H
#define MOCK_PGMSPACE_H

/* Mock pgmspace.h for host compilation of Z80 disassembler tests
 * Replaces AVR-specific PROGMEM functionality with standard C
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// On host, PROGMEM has no effect - data is in normal memory
#define PROGMEM

// On host, reading from program memory is just normal memory access
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))

// PSTR is just a normal string literal on host
#define PSTR(s) (s)

// printf_P is just printf on host
#define printf_P printf

// sprintf_P is just sprintf on host
#define sprintf_P sprintf

#endif // MOCK_PGMSPACE_H
