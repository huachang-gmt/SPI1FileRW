#include "sdcard.h"

/* =========================
 * PRIVATE VARIABLES
 * ========================= */

static FATFS sd_fs;
static FIL   sd_file;

static uint8_t sd_mounted = 0;
static uint8_t sd_opened  = 0;

/* =========================
 * SD_Mount
 * ========================= */

FRESULT SD_Mount(void)
{
    FRESULT fr;

    if(sd_mounted)
        return FR_OK;

    if(disk_initialize(0) != 0)
        return FR_NOT_READY;

    fr = f_mount(&sd_fs, "", 1);

    if(fr == FR_OK)
        sd_mounted = 1;

    return fr;
}

/* =========================
 * SD_Unmount
 * ========================= */

FRESULT SD_Unmount(void)
{
    FRESULT fr;

    if(sd_opened)
        return FR_LOCKED;

    fr = f_mount(NULL, "", 1);

    if(fr == FR_OK)
        sd_mounted = 0;

    return fr;
}

/* =========================
 * SD_Open
 * ========================= */

FRESULT SD_Open(const char *filename)
{
    FRESULT fr;

    if(!sd_mounted)
        return FR_NOT_READY;

    if(sd_opened)
        return FR_LOCKED;

    /*
     * OPEN OR CREATE FILE
     */
    fr = f_open(&sd_file,
                filename,
                FA_OPEN_ALWAYS | FA_WRITE);

    if(fr != FR_OK)
        return fr;

    /*
     * MOVE TO FILE END
     *
     * 非常重要：
     * 後續 write 才會是 append
     */
    fr = f_lseek(&sd_file, f_size(&sd_file));

    if(fr != FR_OK)
    {
        f_close(&sd_file);
        return fr;
    }

    sd_opened = 1;

    return FR_OK;
}

/* =========================
 * SD_Write
 * ========================= */

FRESULT SD_Write(const void *data, UINT length)
{
    UINT bw;
    FRESULT fr;

    if(!sd_opened)
        return FR_INVALID_OBJECT;

    fr = f_write(&sd_file,
                 data,
                 length,
                 &bw);

    if(fr != FR_OK)
        return fr;

    if(bw != length)
        return FR_DISK_ERR;

    /*
     * 強制同步到 SD
     *
     * 避免突然斷電 FAT 損毀
     */
    fr = f_sync(&sd_file);

    return fr;
}

/* =========================
 * SD_Close
 * ========================= */

FRESULT SD_Close(void)
{
    FRESULT fr;

    if(!sd_opened)
        return FR_INVALID_OBJECT;

    /*
     * 最後再 sync 一次
     */
    f_sync(&sd_file);

    fr = f_close(&sd_file);

    if(fr == FR_OK)
        sd_opened = 0;

    return fr;
}

/* =========================
 * SD_IsMounted
 * ========================= */

uint8_t SD_IsMounted(void)
{
    return sd_mounted;
}

/* =========================
 * SD_IsOpened
 * ========================= */

uint8_t SD_IsOpened(void)
{
    return sd_opened;
}
