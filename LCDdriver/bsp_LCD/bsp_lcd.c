#include "bsp_lcd.h"
#include "touch.h"
#include "bsp_key.h"
#include "as608.h"
#include "stdio.h"
#include "RC522.h"
#include "24C02_EEPROM.h"
#include "sim800c.h"
KEYS	curKey;
// 画出下方的屏幕中按键
void LCD_Partition(void)
{
	uint16_t i,x= 0,y = 170;
	POINT_COLOR=RED;
	LCD_Fill(x,y,x+240,y+150,WHITE);
	/* 此处就是画一些条条框框来放置相应的UI值 */
	LCD_DrawRectangle(x,y,x+240,y+150);	//画矩形						   
	LCD_DrawRectangle(x+80,y,x+160,y+150);	 
	LCD_DrawRectangle(x,y+30,x+240,y+60);
	LCD_DrawRectangle(x,y+90,x+240,y+120);
	POINT_COLOR=BLUE;
	LCD_ShowString(x+4,y+7,80,16,16,(uint8_t *)"Del FinPt");// 删除指纹按键	
	LCD_ShowString(x+4+80,y+7,80,16,16,(uint8_t *)"Cge PasWd");// 修改密码
	LCD_ShowString(x+4+80*2,y+7,80,16,16,(uint8_t *)"Cre FinPt");// 新建指纹按键	
	for(i=1;i<10;i++)//数字键盘
	{
		// 显示1至9
		LCD_ShowNum(x+36+((i-1)%3)*80,y+7+30*(((i-1)/3)+1),i,1,16);
	}
	// 显示剩下的# 0 *
	LCD_ShowString(36, y+7+30*4, 8,16,16, (uint8_t *)"*");  // 这个按键用来触摸时，删除错误的按键。
	LCD_ShowNum(80+36, y+7+30*4, 0, 1, 16);
	LCD_ShowString(160+36, y+7+30*4, 8,16,16,(uint8_t *)"#");  // 这个按键用来触摸时，提交密码。
}


// 绘制上方的部分，这部分用来展示按键的作用
/* 按键的作用
key1-触摸设置
key2-指纹设置
key3-卡片设置
key4-密码设置
*/
void Key_Press(void)
{
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[1] Touch settings");
	LCD_ShowString(10,20,240,16,16, (uint8_t*)"[2] Fingerprint settings");
	LCD_ShowString(10,40,240,16,16, (uint8_t*)"[3] IDCard settings");
	LCD_ShowString(10,60, 240,16,16,(uint8_t*)"[4] Password settings");
}


// 屏幕显示初始化函数
void Bsp_Lcd_Init(void)
{
	LCD_Partition();
	Key_Press();
}
// 触摸设置
void Touch_Set(void)
{
	// 清除y方向上从0到80的区域
	LCD_Fill(0, 0, 280, 100, WHITE); 
	// 最上方显示选择的按键
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[1] Touch settings");
	// 触摸校正函数，在touch.c中定义
	TouchCalibrate();
	// 显示完成后重新显示主界面
	HAL_Delay(2000);
	LCD_Fill(0, 0, 280, 100, WHITE);
	Bsp_Lcd_Init();
}

// 指纹设置
void Finger_Set(void)
{
	LCD_Fill(0, 0, 280, 100, WHITE); 
	// 查看指纹
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[1] View the fingerprint");
	// 删除指纹
	LCD_ShowString(10,20,240,16,16, (uint8_t*)"[2] Delete the fingerprint");
	// 添加指纹
	LCD_ShowString(10,40,240,16,16, (uint8_t*)"[3] Add a fingerprint");
	// 重置指纹
	LCD_ShowString(10,60, 240,16,16,(uint8_t*)"[4] Reset the fingerprint");
	uint16_t *ValidN = 0;  // 指纹个数
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
			case	KEY1:// 查看指纹
			{
				printf("查看指纹");
				PS_ValidTempleteNum(ValidN);
				i = 0;
				break;
			}
			case	KEY2:// 删除指纹
			{
				printf("删除指纹");
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
			case	KEY3:// 添加指纹
			{
				printf("添加指纹");
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
			case	KEY4:// 重置指纹
			{
				printf("重置指纹");
				Del_FR_Lib();
				i = 0;
				break;
			}
		}//end switch	
	}
	LCD_Fill(0, 0, 280, 100, WHITE);
	Bsp_Lcd_Init();
}

// 定义一个用来存储卡片类型的数组
uint8_t CardID[4] = {0};
// 卡片设置
void IDCard_Set(void)
{
	LCD_Fill(0, 0, 280, 100, WHITE); 
	// 查看卡片
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[1] View the IDCard");
	// 删除卡片
	LCD_ShowString(10,20,240,16,16, (uint8_t*)"[2] Delete the IDCard");
	// 添加卡片
	LCD_ShowString(10,40,240,16,16, (uint8_t*)"[3] Add a IDCard");
	// 重置卡片
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
			case	KEY1:// 查看卡片
			{
				printf("查看卡片");
				// 创建一个循环，最大放七个卡片
				for(uint8_t j = 0;j<7;j++)
				{
					EP24C_ReadBytes(CARD_ID_ADDR, CardID, sizeof(CardID));
					if(CardID != 0)
					{
						printf("%d存在卡片，卡号:",j+1);
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
			case	KEY2:// 删除卡片
			{
				printf("删除卡片");
				while(i)
				{
					num = Touch_Num();
					if(num != 0)
					{
						// 将选定的位置清0
						EP24C_WriteInOnePage((CARD_ID_ADDR+((num-1)*8)), 0, 8);
						
						
					}
					i = 0;
				}
				break;
			}
			case	KEY3:// 添加卡片
			{
				// 功能测试出错
				// 定义一个用来存储卡片类型的数组
				uint8_t CARD_ID[10] = {0};
				printf("添加卡片");
//					num = Touch_Num();
					// 延时消抖
				HAL_Delay(200);
//					printf("请放卡片");
				while(i)
				{
					// 判断有无卡靠近
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
						// 写入卡ID
						EP24C_WriteInOnePage(CARD_ID_ADDR, CARD_ID,1);
						printf("卡片添加成功\r\n");
						i = 0;
					}	
					HAL_Delay(200);
				}					
				
				break;
			}
			case	KEY4:// 重置卡片
			{
				printf("重置卡片");
				for(uint8_t j = 0;j<7;j++)
				{
					// 将这些卡片的位置全部清0
					EP24C_WriteInOnePage((CARD_ID_ADDR+(j*8)), CardID, 8);
				}
				i = 0;
				break;
			}
		}//end switch	
	}
	
	
	// 操作完成后显示主界面
	LCD_Fill(0, 0, 280, 100, WHITE);
	Bsp_Lcd_Init();
}

// 密码设置
void Password_Set(void)
{
	LCD_Fill(0, 0, 280, 100, WHITE); 
	LCD_ShowString(10,0, 240,16,16,(uint8_t*)"[4] Password settings");
	// 画出按键对应的选项
	uint16_t x= 0,y = 20;
	POINT_COLOR=RED;  // 将颜色设置为红色
	/* 此处就是画一些条条框框来放置相应的UI值 */
	LCD_DrawRectangle(x,y,x+239,y+60);	// 画矩形					    
	POINT_COLOR=BLUE;  // 将颜色设置为蓝色
	// 设置方框内显示内容
	LCD_ShowString(x+20,y+10,240,16,16,(uint8_t *)"View the password");
	LCD_ShowString(x+20,y+30,240,16,16,(uint8_t *)"Please check your phone");  // 查看密码（将密码发送到手机上）
	PassWordPhone();
	
	
}


