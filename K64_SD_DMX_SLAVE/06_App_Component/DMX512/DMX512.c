//===========================================================================
//文件名称：DMX512.h
//功能概要：DMX512信号输出驱动
//版权所有：苏州大学嵌入式中心(sumcu.suda.edu.cn)
//版本更新：2016-04-24
//===========================================================================

#include "DMX512.h"

void ret_zero_send(uint16_t port_pin,uint_8 num);

//===========================================================================
//函数名称：Chip_DMX512APN_WriteAddr
//函数返回：0:写址成功
//       1:通道数有误
//       2:超过目前最大的写址个数
//参数说明：channelNum：通道号 0-8,0通道用作其他用途
//       startAddr：起始地址，12位最大地址是4096
//       stepChannel：通道数，这里设置3或者4
//	     chipNum:芯片数目，指并联的芯片数目
//功能概要：对DMX512APN写址，设置串口发送方式，波特率为250Kbps,先发送低地址字节，再发送高字节地址，最后发送校验码。
//       符合DMX512协议，一次写多个芯片的地址，写址时序参考文档
//===========================================================================
uint8_t Chip_DMX512APN_WriteAddr(uint8_t channelNum, uint16_t startAddr,
		uint8_t stepChannel,uint16_t chipNum)
{
	uint32_t i;
	uint8_t HA,LA,h;
	uint_8 uartnum;
	uint_16 data_tx;

	//通道数有误,返回1
	if(stepChannel !=3 && stepChannel !=4)
		return 1;
	//超过目前最大的写址个数，返回2
	if(chipNum > 512)
		return 2;

	switch(channelNum)
	{
		case 1:
			data_tx = CH1_TX;
			uartnum = CH1_UART;
			break;
		case 2:
			data_tx = CH2_TX;
			uartnum = CH2_UART;
			break;
		case 3:
			data_tx = CH3_TX;
			uartnum = CH3_UART;
			break;
		case 4:
			data_tx = K64_TX;
			uartnum = K64_UART;
			break;
		case 5:
//			data_tx = CH5_TX;
//			uartnum = CH5_UART;
			break;
	}

	gpio_init(data_tx,1,0);   //灯控串口
	//reset 500ms
	delay_us(500000);
	// after break
	gpio_set(data_tx,1);
	delay_us(10);

	uart_init_Reverse(uartnum, DMX_BOUND);//串口数据从低位开始发送
	uart_send1(uartnum,0);   //start code

	//DMX512AP-N芯片的写址规范
	for(i=0;i<chipNum;i++)  //写chipNum个芯片的地址
	{
		LA=(uint8_t)startAddr;
		HA=(uint8_t)(startAddr>>8);
		h=~HA;
		h=h<<4;
		HA=HA+h;
		startAddr=startAddr+stepChannel;   //下一个芯片地址
		uart_send1(uartnum,LA);    //发送低地址
		uart_send1(uartnum,HA);    //发送高地址
		uart_send1(uartnum,0xD2);  //发送校验字节
	}
	return 0;
}

