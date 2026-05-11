#include "fatfs.h"

/* USER CODE BEGIN Variables */
FATFS USERFatFS;
FIL USERFile;
/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* 不使用 STM32 LinkDriver（避免和自寫 diskio 衝突） */

    /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @retval Time in DWORD
  */
__weak DWORD get_fattime(void)
{
    return 0;
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */