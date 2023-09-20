#ifndef __24C02_EEPROM_H
#define __24C02_EEPROM_H	

#include "main.h"
#include "i2c.h"

#define    I2C_HANDLE     hi2c2  // I2C接口外设对象变量
#define    DEV_ADDR_24CXX 0x00A0  // 24c02的写地址


#define    PASSWORD_PARA_ADDR        16    //密码在24C02中的首地址,必须是页的起始地址，也就是8的整数倍

#define    PAGE_SIZE_24CXX  0x0008  // 24c02的page大小为8字节
#define    MEM_SIZE_24CXX  (uint16_t)256  // 24c02总容量为256字节



//检查设备是否准备好 I2C 通信，返回HAL_OK 表示OK
HAL_StatusTypeDef EP24C_IsDeviceReady(void);

//向任意地址写入1字节的数据，memAddr 是存储器内部地址，byteData 是需要写入的1字节数据
HAL_StatusTypeDef EP24C_WriteOneByte(uint16_t memAddress, uint8_t byteData);


//从意地址读出1字节的数据，memAddr 是存储器内部地址，byteData是读出的1字节数据
HAL_StatusTypeDef EP24C_ReadOneByte(uint16_t memAddress, uint8_t byteData);


//连续读取数据，任意地址，任意长度，不受页的限制
HAL_StatusTypeDef EP24C_ReadBytes(uint16_t memAddress, uint8_t *pBuffer, uint16_t bufferLen);


//限定在一个页内写入连续数据，最多 8 字节。从任意起始地址开始，但起始地址+数据长度不能超过页边界
HAL_StatusTypeDef EP24C_WriteInOnePage(uint16_t memAddress, uint8_t *pBuffer,uint16_t bufferLen);


//写任意长的数据，可以超过8字节，但数据地址必须从页首开始，即8XN。自动分解为多次写入
HAL_StatusTypeDef EP24C_WriteLongData(uint16_t memAddress, uint8_t *pBuffer,uint16_t bufferLen);
#endif /* __24C02_EEPROM_H */
