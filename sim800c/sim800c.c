#include "sim800c.h"
#include "24C02_EEPROM.h"
#include "stdio.h"

uint8_t USART5_RX_BUF[USART5_MAX_RECV_LEN]; 				//���ջ���,���USART3_MAX_RECV_LEN���ֽ�.
// ����ָ��AT+CMGF=1����Ϊ�ı�ģʽ������OK
uint8_t CMGF[] = "AT+CMGF=1\r\n";
// ����AT+CSCS="GSM"Ϊ����Ϊȱʡ�ַ������ڷ���Ӣ�Ķ��š�
uint8_t CSCS[] = "AT+CSCS=\"GSM\"\r\n";
// ����AT+CMGS="155XXXXXX46",����<��
uint8_t CMGS[] = "AT+CMGS=\"15319877793\"\r\n";  // �Ƹ��ֻ���
// ������������
uint8_t PassWord[6] = {0};
// �������ݷ�����Ϻ�ʹ��16���Ʒ���1A(�޻س�)��ɷ��Ͷ��Ų�����
uint8_t OverTransmit[] = {0x1a};



void SIM800C_Init(void)
{
	// ��ʼ�����պͷ����ж�
	HAL_UART_Receive_IT(&SIM800C_UART, USART5_RX_BUF, sizeof(USART5_RX_BUF));
	HAL_UART_Transmit_IT(&SIM800C_UART, USART5_RX_BUF, sizeof(USART5_RX_BUF));
}

void PassWordPhone(void)
{
	// ��ȡ����
	EP24C_ReadBytes(PASSWORD_PARA_ADDR,PassWord,sizeof(PassWord));
	// �������뵽�ֻ���
	// ����ָ��AT+CMGF=1����Ϊ�ı�ģʽ
	HAL_UART_Transmit_IT(&SIM800C_UART,CMGF,sizeof(CMGF));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(500);
	// ����AT+CSCS="GSM"Ϊ����Ϊȱʡ�ַ������ڷ���Ӣ�Ķ���
	HAL_UART_Transmit_IT(&SIM800C_UART,CSCS,sizeof(CSCS));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(500);
	// ����AT+CMGS="155XXXXXX46",����<
	HAL_UART_Transmit_IT(&SIM800C_UART,CMGS,sizeof(CMGS));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(500);
	// ��������
	HAL_UART_Transmit_IT(&SIM800C_UART,PassWord,sizeof(PassWord));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(500);
	// ʹ��16���Ʒ���1A(�޻س�)��ɷ��Ͷ��Ų���
	HAL_UART_Transmit_IT(&SIM800C_UART,OverTransmit,sizeof(OverTransmit));
	while(__HAL_UART_GET_FLAG(&SIM800C_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(500);
	printf("���ŷ������\r\n");
}


