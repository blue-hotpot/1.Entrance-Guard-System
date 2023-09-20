#include "touch.h"
#include "tim.h"
#include "touch.h"
#include "lcd.h"
#include "24C02_EEPROM.h"
#include "8266.h"
#include "as608.h"

/* ����У���������� */
#define LCD_ADJX_MIN (10)                      //��ȡ�ĸ������СXֵ
#define LCD_ADJX_MAX (240 -  LCD_ADJX_MIN) //��ȡ�ĸ�������Xֵ
#define LCD_ADJY_MIN (10)                      //��ȡ�ĸ������СYֵ
#define LCD_ADJY_MAX (320 - LCD_ADJY_MIN) //��ȡ�ĸ�������Yֵ

#define LCD_ADJ_X (LCD_ADJX_MAX - LCD_ADJY_MIN)//��ȡ����Ŀ��
#define LCD_ADJ_Y (LCD_ADJY_MAX - LCD_ADJY_MIN)//��ȡ����ĸ߶�

#define TOUCH_READ_TIMES 5     //һ�ζ�ȡ����ֵ�Ĵ���

#define TOUCH_X_CMD      0xD0  //��ȡX������
#define TOUCH_Y_CMD      0x90  //��ȡY������

#define TOUCH_MAX        20    //Ԥ�ڲ�ֵ
#define TOUCH_X_MAX      4000  //X�����ֵ
#define TOUCH_X_MIN      100   //X����Сֵ
#define TOUCH_Y_MAX      4000  //Y�����ֵ
#define TOUCH_Y_MIN      100   //Y����Сֵ

//������SPI�ӿڵĻ����������
#define	MISO_Read()		HAL_GPIO_ReadPin(T_MISO_GPIO_Port,T_MISO_Pin)

#define	MOSI_Out0()		HAL_GPIO_WritePin(T_MOSI_GPIO_Port,T_MOSI_Pin,GPIO_PIN_RESET)
#define	MOSI_Out1()		HAL_GPIO_WritePin(T_MOSI_GPIO_Port,T_MOSI_Pin,GPIO_PIN_SET)

#define	SCK_Out0()		HAL_GPIO_WritePin(T_SCK_GPIO_Port,T_SCK_Pin,GPIO_PIN_RESET)
#define	SCK_Out1()		HAL_GPIO_WritePin(T_SCK_GPIO_Port,T_SCK_Pin,GPIO_PIN_SET)

#define	TCS_Out0()		HAL_GPIO_WritePin(TP_CS_GPIO_Port,TP_CS_Pin,GPIO_PIN_RESET)
#define	TCS_Out1()		HAL_GPIO_WritePin(TP_CS_GPIO_Port,TP_CS_Pin,GPIO_PIN_SET)

#define TOUCH_AdjDelay500ms()  HAL_Delay(500)

TouchPointDef TouchPoint;   //����������

TouchParaDef TouchPara;     //������У������,ȫ�־�̬����


//SPIд����
//������д��1byte����
//num:Ҫд�������
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
		SCK_Out1();		//��������Ч
	}
}

//SPI������ ,���ģ��SPI
//�Ӵ�����IC��ȡadcֵ
//CMD:ָ��
//����ֵ:����������	   
uint16_t TOUCH_Read_AD(uint8_t CMD)
{ 	 
	uint8_t count=0;
	uint16_t Num=0;

	SCK_Out0();	//TCLK=0;		//������ʱ��
	MOSI_Out0();	//TDIN=0; 		//����������
	TCS_Out0();		//TCS=0; 		//ѡ�д�����IC
	TOUCH_Write_Byte(CMD);//����������
	Delay_us(6);	//delay_us(6);	//ADS7846��ת��ʱ���Ϊ6us

	SCK_Out0();	//TCLK=0;
	Delay_us(1);	//delay_us(1);
	SCK_Out1();	//TCLK=1;			//��1��ʱ�ӣ����BUSY
	Delay_us(1);	//delay_us(1);
	SCK_Out0();	//TCLK=0;

	for(count=0;count<16;count++)//����16λ����,ֻ�и�12λ��Ч 
	{ 				  
		Num<<=1; 	 
		SCK_Out0();	//TCLK=0;	//�½�����Ч
		Delay_us(1);	//delay_us(1);
		SCK_Out1();	//TCLK=1;
		if (MISO_Read())	//if(DOUT)
			Num++;
	}
	Num>>=4;   		//ֻ�и�12λ��Ч.
	TCS_Out1();		//TCS=1;		//�ͷ�Ƭѡ
	return(Num);   
}

