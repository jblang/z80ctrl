#ifndef MSXKEY_H
#define MSXKEY_H

#define MSX_KEY_COL MSX_KEY_BASE
#define MSX_KEY_ROW MSX_KEY_BASE+1

uint8_t msx_scanrow(void);
uint8_t msx_setrow(uint8_t row);

#endif