#ifndef __SD_FILEMANAGER_H
#define __SD_FILEMANAGER_H

#include "fatfs.h"

/* =========================
 * FILE MANAGER API
 * ========================= */

/*
 * Initialize File Manager
 */
FRESULT SD_FILEMANAGER_Init(void);

/*
 * Append Line to File
 */
FRESULT SD_FILEMANAGER_AppendLine(
    const char *filename,
    const char *text
);

/*
 * Delete File
 */
FRESULT SD_FILEMANAGER_Delete(const char *filename);

/* 
 * READ FILE
 */ 
FRESULT SD_FILEMANAGER_Read(
    const char *filename,
    void *buffer,
    UINT buffer_size,
    uint64_t offset,
    UINT *read_bytes
);

/* 
 * STORAGE INFO
 */

typedef struct
{
    uint64_t total_bytes;
    uint64_t used_bytes;
    uint64_t free_bytes;
} SD_StorageInfo_t;

FRESULT SD_FILEMANAGER_GetStorageInfo(
    SD_StorageInfo_t *info
);

/* 
 * READ FILE LINE BY LINE   讀取指定 line 的內容到 buffer 中，buffer_size 是 buffer 的大小，確保足夠存放該行內容
  - filename: 要讀取的檔案名稱
  - buffer: 用於存放讀取到的行內容的緩衝區
  - buffer_size: 緩衝區的大小，確保足夠存放該行內容
 *  - line_number 從 1 開始計數
 */
FRESULT SD_FILEMANAGER_ReadLine(
    const char *filename,
    char *buffer,
    UINT buffer_size,
    uint64_t line_number
);

/* 
 * DELETE FILE LINE BY LINE  刪除指定行，line_count 是要刪除的行數，從 start_line 開始連續刪除 line_count 行
  - filename: 要操作的檔案名稱
  - start_line: 從哪一行開始刪除，從 1 開始計數
  - line_count: 要刪除的行數，從 start_line 開始連續刪除 line_count 行
 */
FRESULT SD_FILEMANAGER_DeleteLines(
    const char *filename,
    uint64_t start_line,
    uint64_t line_count,
    char *buffer,
    UINT buffer_size
);


/* =========================
 * NEW: LINE COUNT
 * ========================= */
FRESULT SD_FILEMANAGER_GetLineCount(
    const char *filename,
    uint64_t *line_count,
    char *buffer,
    UINT buffer_size
);


#endif