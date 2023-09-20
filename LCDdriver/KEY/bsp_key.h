#ifndef __BSP_KEY_H
#define __BSP_KEY_H	

#include "main.h"
// 按键的枚举变量
typedef enum
{
			KEY0 = 0,   // 没有按键按下
			KEY1,
			KEY2,
			KEY3,	
			KEY4,
} KEYS;

#define KEY_WAIT_ALWAYS   0  // 作为ScanKeys的输入参数，表示一直等待按键输入
KEYS ScanKeys(uint32_t timeout);






#endif /* __BSP_KEY_H */
