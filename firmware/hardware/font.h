#ifndef HARDWARE_FONT_H
#define HARDWARE_FONT_H

#include "util/pgmspace.h"

const char font[] PROGMEM = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x38,0x44,0x6C,0x44,0x54,0x44,0x38,0x00,
    0x38,0x7C,0x54,0x7C,0x44,0x7C,0x38,0x00,
    0x00,0x28,0x7C,0x7C,0x7C,0x38,0x10,0x00,
    0x00,0x10,0x38,0x7C,0x7C,0x38,0x10,0x00,
    0x10,0x38,0x38,0x10,0x7C,0x7C,0x10,0x00,
    0x00,0x10,0x38,0x7C,0x7C,0x10,0x38,0x00,
    0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,
    0xFC,0xFC,0xFC,0xCC,0xCC,0xFC,0xFC,0xFC,
    0x00,0x00,0x78,0x48,0x48,0x78,0x00,0x00,
    0xFC,0xFC,0x84,0xB4,0xB4,0x84,0xFC,0xFC,
    0x00,0x1C,0x0C,0x34,0x48,0x48,0x30,0x00,
    0x38,0x44,0x44,0x38,0x10,0x38,0x10,0x00,
    0x10,0x18,0x14,0x10,0x30,0x70,0x60,0x00,
    0x0C,0x34,0x2C,0x34,0x2C,0x6C,0x60,0x00,
    0x00,0x54,0x38,0x6C,0x38,0x54,0x00,0x00,
    0x20,0x30,0x38,0x3C,0x38,0x30,0x20,0x00,
    0x08,0x18,0x38,0x78,0x38,0x18,0x08,0x00,
    0x10,0x38,0x7C,0x10,0x7C,0x38,0x10,0x00,
    0x28,0x28,0x28,0x28,0x28,0x00,0x28,0x00,
    0x3C,0x54,0x54,0x34,0x14,0x14,0x14,0x00,
    0x38,0x44,0x30,0x28,0x18,0x44,0x38,0x00,
    0x00,0x00,0x00,0x00,0x00,0x78,0x78,0x00,
    0x10,0x38,0x7C,0x10,0x7C,0x38,0x10,0x38,
    0x10,0x38,0x7C,0x10,0x10,0x10,0x10,0x00,
    0x10,0x10,0x10,0x10,0x7C,0x38,0x10,0x00,
    0x00,0x10,0x18,0x7C,0x18,0x10,0x00,0x00,
    0x00,0x10,0x30,0x7C,0x30,0x10,0x00,0x00,
    0x00,0x00,0x00,0x40,0x40,0x40,0x7C,0x00,
    0x00,0x28,0x28,0x7C,0x28,0x28,0x00,0x00,
    0x10,0x10,0x38,0x38,0x7C,0x7C,0x00,0x00,
    0x7C,0x7C,0x38,0x38,0x10,0x10,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x10,0x38,0x38,0x10,0x10,0x00,0x10,0x00,
    0x6C,0x6C,0x48,0x00,0x00,0x00,0x00,0x00,
    0x00,0x28,0x7C,0x28,0x28,0x7C,0x28,0x00,
    0x20,0x38,0x40,0x30,0x08,0x70,0x10,0x00,
    0x64,0x64,0x08,0x10,0x20,0x4C,0x4C,0x00,
    0x20,0x50,0x50,0x20,0x54,0x48,0x34,0x00,
    0x30,0x30,0x20,0x00,0x00,0x00,0x00,0x00,
    0x10,0x20,0x20,0x20,0x20,0x20,0x10,0x00,
    0x20,0x10,0x10,0x10,0x10,0x10,0x20,0x00,
    0x00,0x28,0x38,0x7C,0x38,0x28,0x00,0x00,
    0x00,0x10,0x10,0x7C,0x10,0x10,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x20,
    0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,
    0x00,0x04,0x08,0x10,0x20,0x40,0x00,0x00,
    0x38,0x44,0x4C,0x54,0x64,0x44,0x38,0x00,
    0x10,0x30,0x10,0x10,0x10,0x10,0x38,0x00,
    0x38,0x44,0x04,0x18,0x20,0x40,0x7C,0x00,
    0x38,0x44,0x04,0x38,0x04,0x44,0x38,0x00,
    0x08,0x18,0x28,0x48,0x7C,0x08,0x08,0x00,
    0x7C,0x40,0x40,0x78,0x04,0x44,0x38,0x00,
    0x18,0x20,0x40,0x78,0x44,0x44,0x38,0x00,
    0x7C,0x04,0x08,0x10,0x20,0x20,0x20,0x00,
    0x38,0x44,0x44,0x38,0x44,0x44,0x38,0x00,
    0x38,0x44,0x44,0x3C,0x04,0x08,0x30,0x00,
    0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x00,
    0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x20,
    0x08,0x10,0x20,0x40,0x20,0x10,0x08,0x00,
    0x00,0x00,0x7C,0x00,0x00,0x7C,0x00,0x00,
    0x20,0x10,0x08,0x04,0x08,0x10,0x20,0x00,
    0x38,0x44,0x04,0x18,0x10,0x00,0x10,0x00,
    0x38,0x44,0x5C,0x54,0x5C,0x40,0x38,0x00,
    0x38,0x44,0x44,0x44,0x7C,0x44,0x44,0x00,
    0x78,0x44,0x44,0x78,0x44,0x44,0x78,0x00,
    0x38,0x44,0x40,0x40,0x40,0x44,0x38,0x00,
    0x78,0x44,0x44,0x44,0x44,0x44,0x78,0x00,
    0x7C,0x40,0x40,0x78,0x40,0x40,0x7C,0x00,
    0x7C,0x40,0x40,0x78,0x40,0x40,0x40,0x00,
    0x38,0x44,0x40,0x5C,0x44,0x44,0x3C,0x00,
    0x44,0x44,0x44,0x7C,0x44,0x44,0x44,0x00,
    0x38,0x10,0x10,0x10,0x10,0x10,0x38,0x00,
    0x04,0x04,0x04,0x04,0x44,0x44,0x38,0x00,
    0x44,0x48,0x50,0x60,0x50,0x48,0x44,0x00,
    0x40,0x40,0x40,0x40,0x40,0x40,0x7C,0x00,
    0x44,0x6C,0x54,0x44,0x44,0x44,0x44,0x00,
    0x44,0x64,0x54,0x4C,0x44,0x44,0x44,0x00,
    0x38,0x44,0x44,0x44,0x44,0x44,0x38,0x00,
    0x78,0x44,0x44,0x78,0x40,0x40,0x40,0x00,
    0x38,0x44,0x44,0x44,0x54,0x48,0x34,0x00,
    0x78,0x44,0x44,0x78,0x48,0x44,0x44,0x00,
    0x38,0x44,0x40,0x38,0x04,0x44,0x38,0x00,
    0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x00,
    0x44,0x44,0x44,0x44,0x44,0x44,0x38,0x00,
    0x44,0x44,0x44,0x44,0x44,0x28,0x10,0x00,
    0x44,0x44,0x54,0x54,0x54,0x54,0x28,0x00,
    0x44,0x44,0x28,0x10,0x28,0x44,0x44,0x00,
    0x44,0x44,0x44,0x28,0x10,0x10,0x10,0x00,
    0x78,0x08,0x10,0x20,0x40,0x40,0x78,0x00,
    0x38,0x20,0x20,0x20,0x20,0x20,0x38,0x00,
    0x00,0x40,0x20,0x10,0x08,0x04,0x00,0x00,
    0x38,0x08,0x08,0x08,0x08,0x08,0x38,0x00,
    0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,
    0x30,0x30,0x10,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,
    0x40,0x40,0x78,0x44,0x44,0x44,0x78,0x00,
    0x00,0x00,0x38,0x44,0x40,0x44,0x38,0x00,
    0x04,0x04,0x3C,0x44,0x44,0x44,0x3C,0x00,
    0x00,0x00,0x38,0x44,0x78,0x40,0x38,0x00,
    0x18,0x20,0x20,0x78,0x20,0x20,0x20,0x00,
    0x00,0x00,0x3C,0x44,0x44,0x3C,0x04,0x38,
    0x40,0x40,0x70,0x48,0x48,0x48,0x48,0x00,
    0x10,0x00,0x10,0x10,0x10,0x10,0x18,0x00,
    0x08,0x00,0x18,0x08,0x08,0x08,0x48,0x30,
    0x40,0x40,0x48,0x50,0x60,0x50,0x48,0x00,
    0x10,0x10,0x10,0x10,0x10,0x10,0x18,0x00,
    0x00,0x00,0x68,0x54,0x54,0x44,0x44,0x00,
    0x00,0x00,0x70,0x48,0x48,0x48,0x48,0x00,
    0x00,0x00,0x38,0x44,0x44,0x44,0x38,0x00,
    0x00,0x00,0x78,0x44,0x44,0x44,0x78,0x40,
    0x00,0x00,0x3C,0x44,0x44,0x44,0x3C,0x04,
    0x00,0x00,0x58,0x24,0x20,0x20,0x70,0x00,
    0x00,0x00,0x38,0x40,0x38,0x04,0x38,0x00,
    0x00,0x20,0x78,0x20,0x20,0x28,0x10,0x00,
    0x00,0x00,0x48,0x48,0x48,0x58,0x28,0x00,
    0x00,0x00,0x44,0x44,0x44,0x28,0x10,0x00,
    0x00,0x00,0x44,0x44,0x54,0x7C,0x28,0x00,
    0x00,0x00,0x48,0x48,0x30,0x48,0x48,0x00,
    0x00,0x00,0x48,0x48,0x48,0x38,0x10,0x60,
    0x00,0x00,0x78,0x08,0x30,0x40,0x78,0x00,
    0x18,0x20,0x20,0x60,0x20,0x20,0x18,0x00,
    0x10,0x10,0x10,0x00,0x10,0x10,0x10,0x00,
    0x30,0x08,0x08,0x0C,0x08,0x08,0x30,0x00,
    0x28,0x50,0x00,0x00,0x00,0x00,0x00,0x00,
    0x10,0x38,0x6C,0x44,0x44,0x7C,0x00,0x00,
    0x38,0x44,0x40,0x40,0x44,0x38,0x10,0x30,
    0x48,0x00,0x48,0x48,0x48,0x58,0x28,0x00,
    0x0C,0x00,0x38,0x44,0x78,0x40,0x38,0x00,
    0x38,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,
    0x28,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,
    0x30,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,
    0x38,0x28,0x38,0x04,0x3C,0x44,0x3C,0x00,
    0x00,0x38,0x44,0x40,0x44,0x38,0x10,0x30,
    0x38,0x00,0x38,0x44,0x78,0x40,0x38,0x00,
    0x28,0x00,0x38,0x44,0x78,0x40,0x38,0x00,
    0x30,0x00,0x38,0x44,0x78,0x40,0x38,0x00,
    0x28,0x00,0x10,0x10,0x10,0x10,0x18,0x00,
    0x10,0x28,0x00,0x10,0x10,0x10,0x18,0x00,
    0x20,0x00,0x10,0x10,0x10,0x10,0x18,0x00,
    0x28,0x00,0x10,0x28,0x44,0x7C,0x44,0x00,
    0x38,0x28,0x38,0x6C,0x44,0x7C,0x44,0x00,
    0x0C,0x00,0x7C,0x40,0x78,0x40,0x7C,0x00,
    0x00,0x00,0x78,0x14,0x7C,0x50,0x3C,0x00,
    0x3C,0x50,0x50,0x7C,0x50,0x50,0x5C,0x00,
    0x38,0x00,0x30,0x48,0x48,0x48,0x30,0x00,
    0x28,0x00,0x30,0x48,0x48,0x48,0x30,0x00,
    0x60,0x00,0x30,0x48,0x48,0x48,0x30,0x00,
    0x38,0x00,0x48,0x48,0x48,0x58,0x28,0x00,
    0x60,0x00,0x48,0x48,0x48,0x58,0x28,0x00,
    0x28,0x00,0x48,0x48,0x48,0x38,0x10,0x60,
    0x48,0x30,0x48,0x48,0x48,0x48,0x30,0x00,
    0x28,0x00,0x48,0x48,0x48,0x48,0x30,0x00,
    0x00,0x10,0x38,0x40,0x40,0x38,0x10,0x00,
    0x18,0x24,0x20,0x78,0x20,0x24,0x5C,0x00,
    0x44,0x28,0x10,0x7C,0x10,0x7C,0x10,0x00,
    0x60,0x50,0x50,0x68,0x5C,0x48,0x48,0x00,
    0x08,0x14,0x10,0x38,0x10,0x10,0x50,0x20,
    0x18,0x00,0x38,0x04,0x3C,0x44,0x3C,0x00,
    0x18,0x00,0x10,0x10,0x10,0x10,0x18,0x00,
    0x18,0x00,0x30,0x48,0x48,0x48,0x30,0x00,
    0x18,0x00,0x48,0x48,0x48,0x58,0x28,0x00,
    0x28,0x50,0x00,0x70,0x48,0x48,0x48,0x00,
    0x28,0x50,0x00,0x48,0x68,0x58,0x48,0x00,
    0x38,0x04,0x3C,0x44,0x3C,0x00,0x3C,0x00,
    0x30,0x48,0x48,0x48,0x30,0x00,0x78,0x00,
    0x10,0x00,0x10,0x30,0x40,0x44,0x38,0x00,
    0x00,0x00,0x7C,0x40,0x40,0x40,0x00,0x00,
    0x00,0x00,0xFC,0x04,0x04,0x00,0x00,0x00,
    0x40,0x48,0x50,0x38,0x44,0x08,0x1C,0x00,
    0x40,0x48,0x50,0x2C,0x54,0x1C,0x04,0x00,
    0x10,0x00,0x10,0x10,0x38,0x38,0x10,0x00,
    0x00,0x00,0x24,0x48,0x24,0x00,0x00,0x00,
    0x00,0x00,0x48,0x24,0x48,0x00,0x00,0x00,
    0x54,0x00,0xA8,0x00,0x54,0x00,0xA8,0x00,
    0x54,0xA8,0x54,0xA8,0x54,0xA8,0x54,0xA8,
    0xA8,0xFC,0x54,0xFC,0xA8,0xFC,0x54,0xFC,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0xF0,0x10,0x10,0x10,0x10,
    0x10,0xF0,0x10,0xF0,0x10,0x10,0x10,0x10,
    0x50,0x50,0x50,0xD0,0x50,0x50,0x50,0x50,
    0x00,0x00,0x00,0xF0,0x50,0x50,0x50,0x50,
    0x00,0xF0,0x10,0xF0,0x10,0x10,0x10,0x10,
    0x50,0xD0,0x10,0xD0,0x50,0x50,0x50,0x50,
    0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
    0x00,0xF0,0x10,0xD0,0x50,0x50,0x50,0x50,
    0x50,0xD0,0x10,0xF0,0x00,0x00,0x00,0x00,
    0x50,0x50,0x50,0xF0,0x00,0x00,0x00,0x00,
    0x10,0xF0,0x10,0xF0,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x1C,0x00,0x00,0x00,0x00,
    0x10,0x10,0x10,0xFC,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xFC,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x1C,0x10,0x10,0x10,0x10,
    0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0x00,
    0x10,0x10,0x10,0xFC,0x10,0x10,0x10,0x10,
    0x10,0x1C,0x10,0x1C,0x10,0x10,0x10,0x10,
    0x50,0x50,0x50,0x5C,0x50,0x50,0x50,0x50,
    0x50,0x5C,0x40,0x7C,0x00,0x00,0x00,0x00,
    0x00,0x7C,0x40,0x5C,0x50,0x50,0x50,0x50,
    0x50,0xDC,0x00,0xFC,0x00,0x00,0x00,0x00,
    0x00,0xFC,0x00,0xDC,0x50,0x50,0x50,0x50,
    0x50,0x5C,0x40,0x5C,0x50,0x50,0x50,0x50,
    0x00,0xFC,0x00,0xFC,0x00,0x00,0x00,0x00,
    0x50,0xDC,0x00,0xDC,0x50,0x50,0x50,0x50,
    0x10,0xFC,0x00,0xFC,0x00,0x00,0x00,0x00,
    0x50,0x50,0x50,0xFC,0x00,0x00,0x00,0x00,
    0x00,0xFC,0x00,0xFC,0x10,0x10,0x10,0x10,
    0x00,0x00,0x00,0xFC,0x50,0x50,0x50,0x50,
    0x50,0x50,0x50,0x7C,0x00,0x00,0x00,0x00,
    0x10,0x1C,0x10,0x1C,0x00,0x00,0x00,0x00,
    0x00,0x1C,0x10,0x1C,0x10,0x10,0x10,0x10,
    0x00,0x00,0x00,0x7C,0x50,0x50,0x50,0x50,
    0x50,0x50,0x50,0xDC,0x50,0x50,0x50,0x50,
    0x10,0xFC,0x00,0xFC,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0xF0,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x1C,0x10,0x10,0x10,0x10,
    0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
    0x00,0x00,0x00,0x00,0xFC,0xFC,0xFC,0xFC,
    0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,
    0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,
    0xFC,0xFC,0xFC,0xFC,0x00,0x00,0x00,0x00,
    0x00,0x00,0x34,0x48,0x48,0x34,0x00,0x00,
    0x00,0x70,0x48,0x70,0x48,0x48,0x70,0x40,
    0x78,0x48,0x40,0x40,0x40,0x40,0x40,0x00,
    0x00,0x7C,0x28,0x28,0x28,0x28,0x28,0x00,
    0x78,0x48,0x20,0x10,0x20,0x48,0x78,0x00,
    0x00,0x00,0x3C,0x48,0x48,0x30,0x00,0x00,
    0x00,0x00,0x48,0x48,0x48,0x70,0x40,0x40,
    0x00,0x00,0x28,0x50,0x10,0x10,0x10,0x00,
    0x38,0x10,0x38,0x44,0x38,0x10,0x38,0x00,
    0x30,0x48,0x48,0x78,0x48,0x48,0x30,0x00,
    0x00,0x38,0x44,0x44,0x28,0x28,0x6C,0x00,
    0x30,0x40,0x20,0x10,0x38,0x48,0x30,0x00,
    0x00,0x00,0x28,0x54,0x54,0x28,0x00,0x00,
    0x00,0x10,0x38,0x54,0x54,0x38,0x10,0x00,
    0x00,0x38,0x40,0x78,0x40,0x38,0x00,0x00,
    0x00,0x30,0x48,0x48,0x48,0x48,0x00,0x00,
    0x00,0x78,0x00,0x78,0x00,0x78,0x00,0x00,
    0x00,0x10,0x38,0x10,0x00,0x38,0x00,0x00,
    0x40,0x30,0x08,0x30,0x40,0x00,0x78,0x00,
    0x08,0x30,0x40,0x30,0x08,0x00,0x78,0x00,
    0x00,0x08,0x14,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x50,0x20,0x00,
    0x00,0x10,0x00,0x7C,0x00,0x10,0x00,0x00,
    0x00,0x28,0x50,0x00,0x28,0x50,0x00,0x00,
    0x30,0x48,0x48,0x30,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,
    0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,
    0x00,0x1C,0x10,0x10,0x50,0x50,0x20,0x00,
    0x50,0x28,0x28,0x28,0x00,0x00,0x00,0x00,
    0x60,0x10,0x20,0x70,0x00,0x00,0x00,0x00,
    0x00,0x00,0x78,0x78,0x78,0x78,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
#endif