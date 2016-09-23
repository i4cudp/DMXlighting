/*
 * bsp.h
 *
 *  Created on: Dec 13, 2012
 *      Author: SY
 */

#ifndef BSP_H_
#define BSP_H_

#include "psptypes.h"
#include "uart.h"
#include "gpio.h"

#define BSP_CORE_CLOCK                  (DEFAULT_SYSTEM_CLOCK)  // 内核时钟频率(Hz)
#define BSP_SYSTEM_CLOCK                (DEFAULT_SYSTEM_CLOCK/2)  // 系统时钟频率(Hz)

// SYSTICK的频率
#define BSP_ALARM_FREQUENCY             (200)
// SYSTICK的周期（ms）5ms/tick
#define BSP_ALARM_RESOLUTION            (1000 / BSP_ALARM_FREQUENCY)

// 定义系统滴答定时器中断
#define BSP_TIMER_INTERRUPT_VECTOR      SysTick_IRQn+16   // 来自于芯片头文件
#define BSP_TIMER_INTERRUPT_PRIORITY    0    // CM0中仅有两个可编程优先级级别

typedef void (*const tIsrFunc)(void);

extern uint_32 _bsp_enable_card ( void );

#endif /* BSP_H_ */
