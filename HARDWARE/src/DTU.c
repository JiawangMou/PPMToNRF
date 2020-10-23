#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "DTU.h"


void DTU_init(void)
{
	GPIO_InitTypeDef GPIO_def;
	TIM_ICInitTypeDef TIM_ICInitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);		//打开通用定时器4的时钟 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);			//打开PB引脚的时钟
	
	GPIO_def.GPIO_Pin = GPIO_Pin_7;
	GPIO_def.GPIO_Mode = GPIO_Mode_IPD;							// 
//	GPIO_def.GPIO_OType = GPIO_OType_PP;						//推免输出
	GPIO_def.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB,&GPIO_def);
	

//数传模块接收机油门摇杆推满100，对应脉宽是0.586ms，推到负满量程-100，对应脉宽是1.414ms
	TIM_TimeBaseInitStruct.TIM_Prescaler = (9-1);				//72MHZ/72等于定时器计数器1秒钟计数的次数，也就是1MHZ，那每计数一次时间为1us
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = TIM4_PERIOD;			//计数65536次,对应时间是65.536ms
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1; 	///设置时钟分割:TDTS = Tck_tim


	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;				//设置为通道2
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;	//设置为下降沿检测
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//TIM Input 1, 2, 3 or 4 is selected to be connected to IC1, IC2, IC3 or IC4, respectively
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;			//Capture performed each time an edge is detected on the capture input
	TIM_ICInitStruct.TIM_ICFilter = 3<<4;						//Fsampling = Fck_int, N=8;
	
	TIM_ICInit(TIM4, &TIM_ICInitStruct);		

	
	NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;				//设置初始化的是TIM4的中断	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 4;		//设置抢占优先级为4
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;				//设置响应优先级（子优先级）为0
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;				//使能定时器4这个中断
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM4,TIM_IT_CC2,ENABLE);						//使能TIM4的IC2通道中断
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);

	TIM_Cmd(TIM4, ENABLE);

}



