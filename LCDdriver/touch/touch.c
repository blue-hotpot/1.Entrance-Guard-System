#include "touch.h"
#include "tim.h"
#include "touch.h"
#include "lcd.h"
#include "24C02_EEPROM.h"
#include "8266.h"
#include "as608.h"

/* 触摸校正因数设置 */
#define LCD_ADJX_MIN (10)                      //读取四个点的最小X值
#define LCD_ADJX_MAX (240 -  LCD_ADJX_MIN) //读取四个点的最大X值
#define LCD_ADJY_MIN (10)                      //读取四个点的最小Y值
#define LCD_ADJY_MAX (320 - LCD_ADJY_MIN) //读取四个点的最大Y值

#define LCD_ADJ_X (LCD_ADJX_MAX - LCD_ADJY_MIN)//读取方框的宽度
#define LCD_ADJ_Y (LCD_ADJY_MAX - LCD_ADJY_MIN)//读取方框的高度

#define TOUCH_READ_TIMES 5     //一次读取触摸值的次数

#define TOUCH_X_CMD      0xD0  //读取X轴命令
#define TOUCH_Y_CMD      0x90  //读取Y轴命令

#define TOUCH_MAX        20    //预期差值
#define TOUCH_X_MAX      4000  //X轴最大值
#define TOUCH_X_MIN      100   //X轴最小值
#define TOUCH_Y_MAX      4000  //Y轴最大值
#define TOUCH_Y_MIN      100   //Y轴最小值

//电阻屏SPI接口的基本输入输出
#define	MISO_Read()		HAL_GPIO_ReadPin(T_MISO_GPIO_Port,T_MISO_Pin)

#define	MOSI_Out0()		HAL_GPIO_WritePin(T_MOSI_GPIO_Port,T_MOSI_Pin,GPIO_PIN_RESET)
#define	MOSI_Out1()		HAL_GPIO_WritePin(T_MOSI_GPIO_Port,T_MOSI_Pin,GPIO_PIN_SET)

#define	SCK_Out0()		HAL_GPIO_WritePin(T_SCK_GPIO_Port,T_SCK_Pin,GPIO_PIN_RESET)
#define	SCK_Out1()		HAL_GPIO_WritePin(T_SCK_GPIO_Port,T_SCK_Pin,GPIO_PIN_SET)

#define	TCS_Out0()		HAL_GPIO_WritePin(TP_CS_GPIO_Port,TP_CS_Pin,GPIO_PIN_RESET)
#define	TCS_Out1()		HAL_GPIO_WritePin(TP_CS_GPIO_Port,TP_CS_Pin,GPIO_PIN_SET)

#define TOUCH_AdjDelay500ms()  HAL_Delay(500)

TouchPointDef TouchPoint;   //触摸点数据

TouchParaDef TouchPara;     //触摸屏校正参数,全局静态变量


//SPI写数据
//向触摸屏写入1byte数据
//num:要写入的数据
void TOUCH_Write_Byte(uint8_t num)
{  
	uint8_t count=0;
	for(count=0;count<8;count++)  
	{ 	  
		if(num & 0x80)
			MOSI_Out1();
		else
			MOSI_Out0();

		num<<=1;    
		SCK_Out0();
		Delay_us(1);
		SCK_Out1();		//上升沿有效
	}
}

