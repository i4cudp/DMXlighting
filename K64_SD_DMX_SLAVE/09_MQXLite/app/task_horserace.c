//===========================================================================
//文件名称：task_uart0_re.c
//功能概要：放置Uart0接收数据帧的解帧任务
//版权所有：苏州大学飞思卡尔嵌入式中心(sumcu.suda.edu.cn)
//版本更新：根据需要更新
//===========================================================================
#include "01_app_include.h"    //应用任务公共头文件


extern uint_8 __attribute__((section (".m_data_2"))) colorBuf[1024];
extern uint_16 lightnum;
//===========================================================================
//任务名称：task_horserace
//功能概要：跑马灯
//参数说明：未使用
//===========================================================================
void task_horserace(uint32_t initial)
{	
	//1. 声明任务使用的变量
    uint_16 i;
    static uint_16 start_light=0;
    static uint_16 end_light=3;
    static uint_8 red_value=0xFF;
    static uint_8 green_value=0x0;
    static uint_8 blue_value=0x0;
    static uint_8 white_value=0x0;
	//2. 给有关变量赋初值

	//3. 进入任务循环体
	while(TRUE) 
	{
		//以下加入用户程序--------------------------------------------------------
		_lwevent_wait_for(&lwevent_group, EVENT_HORSERACE, FALSE, NULL);

        for(i=0;i<lightnum;i++)
        {
			colorBuf[i*3]=0x0;
        	if(i>=start_light && i<=end_light)
        		colorBuf[i*3]=0xFF;
        	colorBuf[i*3+1]=green_value;
        	if(i>=start_light+3 && i<=end_light+3)
        		colorBuf[i*3+1]=0xFF;
        	colorBuf[i*3+2]=blue_value;
        	if(i>=start_light+6 && i<=end_light+6)
        		colorBuf[i*3+2]=0xFF;

        }
        start_light++;
        end_light++;
        if(start_light>lightnum)
        {
        	start_light=0;
        	end_light=3;
        }
	    _time_delay_ticks(20);			          //延时1s，(放弃CPU控制权)

	}//任务循环体end_while
}
