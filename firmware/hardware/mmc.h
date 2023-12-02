/*-----------------------------------------------------------------------*/
/** @file mmc.h Low level disk interface modlue include file        */
/*   (C)ChaN, 2016
/-----------------------------------------------------------------------*/

#ifndef HARDWARE_MMC_H
#define HARDWARE_MMC_H

#include "fatfs/diskio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------*/
/* Prototypes for disk control functions */

DSTATUS mmc_disk_initialize(void);
DSTATUS mmc_disk_status(void);
DRESULT mmc_disk_read(BYTE* buff, DWORD sector, UINT count);
DRESULT mmc_disk_write(const BYTE* buff, DWORD sector, UINT count);
DRESULT mmc_disk_ioctl(BYTE cmd, void* buff);
void mmc_disk_timerproc(void);

#ifdef __cplusplus
}
#endif

#endif
