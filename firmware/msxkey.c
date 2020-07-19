#include <stdint.h>
#include <avr/pgmspace.h>

#include "msxkey.h"
#include "uart.h"

// Table to translate ASCII to MSX keyboard matrix
// Upper nybble is row; lower nybble is column
// Bit 4 set in lower nybble means key is shifted (|8)
// 0xFF means the character is not supported by the MSX keyboardc
const uint8_t msx_key_matrix[128] PROGMEM = {
    0xFF,   0xFF,   0x30|8, 0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   // 00-07
    0x75,   0x73,   0xFF,   0xFF,   0xFF,   0x77,   0xFF,   0xFF,   // 08-0F
    0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   // 10-17
    0xFF,   0xFF,   0xFF,   0xFF,   0x72,   0xFF,   0xFF,   0xFF,   // 18-1F
    0x80|8, 0x01|8, 0x20|8, 0x03|8, 0x04|8, 0x05|8, 0x07|8, 0x20,   // 20-27
    0x11|8, 0x00|8, 0x10|8, 0x13|8, 0x22,   0x12,   0x23,   0x24,   // 28-2F
    0x00,   0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07,   // 30-37
    0x10,   0x11,   0x17|8, 0x17,   0x22|8, 0x13,   0x23|8, 0x24|8, // 38-3F
    0x02|8, 0x26|8, 0x27|8, 0x30|8, 0x31|8, 0x32|8, 0x33|8, 0x34|8, // 40-47
    0x35|8, 0x36|8, 0x37|8, 0x40|8, 0x41|8, 0x42|8, 0x43|8, 0x44|8, // 48-4F
    0x45|8, 0x46|8, 0x47|8, 0x50|8, 0x51|8, 0x52|8, 0x53|8, 0x54|8, // 50-57
    0x55|8, 0x56|8, 0x57|8, 0x15,   0x14,   0x16,   0x06|8, 0x12|8, // 58-5F
    0x21,   0x26,   0x27,   0x30,   0x31,   0x32,   0x33,   0x34,   // 60-67
    0x35,   0x36,   0x37,   0x40,   0x41,   0x42,   0x43,   0x44,   // 68-6F
    0x45,   0x46,   0x47,   0x50,   0x51,   0x52,   0x53,   0x54,   // 70-77
    0x55,   0x56,   0x57,   0x15|8, 0x14|8, 0x16|8, 0x21|8, 0x83    // 78-7F
};

#define NO_KEY 0xFF
#define SHIFT_KEY 0x60
#define CTRL_KEY 0x61

#define ROW(s) (s >> 4)
#define SHIFTED(s) (s & 8)
#define BITS(s) ~(1 << (s & 7))

static uint8_t current_row = 0;
static uint8_t shift_step = 0;

uint8_t msx_scanrow(void)
{
    if (uart_testrx(0) == 0)
        return NO_KEY;

    uint8_t c = uart_peek(0) & 0x7F;
    uint8_t scancode = pgm_read_byte(&msx_key_matrix[c]);
    uint8_t modkey = (c < 0x20) ? CTRL_KEY : SHIFT_KEY;

    if (scancode == NO_KEY) {
        uart_getc(0);
        return NO_KEY;
    } else if (!SHIFTED(scancode) && current_row == ROW(scancode)) {
        uart_getc(0);
        return BITS(scancode);
    } else if (SHIFTED(scancode)) {
        if (shift_step == 0 && current_row == ROW(modkey)) {
            shift_step = 1;
            return BITS(modkey);
        } else if (shift_step == 1 && current_row == ROW(scancode)) {
            shift_step = 2;
            return BITS(scancode);
        } else if (shift_step == 2 && current_row == ROW(modkey)) {
            shift_step = 0;
            uart_getc(0);
            return BITS(modkey);
        }
    }
    return NO_KEY;
}

uint8_t msx_setrow(uint8_t row)
{
    current_row = row & 0xF;
}