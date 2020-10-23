#include <stdlib.h>
#include "joystick.h"
#include "adc.h"
#include "config_param.h"

/********************************************************************************	 
 * ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
 * ALIENTEK MiniFly_Remotor
 * ҡ����������	
 * ����ԭ��@ALIENTEK
 * ������̳:www.openedv.com
 * ��������:2018/6/1
 * �汾��V1.0
 * ��Ȩ���У�����ؾ���
 * Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
 * All rights reserved
********************************************************************************/

//ҡ���м��������ֵ��ADCֵ��
#define MID_DB_THRUST		150	
#define MID_DB_YAW			150	
#define MID_DB_PITCH		150
#define MID_DB_ROLL			150

//ҡ��������������ֵ��ADCֵ��
#define DB_RANGE			10

//��ȡҡ�˷���ʱ�������м�ķ�Χֵ��ADCֵ��
#define DIR_MID_THRUST		800
#define DIR_MID_YAW			800
#define DIR_MID_PITCH		800
#define DIR_MID_ROLL		800

#define THRUST_NUM				1
#define YAW_NUM					0
#define PITCH_NUM				2
#define ROLL_NUM				3	

extern int32_t Receive_length[10];

static bool isInit;
static joystickParam_t* jsParam;

/*ȥ����������*/
int deadband(int value, const int threshold)
{
	if (abs(value) < threshold)
	{
		value = 0;
	}
	else if (value > 0)
	{
		value -= threshold;
	}
	else if (value < 0)
	{
		value += threshold;
	}
	return value;
}

/*ҡ�˳�ʼ��*/
void joystickInit(void)
{
	if(isInit) return;
	Adc_Init();
	jsParam = &configParam.jsParam;
	isInit = true;
}

/*��ȡҡ��ADCֵ*/
void getFlyDataADCValue(joystickFlyui16_t *adcValue)
{
	adcValue->thrust = getAdcValue(ADC_THRUST);
	adcValue->roll = getAdcValue(ADC_ROLL);
	adcValue->pitch = getAdcValue(ADC_PITCH);
	adcValue->yaw = getAdcValue(ADC_YAW);
}

/*PPMֵת���ɷɿ����ݰٷֱ�*/
void PPMtoFlyDataPercent(joystickFlyf_t *percent)
{
	s16 PPMValue;
	
	//THRUST
	PPMValue = Receive_length[THRUST_NUM] - jsParam->thrust.mid;
	PPMValue = deadband(PPMValue,MID_DB_THRUST);
	if(PPMValue>=0)
		percent->thrust = (float)PPMValue/(jsParam->thrust.range_pos-MID_DB_THRUST-DB_RANGE);
	else
		percent->thrust = (float)PPMValue/(jsParam->thrust.range_neg-MID_DB_THRUST-DB_RANGE);
	
	//ROLL
	PPMValue = Receive_length[ROLL_NUM] - jsParam->roll.mid;
	PPMValue = deadband(PPMValue, MID_DB_ROLL);
	if(PPMValue >= 0)
		percent->roll = (float)PPMValue/(jsParam->roll.range_pos-MID_DB_ROLL-DB_RANGE);
	else
		percent->roll = (float)PPMValue/(jsParam->roll.range_neg-MID_DB_ROLL-DB_RANGE);
	
	//PITCH
	PPMValue = Receive_length[PITCH_NUM] - jsParam->pitch.mid;
	PPMValue = deadband(PPMValue, MID_DB_PITCH);
	if(PPMValue >= 0)
		percent->pitch = (float)PPMValue/(jsParam->pitch.range_pos-MID_DB_PITCH-DB_RANGE);
	else
		percent->pitch = (float)PPMValue/(jsParam->pitch.range_neg-MID_DB_PITCH-DB_RANGE);
	
	//YAW
	PPMValue = Receive_length[YAW_NUM] - jsParam->yaw.mid;
	PPMValue = deadband(PPMValue, MID_DB_YAW);
	if(PPMValue >= 0)
		percent->yaw = (float)PPMValue/(jsParam->yaw.range_pos-MID_DB_YAW-DB_RANGE);
	else
		percent->yaw = (float)PPMValue/(jsParam->yaw.range_neg-MID_DB_YAW-DB_RANGE);
}

