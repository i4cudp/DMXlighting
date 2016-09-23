#include "01_app_include.h"    //应用任务公共头文件

extern uint_32 FrameNum;       //视频总帧数
extern uint_16 timeInterval;   //时间间隔
static uint_16 sfcount = 0;

extern uint_16 lightnum;
extern uint_8 colorBuf[4096];
extern bool g_bDMXWaitEvent;
uint8_t effectflag;
void K64_send_cmd(uint_8 cmd);    //发送命令

//===========================================================================
//任务名称：task_DMX512
//功能概要：判断是否超过时间间隔，超过则遍历链表，更新缓冲区，输出DMX512信号
//参数说明：未使用
//===========================================================================
void task_DMX512(uint32_t initial_data)
{
	static uint_32 sfcount = 0;
	static uint_8 scmdsyn = 0;
	uint_8 i;
	TBufNode* pnode = NULL;
    MQX_TICK_STRUCT dmx_mqx_tick;    //用于判断输出dmx信号的时间

	_time_get_ticks(&dmx_mqx_tick);

//	//等待时事件位EVENT_DMX512置位，由task_sdhc任务完成预取过程
//	_lwevent_wait_for(&lwevent_group, EVENT_DMX512, FALSE, NULL);
	
    while(TRUE)
    {
    	g_bDMXWaitEvent = TRUE;

	    //等待时事件位EVENT_DMX512置位，由task_sdhc任务完成预取过程
    	_lwevent_wait_for(&lwevent_group, EVENT_DMX512, FALSE, NULL);
		g_bDMXWaitEvent = FALSE;

		//超过时间间隔，从链表上取新的node显示
		if(is_timeout(dmx_mqx_tick.TICKS[0], 30))
		{
			_time_get_ticks(&dmx_mqx_tick);
			
			for(i = 0; i < CHNUM_DMX512; i++)
	    	{
				pnode = NULL;
				listnode_get(&g_tfdatalist[i], &pnode);
				if(pnode != NULL)
				{
					//通道被选中
					if(pnode->CHSeleted)
					{
						if(effectflag == 0)
						    DMX512_WriteData(pnode->CHNum, 1, 3, (pnode->len)/3, pnode->pbuf);
						else
							DMX512_WriteData(pnode->CHNum, 1, 3, lightnum, colorBuf);
					}
					listnode_put(&g_tffreelist[i], pnode);
				}
	    	}

//			scmdsyn++;
//			sfcount++;
//			//每100帧同步一次
//			if(scmdsyn == 100)
//			{
////				K64_send_cmd(SYN);
//				scmdsyn = 0;
//			}
//			if(sfcount > FrameNum)
//			{
//				sfcount = 0;
//			}
		}

        _time_delay_ticks(10);   //放弃CPU由SD任务执行
    }
}

//===========================================================================
//函数名称：K64_send_cmd
//功能概要：发送控制器之间的命令
//参数说明：cmd命令参数
//===========================================================================
void K64_send_cmd(uint_8 cmd)
{
	uint_8 k, cmd_array[250], chk;

    switch(cmd)
    {
        case SYN:
			cmd_array[0]=0x07;
			cmd_array[1]=0x01;
			cmd_array[2]=(uint_8)(sfcount>>24);
			cmd_array[3]=(uint_8)(sfcount>>16);
			cmd_array[4]=(uint_8)(sfcount>>8);
        	cmd_array[5]=(uint_8)sfcount;
			for(k=0;k<6;k++)
			{
				chk^=cmd_array[k];
			}
			cmd_array[6]=chk;
        	uart_sendN(UART_Debug,7,cmd_array);
    	    break;
        default :
        	break;
    }
}
