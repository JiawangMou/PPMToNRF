#include <stdlib.h>
#include "joystick.h"
#include "adc.h"
#include "config_param.h"

/********************************************************************************	 
 * 本程序只供学习使用，未经作者许可，不得用于其它任何用途
 * ALIENTEK MiniFly_Remotor
 * 摇杆驱动代码	
 * 正点原子@ALIENTEK
 * 技术论坛:www.openedv.com
 * 创建日期:2018/6/1
 * 版本：V1.0
 * 版权所有，盗版必究。
 * Copyright(C) 广州市星翼电子科技有限公司 2014-2024
 * All rights reserved
********************************************************************************/

//摇杆中间软件死区值（ADC值）
#define MID_DB_THRUST		150	
#define MID_DB_YAW			150	
#define MID_DB_PITCH		150
#define MID_DB_ROLL			150

//摇杆上下量程死区值（ADC值）
#define DB_RANGE			10

//获取摇杆方向时定义在中间的范围值（ADC值）
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

/*去除死区函数*/
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

/*摇杆初始化*/
void joystickInit(void)
{
	if(isInit) return;
	Adc_Init();
	jsParam = &configParam.jsParam;
	isInit = true;
}

/*获取摇杆ADC值*/
void getFlyDataADCValue(joystickFlyui16_t *adcValue)
{
	adcValue->thrust = getAdcValue(ADC_THRUST);
	adcValue->roll = getAdcValue(ADC_ROLL);
	adcValue->pitch = getAdcValue(ADC_PITCH);
	adcValue->yaw = getAdcValue(ADC_YAW);
}

/*PPM值转换成飞控数据百分比*/
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

/*获取摇杆1方向*/
/*mode:0,不支持连续按;1,支持连续按*/
enum dir_e getJoystick1Dir(u8 mode)
{
	enum dir_e ret=CENTER;
	joystickFlyui16_t adcValue;
	static bool havebackToCenter = true;
	
	getFlyDataADCValue(&adcValue);
	if(mode) havebackToCenter = true;
	if(havebackToCenter == true)//摇杆回到过中间位置
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
		
		havebackToCenter = false;//摇杆离开了中间位置
		if(ret == CENTER)//摇杆依然在中间位置
			havebackToCenter = true;
	}
	else if( adcValue.thrust >= (jsParam->thrust.mid-DIR_MID_THRUST) &&
			 adcValue.thrust <= (jsParam->thrust.mid+DIR_MID_THRUST) &&
			 adcValue.yaw >= (jsParam->yaw.mid-DIR_MID_YAW) &&
			 adcValue.yaw <= (jsParam->yaw.mid+DIR_MID_YAW) 
		   )//摇杆离开了中间位置，现在查询摇杆是否回中
	{
		havebackToCenter = true;
		ret = CENTER;
	}
	
	return ret;
}

/*获取摇杆2方向*/
/*mode:0,不支持连续按;1,支持连续按*/
enum dir_e getJoystick2Dir(u8 mode)
{
	enum dir_e ret = CENTER;
	joystickFlyui16_t adcValue;
	static bool havebackToCenter = true;
	
	getFlyDataADCValue(&adcValue);
	if(mode) havebackToCenter = true;
	if(havebackToCenter == true)//摇杆回到过中间位置
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

		havebackToCenter = false;//摇杆离开了中间位置
		if(ret == CENTER)//摇杆依然在中间位置
			havebackToCenter = true;
	}
	else if( adcValue.pitch >= (jsParam->pitch.mid-DIR_MID_PITCH) &&
			 adcValue.pitch <= (jsParam->pitch.mid+DIR_MID_PITCH) &&
			 adcValue.roll >= (jsParam->roll.mid-DIR_MID_ROLL) &&
			 adcValue.roll <= (jsParam->roll.mid+DIR_MID_ROLL) 
		   )//摇杆离开了中间位置，现在查询摇杆是否回中
	{
		havebackToCenter = true;
		ret = CENTER;
	}
	
	return ret;
}

