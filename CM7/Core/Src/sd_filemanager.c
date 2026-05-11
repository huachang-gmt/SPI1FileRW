#include "sd_filemanager.h"
#include "sdcard.h"
#include "string.h"
#include "stdio.h"
#include "ff.h"
#include <stdbool.h>

#define SD_TEMP_FILE "SD_TEMP.TMP" 

/* =========================
 * MAX READLINE SCAN LIMIT
 * 防止超大檔案掃描卡死
 * ========================= */
#define SD_MAX_READLINE_SCAN_BYTES   (2UL * 1024UL * 1024UL) // 2MB

/* =========================
 * SD_FILEMANAGER_Init
 * ========================= */

FRESULT SD_FILEMANAGER_Init(void)
{
    return SD_Mount();
}


/* =========================
 * SD_FILEMANAGER_Delete
 * ========================= */
FRESULT SD_FILEMANAGER_Delete(const char *filename)
{
    FRESULT fr;

    FILINFO fno;

    /*
     * SD not mounted
     */
    if(!SD_IsMounted())
        return FR_NOT_READY;

    /*
     * Check file exists
     */
    fr = f_stat(filename, &fno);

    if(fr == FR_NO_FILE)
    {
        /*
         * File not exist
         * 視為成功
         */
        return FR_OK;
    }

    if(fr != FR_OK)
        return fr;

    /*
     * Delete file
     */
    return f_unlink(filename);
}

/* =========================
 * SD_FILEMANAGER_Read
 * ========================= */

FRESULT SD_FILEMANAGER_Read(
    const char *filename,
    void *buffer,
    UINT buffer_size,
    uint64_t offset,
    UINT *read_bytes
)
{
    FIL file;

    FRESULT fr;

    /* =========================
     * parameter check
     * ========================= */
    if(filename == NULL ||
       buffer == NULL ||
       buffer_size == 0 ||
       read_bytes == NULL)
    {
        return FR_INVALID_PARAMETER;
    }

    *read_bytes = 0;

    /* =========================
     * SD mounted check
     * ========================= */
    if(!SD_IsMounted())
        return FR_NOT_READY;

    /* =========================
     * open file
     * ========================= */
    fr = f_open(&file,
                filename,
                FA_READ);

    if(fr != FR_OK)
        return fr;

    /* =========================
     * IMPORTANT:
     * seek to offset
     * ========================= */
    fr = f_lseek(&file,
                 (FSIZE_t)offset);

    if(fr != FR_OK)
    {
        f_close(&file);

        return fr;
    }

    /* =========================
     * read chunk
     * ========================= */
    fr = f_read(&file,
                buffer,
                buffer_size,
                read_bytes);

    if(fr != FR_OK)
    {
        f_close(&file);

        return fr;
    }

    /* =========================
     * close file
     * ========================= */
    f_close(&file);

    return FR_OK;
}

/* =========================
 * SD FILEMANAGER_AppendLine
 * ========================= */ 
FRESULT SD_FILEMANAGER_AppendLine(
    const char *filename,
    const char *text
)
{
    FIL file;
    FRESULT fr;
    UINT bw;
    UINT text_length;

    const char crlf[2] = "\r\n";

    /* =========================
     * parameter check
     * ========================= */
    if(filename == NULL || text == NULL)
        return FR_INVALID_PARAMETER;

    text_length = (UINT)strlen(text);

    if(!SD_IsMounted())
        return FR_NOT_READY;

    /* =========================
     * open file (append mode)
     * ========================= */
    fr = f_open(&file,
                filename,
                FA_OPEN_ALWAYS | FA_WRITE);

    if(fr != FR_OK)
        return fr;

    /* =========================
     * seek to end
     * ========================= */
    fr = f_lseek(&file, f_size(&file));

    if(fr != FR_OK)
    {
        f_close(&file);
        return fr;
    }

    /* =========================
     * write text
     * ========================= */
    fr = f_write(&file,
                 text,
                 text_length,
                 &bw);

    if(fr != FR_OK || bw != text_length)
    {
        f_close(&file);
        return FR_DISK_ERR;
    }

    /* =========================
     * ALWAYS enforce line format
     * (standard log system)
     * ========================= */
    fr = f_write(&file,
                 crlf,
                 2,
                 &bw);

    if(fr != FR_OK || bw != 2)
    {
        f_close(&file);
        return FR_DISK_ERR;
    }

    /* =========================
     * flush
     * ========================= */
    fr = f_sync(&file);

    if(fr != FR_OK)
    {
        f_close(&file);
        return fr;
    }

    f_close(&file);

    return FR_OK;
}

/* =========================
 * SD_FILEMANAGER_GetStorageInfo
 * ========================= */ 