/*��ȡҡ��1����*/
/*mode:0,��֧��������;1,֧��������*/
enum dir_e getJoystick1Dir(u8 mode)
{
	enum dir_e ret=CENTER;
	joystickFlyui16_t adcValue;
	static bool havebackToCenter = true;
	
	getFlyDataADCValue(&adcValue);
	if(mode) havebackToCenter = true;
	if(havebackToCenter == true)//ҡ�˻ص����м�λ��
	{
		if(adcValue.thrust > (jsParam->thrust.mid+DIR_MID_THRUST))
			ret = FORWARD;
		else if(adcValue.thrust < (jsParam->thrust.mid-DIR_MID_THRUST))
			ret = BACK;
		
		if(ret==BACK && adcValue.yaw>(jsParam->yaw.mid+DIR_MID_YAW))
			ret = BACK_RIGHT;
		else if(ret==BACK && adcValue.yaw<(jsParam->yaw.mid-DIR_MID_YAW))
			ret = BACK_LEFT;
		else if(adcValue.yaw > (jsParam->yaw.mid+DIR_MID_YAW))
			ret = RIGHT;
		else if(adcValue.yaw < (jsParam->yaw.mid-DIR_MID_YAW))
			ret = LEFT;
		
		havebackToCenter = false;//ҡ���뿪���м�λ��
		if(ret == CENTER)//ҡ����Ȼ���м�λ��
			havebackToCenter = true;
	}
	else if( adcValue.thrust >= (jsParam->thrust.mid-DIR_MID_THRUST) &&
			 adcValue.thrust <= (jsParam->thrust.mid+DIR_MID_THRUST) &&
			 adcValue.yaw >= (jsParam->yaw.mid-DIR_MID_YAW) &&
			 adcValue.yaw <= (jsParam->yaw.mid+DIR_MID_YAW) 
		   )//ҡ���뿪���м�λ�ã����ڲ�ѯҡ���Ƿ����
	{
		havebackToCenter = true;
		ret = CENTER;
	}
	
	return ret;
}

/*��ȡҡ��2����*/
/*mode:0,��֧��������;1,֧��������*/
enum dir_e getJoystick2Dir(u8 mode)
{
	enum dir_e ret = CENTER;
	joystickFlyui16_t adcValue;
	static bool havebackToCenter = true;
	
	getFlyDataADCValue(&adcValue);
	if(mode) havebackToCenter = true;
	if(havebackToCenter == true)//ҡ�˻ص����м�λ��
	{	
		if(adcValue.pitch > (jsParam->pitch.mid+DIR_MID_PITCH))
			ret = FORWARD;
		else if(adcValue.pitch < (jsParam->pitch.mid-DIR_MID_PITCH))
			ret = BACK;
		
		if(ret==BACK && adcValue.roll>(jsParam->roll.mid+DIR_MID_ROLL))
			ret = BACK_RIGHT;
		else if(ret==BACK && adcValue.roll<(jsParam->roll.mid-DIR_MID_ROLL))
			ret = BACK_LEFT;
		else if(adcValue.roll>(jsParam->roll.mid+DIR_MID_ROLL))
			ret = RIGHT;
		else if(adcValue.roll<(jsParam->roll.mid-DIR_MID_ROLL))
			ret = LEFT;

		havebackToCenter = false;//ҡ���뿪���м�λ��
		if(ret == CENTER)//ҡ����Ȼ���м�λ��
			havebackToCenter = true;
	}
	else if( adcValue.pitch >= (jsParam->pitch.mid-DIR_MID_PITCH) &&
			 adcValue.pitch <= (jsParam->pitch.mid+DIR_MID_PITCH) &&
			 adcValue.roll >= (jsParam->roll.mid-DIR_MID_ROLL) &&
			 adcValue.roll <= (jsParam->roll.mid+DIR_MID_ROLL) 
		   )//ҡ���뿪���м�λ�ã����ڲ�ѯҡ���Ƿ����
	{
		havebackToCenter = true;
		ret = CENTER;
	}
	
	return ret;
}

