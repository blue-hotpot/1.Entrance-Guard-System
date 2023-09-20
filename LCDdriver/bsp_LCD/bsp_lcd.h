/**********************************
*这个文件中主要实现lcd显示相关的函数*
***********************************/
#include "main.h"
#include "lcd.h"

// 画出下方的屏幕中按键
void LCD_Partition(void);
// 绘制上方的部分，这部分用来展示按键的作用
void Key_Press(void);
// 屏幕显示初始化函数
void Bsp_Lcd_Init(void);
// 触摸设置
void Touch_Set(void);
// 指纹设置
void Finger_Set(void);
// 卡片设置
void IDCard_Set(void);
// 密码设置
void Password_Set(void);