uint16_t TOUCH_ReadData(uint8_t cmd)
{
	uint8_t i, j;
	uint16_t readValue[TOUCH_READ_TIMES], value;
	uint32_t totalValue;

    /* ��ȡTOUCH_READ_TIMES�δ���ֵ */
    for(i=0; i<TOUCH_READ_TIMES; i++)
    {   /* ��Ƭѡ */
//    	TCS_Out0();		//TCS=0;
        /* �ڲ��ģʽ�£�XPT2046ת����Ҫ24��ʱ�ӣ�8��ʱ���������֮��1��ʱ��ȥ�� */
        /* æ�źţ��������12λת�������ʣ��3��ʱ���Ǻ���λ */    
        readValue[i]=TOUCH_Read_AD(cmd); // �������ѡ��X�����Y�� 
//        TCS_Out1();		//TCS=1;
    }

    /* �˲����� */
    /* ���ȴӴ�С���� */
    for(i=0; i<(TOUCH_READ_TIMES - 1); i++)
    {
    	for(j=i+1; j<TOUCH_READ_TIMES; j++)
    	{
    		/* ����ֵ�Ӵ�С�������� */
    		if(readValue[i] < readValue[j])
    		{
    			value = readValue[i];
    			readValue[i] = readValue[j];
    			readValue[j] = value;
    		}
    	}
    }
   
    /* ȥ�����ֵ��ȥ����Сֵ����ƽ��ֵ */
    j = TOUCH_READ_TIMES - 1;
    totalValue = 0;
    for(i=1; i<j; i++)     //��y��ȫ��ֵ
    {
    	totalValue += readValue[i];
    }
    value = totalValue / (TOUCH_READ_TIMES - 2);

    return value;
}

//����ֵΪ0��ʾ�д�������
uint8_t TOUCH_ReadXY(uint16_t *xValue, uint16_t *yValue)
{   
	uint16_t xValue1, yValue1, xValue2, yValue2;

	xValue1 = TOUCH_Read_AD(TOUCH_X_CMD);
	yValue1 = TOUCH_Read_AD(TOUCH_Y_CMD);
	xValue2 = TOUCH_Read_AD(TOUCH_X_CMD);
	yValue2 = TOUCH_Read_AD(TOUCH_Y_CMD);

	/* �鿴������֮��Ĳ���ֵ��� */
	if(xValue1 > xValue2)
		*xValue = xValue1 - xValue2;
	else
		*xValue = xValue2 - xValue1;

	if(yValue1 > yValue2)
		*yValue = yValue1 - yValue2;
	else
		*yValue = yValue2 - yValue1;

	/* �жϲ�����ֵ�Ƿ��ڿɿط�Χ�� */
	if((*xValue > TOUCH_MAX+0) || (*yValue > TOUCH_MAX+0))  
		return 0xFF;

	/* ��ƽ��ֵ */
	*xValue = (xValue1 + xValue2) / 2;
	*yValue = (yValue1 + yValue2) / 2;

	/* �жϵõ���ֵ���Ƿ���ȡֵ��Χ֮�� */
	if((*xValue > TOUCH_X_MAX+0) || (*xValue < TOUCH_X_MIN)
			|| (*yValue > TOUCH_Y_MAX+0) || (*yValue < TOUCH_Y_MIN))
		return 0xFF;
	else
		return 0; 	//��ȡ�ɹ����д��ز���
}

uint8_t TOUCH_ReadAdjust(uint16_t x, uint16_t y, uint16_t *xValue, uint16_t *yValue)
{
	uint8_t i;
	uint32_t timeCont=0;

	/* ��ȡУ��������� */
	LCD_Clear(WHITE);
	LCD_DrowSign(x, y, RED);
	i = 0;
	while(1)
	{
		if(!TOUCH_ReadXY(xValue, yValue))
		{
			i++;
			if(i > 10)         //��ʱһ�£��Զ�ȡ���ֵ
			{
				LCD_DrowSign(x, y, WHITE);
				return 0;
			}

		}
		timeCont++;
		/* ��ʱ�˳� */
		if(timeCont > 0xFFFFFFFE)
		{
			LCD_DrowSign(x, y, WHITE);
			return 0xFF;
		}
	}
}

