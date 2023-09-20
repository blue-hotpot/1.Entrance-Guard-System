#include "bsp_lcd.h"
#include "touch.h"
#include "bsp_key.h"
#include "as608.h"
#include "stdio.h"
#include "RC522.h"
#include "24C02_EEPROM.h"
#include "sim800c.h"
KEYS	curKey;
// �����·�����Ļ�а���
void LCD_Partition(void)
{
	uint16_t i,x= 0,y = 170;
	POINT_COLOR=RED;
	LCD_Fill(x,y,x+240,y+150,WHITE);
	/* �˴����ǻ�һЩ���������������Ӧ��UIֵ */
	LCD_DrawRectangle(x,y,x+240,y+150);	//������						   
	LCD_DrawRectangle(x+80,y,x+160,y+150);	 
	LCD_DrawRectangle(x,y+30,x+240,y+60);
	LCD_DrawRectangle(x,y+90,x+240,y+120);
	POINT_COLOR=BLUE;
	LCD_ShowString(x+4,y+7,80,16,16,(uint8_t *)"Del FinPt");// ɾ��ָ�ư���	
	LCD_ShowString(x+4+80,y+7,80,16,16,(uint8_t *)"Cge PasWd");// �޸�����
	LCD_ShowString(x+4+80*2,y+7,80,16,16,(uint8_t *)"Cre FinPt");// �½�ָ�ư���	
	for(i=1;i<10;i++)//���ּ���
	{
		// ��ʾ1��9
		LCD_ShowNum(x+36+((i-1)%3)*80,y+7+30*(((i-1)/3)+1),i,1,16);
	}
	// ��ʾʣ�µ�# 0 *
	LCD_ShowString(36, y+7+30*4, 8,16,16, (uint8_t *)"*");  // ���������������ʱ��ɾ������İ�����
	LCD_ShowNum(80+36, y+7+30*4, 0, 1, 16);
	LCD_ShowString(160+36, y+7+30*4, 8,16,16,(uint8_t *)"#");  // ���������������ʱ���ύ���롣
}


// �����Ϸ��Ĳ��֣��ⲿ������չʾ����������
/* ����������
key1-��������
key2-ָ������
key3-��Ƭ����
key4-��������
*/
void Key_Press(void)
{
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[1] Touch settings");
	LCD_ShowString(10,20,240,16,16, (uint8_t*)"[2] Fingerprint settings");
	LCD_ShowString(10,40,240,16,16, (uint8_t*)"[3] IDCard settings");
	LCD_ShowString(10,60, 240,16,16,(uint8_t*)"[4] Password settings");
}


// ��Ļ��ʾ��ʼ������
void Bsp_Lcd_Init(void)
{
	LCD_Partition();
	Key_Press();
}
// ��������
void Touch_Set(void)
{
	// ���y�����ϴ�0��80������
	LCD_Fill(0, 0, 280, 100, WHITE); 
	// ���Ϸ���ʾѡ��İ���
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[1] Touch settings");
	// ����У����������touch.c�ж���
	TouchCalibrate();
	// ��ʾ��ɺ�������ʾ������
	HAL_Delay(2000);
	LCD_Fill(0, 0, 280, 100, WHITE);
	Bsp_Lcd_Init();
}

// ָ������
void Finger_Set(void)
{
	LCD_Fill(0, 0, 280, 100, WHITE); 
	// �鿴ָ��
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[1] View the fingerprint");
	// ɾ��ָ��
	LCD_ShowString(10,20,240,16,16, (uint8_t*)"[2] Delete the fingerprint");
	// ���ָ��
	LCD_ShowString(10,40,240,16,16, (uint8_t*)"[3] Add a fingerprint");
	// ����ָ��
	LCD_ShowString(10,60, 240,16,16,(uint8_t*)"[4] Reset the fingerprint");
	uint16_t *ValidN = 0;  // ָ�Ƹ���
	uint8_t num = 0;
	uint8_t i = 1;
	HAL_Delay(100);
	while(i)
	{
		curKey=ScanKeys(KEY_WAIT_ALWAYS);
		switch(curKey)
		{
			case  KEY0:
				break;
			case	KEY1:// �鿴ָ��
			{
				printf("�鿴ָ��");
				PS_ValidTempleteNum(ValidN);
				i = 0;
				break;
			}
			case	KEY2:// ɾ��ָ��
			{
				printf("ɾ��ָ��");
				while(i)
				{
					num = Touch_Num();
					if(num != 0)
					{
						Del_FR(num);
						i = 0;
						break;
					}
				}
				break;
			}
			case	KEY3:// ���ָ��
			{
				printf("���ָ��");
				while(i)
				{
					num = Touch_Num();
					if(num != 0)
					{
						Add_FR(num);
						i = 0;
						break;
					}
				}
				break;
			}
			case	KEY4:// ����ָ��
			{
				printf("����ָ��");
				Del_FR_Lib();
				i = 0;
				break;
			}
		}//end switch	
	}
	LCD_Fill(0, 0, 280, 100, WHITE);
	Bsp_Lcd_Init();
}