FRESULT SD_FILEMANAGER_GetStorageInfo(
    SD_StorageInfo_t *info
)
{
    FATFS *fs;
    DWORD fre_clust;
    FRESULT fr;

    DWORD total_sectors;
    DWORD free_sectors;

    /* =========================
     * 0. check pointer
     * ========================= */
    if(info == NULL)
        return FR_INVALID_PARAMETER;

    /* =========================
     * 1. check mount
     * ========================= */
    if(!SD_IsMounted())
        return FR_NOT_READY;

    /* =========================
     * 2. get free clusters
     * ========================= */
    fr = f_getfree("", &fre_clust, &fs);

    if(fr != FR_OK)
        return fr;

    /* =========================
     * 3. calculate sectors
     * ========================= */
    total_sectors = (fs->n_fatent - 2) * fs->csize;
    free_sectors  = fre_clust * fs->csize;

    /* =========================
     * 4. convert to bytes (IMPORTANT: 64-bit)
     * ========================= */
    info->total_bytes = (uint64_t)total_sectors * 512ULL;
    info->free_bytes  = (uint64_t)free_sectors  * 512ULL;
    info->used_bytes  = info->total_bytes - info->free_bytes;

    return FR_OK;
}

/* =========================
 * Read specific line
 * ========================= */
FRESULT SD_FILEMANAGER_ReadLine(
    const char *filename,
    char *buffer,
    UINT buffer_size,
    uint64_t line_number
)
{
    FIL file;
    FRESULT fr;
    UINT br;

    /* =========================
     * ⚡ FAST BUFFER MODE
     * ========================= */
    char read_buf[512];   
    UINT index = 0;
    uint64_t current_line = 1;

    /* overflow protection */
    bool overflow = false;

    /* =========================
     * parameter check
     * ========================= */
    if(filename == NULL ||
       buffer == NULL ||
       buffer_size == 0)
    {
        return FR_INVALID_PARAMETER;
    }

    memset(buffer, 0, buffer_size);

    if(!SD_IsMounted())
        return FR_NOT_READY;

    fr = f_open(&file, filename, FA_READ);

    if(fr != FR_OK)
        return fr;

    /* =========================
     * ⚡ BLOCK SCAN MODE
     * ========================= */
    while(1)
    {
        fr = f_read(&file,
                    read_buf,
                    sizeof(read_buf),
                    &br);

        if(fr != FR_OK)
        {
            f_close(&file);
            return fr;
        }

        if(br == 0)
            break;

        /* =========================
         * scan block in RAM
         * ========================= */
        for(UINT i = 0; i < br; i++)
        {
            char ch = read_buf[i];

            if(current_line == line_number)
            {
                if(!overflow)
                {
                    if(index < (buffer_size - 1))
                    {
                        buffer[index++] = ch;
                    }
                    else
                    {
                        /* buffer full → stop writing */
                        overflow = true;
                    }
                }
            }

            if(ch == '\n')
            {
                if(current_line == line_number)
                {
                    buffer[buffer_size - 1] = '\0';
                    f_close(&file);

                    /* signal overflow case */
                    if(overflow)
                        return FR_OK;   // 可改 FR_TRUNCATED（如果想更嚴格）

                    return FR_OK;
                }

                current_line++;
            }
        }
    }

    /* =========================
     * EOF handling
     * ========================= */
    if(current_line == line_number && index > 0)
    {
        buffer[index] = '\0';
        f_close(&file);

        if(overflow)
            return FR_OK;

        return FR_OK;
    }

    f_close(&file);

    return FR_INVALID_PARAMETER;
}

/* =========================
 * SD_FILEMANAGER_DeleteLines
 * ========================= */

 // 支援大檔案
