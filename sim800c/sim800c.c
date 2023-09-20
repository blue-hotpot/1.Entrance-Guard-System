#include "sim800c.h"
#include "24C02_EEPROM.h"
#include "stdio.h"

uint8_t USART5_RX_BUF[USART5_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.
// 发送指令AT+CMGF=1设置为文本模式，返回OK
uint8_t CMGF[] = "AT+CMGF=1\r\n";
// 发送AT+CSCS="GSM"为设置为缺省字符集用于发纯英文短信。
uint8_t CSCS[] = "AT+CSCS=\"GSM\"\r\n";
// 发送AT+CMGS="155XXXXXX46",返回<。
uint8_t CMGS[] = "AT+CMGS=\"15319877793\"\r\n";  // 浩哥手机号
// 存放密码的数组
uint8_t PassWord[6] = {0};
// 短信内容发送完毕后，使用16进制发送1A(无回车)完成发送短信操作。
uint8_t OverTransmit[] = {0x1a};



void SIM800C_Init(void)
{
	// 初始化接收和发送中断
	HAL_UART_Receive_IT(&SIM800C_UART, USART5_RX_BUF, sizeof(USART5_RX_BUF));
	HAL_UART_Transmit_IT(&SIM800C_UART, USART5_RX_BUF, sizeof(USART5_RX_BUF));
}

void PassWordPhone(void)
{
	// 获取密码
	EP24C_ReadBytes(PASSWORD_PARA_ADDR,PassWord,sizeof(PassWord));
	// 发送密码到手机上
	// 发送指令AT+CMGF=1设置为文本模式
	HAL_UART_Transmit_IT(&SIM800C_UART,CMGF,sizeof(CMGF));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(500);
	// 发送AT+CSCS="GSM"为设置为缺省字符集用于发纯英文短信
	HAL_UART_Transmit_IT(&SIM800C_UART,CSCS,sizeof(CSCS));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(500);
	// 发送AT+CMGS="155XXXXXX46",返回<
	HAL_UART_Transmit_IT(&SIM800C_UART,CMGS,sizeof(CMGS));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(500);
	// 发送密码
	HAL_UART_Transmit_IT(&SIM800C_UART,PassWord,sizeof(PassWord));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(500);
	// 使用16进制发送1A(无回车)完成发送短信操作
	HAL_UART_Transmit_IT(&SIM800C_UART,OverTransmit,sizeof(OverTransmit));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(500);
	printf("短信发送完成\r\n");
}