//������У׼��������LCD��4��������ʾʮ�ַ��ţ��û�������ȡ��������㴥��������
void TOUCH_Adjust(void)
{
	uint16_t px[2], py[2], xPot[4], yPot[4];
	float xFactor, yFactor;

	/* ��ȡ��һ���� */
	if(TOUCH_ReadAdjust(LCD_ADJX_MIN, LCD_ADJY_MIN, &xPot[0], &yPot[0]))
		return;
	TOUCH_AdjDelay500ms();

	/* ��ȡ�ڶ����� */
	if(TOUCH_ReadAdjust(LCD_ADJX_MIN, LCD_ADJY_MAX, &xPot[1], &yPot[1]))
		return;
	TOUCH_AdjDelay500ms();

	/* ��ȡ�������� */
	if(TOUCH_ReadAdjust(LCD_ADJX_MAX, LCD_ADJY_MIN, &xPot[2], &yPot[2]))
		return;
	TOUCH_AdjDelay500ms();

	/* ��ȡ���ĸ��� */
	if(TOUCH_ReadAdjust(LCD_ADJX_MAX, LCD_ADJY_MAX, &xPot[3], &yPot[3]))
		return;
	TOUCH_AdjDelay500ms();

	/* �����ȡ�����ĸ�������ݣ����ϳɶԽǵ������� */
	px[0] = (xPot[0] + xPot[1]) / 2;
	py[0] = (yPot[0] + yPot[2]) / 2;
	px[1] = (xPot[3] + xPot[2]) / 2;
	py[1] = (yPot[3] + yPot[1]) / 2;

	/* ����������� */
	xFactor = (float)LCD_ADJ_X / (px[1] - px[0]);
	yFactor = (float)LCD_ADJ_Y / (py[1] - py[0]);

	/* ���ƫ���� */
	TouchPara.xOffset = (int16_t)LCD_ADJX_MAX - ((float)px[1] * xFactor);
	TouchPara.yOffset = (int16_t)LCD_ADJY_MAX - ((float)py[1] * yFactor);

	/* �����������������ݴ���Ȼ�󱣴� */
	TouchPara.xFactor = xFactor ;
	TouchPara.yFactor = yFactor ;

	TouchPara.isSaved = TOUCH_PARA_SAVED;
}

uint8_t TOUCH_Scan(void)
{
	if(TOUCH_ReadXY(&TouchPoint.Vx, &TouchPoint.Vy)) //û�д���
		return 0xFF;

	/* ������������ֵ���������������ֵ */
	TouchPoint.Lcdx = TouchPoint.Vx * TouchPara.xFactor + TouchPara.xOffset;
	TouchPoint.Lcdy = TouchPoint.Vy * TouchPara.yFactor + TouchPara.yOffset;

	/* �鿴��������ֵ�Ƿ񳬹�������С */
	if(TouchPoint.Lcdx > 240)
		TouchPoint.Lcdx = 240;

	if(TouchPoint.Lcdy > 320)
		TouchPoint.Lcdy = 320;

	return 0;   
}