//===========================================================================
//函数名称：Chip_SM16512_WriteAddr
//函数返回：0:写址成功
//       1:通道数有误
//       2:超过目前最大的写址个数
//参数说明：channelNum：通道号 0-8,0通道用作其他用途
//       startAddr：起始地址，12位最大地址是4096
//       stepChannel：通道数，这里设置3或者4
//	     chipNum:芯片数目，指并联的芯片数目
//功能概要：对SM16512写址，设置串口发送方式，波特率为250Kbps,先发送低地址字节，再发送高字节地址，
//       最后发送校验码。符合DMX512协议，一次写多个芯片的地址，写址时序参考文档
//===========================================================================
uint8_t Chip_SM16512_WriteAddr(uint8_t channelNum, uint16_t startAddr,
		uint8_t stepChannel,uint16_t chipNum)
{
	uint32_t i,j;
	uint8_t HA,LA,h;
	uint_8 uartnum;
	uint_16 data_tx;

	uint16_t startAddrtemp=startAddr;
	//通道数有误,返回1
	if(stepChannel !=3 && stepChannel !=4)
		return 1;
	//超过目前最大的写址个数，返回2
	if(chipNum > 512)
		return 2;

	switch(channelNum)
	{
		case 1:
			data_tx = CH1_TX;
			uartnum = CH1_UART;
			break;
		case 2:
			data_tx = CH2_TX;
			uartnum = CH2_UART;
			break;
		case 3:
			data_tx = CH3_TX;
			uartnum = CH3_UART;
			break;
		case 4:
			data_tx = K64_TX;
			uartnum = K64_UART;
			break;
		case 5:
//			data_tx = CH5_TX;
//			uartnum = CH5_UART;
			break;
	}

	for(i=0;i<4;i++)
	{
		gpio_init(data_tx,1,0);   //灯控串口
		//reset 100ms
		delay_us(100000);
		// after break
		gpio_set(data_tx,1);
		delay_us(12);
		uart_init(uartnum, DMX_BOUND);//串口数据从低位开始发送
		uart_send1(uartnum,0);   //start code
		for(j=0;j<10;j++)
		{
			uart_send1(uartnum,0);   //10字节全0数据
		}
		delay_us(82);
	}
	gpio_init(data_tx,1,0);
	//归零码部分
	delay_us(2800);
	for(i=0;i<3;i++)
	{
		ret_zero_send(data_tx,0);
	}
	for(i=0;i<3;i++)
	{
		gpio_init(data_tx,1,0);
	    delay_us(2000);
	    for(j=0;j<chipNum;j++)
	    {
	    	LA=(uint8_t)startAddr;
			HA=(uint8_t)(startAddr>>8);
			h=~HA;
			h=h<<4;
			h=h&0x80;
			HA=HA+h;
			HA=HA+0x30;
			startAddr=startAddr+stepChannel;   //下一个芯片地址
			ret_zero_send(data_tx,LA);    //发送低地址
			ret_zero_send(data_tx,HA);    //发送高地址
			ret_zero_send(data_tx,0xD2);  //发送校验字节
	    }
	    startAddr=startAddrtemp;
	}
	return 0;
}
//===========================================================================
//函数名称：DMX512_WriteData
//函数返回：0:写址成功
//       1:通道数有误
//       2:超过目前最大的写址个数
//       3:指针为空，写数据失败
//参数说明：channelNum：通道号 0-8,0通道用作其他用途
//       startAddr：起始地址，12位最大地址是4096
//       stepChannel：通道数，这里设置3或者4
//	     chipNum:芯片数目，指并联的芯片数目
//       Red,Green,Blue,White:RGB(W)三种(四种)颜色是一个像素点，亮度等级是0-255
//       当stepChannel为4的时候，发送白色像素
//功能概要：设置串口发送方式，波特率为250-750Kbps,每个像素由RGB三个像素点组成(或四色)
//===========================================================================
uint8_t DMX512_WriteData(uint8_t channelNum, int16_t startAddr,
		uint8_t stepChannel, uint16_t chipNum, uint8_t *pColor)
{
	uint32_t i;
	uint_8 uartnum;
	uint_16 data_tx, rs485dir;

	//通道数有误,返回1
	if(stepChannel != 3 && stepChannel != 4)
		return 1;
	//超过目前最大的写址个数，返回2
	if(chipNum > 683)
		return 2;
	//指针为空，写数据失败，返回3
	if(pColor==0)
        return 3;

	switch(channelNum)
	{
		case 1:
			data_tx = CH1_TX;
			uartnum = CH1_UART;
			rs485dir = CH1_DIR;
			break;
		case 2:
			data_tx = CH2_TX;
			uartnum = CH2_UART;
			rs485dir = CH2_DIR;
			break;
		case 3:
			data_tx = CH3_TX;
			uartnum = CH3_UART;
			rs485dir = CH3_DIR;
			break;
		case 4:
			data_tx = CH4_TX;
			uartnum = CH4_UART;
			rs485dir = CH4_DIR;
			break;
		case 5:
//			data_tx = CH5_TX;
//			uartnum = CH5_UART;
			break;
	}

	gpio_init(rs485dir, 1, 1);   //485输出
	// break >88us
	gpio_init(data_tx, 1, 0);   //灯控串口
	delay_us(92);
	// after break >8us
	gpio_set(data_tx, 1);
	delay_us(10);

	uart_init(uartnum, DMX_BOUND);//初始化串口，两个停止位，250K波特率
	uart_send1(uartnum, 0);   //start code
	//补空起始地之前的数据
	for(i = 0; i < startAddr - 1; i++)
	{
		uart_send1(uartnum, 0);
	}
    //发送用户数据
	for(i = 0; i < chipNum; i++)
	{
		uart_send1(uartnum,pColor[i*stepChannel]);
		uart_send1(uartnum,pColor[i*stepChannel+1]);
		uart_send1(uartnum,pColor[i*stepChannel+2]);
//		if(4 == stepChannel)
//			uart_send1(uartnum, pColor[i * stepChannel + 3]);
	}
	delay_us(1000);
	gpio_set(rs485dir, 0);    //485切换成接收状态
	return 0;
}

//===========================================================================
//函数名称：ret_zero_send
//函数返回：无
//参数说明：port_pin：port_pin：(端口号)|(引脚号)（如：(PORTB)|(5) 表示为B口5号脚）
//       num：输出的数据
//功能概要：归零码输出
//===========================================================================
void ret_zero_send(uint16_t port_pin,uint_8 num)
{
	uint_8 i;
	gpio_init(port_pin,1,0);   //灯控串口
	for(i=0;i<8;i++)
	{
		if((num&0x80)!=0)  //判断该位是否为0，是1
		{
			gpio_set(port_pin,1);
			delay_us(30);
			gpio_set(port_pin,0);
			delay_us(111);
		}
		//判断位为0
		else
		{
			//高电平，8us
			gpio_set(port_pin,1);
			delay_us(6);
			//低电平139us
			gpio_set(port_pin,0);
			delay_us(136);
		}
		num=num<<1;
	}
}
