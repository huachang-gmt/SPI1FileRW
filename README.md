# SD Card API 使用手冊 (SPI 版本)

## 概述
本文件基於 **FATFS** 的 SD 卡檔案管理 API，適用於 **STM32 SPI SD Card** 專案。

### 主要功能：
*   SD 卡初始化與掛載
*   檔案讀取、刪除
*   文字行追加 (Append)
*   指定行讀取與刪除
*   檔案行數統計
*   SD 卡容量資訊取得

### 模組依賴：
*   FATFS (`ff.h`)
*   SD Driver (`sdcard.h`)
*   STM32 HAL 庫

---

### 硬體說明：

**MCU**
*   STM32H755ZIT6

**開發板**
*   NUCLEO-H755ZI-Q

**軟體開發工具**
*   STM32CubeIDE
*   STM32CubeMX
*   STM32CubeProgrammer

**SPI 與 SD 卡模組 接線分佈說明**

| SPI 腳位 | MCU 腳位 |
| :--- | :--- |
| `CS` | PA4 |
| `SCK` | PA5 |
| `MISO` | PA6 |
| `MOSI` | PA7 |
| `VCC` | 3.3V |
| `GND` | GND |

---

## API Reference

### 1. SD_FILEMANAGER_Init
**原型**
```c
FRESULT SD_FILEMANAGER_Init(void);
```
**功能**
初始化並掛載 SD 卡檔案系統。

| 回傳值 | 說明 |
| :--- | :--- |
| `FR_OK` | 初始化成功 |
| `FR_NOT_READY` | SD 卡不存在或初始化失敗 |
| 其他 `FRESULT` | FATFS 相關錯誤碼 |

**範例**
```c
FRESULT fr = SD_FILEMANAGER_Init();
if(fr != FR_OK) {
    BSP_LED_On(LED_RED);
}
```

---

### 2. SD_FILEMANAGER_Delete
**原型**
```c
FRESULT SD_FILEMANAGER_Delete(const char *filename);
```
**功能**
刪除指定檔案。若檔案不存在，仍視為成功。

| 參數 | 說明 |
| :--- | :--- |
| `filename` | 欲刪除的檔案名稱 |

| 回傳值 | 說明 |
| :--- | :--- |
| `FR_OK` | 成功 |
| `FR_NOT_READY` | SD 未掛載 |
| `FR_INVALID_NAME` | 檔名錯誤 |

**範例**
```c
SD_FILEMANAGER_Delete("log.txt");
```

---

### 3. SD_FILEMANAGER_Read
**原型**
```c
FRESULT SD_FILEMANAGER_Read(const char *filename, void *buffer, UINT buffer_size, uint64_t offset, UINT *read_bytes);
```
**功能**
從指定 `offset` 位置讀取檔案資料，支援大檔案讀取。

| 參數 | 說明 |
| :--- | :--- |
| `filename` | 檔案名稱 |
| `buffer` | 資料接收緩衝區 |
| `buffer_size` | 緩衝區大小 |
| `offset` | 讀取起始位置 |
| `read_bytes` | 實際讀取位元組數輸出 |

**範例**
```c
char buf[128];
UINT br;
SD_FILEMANAGER_Read("test.txt", buf, sizeof(buf), 0, &br);
```

---

### 4. SD_FILEMANAGER_AppendLine
**原型**
```c
FRESULT SD_FILEMANAGER_AppendLine(const char *filename, const char *text);
```
**功能**
在檔案尾端追加一行文字。系統會自動在末端加入 `\r\n`。

| 參數 | 說明 |
| :--- | :--- |
| `filename` | 檔案名稱 |
| `text` | 欲追加的文字內容 |

**範例**
```c
SD_FILEMANAGER_AppendLine("log.txt", "System Boot OK");
```

---

### 5. SD_FILEMANAGER_GetStorageInfo
**原型**
```c
FRESULT SD_FILEMANAGER_GetStorageInfo(SD_StorageInfo_t *info);
```
**功能**
取得 SD 卡容量資訊（總量、已用、剩餘）。

**結構定義**
```c
typedef struct {
    uint64_t total_bytes;
    uint64_t used_bytes;
    uint64_t free_bytes;
} SD_StorageInfo_t;
```

**範例**
```c
SD_StorageInfo_t info;
SD_FILEMANAGER_GetStorageInfo(&info);
```

---

### 6. SD_FILEMANAGER_ReadLine
**原型**
```c
FRESULT SD_FILEMANAGER_ReadLine(const char *filename, char *buffer, UINT buffer_size, uint64_t line_number);
```
**功能**
讀取指定行號的內容（行號從 1 開始統計）。

| 參數 | 說明 |
| :--- | :--- |
| `line_number` | 欲讀取的行號 |

**範例**
```c
char line[128];
SD_FILEMANAGER_ReadLine("log.txt", line, sizeof(line), 3); // 讀取第 3 行
```

---

### 7. SD_FILEMANAGER_DeleteLines
**原型**
```c
FRESULT SD_FILEMANAGER_DeleteLines(const char *filename, uint64_t start_line, uint64_t line_count, char *buffer, UINT buffer_size);
```
**功能**
刪除指定範圍的行數。

**範例**
```c
char tmp[128];
SD_FILEMANAGER_DeleteLines("log.txt", 2, 3, tmp, sizeof(tmp)); // 刪除第 2~4 行
```

---

### 8. SD_FILEMANAGER_GetLineCount
**原型**
```c
FRESULT SD_FILEMANAGER_GetLineCount(const char *filename, uint64_t *line_count, char *buffer, UINT buffer_size);
```
**功能**
統計檔案總行數。

---

## 設計特性

1.  **大檔案支援**：採用 Block Scan 與 Chunk Buffer 機制，避免一次性將全檔載入 RAM造成 Overflow。
2.  **64-bit 支援**：Offset 與容量統計均使用 `uint64_t`，支援超過 4GB 的邏輯容量。
3.  **CRLF 標準化**：所有 Append 功能自動補齊 `\r\n`，確保 Windows 與多數編輯器相容性。
4.  **安全 Replace 機制**：`DeleteLines` 採用暫存檔 (Temp File) 轉換機制，降低因斷電導致原始檔案損毀的風險。

---

## 建議與注意事項

### 建議 Buffer 大小
| API | 建議值 |
| :--- | :--- |
| `ReadLine` | 128 ~ 512 bytes |
| `DeleteLines` | 128 ~ 512 bytes |
| `Read` (Data) | 512 ~ 4096 bytes |

### 注意事項
*   **必須先掛載**：所有 API 調用前必須先執行 `SD_FILEMANAGER_Init()`。
*   **執行緒安全**：目前版本非 Thread-Safe。若在 RTOS 多任務環境下使用，請自行加入 **Mutex** 保護。
*   **STM32H7 效能建議**：
    *   開啟 SPI DMA。
    *   啟用 FATFS Fast Seek。
    *   注意 Cache Coherency (Clean/Invalidate)。

---
*文件結束*
```