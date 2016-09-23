//===========================================================================
//�ļ����ƣ�task_uart0_re.c
//���ܸ�Ҫ������Uart0��������֡�Ľ�֡����
//��Ȩ���У����ݴ�ѧ��˼����Ƕ��ʽ����(sumcu.suda.edu.cn)
//�汾���£�������Ҫ����
//===========================================================================
#include "01_app_include.h"    //Ӧ�����񹫹�ͷ�ļ�

extern uint_8 __attribute__((section (".m_data_2"))) colorBuf[1024];
extern uint_16 lightnum;
//===========================================================================
//�������ƣ�task_waterlamp
//���ܸ�Ҫ������������ˮ��
//����˵����δʹ��
//===========================================================================
void task_waterlamp(uint32_t initial)
{	
	//1. ��������ʹ�õı���
    uint_16 i;
    static uint_8 red_value=0xFF;
    static uint_8 green_value=0x0;
    static uint_8 blue_value=0x0;
    static uint_8 white_value=0x0;
	//2. ���йر�������ֵ

	//3. ��������ѭ����
	while(TRUE) 
	{
		//���¼����û�����--------------------------------------------------------
		_lwevent_wait_for(&lwevent_group, EVENT_WATERLAMP, FALSE, NULL);

        for(i=0;i<lightnum;i++)
        {
        	colorBuf[i*3]=red_value;
			colorBuf[i*3+1]=green_value;
			colorBuf[i*3+2]=blue_value;
        }

        if(blue_value==0)
        {
        	red_value=red_value-5;
        	green_value=green_value+5;
        }
        if(red_value==0)
        {
        	green_value=green_value-5;
        	blue_value=blue_value+5;
        }
        if(green_value==0)
        {
        	blue_value=blue_value-5;
        	red_value=red_value+5;
        }

	    _time_delay_ticks(20);			          //��ʱ1s��(����CPU����Ȩ)

	}//����ѭ����end_while
}
