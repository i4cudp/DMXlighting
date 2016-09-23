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
//�������ƣ�task_horserace
//���ܸ�Ҫ�������
//����˵����δʹ��
//===========================================================================
void task_horserace(uint32_t initial)
{	
	//1. ��������ʹ�õı���
    uint_16 i;
    static uint_16 start_light=0;
    static uint_16 end_light=3;
    static uint_8 red_value=0xFF;
    static uint_8 green_value=0x0;
    static uint_8 blue_value=0x0;
    static uint_8 white_value=0x0;
	//2. ���йر�������ֵ

	//3. ��������ѭ����
	while(TRUE) 
	{
		//���¼����û�����--------------------------------------------------------
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
	    _time_delay_ticks(20);			          //��ʱ1s��(����CPU����Ȩ)

	}//����ѭ����end_while
}
