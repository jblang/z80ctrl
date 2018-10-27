#include "segactrl.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <uart.h>

#define IOX_ADDR 1

// Select registers
#ifdef IOX_ADDR
#define SELECT_IODIR IODIRA0
#define SELECT_GPIO GPIOA0
#define SELECT (1<<7)
#else
#define SELECT_DDR DDRB
#define SELECT_PORT PORTB
#define SELECT (1<<0)
#endif

// Select macros
#ifdef IOX_ADDR
#define SELECT_OUTPUT iox_write(IOX_ADDR, SELECT_IODIR, iox_read(IOX_ADDR, SELECT_IODIR) & ~SELECT);
#define SELECT_HI iox_write(IOX_ADDR, SELECT_GPIO, iox_read(IOX_ADDR, SELECT_GPIO) | SELECT)
#define SELECT_LO iox_write(IOX_ADDR, SELECT_GPIO, iox_read(IOX_ADDR, SELECT_GPIO) & ~SELECT)
#else
#define SELECT_OUTPUT SELECT_DDR |= SELECT;
#define SELECT_HI SELECT_PORT |= SELECT
#define SELECT_LO SELECT_PORT &= ~SELECT
#endif

// Button registers
#ifdef IOX_ADDR
#define BUTTONS_IODIR IODIRA0
#define BUTTONS_GPIO GPIOA0
#define BUTTONS_GPPU GPPUA0
#define BUTTONS_SHIFT 0
#else
#define BUTTONS_DDR DDRD
#define BUTTONS_PIN PIND
#define BUTTONS_PORT PORTD
#define BUTTONS_SHIFT 2
#endif

// Bit positions for button signals
#define UP (1<<0)
#define DOWN (1<<1)
#define LEFT (1<<2)
#define RIGHT (1<<3)
#define FIRE (1<<4)
#define ALT (1<<5)

// Alternate button names
#define A FIRE
#define B FIRE
#define C ALT
#define START ALT
#define X LEFT
#define Y DOWN
#define Z UP
#define MODE RIGHT

// Signal combinations
#define ALL (UP|DOWN|LEFT|RIGHT|FIRE|ALT)
#define GENESIS3 (LEFT|RIGHT)
#define GENESIS6 (UP|DOWN)

// Button macros
#ifdef IOX_ADDR
#define BUTTONS_INPUT iox_write(IOX_ADDR, BUTTONS_IODIR, iox_read(IOX_ADDR, BUTTONS_IODIR) | (ALL<<BUTTONS_SHIFT)); 
#define BUTTONS_PULLUP iox_write(IOX_ADDR, BUTTONS_GPPU, iox_read(IOX_ADDR, BUTTONS_GPPU) | (ALL<<BUTTONS_SHIFT)); 
#define GET_BUTTONS (iox_read(IOX_ADDR, BUTTONS_GPIO)>>BUTTONS_SHIFT)
#else
#define BUTTONS_INPUT BUTTONS_DDR &= ~(ALL<<BUTTONS_SHIFT)
#define BUTTONS_PULLUP BUTTONS_PORT |= (ALL<<BUTTONS_SHIFT)
#define GET_BUTTONS (BUTTONS_PIN>>BUTTONS_SHIFT)
#endif

// initialize controller interface
void sega_init() {
#ifdef IOX_ADDR
  iox_init();
#endif
  BUTTONS_INPUT;
  BUTTONS_PULLUP;
  SELECT_OUTPUT;
  SELECT_HI;
  iox_write(IOX_ADDR, IODIRB0, 0x00);
  iox_write(IOX_ADDR, GPIOB0, 0xFF);
  _delay_ms(1000);
  iox_write(IOX_ADDR, GPIOB0, 0x00);
}

// read controller, return individual buttons in structure
button_t sega_read()
{
    // Sega 6-button protocol description
    // https://applause.elfmimi.jp/md6bpad-e.html

  button_t buttons;

  cli();
  for (uint8_t i = 0; i < 8; i++) {
    if (i & 1)
      SELECT_HI;
    else
      SELECT_LO;
    _delay_us(10);
    uint8_t raw = GET_BUTTONS;
    switch(i) {
      case 2:
        buttons.genesis3 = !(raw & GENESIS3);
        if (buttons.genesis3) {
          buttons.a = !(raw & A);
          buttons.start = !(raw & START);
        }
        break;
      case 3:
        buttons.up = !(raw & UP);
        buttons.down = !(raw & DOWN);
        buttons.left = !(raw & LEFT);
        buttons.right = !(raw & RIGHT);
        buttons.b = !(raw & B);
        buttons.c = !(raw & C);
        break;
      case 4:
        buttons.genesis6 = !(raw & GENESIS6);
        break;
      case 5:
        if (buttons.genesis6) {
          buttons.x = !(raw & X);
          buttons.y = !(raw & Y);
          buttons.z = !(raw & Z);
          buttons.mode = !(raw & MODE);
        }
        break;
    }
  }
  sei();
  return buttons;
}

// whether current mode is keypad or joystick
uint8_t coleco_currmode = COLECO_JOYSTICK;

// set current mode for coleco controller
void coleco_mode(uint8_t mode)
{
  coleco_currmode = mode;
}

#define COLECO_UP 0
#define COLECO_RIGHT 1
#define COLECO_DOWN 2
#define COLECO_LEFT 3
#define COLECO_FIRE 6

// keypad button to dpad translations
const uint8_t coleco_keypad[] = {
  /* 0 */ (1 << COLECO_UP) | (1 << COLECO_DOWN),
  /* 1 */ (1 << COLECO_RIGHT),
  /* 2 */ (1 << COLECO_LEFT),
  /* 3 */ (1 << COLECO_UP) | (1 << COLECO_RIGHT),
  /* 4 */ (1 << COLECO_LEFT) | (1 << COLECO_DOWN) | (1 << COLECO_UP),
  /* 5 */ (1 << COLECO_DOWN) | (1 << COLECO_LEFT),
  /* 6 */ (1 << COLECO_UP),
  /* 7 */ (1 << COLECO_RIGHT) | (1 << COLECO_LEFT),
  /* 8 */ (1 << COLECO_RIGHT) | (1 << COLECO_LEFT) | (1 << COLECO_DOWN),
  /* 9 */ (1 << COLECO_DOWN)
};

#define COLECO_STAR ((1 << COLECO_DOWN) | (1 << COLECO_RIGHT))
#define COLECO_POUND  ((1 << COLECO_UP) | (1 << COLECO_LEFT))

// read controller and translate into coleco port values
uint8_t coleco_read(void) 
{
  button_t buttons = sega_read();
  if (coleco_currmode == COLECO_JOYSTICK)
    return 
      ~((buttons.up << COLECO_UP) |
      (buttons.right << COLECO_RIGHT) |
      (buttons.down << COLECO_DOWN) |
      (buttons.left << COLECO_LEFT) |
      (buttons.b << COLECO_FIRE)) & 0x4F;
  else {
      uint8_t code = 0;
      if (uart_testrx(0)) {
        uint8_t chr = uart_getc(0);
        if (chr >= '0' && chr <= '9')
          code = coleco_keypad[chr - '0'];
        else if (chr == '*')
          code = COLECO_STAR;
        else if (chr == '#')
          code = COLECO_POUND;
      } else {
        if (buttons.x)
          code = coleco_keypad[1];
      }
      return ~(code | (buttons.a << COLECO_FIRE)) & 0x4F;
  }
}