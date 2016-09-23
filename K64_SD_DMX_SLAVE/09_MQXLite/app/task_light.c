#include "01_app_include.h"    //应用任务公共头文件


//===========================================================================
//任务名称：task_light
//功能概要：指示灯闪烁
//参数说明：未使用
//===========================================================================
void task_light(uint32_t initial_data)
{
//	spi_enable_re_int(SPI_0);

//	gpio_init(PTA_NUM|17, 1, 1);
//	gpio_set(PTA_NUM|17, 0);
//	spi_init(SPI_0,SLAVE,CPOL_0,CPHA_0);   //K64作为从机同KW01通过SPI0进行通信

    while (TRUE)
    {

//    	gpio_init(PTA_NUM|17, 1, 1);
//    	gpio_set(PTA_NUM|17, 0);
//    	spi_init(SPI_0,SLAVE,CPOL_0,CPHA_0);   //K64作为从机同KW01通过SPI0进行通信

    	light_change(LIGHT_GREEN);                  //红灯亮
	    _time_delay_ticks(200);			          //延时1s，(放弃CPU控制权)
    }
}


