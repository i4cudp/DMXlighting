/*
 * tasks.c
 *
 *  Created on: Dec 13, 2012
 *      Author: SY
 */
#include "01_app_include.h"

#define GLOBLE_VAR
// 为自启动任务注册任务栈
const uint_8 * mqx_task_stack_pointers[] = 
{
    task_main_stack,
    NULL
};
// 任务模板列表
const TASK_TEMPLATE_STRUCT MQX_template_list[] =
{
    // 任务编号,       任务函数,         任务栈大小,               优先级, 任务名,             任务属性
    {TASK_MAIN,     task_main,     TASK_MAIN_STACK_SIZE,     7,  "task_main",       MQX_AUTO_START_TASK},
    {TASK_LIGHT,    task_light,    TASK_LIGHT_STACK_SIZE,    12, "task_light",      0},
	{TASK_DMX512,   task_DMX512,   TASK_DMX512_STACK_SIZE,   8,  "task_DMX512",     0},
	{TASK_SDHC,     task_sdhc,     TASK_SDHC_STACK_SIZE,     10, "task_sdhc",       0},
//	{TASK_ENET,     task_enet,     TASK_ENET_STACK_SIZE,     8,  "task_enet",       0},
	{TASK_DETECT,   task_detect,   TASK_DETECT_STACK_SIZE,   11, "task_detect",     0},
	{TASK_EFFECTS,  task_effects,  TASK_EFFECTS_STACK_SIZE,  8,  "task_effects",    0},
	{TASK_WATERLAMP,task_waterlamp,TASK_WATERLAMP_STACK_SIZE,8,  "task_waterlamp",  0},
	{TASK_HORSERACE,task_horserace,TASK_HORSERACE_STACK_SIZE,8,  "task_horserace",  0},
	{TASK_UARTSAVE, task_uartsave, TASK_UARTSAVE_STACK_SIZE, 8,  "task_uartsave",   0},
    {0}
};
