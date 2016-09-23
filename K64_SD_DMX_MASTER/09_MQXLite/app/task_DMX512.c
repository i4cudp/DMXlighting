#include "01_app_include.h"    //Ӧ�����񹫹�ͷ�ļ�

extern uint_32 FrameNum;       //��Ƶ��֡��
extern uint_16 timeInterval;   //ʱ����
static uint_16 sfcount = 0;

extern uint_16 lightnum;
extern uint_8 colorBuf[4096];
extern bool g_bDMXWaitEvent;
uint8_t effectflag;
void K64_send_cmd(uint_8 cmd);    //��������

//===========================================================================
//�������ƣ�task_DMX512
//���ܸ�Ҫ���ж��Ƿ񳬹�ʱ����������������������»����������DMX512�ź�
//����˵����δʹ��
//===========================================================================
void task_DMX512(uint32_t initial_data)
{
	static uint_32 sfcount = 0;
	static uint_8 scmdsyn = 0;
	uint_8 i;
	TBufNode* pnode = NULL;
    MQX_TICK_STRUCT dmx_mqx_tick;    //�����ж����dmx�źŵ�ʱ��

	_time_get_ticks(&dmx_mqx_tick);

//	//�ȴ�ʱ�¼�λEVENT_DMX512��λ����task_sdhc�������Ԥȡ����
//	_lwevent_wait_for(&lwevent_group, EVENT_DMX512, FALSE, NULL);
	
    while(TRUE)
    {
    	g_bDMXWaitEvent = TRUE;

	    //�ȴ�ʱ�¼�λEVENT_DMX512��λ����task_sdhc�������Ԥȡ����
    	_lwevent_wait_for(&lwevent_group, EVENT_DMX512, FALSE, NULL);
		g_bDMXWaitEvent = FALSE;

		//����ʱ��������������ȡ�µ�node��ʾ
		if(is_timeout(dmx_mqx_tick.TICKS[0], 30))
		{
			_time_get_ticks(&dmx_mqx_tick);
			
			for(i = 0; i < CHNUM_DMX512; i++)
	    	{
				pnode = NULL;
				listnode_get(&g_tfdatalist[i], &pnode);
				if(pnode != NULL)
				{
					//ͨ����ѡ��
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
//			//ÿ100֡ͬ��һ��
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

        _time_delay_ticks(10);   //����CPU��SD����ִ��
    }
}

//===========================================================================
//�������ƣ�K64_send_cmd
//���ܸ�Ҫ�����Ϳ�����֮�������
//����˵����cmd�������
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
