#ifndef __MENUL3_ITEM_H
#define __MENUL3_ITEM_H
#include <stdint.h>
#include "gui_menu.h"

/********************************************************************************	 
 * ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
 * ALIENTEK MiniFly_Remotor
 * �����˵���������		
 * ����ԭ��@ALIENTEK
 * ������̳:www.openedv.com
 * ��������:2018/6/1
 * �汾��V1.0
 * ��Ȩ���У�����ؾ���
 * Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
 * All rights reserved
********************************************************************************/

extern MenuItem_Typedef  ledringMenu[];
extern MenuItem_Typedef  cameraMenu[];
extern MenuItem_Typedef  opflowMenu[];

void ledringMenuInit(void);
void cameraMenuInit(void);
void vl53lxxMenuInit(void);

#endif /*__MENUL3_ITEM_H*/

