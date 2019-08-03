#ifndef FILEDMA_H
#define FILEDMA_H

#include "ff.h"

#ifndef FILE_DMA
#define FILE_DMA 0xB
#endif

FRESULT file_to_mem(FIL *fp, uint16_t base, UINT btr, UINT *br);
FRESULT mem_to_file(FIL *fp, uint16_t base, UINT btw, UINT *bw);

uint8_t file_dma_reset();
void file_dma_command(uint8_t data);
void file_dma_execute();
#endif