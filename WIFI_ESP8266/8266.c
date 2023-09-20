#include "8266.h"
/*
1.AT+CWMODE=3 ��ģ������ΪAP+STAģʽ
2.AT+RST ����ģ��
3.AT+CWJAP="123123123","12345679" ����ESP8266���ƺ�����
4.AT+CIPMUX=1 ����Ϊ��·����
5.AT+CIPSERVER=1,8848 1Ϊ������ţ�8848�Ƕ˿ںţ����Լ��趨
6.AT+CIFSR IP��ַ��ѯ

*/

/*
1.AT+CWMODE=2   /����Ϊ AP ģʽ
2.AT+CWSAP="ESP8266","12345678",11,3   /���� wifi �ȵ�
3.AT+CIPMUX=1 ����Ϊ��·����
4.AT+CIPSERVER=1,5000    /���������
5.AT+CIFSR   /�鿴 mac ��ַ�� IP
6.AT+CIFSR IP��ַ��ѯ
7.���ֻ��������ESP8266
8.AT+CIPSEND=0,5   /��������
AT+CIPMODE=1������͸��ģʽ
AT+CIPSEND��͸��ģʽ�£���������
+++���˳�͸��ģʽ��
*/

/*
���Ӱ����Ʋ��裺��only_print��
								1.����Wi-Fiģʽ:AT+CWMODE=1
								2.����·����:AT+CWJAP="123123123","12345679"
								3.ʹ�� SNTP �������������й�ʱ��:AT+CIPSNTPCFG=1,8,"ntp1.aliyun.com"
								4.���õ�¼����:AT+MQTTUSERCFG=0,1,"NULL","only_print&iwjauHr3P6U","d38974e0373d5e2cc683d0cd05b0dc5cec7cc9eb77f9ded0edfe0dea73feb9e9",0,0,""
								5.���� MQTT �û�����:AT+MQTTCLIENTID=0,"iwjauHr3P6U.only_print|securemode=2\,signmethod=hmacsha256\,timestamp=1686622748083|"
								6.���� MQTT Broker:AT+MQTTCONN=0,"iot-06z00fhtpniomkb.mqtt.iothub.aliyuncs.com",1883,1
								7.����Topic:AT+MQTTSUB=0,"/iwjauHr3P6U/only_print/user/get",1
								8.������ģ������:AT+MQTTPUB=0,"/sys/iwjauHr3P6U/only_print/thing/event/property/post","{params:{\"CurrentTemperature\":32}}",0,0

*/

//ע�⣺
//ĩβ�ǵü�\r\n
//�ַ�������� " ��Ҫת������ǰ����Ҫ�� \���ܵõ���ȷ��"
//�ַ��������һ��\���ַ�������Ҫʹ��\\

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
uint8_t USART1_RX_BUF[USART1_MAX_RECV_LEN]; 				//���ջ���,���USART3_MAX_RECV_LEN���ֽ�.

uint8_t USART1_RX_STA= 0;//�����Ƿ���յ�����

uint8_t USART2_RX_BUF[USART1_MAX_RECV_LEN]; 				//���ջ���,���USART3_MAX_RECV_LEN���ֽ�.

uint8_t USART2_RX_STA= 0;//�����Ƿ���յ�����

// ����Wi-Fiģʽ
uint8_t cwmode[] = "AT+CWMODE=1\r\n";
// ����·����
uint8_t cwjap[] = "AT+CWJAP=\"123123123\",\"12345679\"\r\n";
// ʹ�� SNTP �������������й�ʱ��
uint8_t cipsntpcfg[] = "AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n";
// ���õ�¼����
uint8_t mqttusercfg[] = "AT+MQTTUSERCFG=0,1,\"NULL\",\"only_print&iwjauHr3P6U\",\"d38974e0373d5e2cc683d0cd05b0dc5cec7cc9eb77f9ded0edfe0dea73feb9e9\",0,0,\"\"\r\n";
// ���� MQTT �û�����
uint8_t mqttclientid[] = "AT+MQTTCLIENTID=0,\"iwjauHr3P6U.only_print|securemode=2\\,signmethod=hmacsha256\\,timestamp=1686622748083|\"\r\n";
// ���� MQTT Broker
uint8_t mqttconn[] = "AT+MQTTCONN=0,\"iot-06z00fhtpniomkb.mqtt.iothub.aliyuncs.com\",1883,1\r\n";
// ����Topic
uint8_t mqttsub[] = "AT+MQTTSUB=0,\"/iwjauHr3P6U/only_print/user/get\",1\r\n";
uint8_t b[] = "123123123\r\n";
void ESP8266_Init(void)
{
	HAL_UARTEx_ReceiveToIdle_DMA(&ESP8266_UART, USART2_RX_BUF, sizeof(USART2_RX_BUF));
	HAL_UARTEx_ReceiveToIdle_DMA(&UART1, USART1_RX_BUF, sizeof(USART1_RX_BUF));
	// �ر�DMA������ж�
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT);
	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx,DMA_IT_HT);
//	HAL_UART_Transmit_IT(&ESP8266_UART,cwmode,sizeof(cwmode));
//	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//�ȴ��������
//	HAL_Delay(10);
//	HAL_UART_Transmit_IT(&ESP8266_UART,cwjap,sizeof(cwjap));
//	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//�ȴ��������
//	HAL_Delay(10);
	HAL_UART_Transmit_IT(&ESP8266_UART,cipsntpcfg,sizeof(cipsntpcfg));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&ESP8266_UART,mqttusercfg,sizeof(mqttusercfg));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&ESP8266_UART,mqttclientid,sizeof(mqttclientid));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&ESP8266_UART,mqttconn,sizeof(mqttconn));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&ESP8266_UART,mqttsub,sizeof(mqttsub));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//�ȴ��������
	HAL_Delay(1000);

}
