#include "01_app_include.h"    //应用任务公共头文件

extern bool g_bSDWaitEvent;
extern bool g_bDMXWaitEvent;
//===========================================================================
//任务名称：task_uartsend
//功能概要：指示灯闪烁
//参数说明：未使用
//===========================================================================
void task_uartsend(uint32_t initial_data)
{

	uint_8 framehead[5],frameend[5];
	struct FileInfo fileinfo2;
	uint_32 len, sumLen = 0;
	uint_8  FileParam[2000];    //文件参数数组

	//帧头
	framehead[0] = 0x53;
	framehead[1] = 0x54;
	framehead[2] = 0xFF;

	//帧尾
	frameend[0] = 0x45;
	frameend[1] = 0x54;
	frameend[2] = 0xFF;


    while (TRUE)
    {
    	 _lwevent_wait_for(&lwevent_group, EVENT_UARTSEND, FALSE, NULL);
    	 znFAT_Open_File(&fileinfo2,"/f0_2CH_1.txt", 0, 1);
//		 g_bSDWaitEvent = FALSE;
//		 g_bDMXWaitEvent = FALSE;
//		 _lwevent_clear(&lwevent_group, EVENT_SDHC);      //禁止读SD卡
//		 _lwevent_clear(&lwevent_group, EVENT_DMX512);    //禁止DMX512信号输出
//
//		 //第0帧，内存切换到g_smallrecvfreelist
//		 //切换前必须等待sd、dmx线程阻塞
//		 while(!g_bSDWaitEvent || !g_bDMXWaitEvent)
//		 {
//			 _time_delay_ticks(2);	 //放弃CPU由SD任务执行
//		 }

		 _lwevent_set(&lwevent_group, EVENT_UARTSENDISR);
//    	uart_send1(UART_1, 0x50);
    	while(1)
    	{
			_lwevent_wait_for(&lwevent_group, EVENT_UARTSENDISR, FALSE, NULL);

			len=znFAT_ReadData(&fileinfo2, sumLen, 2000, FileParam);
			if(len!=0)
			{
				//此帧帧长
				framehead[3] = len>>8;
				framehead[4] = len;

				sumLen += len;

				_time_delay_ticks(3);
//				delay_ms(50);
				uart_send1(UART_4, 0xEE);
				uart_sendN (UART_1 ,5 ,framehead);
				uart_sendN (UART_1 ,len ,FileParam);
				uart_sendN (UART_1 ,3 ,frameend);
				uart_send1(UART_4, 0xAA);
				_lwevent_clear(&lwevent_group, EVENT_UARTSENDISR);

				_time_delay_ticks(1);
//				delay_ms(50);
				gpio_set(K64_DIR, 0); //串口变为接收模式，用于接收应答帧
				_time_delay_ticks(1);

				light_change(LIGHT_GREEN);
			}
			else
			{
				//此帧帧长
				framehead[3] = 0x00;
				framehead[4] = 0x05;
				uart_sendN (UART_1 ,5 ,framehead);
				uart_send1 (UART_1, 0xFF);
				uart_send1 (UART_1, 0xFF);
				uart_send1 (UART_1, 0x00);
				uart_send1 (UART_1, 0xFF);
				uart_send1 (UART_1, 0xFF);
				uart_sendN (UART_1 ,3 ,frameend);
				_lwevent_clear(&lwevent_group, EVENT_UARTSENDISR);
				break;
	//    		sumLen=0;
	//    		light_control(LIGHT_RED, LIGHT_OFF);
			}

    	}

    	sumLen=0;
    	znFAT_Close_File(&fileinfo2);
    	_lwevent_clear(&lwevent_group, EVENT_UARTSEND);
    	_lwevent_set(&lwevent_group, EVENT_SDHC);    //允许执行读SD卡任务
//	    _time_delay_ticks(5);			          //延时1s，(放弃CPU控制权)
    }
}


