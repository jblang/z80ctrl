#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include <avr/io.h>
#include "iox.h"

// Button return type
typedef struct button_t {
  uint8_t genesis3 : 1;
  uint8_t genesis6 : 1;
  uint8_t up : 1;
  uint8_t down : 1;
  uint8_t left : 1;
  uint8_t right : 1;
  uint8_t a : 1;
  uint8_t b : 1;
  uint8_t c : 1;
  uint8_t x : 1;
  uint8_t y : 1;
  uint8_t z : 1;
  uint8_t start : 1;
  uint8_t mode : 1;
} button_t;

void sega_init(void);
button_t sega_read(void);

#define COLECO_JOYPORT1 0xFC
#define COLECO_JOYPORT2 0xFF

#define COLECO_KEYSEL 0x80
#define COLECO_JOYSEL 0xC0

#define COLECO_KEYPAD 0
#define COLECO_JOYSTICK 1

void coleco_mode(uint8_t mode);
uint8_t coleco_read(void);

#endif
