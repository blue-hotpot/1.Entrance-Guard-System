#ifndef __8266_H
#define __8266_H


#include "main.h"
#include "usart.h"
#define USART1_MAX_RECV_LEN		400					//最大接收缓存字节数
#define USART2_MAX_RECV_LEN		400					//最大接收缓存字节数
#define ESP8266_UART   	huart2
#define UART1   	huart1
extern uint8_t USART1_RX_BUF[USART1_MAX_RECV_LEN];
extern uint8_t USART2_RX_BUF[USART1_MAX_RECV_LEN];
extern uint8_t USART1_RX_STA;
extern uint8_t USART2_RX_STA;
void ESP8266_Init(void);

#endif /* __8266_H */
