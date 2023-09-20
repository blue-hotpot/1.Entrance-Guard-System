#ifndef __TOUCH_H
#define __TOUCH_H	

#include "main.h"
#include 	"lcd.h"
// 微秒级延时
void Delay_us(uint16_t delay);

/* 定义数据类型 */
// 触摸点数据结构体定义
typedef struct 
{
	uint16_t Vx;	//XPT2046输出的X轴电压值
	uint16_t Vy;	//XPT2046输出的Y轴电压值
	uint16_t Lcdx;	//计算的LCD坐标X
	uint16_t Lcdy;	//计算的LCD坐标Y
} TouchPointDef;
extern TouchPointDef TouchPoint;	//触摸点数据全局变量

// 电阻触摸屏校正参数, 需要保存到EEPROM
typedef struct{
	uint8_t isSaved;    // 参数是否已保存到EEPROM
	int16_t xOffset;	//偏移量
	int16_t yOffset;
	float xFactor;		//相乘因子
	float yFactor;
} TouchParaDef;
extern TouchParaDef TouchPara;     //触摸屏校准参数全局变量

#define TOUCH_PARA_SAVED      'A'    //表示触摸校准参数准备好了
#define TOUCH_PARA_ADDR        80    //校正参数在24C02中的首地址,必须是页的起始地址，也就是8的整数倍


//触摸屏校正,会清除屏幕，依次在屏幕四个角上显示红色十字符号，点击进行测试。
//计算的校准参数保存在变量TouchPara里，并保存到EEPROM
void TOUCH_Adjust(void);

//触摸屏扫描，返回值为0表示有触摸操作,触摸点保存到变量TouchPoint里
uint8_t TOUCH_Scan(void);

void TOUCH_ScanAfterINT(void);	//T_PEN中断后读取

// 新定义的两个函数
//显示触摸屏参数,即全局变量TouchPara的数据
void	ShowTouchPara(void);

//进行触摸屏测试，内部会调用TOUCH_Adjust()
void	TouchCalibrate(void);
// 这个函数用来检测触摸的位置以及做出反应
uint8_t Touch_Num(void);


// 这个函数用来检测触摸的位置以及做出反应
void Touch_Point(void);


#endif /* __TOUCH_H */
