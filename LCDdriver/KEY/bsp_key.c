#include "bsp_key.h"
#include "lcd.h"
#include "touch.h"
// 轮询4个按键，返回按键值
KEYS ScanKeys(uint32_t timeout)
{
	KEYS key = KEY0;
	// 按下按键1
	if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
		{
		key = KEY1;
		}
	}
	else if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
	{
		
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
		{
		key = KEY2;
		}
	}
	else if((HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == GPIO_PIN_RESET))
	{
		HAL_Delay(10);
		if((HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == GPIO_PIN_RESET))
		{
		key = KEY3;
		}
	}
	else if((HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == GPIO_PIN_SET))
	{
		HAL_Delay(10);
		if((HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == GPIO_PIN_SET))
		{
		key = KEY4;
		}
	}

	return key;
}

