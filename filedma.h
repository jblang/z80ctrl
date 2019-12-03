#ifndef FILEDMA_H
#define FILEDMA_H

#include "ff.h"

#ifndef FILE_DMA
#define FILE_DMA 0xB
#endif

uint8_t file_dma_reset();
void file_dma_command(uint8_t data);
void file_dma_execute();

#endif