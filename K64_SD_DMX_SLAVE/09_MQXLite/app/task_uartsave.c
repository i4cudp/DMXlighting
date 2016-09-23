#include "01_app_include.h"    //应用任务公共头文件

struct FileInfo fileinfo2;
struct DateTime dt2;

extern bool g_bSDWaitEvent;
extern bool g_bDMXWaitEvent;

//===========================================================================
//任务名称：task_uartsave
//功能概要：socket建立
//参数说明：未使用
//===========================================================================
void task_uartsave(uint32_t initial_data)
{
//	TBufNode* pnode = NULL;
	uint_32 count=0;
//	uint_8 timeout_flag=0;
//	uint_8 filerec[5000];
//	struct DateTime dt1;
	uint_16 framelen;
	uint_8 framerec[5];

	framerec[0] = 0x53;
	framerec[1] = 0x01;
	framerec[2] = 0x00;
	framerec[3] = 0x55;

	dt2.date.year = 2016;
	dt2.date.month = 9;
	dt2.date.day = 6;
	dt2.time.hour = 17;
	dt2.time.min = 57;
	dt2.time.sec =30;
	framelen=0;
	isuartsaveover=1;

	znFAT_Create_File(&fileinfo2,"/f0_2CH.txt",&dt2);


	//删掉原文件，重新建立一个同名文件
	 if(fileinfo2.File_Size > 0)
	 {
		dt2.date=fileinfo2.File_CDate;
		dt2.time=fileinfo2.File_CTime;
		znFAT_Close_File(&fileinfo2);
		znFAT_Delete_File("/f0_2CH.txt");
		znFAT_Create_File(&fileinfo2,"/f0_2CH.txt",&dt2);
		znFAT_Open_File(&fileinfo2,"/f0_2CH.txt",0,1);
//		/*if(!len)
//		{
//			ack = REVERSE16(eSERVERACK_ERR_NONE);
//			pack = (uint_8 *)&ack;
//			printf("[ServerWriteSDHC]:New File open correct!\r\n");
//
//			//只在收到第0帧的情况下进行ACK
//			ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route,
//				s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
//				acktype, pack, acklen, *ptcmd->pret);
//		}*/
	 }

		znFAT_Open_File(&fileinfo2,"/f0_2CH.txt",0,1);

//	 printf("Write SD times = %d\r\n",count);
	while(1)
	{

		_lwevent_wait_for(&lwevent_group, EVENT_UARTSAVEISR, FALSE, NULL);

		if(uart_recvBuf[3]==0&&uart_recvBuf[4]==5&&uart_recvBuf[5]==0xFF&&uart_recvBuf[6]==0xFF
				&&uart_recvBuf[7]==0x00&&uart_recvBuf[8]==0xFF&&uart_recvBuf[9]==0xFF) //若文件传输结束了
		{
			isuartsaveover=1;
			_lwevent_clear(&lwevent_group, EVENT_UARTSAVEISR);
			_lwevent_set(&lwevent_group, EVENT_SDHC);    //允许执行读SD卡任务
		}
		else
		{
			if(isuartsaveover==1)
			{
				isuartsaveover=0;
				 g_bSDWaitEvent = FALSE;
				 g_bDMXWaitEvent = FALSE;
				 _lwevent_clear(&lwevent_group, EVENT_SDHC);      //禁止读SD卡
				 _lwevent_clear(&lwevent_group, EVENT_DMX512);    //禁止DMX512信号输出

				 //第0帧，内存切换到g_smallrecvfreelist
				 //切换前必须等待sd、dmx线程阻塞
				 while(!g_bSDWaitEvent || !g_bDMXWaitEvent)
				 {
					 _time_delay_ticks(2);	 //放弃CPU由SD任务执行
				 }

				 //删掉原文件，重新建立一个同名文件
				 if(fileinfo2.File_Size >= 0)
				 {
					dt1.date=fileinfo2.File_CDate;
					dt1.time=fileinfo2.File_CTime;
					znFAT_Close_File(&fileinfo2);
					znFAT_Delete_File("/f0_2CH.txt");
					znFAT_Create_File(&fileinfo2,"/f0_2CH.txt",&dt1);
					znFAT_Open_File(&fileinfo2,"/f0_2CH.txt",0,1);
					/*if(!len)
					{
						ack = REVERSE16(eSERVERACK_ERR_NONE);
						pack = (uint_8 *)&ack;
						printf("[ServerWriteSDHC]:New File open correct!\r\n");

						//只在收到第0帧的情况下进行ACK
						ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route,
							s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
							acktype, pack, acklen, *ptcmd->pret);
					}*/
				 }
			}
			framelen = uart_recvBuf[3]<<8|uart_recvBuf[4];
			znFAT_Open_File(&fileinfo2,"/f0_2CH.txt",0,1);
			//关中断
			DISABLE_INTERRUPTS;
			//_time_get_ticks(&mqx_tick1);
			znFAT_WriteData(&fileinfo2, framelen, &uart_recvBuf[5]);
			count++;

			//_time_get_ticks(&mqx_tick2);
			//printf("Write SD time = %d\r\n",mqx_tick2.TICKS[0]-mqx_tick1.TICKS[0]);
			//开中断
			ENABLE_INTERRUPTS;
			//printf("End Write SD!!!\r\n");

//			printf("Write SD times = %d\r\n",count);
			light_change(LIGHT_GREEN);                  //红灯亮
			znFAT_Close_File(&fileinfo2);

			_time_delay_ticks(3);
			uart_send1(UART_4, 0xBB);
//			delay_ms(50);
			uart_sendN(UART_1 ,4 ,framerec);
//			delay_ms(50);
			uart_send1(UART_4, 0xCC);

			_lwevent_clear(&lwevent_group, EVENT_UARTSAVEISR);
			_time_delay_ticks(1);
			gpio_set(K64_DIR, 0);
			_time_delay_ticks(1);   //放弃CPU由SD任务执行




		}
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

		//互斥操作
//		DISABLE_INTERRUPTS;
//		listnode_get(&g_enetrecvdatalist, &pnode);
////		ENABLE_INTERRUPTS;
//		if(pnode != NULL)
//		{
//			if(!is_timeout(uart_mqx_tick.TICKS[0], 400))
//			{
//				if(pnode->len == 1000)
//				{
//
//					//关中断
//					DISABLE_INTERRUPTS;
//					//_time_get_ticks(&mqx_tick1);
//					znFAT_WriteData(&fileinfo2, pnode->len, pnode->pbuf);
//					count++;
//
//					//_time_get_ticks(&mqx_tick2);
//					//printf("Write SD time = %d\r\n",mqx_tick2.TICKS[0]-mqx_tick1.TICKS[0]);
//					//开中断
//					ENABLE_INTERRUPTS;
//					//printf("End Write SD!!!\r\n");
//
//					printf("Write SD times = %d\r\n",count);
//					light_change(LIGHT_GREEN);                  //红灯亮
//				}
//			}
//			else if(timeout_flag==0)
//			{
//				timeout_flag=1;
//				znFAT_Open_File(&fileinfo2,"/f0_2CH1.txt",0,1);
//				//关中断
//				DISABLE_INTERRUPTS;
//				//_time_get_ticks(&mqx_tick1);
//				if(recflag==0)//A缓冲区可写
//				{
//					znFAT_WriteData(&fileinfo2, reccount, filerec_a);
//				}
//				else if(recflag==1)//B缓冲区可写
//				{
//					znFAT_WriteData(&fileinfo2, reccount, filerec_b);
//				}
//
//				count++;
//
//				//_time_get_ticks(&mqx_tick2);
//				//printf("Write SD time = %d\r\n",mqx_tick2.TICKS[0]-mqx_tick1.TICKS[0]);
//				//开中断
//				ENABLE_INTERRUPTS;
//				printf("Timeout Write SD times = %d,%d\r\n",count,reccount);
//				//printf("End Write SD!!!\r\n");
//				znFAT_Close_File(&fileinfo2);
//
//			}
//
//			//归还freelist
//			pnode->len = 0;
//		}
//
//
//
//
//
//		//互斥操作
//		DISABLE_INTERRUPTS;
//		if(pnode->size == LARGE_BUF_NODE_SIZE)
//		{
//			listnode_put(&g_largerecvfreelist, pnode);
//		}
//		ENABLE_INTERRUPTS;


//        _time_delay_ticks(1);   //放弃CPU由SD任务执行

//		pnode = null;
	}
}