//SPI读数据 ,软件模拟SPI
//从触摸屏IC读取adc值
//CMD:指令
//返回值:读到的数据	   
uint16_t TOUCH_Read_AD(uint8_t CMD)
{ 	 
	uint8_t count=0;
	uint16_t Num=0;

	SCK_Out0();	//TCLK=0;		//先拉低时钟
	MOSI_Out0();	//TDIN=0; 		//拉低数据线
	TCS_Out0();		//TCS=0; 		//选中触摸屏IC
	TOUCH_Write_Byte(CMD);//发送命令字
	Delay_us(6);	//delay_us(6);	//ADS7846的转换时间最长为6us

	SCK_Out0();	//TCLK=0;
	Delay_us(1);	//delay_us(1);
	SCK_Out1();	//TCLK=1;			//给1个时钟，清除BUSY
	Delay_us(1);	//delay_us(1);
	SCK_Out0();	//TCLK=0;

	for(count=0;count<16;count++)//读出16位数据,只有高12位有效 
	{ 				  
		Num<<=1; 	 
		SCK_Out0();	//TCLK=0;	//下降沿有效
		Delay_us(1);	//delay_us(1);
		SCK_Out1();	//TCLK=1;
		if (MISO_Read())	//if(DOUT)
			Num++;
	}
	Num>>=4;   		//只有高12位有效.
	TCS_Out1();		//TCS=1;		//释放片选
	return(Num);   
}

uint16_t TOUCH_ReadData(uint8_t cmd)
{
	uint8_t i, j;
	uint16_t readValue[TOUCH_READ_TIMES], value;
	uint32_t totalValue;

    /* 读取TOUCH_READ_TIMES次触摸值 */
    for(i=0; i<TOUCH_READ_TIMES; i++)
    {   /* 打开片选 */
//    	TCS_Out0();		//TCS=0;
        /* 在差分模式下，XPT2046转换需要24个时钟，8个时钟输入命令，之后1个时钟去除 */
        /* 忙信号，接着输出12位转换结果，剩下3个时钟是忽略位 */    
        readValue[i]=TOUCH_Read_AD(cmd); // 发送命令，选择X轴或者Y轴 
//        TCS_Out1();		//TCS=1;
    }

    /* 滤波处理 */
    /* 首先从大到小排序 */
    for(i=0; i<(TOUCH_READ_TIMES - 1); i++)
    {
    	for(j=i+1; j<TOUCH_READ_TIMES; j++)
    	{
    		/* 采样值从大到小排序排序 */
    		if(readValue[i] < readValue[j])
    		{
    			value = readValue[i];
    			readValue[i] = readValue[j];
    			readValue[j] = value;
    		}
    	}
    }
   
    /* 去掉最大值，去掉最小值，求平均值 */
    j = TOUCH_READ_TIMES - 1;
    totalValue = 0;
    for(i=1; i<j; i++)     //求y的全部值
    {
    	totalValue += readValue[i];
    }
    value = totalValue / (TOUCH_READ_TIMES - 2);

    return value;
}

//返回值为0表示有触摸操作
uint8_t TOUCH_ReadXY(uint16_t *xValue, uint16_t *yValue)
{   
	uint16_t xValue1, yValue1, xValue2, yValue2;

	xValue1 = TOUCH_Read_AD(TOUCH_X_CMD);
	yValue1 = TOUCH_Read_AD(TOUCH_Y_CMD);
	xValue2 = TOUCH_Read_AD(TOUCH_X_CMD);
	yValue2 = TOUCH_Read_AD(TOUCH_Y_CMD);

	/* 查看两个点之间的采样值差距 */
	if(xValue1 > xValue2)
		*xValue = xValue1 - xValue2;
	else
		*xValue = xValue2 - xValue1;

	if(yValue1 > yValue2)
		*yValue = yValue1 - yValue2;
	else
		*yValue = yValue2 - yValue1;

	/* 判断采样差值是否在可控范围内 */
	if((*xValue > TOUCH_MAX+0) || (*yValue > TOUCH_MAX+0))  
		return 0xFF;

	/* 求平均值 */
	*xValue = (xValue1 + xValue2) / 2;
	*yValue = (yValue1 + yValue2) / 2;

	/* 判断得到的值，是否在取值范围之内 */
	if((*xValue > TOUCH_X_MAX+0) || (*xValue < TOUCH_X_MIN)
			|| (*yValue > TOUCH_Y_MAX+0) || (*yValue < TOUCH_Y_MIN))
		return 0xFF;
	else
		return 0; 	//读取成功，有触控操作
}

