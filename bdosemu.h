#ifndef BDOSEMU_H
#define BDOSEMU_H
#include <stdint.h>

#include "ff.h"

#ifndef BDOS_DMA
#define BDOS_DMA 0xC
#endif

uint8_t bdos_dma_reset();
void bdos_dma_command(uint8_t data);
void bdos_dma_execute();
void bdos_init(int argc, char *argv[]);

#endif