// ����һ�������洢��Ƭ���͵�����
uint8_t CardID[4] = {0};
// ��Ƭ����
void IDCard_Set(void)
{
	LCD_Fill(0, 0, 280, 100, WHITE); 
	// �鿴��Ƭ
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[1] View the IDCard");
	// ɾ����Ƭ
	LCD_ShowString(10,20,240,16,16, (uint8_t*)"[2] Delete the IDCard");
	// ��ӿ�Ƭ
	LCD_ShowString(10,40,240,16,16, (uint8_t*)"[3] Add a IDCard");
	// ���ÿ�Ƭ
	LCD_ShowString(10,60, 240,16,16,(uint8_t*)"[4] Reset the IDCard");
	uint8_t i = 1;
	uint8_t num = 0;
	HAL_Delay(100);
	while(i)
	{
		curKey=ScanKeys(KEY_WAIT_ALWAYS);
		switch(curKey)
		{
			case  KEY0:
				break;
			case	KEY1:// �鿴��Ƭ
			{
				printf("�鿴��Ƭ");
				// ����һ��ѭ���������߸���Ƭ
				for(uint8_t j = 0;j<7;j++)
				{
					EP24C_ReadBytes(CARD_ID_ADDR, CardID, sizeof(CardID));
					if(CardID != 0)
					{
						printf("%d���ڿ�Ƭ������:",j+1);
						for(uint8_t	k = 0;k<4;k++)
						{
							printf("%X",CardID[k]);
						}
						printf("\r\n");
						
					}
					
				}
				i = 0;
				break;
			}
			case	KEY2:// ɾ����Ƭ
			{
				printf("ɾ����Ƭ");
				while(i)
				{
					num = Touch_Num();
					if(num != 0)
					{
						// ��ѡ����λ����0
						EP24C_WriteInOnePage((CARD_ID_ADDR+((num-1)*8)), 0, 8);
						
						
					}
					i = 0;
				}
				break;
			}
			case	KEY3:// ��ӿ�Ƭ
			{
				// ���ܲ��Գ���
				// ����һ�������洢��Ƭ���͵�����
				uint8_t CARD_ID[10] = {0};
				printf("��ӿ�Ƭ");
//					num = Touch_Num();
					// ��ʱ����
				HAL_Delay(200);
//					printf("��ſ�Ƭ");
				while(i)
				{
					// �ж����޿�����
					i = PcdRequest(PICC_REQALL,CARD_ID);
					if(i == MI_OK)
					{
						for(uint8_t j = 0;j<4;j++)
						{
							printf("%X",CARD_ID[j]);
						}
						printf("\r\n");
					}
					i = PcdAnticoll(CARD_ID);
					if(i == MI_OK)
					{
						for(uint8_t j = 0;j<4;j++)
						{
							printf("%X",CARD_ID[j]);
						}
						printf("\r\n");
						// д�뿨ID
						EP24C_WriteInOnePage(CARD_ID_ADDR, CARD_ID,1);
						printf("��Ƭ��ӳɹ�\r\n");
						i = 0;
					}	
					HAL_Delay(200);
				}					
				
				break;
			}
			case	KEY4:// ���ÿ�Ƭ
			{
				printf("���ÿ�Ƭ");
				for(uint8_t j = 0;j<7;j++)
				{
					// ����Щ��Ƭ��λ��ȫ����0
					EP24C_WriteInOnePage((CARD_ID_ADDR+(j*8)), CardID, 8);
				}
				i = 0;
				break;
			}
		}//end switch	
	}
	
	
	// ������ɺ���ʾ������
	LCD_Fill(0, 0, 280, 100, WHITE);
	Bsp_Lcd_Init();
}

// ��������
void Password_Set(void)
{
	LCD_Fill(0, 0, 280, 100, WHITE); 
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[4] Password settings");
	// ����������Ӧ��ѡ��
	uint16_t x= 0,y = 20;
	POINT_COLOR=RED;  // ����ɫ����Ϊ��ɫ
	/* �˴����ǻ�һЩ���������������Ӧ��UIֵ */
	LCD_DrawRectangle(x,y,x+239,y+60);	// ������					    
	POINT_COLOR=BLUE;  // ����ɫ����Ϊ��ɫ
	// ���÷�������ʾ����
	LCD_ShowString(x+20,y+10,240,16,16,(uint8_t *)"View the password");
	LCD_ShowString(x+20,y+30,240,16,16,(uint8_t *)"Please check your phone");  // �鿴���루�����뷢�͵��ֻ��ϣ�
	PassWordPhone();
	
	
}