uint8_t TOUCH_ReadAdjust(uint16_t x, uint16_t y, uint16_t *xValue, uint16_t *yValue)
{
	uint8_t i;
	uint32_t timeCont=0;

	/* 读取校正点的坐标 */
	LCD_Clear(WHITE);
	LCD_DrowSign(x, y, RED);
	i = 0;
	while(1)
	{
		if(!TOUCH_ReadXY(xValue, yValue))
		{
			i++;
			if(i > 10)         //延时一下，以读取最佳值
			{
				LCD_DrowSign(x, y, WHITE);
				return 0;
			}

		}
		timeCont++;
		/* 超时退出 */
		if(timeCont > 0xFFFFFFFE)
		{
			LCD_DrowSign(x, y, WHITE);
			return 0xFF;
		}
	}
}

//触摸屏校准，依次在LCD的4个角上显示十字符号，用户点击后获取输出，计算触摸屏参数
void TOUCH_Adjust(void)
{
	uint16_t px[2], py[2], xPot[4], yPot[4];
	float xFactor, yFactor;

	/* 读取第一个点 */
	if(TOUCH_ReadAdjust(LCD_ADJX_MIN, LCD_ADJY_MIN, &xPot[0], &yPot[0]))
		return;
	TOUCH_AdjDelay500ms();

	/* 读取第二个点 */
	if(TOUCH_ReadAdjust(LCD_ADJX_MIN, LCD_ADJY_MAX, &xPot[1], &yPot[1]))
		return;
	TOUCH_AdjDelay500ms();

	/* 读取第三个点 */
	if(TOUCH_ReadAdjust(LCD_ADJX_MAX, LCD_ADJY_MIN, &xPot[2], &yPot[2]))
		return;
	TOUCH_AdjDelay500ms();

	/* 读取第四个点 */
	if(TOUCH_ReadAdjust(LCD_ADJX_MAX, LCD_ADJY_MAX, &xPot[3], &yPot[3]))
		return;
	TOUCH_AdjDelay500ms();

	/* 处理读取到的四个点的数据，整合成对角的两个点 */
	px[0] = (xPot[0] + xPot[1]) / 2;
	py[0] = (yPot[0] + yPot[2]) / 2;
	px[1] = (xPot[3] + xPot[2]) / 2;
	py[1] = (yPot[3] + yPot[1]) / 2;

	/* 求出比例因数 */
	xFactor = (float)LCD_ADJ_X / (px[1] - px[0]);
	yFactor = (float)LCD_ADJ_Y / (py[1] - py[0]);

	/* 求出偏移量 */
	TouchPara.xOffset = (int16_t)LCD_ADJX_MAX - ((float)px[1] * xFactor);
	TouchPara.yOffset = (int16_t)LCD_ADJY_MAX - ((float)py[1] * yFactor);

	/* 将比例因数进行数据处理，然后保存 */
	TouchPara.xFactor = xFactor ;
	TouchPara.yFactor = yFactor ;

	TouchPara.isSaved = TOUCH_PARA_SAVED;
}

uint8_t TOUCH_Scan(void)
{
	if(TOUCH_ReadXY(&TouchPoint.Vx, &TouchPoint.Vy)) //没有触摸
		return 0xFF;

	/* 根据物理坐标值，计算出彩屏坐标值 */
	TouchPoint.Lcdx = TouchPoint.Vx * TouchPara.xFactor + TouchPara.xOffset;
	TouchPoint.Lcdy = TouchPoint.Vy * TouchPara.yFactor + TouchPara.yOffset;

	/* 查看彩屏坐标值是否超过彩屏大小 */
	if(TouchPoint.Lcdx > 240)
		TouchPoint.Lcdx = 240;

	if(TouchPoint.Lcdy > 320)
		TouchPoint.Lcdy = 320;

	return 0;   
}

