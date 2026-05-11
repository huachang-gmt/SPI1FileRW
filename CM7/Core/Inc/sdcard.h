#ifndef __SDCARD_H
#define __SDCARD_H

#include "fatfs.h"
#include "main.h"

/* =========================
 * SD CARD API
 * ========================= */

/*
 * SD Card Mount
 */
FRESULT SD_Mount(void);

/*
 * SD Card Unmount
 */
FRESULT SD_Unmount(void);

/*
 * Open File
 */
FRESULT SD_Open(const char *filename);

/*
 * Write Data
 */
FRESULT SD_Write(const void *data, UINT length);

/*
 * Close File
 */
FRESULT SD_Close(void);

/*
 * Status
 */
uint8_t SD_IsMounted(void);

uint8_t SD_IsOpened(void);

#endif