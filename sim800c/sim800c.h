#ifndef __SIM800C_H
#define __SIM800C_H

#include "main.h"
#include "usart.h"

#define SIM800C_UART 	huart5  // ���崮��5��sim�Ĵ���
#define USART5_MAX_RECV_LEN		400					//�����ջ����ֽ���
void SIM800C_Init(void);

void PassWordPhone(void);


#endif /* __SIM800C_H */
