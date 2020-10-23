#include "usart.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "oled.h"
#include "24l01.h"
#include "adc.h"
#include "hw_config.h"
#include "config_param.h"
#include "beep.h"
#include "radiolink.h"
#include "usblink.h"
#include "remoter_ctrl.h"
#include "atkp.h"
#include "display.h"
#include "keyTask.h"
/* FreeRtos includes */
#include "FreeRTOS.h"
#include "task.h"
#include "DTU.h"
#include <stdlib.h>
#include <string.h>

/********************************************************************************	 
 * ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
 * ALIENTEK MiniFly_Remotor
 * main.c	
 * ����ϵͳ��ʼ���ʹ�������
 * ����ԭ��@ALIENTEK
 * ������̳:www.openedv.com
 * ��������:2018/6/1
 * �汾��V1.0
 * ��Ȩ���У�����ؾ���
 * Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
 * All rights reserved
********************************************************************************/

static TaskHandle_t startTaskHandle;
static void startTask(void *param);

int32_t Receive_length[10] = {0};
int32_t receive_length = 0;
volatile uint32_t Last_count = 0;
volatile uint32_t New_count = 0;
volatile uint8_t Channel_status = 1;

volatile uint8_t Channel_Num = 0;
volatile uint8_t Update_Num = 0;
volatile uint8_t Receive_complete_flag = 0;
volatile uint8_t Receive_Wrong_flag = 0;
// //while loop time control flag��when Do_flag = 1��while loop can execute
volatile uint8_t Do_Flag = 0;	
u16 T = 0;

int main(void)
{	
	NVIC_SetVectorTable(FIRMWARE_START_ADDR,0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);/*�ж����ó�ʼ��*/
	delay_init(); 		/*delay��ʼ��*/
	configParamInit();	/*���ò�����ʼ��*/

	/* ��ֹJTAʹ��SWD�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	DTU_init();
	// ledInit();			/*led��ʼ��*/
	// oledInit(); 			/*oled��ʼ��*/
	// beepInit();			/*��������ʼ��*/
	// keyInit();			/*������ʼ��*/
	joystickInit();		/*ҡ�˳�ʼ��*/
	// usb_vcp_init();		/*usb���⴮�ڳ�ʼ��*/
//	radiolinkInit();		/*����ͨ�ų�ʼ��*/
	// usblinkInit();		/*usbͨ�ų�ʼ��*/
	// displayInit();		/*��ʾ��ʼ��*/
	
	xTaskCreate(startTask, "START_TASK", 100, NULL, 2, &startTaskHandle);/*������ʼ����*/
	vTaskStartScheduler();/*�����������*/
	
	while(1){};/* ������Ⱥ󲻻�ִ�е��� */
}

/*��������*/
void startTask(void *param)
{
	taskENTER_CRITICAL();	/*�����ٽ���*/
	
//	xTaskCreate(radiolinkTask, "RADIOLINK", 100, NULL, 6, &radiolinkTaskHandle);/*����������������*/
	
	// xTaskCreate(usblinkTxTask, "USBLINK_TX", 100, NULL, 5, NULL);	/*����usb��������*/
	// xTaskCreate(usblinkRxTask, "USBLINK_RX", 100, NULL, 5, NULL);	/*����usb��������*/
	
	xTaskCreate(commanderTask, "COMMANDER", 100, NULL, 4, NULL);	/*�����ɿ�ָ�������*/
	
	// xTaskCreate(keyTask, "BUTTON_SCAN", 100, NULL, 3, NULL);		/*��������ɨ������*/
	
	// xTaskCreate(displayTask, "DISPLAY", 200, NULL, 1, NULL);		/*������ʾ����*/
	
//	xTaskCreate(configParamTask, "CONFIG_TASK", 100, NULL, 1, NULL);/*����������������*/
	
//	xTaskCreate(radiolinkDataProcessTask, "DATA_PROCESS", 100, NULL, 6, NULL);	/*��������ͨ�����ݴ�������*/
	// xTaskCreate(usblinkDataProcessTask, "DATA_PROCESS", 100, NULL, 6, NULL);	/*����USBͨ�����ݴ�������*/
	
	vTaskDelete(startTaskHandle);									/*ɾ����ʼ����*/
	
	taskEXIT_CRITICAL();	/*�˳��ٽ���*/
}

// void TIM4_IRQHandler()
// {
// 	if( TIM_GetITStatus(TIM4,TIM_IT_CC2) == 1 )
// 	{
// 		New_count = TIM_GetCapture2(TIM4);
// 		receive_length = New_count + ((uint32_t)Update_Num * TIM4_PERIOD) - Last_count;
// 		if(( receive_length < 1000 ) || ( receive_length > 2000 ))
// 		{
// 			if(Channel_Num == 10)
// 				Receive_complete_flag = 1;							//ָ�������ɱ�־λ��λ
// 			else
// 				memset(&Receive_length, 0, sizeof(Receive_length));	//��Receive_Length��������		
// 			Channel_Num = 0;
// 		}else
// 		{
// 			Receive_length[Channel_Num] = receive_length;
// 			Channel_Num++;
// 		}	
// 		Last_count = New_count;
// 		Update_Num = 0;
// 	}
// 	if(TIM_GetITStatus(TIM4,TIM_IT_Update) == 1)
// 	{				
// 		Update_Num++;
// 		TIM_ClearFlag(TIM4,TIM_IT_Update);
// 	}
// }
void TIM4_IRQHandler()
{
	if( TIM_GetITStatus(TIM4,TIM_IT_CC2) == 1 )
	{

	 	New_count = TIM_GetCapture2(TIM4);
		if( Channel_status != 1 )
		{
			Receive_length[Channel_Num] = New_count + ((uint32_t)Update_Num * TIM4_PERIOD) - Last_count;
			if(( Receive_length[Channel_Num] <= 8000 ) || ( Receive_length[Channel_Num] >= 16000 ))
				Receive_Wrong_flag = 1;
			Channel_Num++;
		}else
		{	
			Channel_Num = 0;
			Channel_status = 0;										//Command_status ��= END 
		}
		Last_count = New_count;
		Update_Num = 0;
	}
	if(TIM_GetITStatus(TIM4,TIM_IT_Update) == 1)
	{				
		Update_Num++;
		if( Update_Num > 3 )										//���յ���ͨ��ָ�����ֵΪ����16000��С��17000������TIM4��ͨ��ָ��û����ǰ���������3��
		{
			if((Channel_Num == 10) && (Receive_Wrong_flag == 0))	//������9��ͨ����ָ��ҽ��յ���ָ����û���쳣��ָ�С��8000����Сָ��ֵ���ʹ���16000�����ָ��ֵ����
			{
				Receive_complete_flag = 1;							//ָ�������ɱ�־λ��λ
			}
			else													//����������μ����������û��һ�εĲ���˵��������ָ��֮�䣬�������ָ����������10˵��ǰ���������ͨ����ָ�����������ʶͨ��ָ����Ժ�������յ���ͨ��ָ��ȫ��������
			{
				memset(&Receive_length, 0, sizeof(Receive_length));	//��Receive_Length��������
				Receive_Wrong_flag = 0;
			}
			Channel_status = 1;									//ָ�������ɱ�־λû����λ���������PWMռ�ձ�
		}
		TIM_ClearFlag(TIM4,TIM_IT_Update);
	}
}