void TOUCH_ScanAfterINT(void)	// T_PEN中断后读取
{
	TouchPoint.Vx = TOUCH_ReadData(TOUCH_X_CMD);
	TouchPoint.Vy = TOUCH_ReadData(TOUCH_Y_CMD);

	/* 根据物理坐标值，计算出彩屏坐标值 */
	TouchPoint.Lcdx = TouchPoint.Vx * TouchPara.xFactor + TouchPara.xOffset;
	TouchPoint.Lcdy = TouchPoint.Vy * TouchPara.yFactor + TouchPara.yOffset;

	/* 查看彩屏坐标值是否超过彩屏大小 */
	if(TouchPoint.Lcdx > 240)
		TouchPoint.Lcdx = 240;

	if(TouchPoint.Lcdy > 320)
		TouchPoint.Lcdy = 320;
}
uint16_t LCD_CurX=0;	//当前位置X
uint16_t LCD_CurY=0;	//当前位置Y
void	ShowTouchPara(void)//显示触摸屏参数
{

	uint16_t  IncY=16;	//Y间距

	LCD_ShowString(10,0,240,16,16,(uint8_t*)"**Parameters of touch screen:");
	LCD_ShowString(0,16,240,16,16,(uint8_t*)"xOffset= ");
	LCD_ShowChar(80, 16,TouchPara.xOffset,16,0);

	LCD_ShowString(0,32,240,16,16,(uint8_t*)"yOffset= ");
	LCD_ShowChar(80, 32,TouchPara.yOffset,16,0);

	LCD_ShowString(0,32+IncY,240,16,16,(uint8_t*)"xFactor= ");
	LCD_ShowxNum(80, 32+IncY,TouchPara.xFactor, 4,16,0);

	LCD_ShowString(0,32+IncY+IncY,240,16,16,(uint8_t*)"yFactor= ");
	LCD_ShowxNum(80, 32+IncY+IncY,TouchPara.yFactor, 4,16,0);
}

