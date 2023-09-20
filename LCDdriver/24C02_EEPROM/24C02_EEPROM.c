#include "24C02_EEPROM.h"

#define   EP24C_TIMEOUT      200  //超时等待时间，单位:节拍数
#define   EP24C_MEMADD_SIZE  I2C_MEMADD_SIZE_8BIT  //存储器地址大小，8位地址


//检查设备是否准备好 I2C 通信，返回HAL_OK 表示OK
HAL_StatusTypeDef EP24C_IsDeviceReady(void)
{
	uint32_t  Trials=10;  //尝试次数
	HAL_StatusTypeDef result=HAL_I2C_IsDeviceReady(&I2C_HANDLE,DEV_ADDR_24CXX,Trials,EP24C_TIMEOUT);
	
	return result;
}

//向任意地址写入1字节的数据，memAddr 是存储器内部地址，byteData 是需要写入的1字节数据
HAL_StatusTypeDef EP24C_WriteOneByte(uint16_t memAddress, uint8_t byteData)
{
	HAL_StatusTypeDef result=HAL_I2C_Mem_Write(&I2C_HANDLE,DEV_ADDR_24CXX, memAddress, EP24C_MEMADD_SIZE, &byteData, 1,EP24C_TIMEOUT);
	return result;

}


//从任意地址读出1字节的数据，memAddr 是存储器内部地址，byteData是读出的1字节数据
HAL_StatusTypeDef EP24C_ReadOneByte(uint16_t memAddress, uint8_t byteData)
{
	HAL_StatusTypeDef result=HAL_I2C_Mem_Read(&I2C_HANDLE,DEV_ADDR_24CXX, memAddress, EP24C_MEMADD_SIZE, &byteData, 1,EP24C_TIMEOUT);
	return result;

}


//连续读取数据，任意地址，任意长度，不受页的限制
HAL_StatusTypeDef EP24C_ReadBytes(uint16_t memAddress, uint8_t *pBuffer,uint16_t bufferLen)
{
	if (bufferLen>MEM_SIZE_24CXX)   //超过总存储容量
		return HAL_ERROR;
	HAL_StatusTypeDef result=HAL_I2C_Mem_Read(&I2C_HANDLE,DEV_ADDR_24CXX,memAddress,EP24C_MEMADD_SIZE,pBuffer,bufferLen,EP24C_TIMEOUT);
	return result;
	
}

//限定在一个页内写入连续数据，最多 8 字节。从任意起始地址开始，但起始地址+数据长度不能超过页边界
HAL_StatusTypeDef EP24C_WriteInOnePage(uint16_t memAddress, uint8_t *pBuffer,uint16_t bufferLen)
{
	if (bufferLen>MEM_SIZE_24CXX)   //超过总存储容量
		return HAL_ERROR;
	HAL_StatusTypeDef result=HAL_I2C_Mem_Write(&I2C_HANDLE,DEV_ADDR_24CXX,memAddress,EP24C_MEMADD_SIZE,pBuffer,bufferLen,EP24C_TIMEOUT);	
	return result;	
		
}

//写任意长的数据，可以超过8字节，但数据地址必须从页首开始，即8XN。自动分解为多次写入
HAL_StatusTypeDef EP24C_WriteLongData(uint16_t memAddress, uint8_t *pBuffer,uint16_t bufferLen)
{
	if (bufferLen>MEM_SIZE_24CXX)   //超过总存储容量
		return HAL_ERROR;
	HAL_StatusTypeDef result=HAL_ERROR;
	if (bufferLen<=PAGE_SIZE_24CXX)  //不超过1个page，直接写入后退出
	{
		result=HAL_I2C_Mem_Write(&I2C_HANDLE,DEV_ADDR_24CXX,memAddress,EP24C_MEMADD_SIZE,pBuffer,bufferLen,EP24C_TIMEOUT);	
		return result;
	}
	uint8_t *pt=pBuffer;	//临时指针，不能改变传入的指针
	uint16_t pageCount=bufferLen/PAGE_SIZE_24CXX;   //Page个数
	for(uint16_t i=0;i<pageCount;i++)  //一次写入一个page 的数据
	{
		result=HAL_I2C_Mem_Write(&I2C_HANDLE,DEV_ADDR_24CXX,memAddress,EP24C_MEMADD_SIZE,pt, PAGE_SIZE_24CXX,EP24C_TIMEOUT);	
		pt += PAGE_SIZE_24CXX;
		memAddress += PAGE_SIZE_24CXX;
		HAL_Delay(5);   //必须有延时，以等待页写完
		if (result != HAL_OK)
			return result;
	
	}
	uint16_t leftBytes=bufferLen%PAGE_SIZE_24CXX; //余数
	if (leftBytes>0)   //写入剩余的数据
		result=HAL_I2C_Mem_Write(&I2C_HANDLE,DEV_ADDR_24CXX,memAddress,EP24C_MEMADD_SIZE,pt, leftBytes,EP24C_TIMEOUT);	
	return result;	
		

}


