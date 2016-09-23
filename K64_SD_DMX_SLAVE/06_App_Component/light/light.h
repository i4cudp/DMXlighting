/*
 * light.h
 *
 *  Created on: 2015年4月1日
 *      Author: SUMCU
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include "gpio.h"

//指示灯端口及引脚定义
#define LIGHT_RED      (PTD_NUM|3)   //LED1灯使用的端口/引脚
#define LIGHT_GREEN    (PTD_NUM|4)   //LED2灯使用的端口/引脚
#define LIGHT_BLUE     (PTD_NUM|5)   //LED3灯使用的端口/引脚

#define CAMERA_POWER   (PTB_NUM|9)   //CAMERA电源控制引脚
#define KW01_RESET     (PTA_NUM|19)   //KW01复位控制引脚


//灯状态宏定义（灯亮、灯暗对应的物理电平由硬件接法决定）
#define LIGHT_ON        0    //灯亮
#define LIGHT_OFF       1    //灯暗

//=================接口函数声明===============================================
//============================================================================
//函数名称：light_init
//函数参数：port_pin：(端口号)|(引脚号)（如：(PORTB)|(5) 表示为B口5号脚）
//       state：设定小灯状态。由宏定义。
//函数返回：无
//功能概要：指示灯驱动初始化。
//============================================================================
void light_init(uint16_t port_pin, uint8_t state);

//============================================================================
//函数名称：light_control
//函数参数：port_pin：(端口号)|(引脚号)（如：(PORTB)|(5) 表示为B口5号脚）
//       state：设定小灯状态。由宏定义。
//函数返回：无
//功能概要：控制指示灯亮暗。
//============================================================================
void light_control(uint16_t port_pin, uint8_t state);

//============================================================================
//函数名称：light_change
//函数参数：port_pin：(端口号)|(引脚号)（如：(PORTB)|(5) 表示为B口5号脚）
//函数返回：无
//功能概要：切换指示灯亮暗。
//============================================================================
void light_change(uint16_t port_pin);

#endif /* LIGHT_H_ */
