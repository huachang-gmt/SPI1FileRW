/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs.h"
#include "string.h"
#include "sd_filemanager.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* DUAL_CORE_BOOT_SYNC_SEQUENCE: Define for dual core boot synchronization    */
/*                             demonstration code based on hardware semaphore */
/* This define is present in both CM7/CM4 projects                            */
/* To comment when developping/debugging on a single core                     */
//#define DUAL_CORE_BOOT_SYNC_SEQUENCE

#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif
#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

/* USER CODE BEGIN PV */
char read_buffer[512];
FRESULT fr;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */
#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
  int32_t timeout;
#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */
/* USER CODE END Boot_Mode_Sequence_0 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
  /* Wait until CPU2 boots and enters in stop mode or timeout*/
  timeout = 0xFFFF;
  while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0));
  if ( timeout < 0 )
  {
  Error_Handler();
  }
#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */
#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
/* When system initialization is finished, Cortex-M7 will release Cortex-M4 by means of
HSEM notification */
/*HW semaphore Clock enable*/
__HAL_RCC_HSEM_CLK_ENABLE();
/*Take HSEM */
HAL_HSEM_FastTake(HSEM_ID_0);
/*Release HSEM in order to notify the CPU2(CM4)*/
HAL_HSEM_Release(HSEM_ID_0,0);
/* wait until CPU2 wakes up from stop mode */
timeout = 0xFFFF;
while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
if ( timeout < 0 )
{
Error_Handler();
}
#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */
  // LED1， LED2， LED3 初始化 
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_YELLOW);
  BSP_LED_Init(LED_RED);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

  HAL_Delay(3000);

  MX_GPIO_Init();

  BSP_LED_On(LED_GREEN);
  HAL_Delay(1000);
  BSP_LED_Off(LED_GREEN); // GPIO 初始化通過

  MX_SPI1_Init();

  BSP_LED_On(LED_YELLOW);
  HAL_Delay(1000);
  BSP_LED_Off(LED_YELLOW);// SPI 1 初始化通過

  MX_FATFS_Init();

  BSP_LED_On(LED_RED);
  HAL_Delay(1000);
  BSP_LED_Off(LED_RED);// FATFS 初始化通過

  // 以上的初始化關鍵點是 檔案： diskio.c 檔案，透過 SPI 根據 SD Protocols 與 SD 卡進行通訊，實現對 SD 卡的讀寫操作，並且提供給 FATFS 使用。 檔案路徑： SPI1FileRW\CM7\Middlewares\Third_Party\FatFs\diskio.c，這個檔案只能透過 STM32CubeIDE 開啟，無法直接在檔案總管中開啟編輯，裡面有一個重要的函式 disk_initialize()，這個函式會呼叫 SD_Init() 來初始化 SD 卡，確保 SD 卡已經準備好進行後續的讀寫操作。

  /* USER CODE BEGIN 2 */

  /*  以下是檔案測試 流程 可以當作 API 範例 */


  
  // =================== 測試範本第二類 開始 ===================
  fr = SD_FILEMANAGER_Init();

  if(fr != FR_OK)
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  //寫入檔案之前，先刪除舊檔案（如果存在的話），確保測試從乾淨的狀態開始
  // 注意：如果 SD 卡裡沒有 "SPIFile.txt" 這個檔案，SD_FILEMANAGER_Delete() 會回傳 FR_NO_FILE，但這對我們來說不是錯誤，因為我們的目標是確保 "SPIFile.txt" 不存在，以便後續測試能夠順利進行。
  fr = SD_FILEMANAGER_Delete("SPIFile.txt");

  if(fr != FR_OK &&
    fr != FR_NO_FILE)
  {
      BSP_LED_On(LED_RED); // FR_NO_FILE 也算成功

      while(1);
  }

  BSP_LED_On(LED_GREEN);
  HAL_Delay(1000);
  BSP_LED_Off(LED_GREEN);
 
  // 建立測試資料  
  fr = SD_FILEMANAGER_AppendLine("SPIFile.txt", "LINE 1");
  if(fr != FR_OK) 
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  fr = SD_FILEMANAGER_AppendLine("SPIFile.txt", "LINE 2");
  if(fr != FR_OK) 
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  fr = SD_FILEMANAGER_AppendLine("SPIFile.txt", "LINE 3");
  if(fr != FR_OK) 
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  fr = SD_FILEMANAGER_AppendLine("SPIFile.txt", "LINE 4");
  if(fr != FR_OK) 
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  fr = SD_FILEMANAGER_AppendLine("SPIFile.txt", "LINE 5");
  if(fr != FR_OK) 
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  BSP_LED_On(LED_YELLOW);
  HAL_Delay(1000);
  BSP_LED_Off(LED_YELLOW);


  // 刪除： LINE 2  LINE 3  
  char tmp_buf[128];

  fr = SD_FILEMANAGER_DeleteLines(
          "SPIFile.txt",
          2,
          2,
          tmp_buf,
          sizeof(tmp_buf)
  );
  
  if(fr != FR_OK)
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  BSP_LED_On(LED_GREEN);
  HAL_Delay(1000);
  BSP_LED_Off(LED_GREEN);

  
  char linebuf[128];

  fr = SD_FILEMANAGER_ReadLine(
          "SPIFile.txt",
          linebuf,
          sizeof(linebuf),
          2
      );

  if(fr != FR_OK)
  {
      BSP_LED_On(LED_RED);

      while(1);
  }

  
  // 驗證： 第 2 行應該是 LINE 4  
  if(strncmp(linebuf,
            "LINE 4",
            strlen("LINE 4")) == 0)
  {
      // PASS
      BSP_LED_On(LED_YELLOW);
      BSP_LED_On(LED_GREEN);

      HAL_Delay(3000);

      BSP_LED_Off(LED_GREEN);
      BSP_LED_Off(LED_YELLOW);

  }
  else
  {
      // FAIL      
      BSP_LED_On(LED_RED);
      BSP_LED_On(LED_YELLOW);

      while(1);
  }


