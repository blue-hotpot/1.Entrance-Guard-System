#ifndef __BSP_KEY_H
#define __BSP_KEY_H	

#include "main.h"
// ������ö�ٱ���
typedef enum
{
			KEY0 = 0,   // û�а�������
			KEY1,
			KEY2,
			KEY3,	
			KEY4,
} KEYS;

#define KEY_WAIT_ALWAYS   0  // ��ΪScanKeys�������������ʾһֱ�ȴ���������
KEYS ScanKeys(uint32_t timeout);






#endif /* __BSP_KEY_H */