void TOUCH_ScanAfterINT(void)	// T_PEN�жϺ��ȡ
{
	TouchPoint.Vx = TOUCH_ReadData(TOUCH_X_CMD);
	TouchPoint.Vy = TOUCH_ReadData(TOUCH_Y_CMD);

	/* ������������ֵ���������������ֵ */
	TouchPoint.Lcdx = TouchPoint.Vx * TouchPara.xFactor + TouchPara.xOffset;
	TouchPoint.Lcdy = TouchPoint.Vy * TouchPara.yFactor + TouchPara.yOffset;

	/* �鿴��������ֵ�Ƿ񳬹�������С */
	if(TouchPoint.Lcdx > 240)
		TouchPoint.Lcdx = 240;

	if(TouchPoint.Lcdy > 320)
		TouchPoint.Lcdy = 320;
}
uint16_t LCD_CurX=0;	//��ǰλ��X
uint16_t LCD_CurY=0;	//��ǰλ��Y
void	ShowTouchPara(void)//��ʾ����������
{

	uint16_t  IncY=16;	//Y���

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

void	TouchCalibrate(void)//���д��������ԣ���ȡ����
{
	LCD_ShowString(10,0,240,16,16,(uint8_t*)"**Touch screen calibration");

	LCD_ShowString(10,16,240,16,16,(uint8_t*)"A red cross will display on");
	LCD_ShowString(10,32,240,16,16,(uint8_t*)"the 4 corners of LCD. ");
	LCD_ShowString(10,48,240,16,16,(uint8_t*)"Touch red cross one by one.");
	LCD_ShowString(10,64,240,16,16,(uint8_t*)"Press any key to start...");

	HAL_Delay(3000);
	TOUCH_Adjust();	//������У��ʱ������
	EP24C_WriteLongData(TOUCH_PARA_ADDR, &TouchPara.isSaved, sizeof(TouchPara));

	LCD_CurY=40;
	ShowTouchPara();//��ʾ����������

	LCD_ShowString(10,80,240,16,16,(uint8_t*)"Press any key to enter GUI");
	HAL_Delay(3000);
}
// a�������浱ǰ�����֣�c�������λ��
uint8_t a,c = 0;
extern uint8_t Password[6];
// λ�ú͵ڼ������ֶ�Ӧ
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
	if(c == 6)  // ���������
		LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is too long!");
		HAL_Delay(3000);
		LCD_Fill(10,130, 240, 170, WHITE);
	return 0;
}
// ����һ���������������
uint8_t PasswordIn[6];
// �������������ⴥ����λ���Լ�������Ӧ
uint8_t Touch_Num(void)
{
	// ������ģ������
	uint8_t mqttpub[] = "AT+MQTTPUB=0,\"/sys/iwjauHr3P6U/only_print/thing/event/property/post\",\"{params:{\\\"CurrentTemperature\\\":30}}\",0,0\r\n";
	// ����ֵ
	uint8_t i;
	if(TOUCH_Scan() == 0)  // ������ɨ��
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
		// *������ʾ�����������������һλ
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
		// #������ʾȷ����������
		else if(TouchPoint.Lcdx>160&&TouchPoint.Lcdy>290)
		{
			c = 0;
			for(uint8_t i = 0;i<6;i++)
			{
				// ����������
				if(PasswordIn[i] != Password[i])
				{
					LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is incorrect!");
					HAL_Delay(3000);
					LCD_Fill(0,130, 239, 170, WHITE);
					return 0;
				}
			}
			// ������ȷ
			LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is correct!");
			HAL_Delay(3000);
			LCD_Fill(0,130, 239, 170, WHITE); 
			HAL_UART_Transmit_IT(&ESP8266_UART,mqttpub,sizeof(mqttpub));
			while(__HAL_UART_GET_FLAG(&ESP8266_UART,UART_FLAG_TC)==RESET);//�ȴ��������
			HAL_Delay(100);
		}
	}
	return 0;
}
// ����λ���������λ��
void TOUCH_Command(void)
{
	// ɾ��ָ��
	if(TouchPoint.Lcdx < 80 && TouchPoint.Lcdy > 170&&TouchPoint.Lcdy<200)
	{
		// ��ѡ����ɾ����ָ��
		LCD_ShowString(10,100,240,16,16,(uint8_t*)"Please select the fingerprint!");
		HAL_Delay(3000);
		LCD_Fill(0,100, 240, 170, WHITE);
		// �������������ⴥ����λ���Լ�������Ӧ
		uint8_t i = Touch_Num();
		// ɾ����Ҫɾ����ָ��ID
		if(i>0)
		{
			Del_FR(i);
		}
	}
	// �޸�����
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
			// �����������
			if(PasswordIn[i] != Password[i])
			{
				LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is incorrect!");
				HAL_Delay(3000);
				LCD_Fill(0, 130,240, 165, WHITE);  //�����Ϣ��ʾ��
				return;
			}
		}
		// ������ȷ
		LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is correct!");
		HAL_Delay(3000);
		LCD_Fill(0, 130,240, 165, WHITE);  //�����Ϣ��ʾ��
		// ���������������
		LCD_ShowString(10,130,240,16,16,(uint8_t*)"Please enter a new password!");
		for(c = 0;c<6;)
		{
			c = 0;
			Touch_Num();
			HAL_Delay(200);
		}
		LCD_Fill(0, 130,240, 146, WHITE);  //�����Ϣ��ʾ��
		// ��������޸ĳɹ�
		LCD_ShowString(10,130,240,16,16,(uint8_t*)"The password is OK!");
		
		HAL_Delay(3000);
		LCD_Fill(0, 130,240, 165, WHITE);  //�����Ϣ��ʾ��
		EP24C_WriteLongData(PASSWORD_PARA_ADDR, PasswordIn,sizeof(PasswordIn));
		c = 0;
	}
	// �½�ָ��
	else if(TouchPoint.Lcdx>160&&TouchPoint.Lcdy > 170&&TouchPoint.Lcdy<200)
	{
		// ��ѡ���½�ָ�Ƶ�λ��
		LCD_ShowString(10,100,240,16,16,(uint8_t*)"Select the new fingerprint");
		HAL_Delay(3000);
		LCD_Fill(0,100, 240, 170, WHITE);
		// �������������ⴥ����λ���Լ�������Ӧ
		uint8_t i = Touch_Num();
		if(i>0)
		{
			// ¼��ָ��ID
			Add_FR(i);
		}
	}

}
void Touch_Point(void)
{
	if(TOUCH_Scan() == 0)  // ������ɨ��
	{
		TOUCH_Command();
		Touch_Num();
	}
}

// ΢�뼶��ʱ
void Delay_us(uint16_t delay)
{
	__HAL_TIM_DISABLE(&htim7);
	__HAL_TIM_SET_COUNTER(&htim7,0);  // ���ü���ֵ��ֵ
	__HAL_TIM_ENABLE(&htim7);
	uint16_t curCnt = 0;
	while(1)  // ��ʼ����
	{
		curCnt = __HAL_TIM_GET_COUNTER(&htim7); 
		if(curCnt>delay) 
			break;
	}
	
	__HAL_TIM_DISABLE(&htim7);
}



