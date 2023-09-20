/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_lcd.h"
#include "24C02_EEPROM.h"
#include "bsp_key.h"
#include "touch.h"
#include "as608.h"
#include <stdio.h>
#include "RC522.h"
#include <string.h>
#include "8266.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Task_Key */
osThreadId_t Task_KeyHandle;
const osThreadAttr_t Task_Key_attributes = {
  .name = "Task_Key",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Touch */
osThreadId_t Task_TouchHandle;
const osThreadAttr_t Task_Touch_attributes = {
  .name = "Task_Touch",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for Task_Finger */
osThreadId_t Task_FingerHandle;
const osThreadAttr_t Task_Finger_attributes = {
  .name = "Task_Finger",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_IDCard */
osThreadId_t Task_IDCardHandle;
const osThreadAttr_t Task_IDCard_attributes = {
  .name = "Task_IDCard",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Key(void *argument);
void Touch(void *argument);
void Finger(void *argument);
void IDCard(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_Key */
  Task_KeyHandle = osThreadNew(Key, NULL, &Task_Key_attributes);

  /* creation of Task_Touch */
  Task_TouchHandle = osThreadNew(Touch, NULL, &Task_Touch_attributes);

  /* creation of Task_Finger */
  Task_FingerHandle = osThreadNew(Finger, NULL, &Task_Finger_attributes);

  /* creation of Task_IDCard */
  Task_IDCardHandle = osThreadNew(IDCard, NULL, &Task_IDCard_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_Key */
/**
  * @brief  Function implementing the Task_Key thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Key */
void Key(void *argument)
{
  /* USER CODE BEGIN Key */
	KEYS	curKey;
  /* Infinite loop */
  for(;;)
  {
    curKey=ScanKeys(KEY_WAIT_ALWAYS);
		switch(curKey)
		{
			case  KEY0:
				break;
			case	KEY1:
			{
				Touch_Set();
				break;
			}
			case	KEY2:
			{
				Finger_Set();
				break;
			}
			case	KEY3:
			{
				IDCard_Set();
				break;
			}
			// 忘记密码
			case	KEY4:
			{
				Password_Set();
				break;
			}
		}//end switch
		vTaskDelay(300);
  }
  /* USER CODE END Key */
}

/* USER CODE BEGIN Header_Touch */
/**
* @brief Function implementing the Task_Touch thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Touch */
void Touch(void *argument)
{
  /* USER CODE BEGIN Touch */
	
  /* Infinite loop */
  for(;;)
  {
    Touch_Point();
		vTaskDelay(300);
  }
  /* USER CODE END Touch */
}

/* USER CODE BEGIN Header_Finger */
/**
* @brief Function implementing the Task_Finger thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Finger */
void Finger(void *argument)
{
  /* USER CODE BEGIN Finger */
	SearchResult seach;
  uint8_t ensure;
  char str[20];
	// 发送物模型数据
	uint8_t mqttpub[] = "AT+MQTTPUB=0,\"/sys/iwjauHr3P6U/only_print/thing/event/property/post\",\"{params:{\\\"CurrentTemperature\\\":10}}\",0,0\r\n";
  /* Infinite loop */
  for(;;)
  {
		//key_num = KEY_Scan(0);
    ensure = PS_GetImage();
    if(ensure == 0x00) //获取图像成功
    {
      ensure = PS_GenChar(CharBuffer1);
      if(ensure == 0x00) //生成特征成功
      {
        ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);
        if(ensure == 0x00) //搜索成功
        {
					printf("指纹验证成功");
					HAL_UART_Transmit_IT(&ESP8266_UART,mqttpub,sizeof(mqttpub));
					while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
					vTaskDelay(100);
          sprintf(str, " ID:%d 得分:%d ", seach.pageID, seach.mathscore);
          vTaskDelay(500);
        }
        else
        {
					printf("验证失败\r\n");
          vTaskDelay(500);
        }
      }
      else
			{};
			printf("请按手指\r\n");
			
    }
    vTaskDelay(1500);
  }
  /* USER CODE END Finger */
}

/* USER CODE BEGIN Header_IDCard */
/**
* @brief Function implementing the Task_IDCard thread.
* @param argument: Not used
* @retval None
*/

/* USER CODE END Header_IDCard */
void IDCard(void *argument)
{
  /* USER CODE BEGIN IDCard */
  // 初始化射频卡模块
  RC522_Init();
	vTaskDelay(300);
	RC522_Init();
	uint8_t CardType[4] = {0};
	uint8_t i;
	// 定义一个用来存储卡片类型的数组
	uint8_t CARD_ID[10] = {0};
		// 发送物模型数据
	uint8_t mqttpub[] = "AT+MQTTPUB=0,\"/sys/iwjauHr3P6U/only_print/thing/event/property/post\",\"{params:{\\\"CurrentTemperature\\\":20}}\",0,0\r\n";
  /* Infinite loop */
  for(;;)
  {
		
		vTaskDelay(10);
		i = PcdRequest(PICC_REQALL,CardType);
		if(i == MI_OK)
		{
			for(uint8_t j = 0;j<4;j++)
			{
				printf("%X",CardType[j]);
			}
			printf("\r\n");
		}
		i = PcdAnticoll(CardType);
		if(i == MI_OK)
		{
			for(uint8_t j = 0;j<4;j++)
			{
				printf("%X",CardType[j]);
			}
			printf("\r\n");
			//处理卡号数据
			sprintf(CARD_ID,"%x%x%x%x",CardType[0],CardType[1],CardType[2],CardType[3]);
//			// 写入卡ID，暂时使用这个方法存储卡ID
//			EP24C_WriteInOnePage(CARD_ID_ADDR, CardType,1);

			// 读取储存在EEPROM中的卡ID
			EP24C_ReadBytes(CARD_ID_ADDR,&CARD_ID[0],1);
			// 判断刚才接触的卡是否是存储在数组中的卡
			if(CARD_ID[0] == CardType[0])
			{
				printf("卡正确，开锁成功");
				HAL_UART_Transmit_IT(&ESP8266_UART,mqttpub,sizeof(mqttpub));
				while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
				vTaskDelay(100);
			}
			else
			{
				printf("未储存此卡");
			}
		}
		vTaskDelay(200);

  }
  /* USER CODE END IDCard */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

