#include "8266.h"
/*
1.AT+CWMODE=3 把模块设置为AP+STA模式
2.AT+RST 重启模块
3.AT+CWJAP="123123123","12345679" 设置ESP8266名称和密码
4.AT+CIPMUX=1 设置为单路连接
5.AT+CIPSERVER=1,8848 1为连接序号，8848是端口号，可自己设定
6.AT+CIFSR IP地址查询

*/

/*
1.AT+CWMODE=2   /设置为 AP 模式
2.AT+CWSAP="ESP8266","12345678",11,3   /开启 wifi 热点
3.AT+CIPMUX=1 设置为单路连接
4.AT+CIPSERVER=1,5000    /开启服务端
5.AT+CIFSR   /查看 mac 地址和 IP
6.AT+CIFSR IP地址查询
7.打开手机软件连接ESP8266
8.AT+CIPSEND=0,5   /启动发送
AT+CIPMODE=1：开启透传模式
AT+CIPSEND：透传模式下，传输数据
+++：退出透传模式传
*/

/*
连接阿里云步骤：（only_print）
								1.设置Wi-Fi模式:AT+CWMODE=1
								2.连接路由器:AT+CWJAP="123123123","12345679"
								3.使能 SNTP 服务器，设置中国时区:AT+CIPSNTPCFG=1,8,"ntp1.aliyun.com"
								4.设置登录密码:AT+MQTTUSERCFG=0,1,"NULL","only_print&iwjauHr3P6U","d38974e0373d5e2cc683d0cd05b0dc5cec7cc9eb77f9ded0edfe0dea73feb9e9",0,0,""
								5.配置 MQTT 用户属性:AT+MQTTCLIENTID=0,"iwjauHr3P6U.only_print|securemode=2\,signmethod=hmacsha256\,timestamp=1686622748083|"
								6.连接 MQTT Broker:AT+MQTTCONN=0,"iot-06z00fhtpniomkb.mqtt.iothub.aliyuncs.com",1883,1
								7.订阅Topic:AT+MQTTSUB=0,"/iwjauHr3P6U/only_print/user/get",1
								8.发送物模型数据:AT+MQTTPUB=0,"/sys/iwjauHr3P6U/only_print/thing/event/property/post","{params:{\"CurrentTemperature\":32}}",0,0

*/

//注意：
//末尾记得加\r\n
//字符串里面的 " 需要转义所以前面需要加 \才能得到正确的"
//字符串里面的一个\在字符串中需要使用\\

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
uint8_t USART1_RX_BUF[USART1_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.

uint8_t USART1_RX_STA= 0;//串口是否接收到数据

uint8_t USART2_RX_BUF[USART1_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.

uint8_t USART2_RX_STA= 0;//串口是否接收到数据

// 设置Wi-Fi模式
uint8_t cwmode[] = "AT+CWMODE=1\r\n";
// 连接路由器
uint8_t cwjap[] = "AT+CWJAP=\"123123123\",\"12345679\"\r\n";
// 使能 SNTP 服务器，设置中国时区
uint8_t cipsntpcfg[] = "AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n";
// 设置登录密码
uint8_t mqttusercfg[] = "AT+MQTTUSERCFG=0,1,\"NULL\",\"only_print&iwjauHr3P6U\",\"d38974e0373d5e2cc683d0cd05b0dc5cec7cc9eb77f9ded0edfe0dea73feb9e9\",0,0,\"\"\r\n";
// 配置 MQTT 用户属性
uint8_t mqttclientid[] = "AT+MQTTCLIENTID=0,\"iwjauHr3P6U.only_print|securemode=2\\,signmethod=hmacsha256\\,timestamp=1686622748083|\"\r\n";
// 连接 MQTT Broker
uint8_t mqttconn[] = "AT+MQTTCONN=0,\"iot-06z00fhtpniomkb.mqtt.iothub.aliyuncs.com\",1883,1\r\n";
// 订阅Topic
uint8_t mqttsub[] = "AT+MQTTSUB=0,\"/iwjauHr3P6U/only_print/user/get\",1\r\n";
uint8_t b[] = "123123123\r\n";
void ESP8266_Init(void)
{
	HAL_UARTEx_ReceiveToIdle_DMA(&ESP8266_UART, USART2_RX_BUF, sizeof(USART2_RX_BUF));
	HAL_UARTEx_ReceiveToIdle_DMA(&UART1, USART1_RX_BUF, sizeof(USART1_RX_BUF));
	// 关闭DMA半完成中断
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT);
	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx,DMA_IT_HT);
//	HAL_UART_Transmit_IT(&ESP8266_UART,cwmode,sizeof(cwmode));
//	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
//	HAL_Delay(10);
//	HAL_UART_Transmit_IT(&ESP8266_UART,cwjap,sizeof(cwjap));
//	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
//	HAL_Delay(10);
	HAL_UART_Transmit_IT(&ESP8266_UART,cipsntpcfg,sizeof(cipsntpcfg));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&ESP8266_UART,mqttusercfg,sizeof(mqttusercfg));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&ESP8266_UART,mqttclientid,sizeof(mqttclientid));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&ESP8266_UART,mqttconn,sizeof(mqttconn));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&ESP8266_UART,mqttsub,sizeof(mqttsub));
	while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
	HAL_Delay(1000);

}
