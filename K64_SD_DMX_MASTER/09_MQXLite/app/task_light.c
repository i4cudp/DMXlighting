#include "01_app_include.h"    //Ӧ�����񹫹�ͷ�ļ�


//===========================================================================
//�������ƣ�task_light
//���ܸ�Ҫ��ָʾ����˸
//����˵����δʹ��
//===========================================================================
void task_light(uint32_t initial_data)
{
	int count=0;
//	spi_enable_re_int(SPI_0);

//	gpio_init(PTA_NUM|17, 1, 1);
//	gpio_set(PTA_NUM|17, 0);
//	spi_init(SPI_0,SLAVE,CPOL_0,CPHA_0);   //K64��Ϊ�ӻ�ͬKW01ͨ��SPI0����ͨ��

    while (TRUE)
    {

//    	if(count>=3&&count<5)
//    	{
//    		_lwevent_set(&lwevent_group, EVENT_UARTSEND);
//    		count=10;
//    	}
//    	else
//    		count++;


//    	gpio_init(PTA_NUM|17, 1, 1);
//    	gpio_set(PTA_NUM|17, 0);
//    	spi_init(SPI_0,SLAVE,CPOL_0,CPHA_0);   //K64��Ϊ�ӻ�ͬKW01ͨ��SPI0����ͨ��


    	light_change(LIGHT_GREEN);                  //�����
	    _time_delay_ticks(200);			          //��ʱ1s��(����CPU����Ȩ)
    }
}


