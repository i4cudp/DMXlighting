#include "01_app_include.h"    //Ӧ�����񹫹�ͷ�ļ�

extern bool g_bSDWaitEvent;
extern bool g_bDMXWaitEvent;
//===========================================================================
//�������ƣ�task_uartsend
//���ܸ�Ҫ��ָʾ����˸
//����˵����δʹ��
//===========================================================================
void task_uartsend(uint32_t initial_data)
{

	uint_8 framehead[5],frameend[5];
	struct FileInfo fileinfo2;
	uint_32 len, sumLen = 0;
	uint_8  FileParam[2000];    //�ļ���������

	//֡ͷ
	framehead[0] = 0x53;
	framehead[1] = 0x54;
	framehead[2] = 0xFF;

	//֡β
	frameend[0] = 0x45;
	frameend[1] = 0x54;
	frameend[2] = 0xFF;


    while (TRUE)
    {
    	 _lwevent_wait_for(&lwevent_group, EVENT_UARTSEND, FALSE, NULL);
    	 znFAT_Open_File(&fileinfo2,"/f0_2CH_1.txt", 0, 1);
//		 g_bSDWaitEvent = FALSE;
//		 g_bDMXWaitEvent = FALSE;
//		 _lwevent_clear(&lwevent_group, EVENT_SDHC);      //��ֹ��SD��
//		 _lwevent_clear(&lwevent_group, EVENT_DMX512);    //��ֹDMX512�ź����
//
//		 //��0֡���ڴ��л���g_smallrecvfreelist
//		 //�л�ǰ����ȴ�sd��dmx�߳�����
//		 while(!g_bSDWaitEvent || !g_bDMXWaitEvent)
//		 {
//			 _time_delay_ticks(2);	 //����CPU��SD����ִ��
//		 }

		 _lwevent_set(&lwevent_group, EVENT_UARTSENDISR);
//    	uart_send1(UART_1, 0x50);
    	while(1)
    	{
			_lwevent_wait_for(&lwevent_group, EVENT_UARTSENDISR, FALSE, NULL);

			len=znFAT_ReadData(&fileinfo2, sumLen, 2000, FileParam);
			if(len!=0)
			{
				//��֡֡��
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
				gpio_set(K64_DIR, 0); //���ڱ�Ϊ����ģʽ�����ڽ���Ӧ��֡
				_time_delay_ticks(1);

				light_change(LIGHT_GREEN);
			}
			else
			{
				//��֡֡��
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
    	_lwevent_set(&lwevent_group, EVENT_SDHC);    //����ִ�ж�SD������
//	    _time_delay_ticks(5);			          //��ʱ1s��(����CPU����Ȩ)
    }
}


