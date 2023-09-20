#ifndef __SIM800C_H
#define __SIM800C_H

#include "main.h"
#include "usart.h"

#define SIM800C_UART 	huart5  // 定义串口5是sim的串口
#define USART5_MAX_RECV_LEN		400					//最大接收缓存字节数
void SIM800C_Init(void);

void PassWordPhone(void);


#endif /* __SIM800C_H */
