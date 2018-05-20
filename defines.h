#ifndef DEFINES_H
#define DEFINES_H

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#ifndef BAUD
#define BAUD  115200
#endif

// Enable SD card support in fatfs
#define DRV_MMC 0

// Number of emulated disk drives
#define NUMDRIVES 26

#endif
