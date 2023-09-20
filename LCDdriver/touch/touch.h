#ifndef __TOUCH_H
#define __TOUCH_H	

#include "main.h"
#include 	"lcd.h"
// ΢�뼶��ʱ
void Delay_us(uint16_t delay);

/* ������������ */
// ���������ݽṹ�嶨��
typedef struct 
{
	uint16_t Vx;	//XPT2046�����X���ѹֵ
	uint16_t Vy;	//XPT2046�����Y���ѹֵ
	uint16_t Lcdx;	//�����LCD����X
	uint16_t Lcdy;	//�����LCD����Y
} TouchPointDef;
extern TouchPointDef TouchPoint;	//����������ȫ�ֱ���

// ���败����У������, ��Ҫ���浽EEPROM
typedef struct{
	uint8_t isSaved;    // �����Ƿ��ѱ��浽EEPROM
	int16_t xOffset;	//ƫ����
	int16_t yOffset;
	float xFactor;		//�������
	float yFactor;
} TouchParaDef;
extern TouchParaDef TouchPara;     //������У׼����ȫ�ֱ���

#define TOUCH_PARA_SAVED      'A'    //��ʾ����У׼����׼������
#define TOUCH_PARA_ADDR        80    //У��������24C02�е��׵�ַ,������ҳ����ʼ��ַ��Ҳ����8��������


//������У��,�������Ļ����������Ļ�ĸ�������ʾ��ɫʮ�ַ��ţ�������в��ԡ�
//�����У׼���������ڱ���TouchPara������浽EEPROM
void TOUCH_Adjust(void);

//������ɨ�裬����ֵΪ0��ʾ�д�������,�����㱣�浽����TouchPoint��
uint8_t TOUCH_Scan(void);

void TOUCH_ScanAfterINT(void);	//T_PEN�жϺ��ȡ

// �¶������������
//��ʾ����������,��ȫ�ֱ���TouchPara������
void	ShowTouchPara(void);

//���д��������ԣ��ڲ������TOUCH_Adjust()
void	TouchCalibrate(void);
// �������������ⴥ����λ���Լ�������Ӧ
uint8_t Touch_Num(void);


// �������������ⴥ����λ���Լ�������Ӧ
void Touch_Point(void);


#endif /* __TOUCH_H */
