#ifndef DISKEMU_H
#define DISKEMU_H

#include <stdint.h>

#define DRIVE_STATUS 0x8
#define DRIVE_CONTROL 0x9
#define DRIVE_DATA 0xA

void drive_unmount(uint8_t drv);
void drive_mount(uint8_t drv, char *filename);
void drive_select(uint8_t newdrv);
uint8_t drive_status();
void drive_control(uint8_t cmd);
uint8_t drive_sector(void);
void drive_write(uint8_t data);
uint8_t drive_read(void);

#endif