FRESULT SD_FILEMANAGER_DeleteLines(
    const char *filename,
    uint64_t start_line,
    uint64_t line_count,
    char *buffer,
    UINT buffer_size
)
{
    FIL file;
    FRESULT fr;
    UINT bw;

    uint64_t current_line = 1;

    /* =========================
     * chunk buffer
     * ========================= */
    static char chunk_buf[1024];

    UINT chunk_index = 0;

    /* =========================
     * parameter check
     * ========================= */
    if(filename == NULL ||
       buffer == NULL ||
       buffer_size == 0 ||
       start_line == 0 ||
       line_count == 0)
    {
        return FR_INVALID_PARAMETER;
    }

    /* =========================
     * SD mounted check
     * ========================= */
    if(!SD_IsMounted())
    {
        return FR_NOT_READY;
    }

    /* =========================
     * remove old temp file
     * ========================= */
    f_unlink(SD_TEMP_FILE);

    HAL_Delay(5);

    /* =========================
     * create empty temp file
     * ========================= */
    fr = f_open(&file,
                SD_TEMP_FILE,
                FA_CREATE_ALWAYS | FA_WRITE);

    if(fr != FR_OK)
    {
        return fr;
    }

    f_close(&file);

    HAL_Delay(5);

    /* =========================
     * open source file
     * ========================= */
    fr = f_open(&file,
                filename,
                FA_READ);

    if(fr != FR_OK)
    {
        return fr;
    }

    /* =========================
     * line scan
     * ========================= */
    while(f_gets(buffer,
                 buffer_size,
                 &file) != NULL)
    {
        UINT len = strlen(buffer);

        /* =========================
         * keep line ?
         * ========================= */
        if(current_line < start_line ||
           current_line >= (start_line + line_count))
        {
            /* =========================
             * chunk full ?
             * flush to temp file
             * ========================= */
            if((chunk_index + len) >= sizeof(chunk_buf))
            {
                f_close(&file);

                HAL_Delay(2);

                fr = f_open(&file,
                            SD_TEMP_FILE,
                            FA_OPEN_APPEND | FA_WRITE);

                if(fr != FR_OK)
                {
                    return fr;
                }

                fr = f_write(&file,
                             chunk_buf,
                             chunk_index,
                             &bw);

                if(fr != FR_OK ||
                   bw != chunk_index)
                {
                    f_close(&file);

                    return FR_DISK_ERR;
                }

                fr = f_sync(&file);

                if(fr != FR_OK)
                {
                    f_close(&file);

                    return fr;
                }

                f_close(&file);

                HAL_Delay(2);

                /* clear chunk */
                chunk_index = 0;

                /* reopen source */
                fr = f_open(&file,
                            filename,
                            FA_READ);

                if(fr != FR_OK)
                {
                    return fr;
                }

                /* IMPORTANT:
                 * reposition file
                 */
                while(current_line > 1)
                {
                    if(f_gets(buffer,
                              buffer_size,
                              &file) == NULL)
                    {
                        break;
                    }

                    current_line--;
                }
            }

            memcpy(&chunk_buf[chunk_index],
                   buffer,
                   len);

            chunk_index += len;
        }

        current_line++;
    }

    f_close(&file);

    HAL_Delay(5);

    /* =========================
     * flush remaining chunk
     * ========================= */
    if(chunk_index > 0)
    {
        fr = f_open(&file,
                    SD_TEMP_FILE,
                    FA_OPEN_APPEND | FA_WRITE);

        if(fr != FR_OK)
        {
            return fr;
        }

        fr = f_write(&file,
                     chunk_buf,
                     chunk_index,
                     &bw);

        if(fr != FR_OK ||
           bw != chunk_index)
        {
            f_close(&file);

            return FR_DISK_ERR;
        }

        fr = f_sync(&file);

        if(fr != FR_OK)
        {
            f_close(&file);

            return fr;
        }

        f_close(&file);
    }

    HAL_Delay(10);

    /* =========================
     * remove original
     * ========================= */
    fr = f_unlink(filename);

    if(fr != FR_OK &&
       fr != FR_NO_FILE)
    {
        return fr;
    }

    HAL_Delay(10);

    /* =========================
     * rename temp -> original
     * ========================= */
    fr = f_rename(SD_TEMP_FILE,
                  filename);

    if(fr != FR_OK)
    {
        return fr;
    }

    return FR_OK;
}



/* =========================
 * LINE COUNT FUNCTION
 * ========================= */
// 目前檔案總共(剩)有幾行

FRESULT SD_FILEMANAGER_GetLineCount(
    const char *filename,
    uint64_t *line_count,
    char *buffer,
    UINT buffer_size
)
{
    FIL file;
    FRESULT fr;
    UINT br;

    /* ⚡ internal fast buffer */
    char read_buf[512];

    uint64_t count = 0;

    /* =========================
     * parameter check
     * ========================= */
    if(filename == NULL ||
       line_count == NULL)
    {
        return FR_INVALID_PARAMETER;
    }

    *line_count = 0;

    if(!SD_IsMounted())
        return FR_NOT_READY;

    fr = f_open(&file, filename, FA_READ);

    if(fr != FR_OK)
        return fr;

    /* =========================
     * FAST BLOCK SCAN MODE
     * ========================= */
    while(1)
    {
        fr = f_read(&file,
                    read_buf,
                    sizeof(read_buf),
                    &br);

        if(fr != FR_OK)
        {
            f_close(&file);
            return fr;
        }

        if(br == 0)
            break;

        /* =========================
         * count newline only
         * ========================= */
        for(UINT i = 0; i < br; i++)
        {
            if(read_buf[i] == '\n')
            {
                count++;
            }
        }
    }

    f_close(&file);

    *line_count = count;

    return FR_OK;
}