void	TouchCalibrate(void)//进行触摸屏测试，获取参数
{
	LCD_ShowString(10,0,240,16,16,(uint8_t*)"**Touch screen calibration");

	LCD_ShowString(10,16,240,16,16,(uint8_t*)"A red cross will display on");
	LCD_ShowString(10,32,240,16,16,(uint8_t*)"the 4 corners of LCD. ");
	LCD_ShowString(10,48,240,16,16,(uint8_t*)"Touch red cross one by one.");
	LCD_ShowString(10,64,240,16,16,(uint8_t*)"Press any key to start...");

	HAL_Delay(3000);
	TOUCH_Adjust();	//触摸屏校正时会清屏
	EP24C_WriteLongData(TOUCH_PARA_ADDR, &TouchPara.isSaved, sizeof(TouchPara));

	LCD_CurY=40;
	ShowTouchPara();//显示触摸屏参数

	LCD_ShowString(10,80,240,16,16,(uint8_t*)"Press any key to enter GUI");
	HAL_Delay(3000);
}
// a用来储存当前的数字，c用来检测位置
uint8_t a,c = 0;
extern uint8_t Password[6];
// 位置和第几个数字对应
uint8_t a_c(uint8_t x)
{
	if(c == 0)
		return 0;
	if(c == 1)
		return 20;
	if(c == 2)
		return 40;
	if(c == 3)
		return 60;
	if(c == 4)
		return 80;
	if(c == 5)
		return 100;
	if(c == 6)  // 密码过长了
		LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is too long!");
		HAL_Delay(3000);
		LCD_Fill(10,130, 240, 170, WHITE);
	return 0;
}
// 定义一个储存密码的数组
uint8_t PasswordIn[6];
// 这个函数用来检测触摸的位置以及做出反应
uint8_t Touch_Num(void)
{
	// 发送物模型数据
	uint8_t mqttpub[] = "AT+MQTTPUB=0,\"/sys/iwjauHr3P6U/only_print/thing/event/property/post\",\"{params:{\\\"CurrentTemperature\\\":30}}\",0,0\r\n";
	// 返回值
	uint8_t i;
	if(TOUCH_Scan() == 0)  // 触摸屏扫描
	{
		// 1
		if(TouchPoint.Lcdx < 80 && TouchPoint.Lcdy > 200&&TouchPoint.Lcdy<230)
		{
			a = a_c(c);
			PasswordIn[c] = 1;
			LCD_ShowNum(a, 150, 1, 1, 16);
			c+=1;
			i = 1;
			return i;
		}
		// 2
		else if(TouchPoint.Lcdx<160&&TouchPoint.Lcdy > 200&&TouchPoint.Lcdy<230&&TouchPoint.Lcdx>80)
		{
			a = a_c(c);
			PasswordIn[c] = 2;
			LCD_ShowNum(a, 150, 2, 1, 16);
			c+=1;
			i = 2;
			return i;
		}
		// 3
		else if(TouchPoint.Lcdx>160&&TouchPoint.Lcdy > 200&&TouchPoint.Lcdy<230)
		{
			a = a_c(c);
			PasswordIn[c] = 3;
			LCD_ShowNum(a, 150, 3, 1, 16);
			c+=1;
			i = 3;
			return i;
		}
		// 4
		else if(TouchPoint.Lcdx< 80&&TouchPoint.Lcdy > 230&&TouchPoint.Lcdy<260)
		{
			a = a_c(c);
			PasswordIn[c] = 4;
			LCD_ShowNum(a, 150, 4, 1, 16);
			c+=1;
			i = 4;
			return i;
		}
		// 5
		else if(TouchPoint.Lcdx<160&&TouchPoint.Lcdy > 230&&TouchPoint.Lcdy<260&&TouchPoint.Lcdx>80)
		{
			a = a_c(c);
			PasswordIn[c] = 5;
			LCD_ShowNum(a, 150, 5, 1, 16);
			c+=1;
			i = 5;
			return i;
		}
		// 6
		else if(TouchPoint.Lcdx>160&&TouchPoint.Lcdy > 230&&TouchPoint.Lcdy<260)
		{
			a = a_c(c);
			PasswordIn[c] = 6;
			LCD_ShowNum(a, 150, 6, 1, 16);
			c+=1;
			i = 6;
			return i;
		}
		// 7
		else if(TouchPoint.Lcdx<80&&TouchPoint.Lcdy>260&&TouchPoint.Lcdy<290)
		{
			a = a_c(c);
			PasswordIn[c] = 7;
			LCD_ShowNum(a, 150, 7, 1, 16);
			c+=1;
			i = 7;
			return i;
		}
		// 8
		else if(TouchPoint.Lcdx<160&&TouchPoint.Lcdx>80&&TouchPoint.Lcdy>260&&TouchPoint.Lcdy<290)
		{
			a = a_c(c);
			PasswordIn[c] = 8;
			LCD_ShowNum(a, 150, 8, 1, 16);
			c+=1;
			i = 8;
			return i;
		}
		// 9
		else if(TouchPoint.Lcdx>160&&TouchPoint.Lcdy>260&&TouchPoint.Lcdy<290)
		{
			a = a_c(c);
			PasswordIn[c] = 9;
			LCD_ShowNum(a, 150, 9, 1, 16);
			c+=1;
			i = 9;
			return i;
		}
		// *——表示输入错误，重新输入上一位
		else if(TouchPoint.Lcdx<80&&TouchPoint.Lcdy>290)
		{
			
			c-=1;
			a = a_c(c);
			LCD_Fill(a, 150, a+20, 166, WHITE);
			i = 0;
			return i;
		}
		// 0
		else if(TouchPoint.Lcdx<160&&TouchPoint.Lcdx>80&&TouchPoint.Lcdy>290)
		{
			a = a_c(c);
			PasswordIn[c] = 0;
			LCD_ShowNum(a, 150, 0, 1, 16);
			c+=1;
			i = 0;
			return i;
		}
		// #——表示确定密码输入
		else if(TouchPoint.Lcdx>160&&TouchPoint.Lcdy>290)
		{
			c = 0;
			for(uint8_t i = 0;i<6;i++)
			{
				// 输出密码错误
				if(PasswordIn[i] != Password[i])
				{
					LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is incorrect!");
					HAL_Delay(3000);
					LCD_Fill(0,130, 239, 170, WHITE);
					return 0;
				}
			}
			// 密码正确
			LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is correct!");
			HAL_Delay(3000);
			LCD_Fill(0,130, 239, 170, WHITE); 
			HAL_UART_Transmit_IT(&ESP8266_UART,mqttpub,sizeof(mqttpub));
			while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//等待发送完成
			HAL_Delay(100);
		}
	}
	return 0;
}
// 触摸位置是命令的位置
void TOUCH_Command(void)
{
	// 删除指纹
	if(TouchPoint.Lcdx < 80 && TouchPoint.Lcdy > 170&&TouchPoint.Lcdy<200)
	{
		// 请选择想删除的指纹
		LCD_ShowString(10,100,240,16,16,(uint8_t*)"Please select the fingerprint!");
		HAL_Delay(3000);
		LCD_Fill(0,100, 240, 170, WHITE);
		// 这个函数用来检测触摸的位置以及做出反应
		uint8_t i = Touch_Num();
		// 删除想要删除的指纹ID
		if(i>0)
		{
			Del_FR(i);
		}
	}
	// 修改密码
	else if(TouchPoint.Lcdx<160&&TouchPoint.Lcdy > 170&&TouchPoint.Lcdy<200&&TouchPoint.Lcdx>80)
	{
		LCD_ShowString(10,100,240,16,16,(uint8_t*)"Please enter previous password!");
//		c = 0;
		for(c = 0;c<6;)
		{
			Touch_Num();
			HAL_Delay(200);
		}
		
		for(uint8_t i = 0;i<6;i++)
		{
			// 输入密码错误
			if(PasswordIn[i] != Password[i])
			{
				LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is incorrect!");
				HAL_Delay(3000);
				LCD_Fill(0, 130,240, 165, WHITE);  //清除信息显示区
				return;
			}
		}
		// 密码正确
		LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is correct!");
		HAL_Delay(3000);
		LCD_Fill(0, 130,240, 165, WHITE);  //清除信息显示区
		// 输出请输入新密码
		LCD_ShowString(10,130,240,16,16,(uint8_t*)"Please enter a new password!");
		for(c = 0;c<6;)
		{
			c = 0;
			Touch_Num();
			HAL_Delay(200);
		}
		LCD_Fill(0, 130,240, 146, WHITE);  //清除信息显示区
		// 输出密码修改成功
		LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is OK!");
		
		HAL_Delay(3000);
		LCD_Fill(0, 130,240, 165, WHITE);  //清除信息显示区
		EP24C_WriteLongData(PASSWORD_PARA_ADDR, PasswordIn,sizeof(PasswordIn));
		c = 0;
	}
	// 新建指纹
	else if(TouchPoint.Lcdx>160&&TouchPoint.Lcdy > 170&&TouchPoint.Lcdy<200)
	{
		// 请选择新建指纹的位置
		LCD_ShowString(10,100,240,16,16,(uint8_t*)"Select the new fingerprint");
		HAL_Delay(3000);
		LCD_Fill(0,100, 240, 170, WHITE);
		// 这个函数用来检测触摸的位置以及做出反应
		uint8_t i = Touch_Num();
		if(i>0)
		{
			// 录入指纹ID
			Add_FR(i);
		}
	}

}
void Touch_Point(void)
{
	if(TOUCH_Scan() == 0)  // 触摸屏扫描
	{
		TOUCH_Command();
		Touch_Num();
	}
}

// 微秒级延时
void Delay_us(uint16_t delay)
{
	__HAL_TIM_DISABLE(&htim7);
	__HAL_TIM_SET_COUNTER(&htim7,0);  // 设置计数值初值
	__HAL_TIM_ENABLE(&htim7);
	uint16_t curCnt = 0;
	while(1)  // 开始计数
	{
		curCnt = __HAL_TIM_GET_COUNTER(&htim7); 
		if(curCnt>delay) 
			break;
	}
	
	__HAL_TIM_DISABLE(&htim7);
}



