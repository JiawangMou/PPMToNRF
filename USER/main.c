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
 * 本程序只供学习使用，未经作者许可，不得用于其它任何用途
 * ALIENTEK MiniFly_Remotor
 * main.c	
 * 包括系统初始化和创建任务
 * 正点原子@ALIENTEK
 * 技术论坛:www.openedv.com
 * 创建日期:2018/6/1
 * 版本：V1.0
 * 版权所有，盗版必究。
 * Copyright(C) 广州市星翼电子科技有限公司 2014-2024
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
// //while loop time control flag，when Do_flag = 1，while loop can execute
volatile uint8_t Do_Flag = 0;	
u16 T = 0;

int main(void)
{	
	NVIC_SetVectorTable(FIRMWARE_START_ADDR,0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);/*中断配置初始化*/
	delay_init(); 		/*delay初始化*/
	configParamInit();	/*配置参数初始化*/

	/* 禁止JTA使能SWD口 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	DTU_init();
	// ledInit();			/*led初始化*/
	// oledInit(); 			/*oled初始化*/
	// beepInit();			/*蜂鸣器初始化*/
	// keyInit();			/*按键初始化*/
	joystickInit();		/*摇杆初始化*/
	// usb_vcp_init();		/*usb虚拟串口初始化*/
//	radiolinkInit();		/*无线通信初始化*/
	// usblinkInit();		/*usb通信初始化*/
	// displayInit();		/*显示初始化*/
	
	xTaskCreate(startTask, "START_TASK", 100, NULL, 2, &startTaskHandle);/*创建起始任务*/
	vTaskStartScheduler();/*开启任务调度*/
	
	while(1){};/* 任务调度后不会执行到这 */
}

/*创建任务*/
void startTask(void *param)
{
	taskENTER_CRITICAL();	/*进入临界区*/
	
//	xTaskCreate(radiolinkTask, "RADIOLINK", 100, NULL, 6, &radiolinkTaskHandle);/*创建无线连接任务*/
	
	// xTaskCreate(usblinkTxTask, "USBLINK_TX", 100, NULL, 5, NULL);	/*创建usb发送任务*/
	// xTaskCreate(usblinkRxTask, "USBLINK_RX", 100, NULL, 5, NULL);	/*创建usb接收任务*/
	
	xTaskCreate(commanderTask, "COMMANDER", 100, NULL, 4, NULL);	/*创建飞控指令发送任务*/
	
	// xTaskCreate(keyTask, "BUTTON_SCAN", 100, NULL, 3, NULL);		/*创建按键扫描任务*/
	
	// xTaskCreate(displayTask, "DISPLAY", 200, NULL, 1, NULL);		/*创建显示任务*/
	
//	xTaskCreate(configParamTask, "CONFIG_TASK", 100, NULL, 1, NULL);/*创建参数配置任务*/
	
//	xTaskCreate(radiolinkDataProcessTask, "DATA_PROCESS", 100, NULL, 6, NULL);	/*创建无线通信数据处理任务*/
	// xTaskCreate(usblinkDataProcessTask, "DATA_PROCESS", 100, NULL, 6, NULL);	/*创建USB通信数据处理任务*/
	
	vTaskDelete(startTaskHandle);									/*删除开始任务*/
	
	taskEXIT_CRITICAL();	/*退出临界区*/
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
// 				Receive_complete_flag = 1;							//指令接受完成标志位置位
// 			else
// 				memset(&Receive_length, 0, sizeof(Receive_length));	//将Receive_Length数组清零		
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
			Channel_status = 0;										//Command_status ！= END 
		}
		Last_count = New_count;
		Update_Num = 0;
	}
	if(TIM_GetITStatus(TIM4,TIM_IT_Update) == 1)
	{				
		Update_Num++;
		if( Update_Num > 3 )										//接收到的通道指令最大值为大于16000，小于17000，所以TIM4在通道指令没发完前最多可能溢出3次
		{
			if((Channel_Num == 10) && (Receive_Wrong_flag == 0))	//接受完9条通道的指令，且接收到的指令中没有异常的指令（小于8000（最小指令值）和大于16000（最大指令值））
			{
				Receive_complete_flag = 1;							//指令接受完成标志位置位
			}
			else													//如果连续两次计数器溢出还没有一次的捕获，说明在两条指令之间，但是如果指令数不等于10说明前面的数据有通道的指令被丢弃或者误识通道指令，所以后面程序将收到的通道指令全部丢弃；
			{
				memset(&Receive_length, 0, sizeof(Receive_length));	//将Receive_Length数组清零
				Receive_Wrong_flag = 0;
			}
			Channel_status = 1;									//指令接收完成标志位没有置位，不会更新PWM占空比
		}
		TIM_ClearFlag(TIM4,TIM_IT_Update);
	}
}