uint64_t line_count = 0;
// 目前檔案總共(剩)有幾行
fr = SD_FILEMANAGER_GetLineCount(
        "SPIFile.txt",
        &line_count,
        tmp_buf,
        sizeof(tmp_buf)
);

if(fr != FR_OK)
{
    BSP_LED_On(LED_RED);
    while(1);
}

  
  // LED 驗證結果
  // 預期：
  // LINE 1
  // LINE 4
  // LINE 5
  // => 共 3 行（因為你刪了 2~3）

  if(line_count == 3)
  {
      // PASS
      BSP_LED_On(LED_GREEN);
      HAL_Delay(3000);
      BSP_LED_Off(LED_GREEN);
  }
  else
  {
      // FAIL
      BSP_LED_On(LED_RED);
      BSP_LED_On(LED_YELLOW);
      while(1);
  }

  // =================== 測試範本第二類 結束 ===================
// 測試驗證 OK    2026-05-11
















  
/*
// 驗證通過 2026-05-11
  // =================== 測試範本第一類 開始 ===================
  // 1. 初始化 SD 卡檔案管理器
  // SD 卡檔案管理器初始化
  fr = SD_FILEMANAGER_Init();

  if(fr != FR_OK &&
     fr != FR_NO_FILE)
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  
  //寫入檔案之前，先刪除舊檔案（如果存在的話），確保測試從乾淨的狀態開始
  // 注意：如果 SD 卡裡沒有 "SPIFile.txt" 這個檔案，SD_FILEMANAGER_Delete() 會回傳 FR_NO_FILE，但這對我們來說不是錯誤，因為我們的目標是確保 "SPIFile.txt" 不存在，以便後續測試能夠順利進行。
  
  fr = SD_FILEMANAGER_Delete("SPIFile.txt");

  if(fr != FR_OK &&
     fr != FR_NO_FILE)
  {
      BSP_LED_On(LED_RED);

      while(1);
  }

  BSP_LED_On(LED_GREEN);
  HAL_Delay(1000);
  BSP_LED_Off(LED_GREEN);


  // 寫入測試（Log = 自動 CRLF）
  fr = SD_FILEMANAGER_AppendLine(
      "SPIFile.txt",
      "STM32H755 SPI SD Test"
  );

  if(fr != FR_OK)
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  fr = SD_FILEMANAGER_AppendLine(
      "SPIFile.txt",
      "Write OK"
  );

  if(fr != FR_OK)
  {
      BSP_LED_On(LED_RED);
      while(1);
  }

  BSP_LED_On(LED_YELLOW);
  HAL_Delay(1000);
  BSP_LED_Off(LED_YELLOW);


  // 追加寫入檔案測試
  SD_FILEMANAGER_AppendLine(
      "SPIFile.txt",
      "<111111> System Boot OK"
  );

  SD_FILEMANAGER_AppendLine(
      "SPIFile.txt",
      "<222222> SPI Init OK"
  );

  SD_FILEMANAGER_AppendLine(
      "SPIFile.txt",
      "<333333> Sensor Ready"
  );

  BSP_LED_On(LED_GREEN);
  BSP_LED_On(LED_YELLOW);
  HAL_Delay(1000);
  BSP_LED_Off(LED_GREEN);
  BSP_LED_Off(LED_YELLOW);


  // 讀取檔案測試
  char read_buffer[256];
  UINT read_bytes = 0;

  fr = SD_FILEMANAGER_Read(
      "SPIFile.txt",
      read_buffer,
      sizeof(read_buffer),
      0,              // offset
      &read_bytes
  );

  if(fr != FR_OK)
  {
      BSP_LED_On(LED_RED);
      while(1);
  }
  
  //  read OK
  BSP_LED_On(LED_YELLOW);
  HAL_Delay(1000);
  BSP_LED_Off(LED_YELLOW);


  SD_StorageInfo_t info;

  fr = SD_FILEMANAGER_GetStorageInfo(&info);

  if(fr != FR_OK)
  {
      BSP_LED_On(LED_RED);
      while(1);
  }


  BSP_LED_On(LED_RED);
  BSP_LED_On(LED_YELLOW);
  HAL_Delay(1000);
  BSP_LED_Off(LED_RED);
  BSP_LED_Off(LED_YELLOW);

  // 
  //printf("Total: %lu KB\r\n", info.total_kb); // 總容量
  //printf("Used : %lu KB\r\n", info.used_kb); // 已使用容量
  //printf("Free : %lu KB\r\n", info.free_kb); // 可用容量
  
  char linebuf[128];

  fr = SD_FILEMANAGER_ReadLine(
          "SPIFile.txt",
          linebuf,
          sizeof(linebuf),
          3
      );

  if(fr != FR_OK)
  {
      BSP_LED_On(LED_RED);

      while(1);
  }

 
  if(strncmp(linebuf,
            "<111111> System Boot OK",
            strlen("<111111> System Boot OK")) == 0)
  {
      // read line OK
      BSP_LED_On(LED_GREEN);
      BSP_LED_On(LED_YELLOW);
      HAL_Delay(3000);
      BSP_LED_Off(LED_YELLOW);
      BSP_LED_Off(LED_GREEN);
  }
  else
  {
      // read line fail
      BSP_LED_On(LED_RED);

      BSP_LED_On(LED_YELLOW);

      while(1);
  }


  // 刪除檔案測試
  fr = SD_FILEMANAGER_Delete("SPIFile.txt");
  if(fr != FR_OK)
  {
      BSP_LED_On(LED_RED);
      while(1);
  }
  // delete OK 
  BSP_LED_On(LED_RED);
  HAL_Delay(3000);
  BSP_LED_Off(LED_RED);

  // =================== 測試範本第一類 結束 ===================
*/










  /* USER CODE END 2 */


  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 9;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq(); // 1. 確保中斷被關閉（防止其他幹擾）

  // 初始化三顆 LED
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_YELLOW);
  BSP_LED_Init(LED_RED);
      // 如果把 SD 卡 拔掉，再 按下 開發板上的 Reset 按鍵，會跑到這裡，開發板的 LED1 綠，LED2 黃，LED3 紅 會一起閃爍。
      // 若再把 SD 卡 插入，再 按下 開發板上的 Reset 按鍵，就又會恢復正常運作： 寫入，讀取，比對 燈號變化
  while (1)
  {
    // 三燈同步翻轉
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // LED1 綠
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_1);  // LED2 黃
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // LED3 紅

    // 軟體延遲 (約 200ms，閃爍速度較快以示警告)
    for (uint32_t i = 0; i < 2000000; i++) {
        __NOP();
    }
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
