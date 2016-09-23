#include "includes.h"
#include "01_app_include.h"

uint_8 CreateFrame(uint_8 Data,uint_8 * buffer);

uint_8 detect_receive[8];
uint16_t effects;
//uint_8 uart_recvBuf[20] = {0};

void UART1_RX_ISR(pointer user_isr_ptr )
{
    uint8_t ch;
    uint8_t flag = 1;
    //关中断
    DISABLE_INTERRUPTS;
    ch = uart_re1(UART_1, &flag);
//    uart_send1(UART_4, ch);
    //组帧成功,置串口0接收完整数据帧事件位
	if(CreateFrame(ch,uart_recvBuf)!=0)    //调用内部函数CreateFrame进行组帧
	{
		gpio_set(K64_DIR, 1); //串口变为发送模式
		uart_send1(UART_4, 0xCC);
		if(uart_recvBuf[0]==0x53&&uart_recvBuf[1]==0x01&&uart_recvBuf[2]==0x00&&uart_recvBuf[3]==0x55)
		{
			uart_send1(UART_4, 0xDD);
			_lwevent_set(&lwevent_group, EVENT_UARTSENDISR);
		}
	}

    //开中断
    ENABLE_INTERRUPTS;
}

void UART2_RX_ISR(void)
{
	uint8_t ch;
	uint8_t flag = 1;
	uart_disable_re_int(UART_2);             //关闭串口接收中断
	ch = uart_re1(UART_2, &flag);
	if(0 == flag)
	{
        uart_send1(UART_4, ch);
	}

    uart_enable_re_int(UART_2);              //打开串口接收中断
}


void UART3_RX_ISR(pointer user_isr_ptr )
{
    uint8_t ch;
    uint8_t flag = 1;
    static uint_8 index;
    //关中断
    DISABLE_INTERRUPTS;
    ch = uart_re1(UART_3, &flag);
    if (0 == flag)
    {
//    	uart_send1(UART_4, ch);
    	detect_receive[index++] = ch;
    	if(8 == index)
    	{
    		 gpio_set(CH1_DIR, 1);   //接收完成，切换成发送状态
             index = 0;
    	}
//        uart_send1(UART_4, ch);
    }
    //开中断
    ENABLE_INTERRUPTS;
}

void UART4_RX_ISR(pointer user_isr_ptr )
{
    uint8_t ch;
    uint8_t flag = 1;
    //关中断
    DISABLE_INTERRUPTS;
    ch = uart_re1(UART_4, &flag);
    if (0 == flag)
    {
        uart_send1(UART_4, ch);
    }
    //开中断
    ENABLE_INTERRUPTS;
}

void GPIO_ISR(pointer user_isr_ptr )
{
	//MQX_TICK_STRUCT mqx_tick1,mqx_tick2;
    //关中断
    DISABLE_INTERRUPTS;

	if(getSn_IR(1) & Sn_IR_RECV)
	{
		//printf("Start enet receive!!\r\n");
		//_time_get_ticks(&mqx_tick1);
		enet_rec(1);
		//_time_get_ticks(&mqx_tick2);
		//printf("Enet receive time = %d\r\n",mqx_tick2.TICKS[0]-mqx_tick1.TICKS[0]);
		//printf("End enet receive!!\r\n");
		setSn_IR(1, Sn_IR_RECV);    //清接收完成标志
	}
//	PORTE_PCR25 |= PORT_PCR_ISF_MASK; 	                 //清标志位
	PORTB_PCR19 |= PORT_PCR_ISF_MASK;
    //开中断
    ENABLE_INTERRUPTS;
}


//===========================================================================
//名称：CreateFrame
//功能概要：组建数据帧，将待组帧数据加入到数据帧中
//参数说明：Data：                            待组帧数据
//          buffer:        数据帧变量
//函数返回：组帧状态    0-组帧未成功，1-组帧成功
//备注：十六进制数据帧格式
//               帧头       + 数据长度         + 有效数据    +  帧尾
//        FrameHead   +   len   + 有效数据     +  FrameTail
//===========================================================================
//uint_8 CreateFrame(uint_8 Data,uint_8 * buffer)
//{
//    static uint_8 frameCount=0,chknum=0;    //组帧计数器
//    uint_8 frameFlag;              //组帧状态
//    frameFlag=0;            //组帧状态 初始化
//    //根据静态变量frameCount组帧
//    if(frameCount==0)
//    {
//    	buffer[0]=Data;    //命令长
//    	frameCount++;
//    	chknum^=Data;
//    }
//    else
//    {
//    	if(frameCount>=1 && frameCount<buffer[0])
//		{
//			buffer[frameCount]=Data;
//			if(frameCount<(buffer[0]-1))   //计算校验值时，不包括校验值本身
//			{
//	    	    chknum^=Data;
//			}
//			frameCount++;
//		}
//    	else
//		{
//    		if(frameCount>=buffer[0] && chknum == buffer[frameCount-1])
//    		{
//    			frameFlag=1;    //组帧成功
//    			frameCount=0;   //计数清0，准备重新组帧
//    			chknum=0;       //校验值清零
//    		}
//    		else    //组帧失败
//    		{
//    			frameCount=0;   //计数清0，准备重新组帧
//    			chknum=0;       //校验值清零
//    		}
//		}
//    }
//    return frameFlag;                 //返回组帧状态
//}
#define FrameHead    (0x53)         //帧头
#define FrameTail    (0x55)         //帧尾
uint8_t CreateFrame(uint8_t Data,uint8_t * buffer)
{
    static uint8_t frameCount=0;    //组帧计数器
    uint8_t frameFlag;              //组帧状态

    frameFlag=0;            //组帧状态 初始化
    //根据静态变量frameCount组帧
    switch(frameCount)
    {
        case 0:    //第一个数据
        {
            if (Data==FrameHead)    //收到数据是帧头FrameHead
            {
                buffer[0]=Data;
                frameCount++;
                frameFlag=0;        //组帧开始
            }
            break;
        }
        case 1:    //第二个数据，该数据是随后接收的数据个数
        {
            buffer[1]=Data;
            frameCount++;
            break;
        }
        default:    //其他情况
        {
            //第二位数据是有效数据长度,根据它接收余下的数据直到帧尾前一位
            if(frameCount>=2 && frameCount<=(buffer[1] + 1))
            {
                buffer[frameCount]=Data;
                frameCount++;
                break;
            }

            //若是末尾数据则执行
            if(frameCount>=(buffer[1]+2))
            {
                if (Data==FrameTail)    //若是帧尾
                {
                    buffer[frameCount]=Data;     //将帧尾0x44存入缓冲区
                    frameFlag=1;    //组帧成功
                }
                frameCount=0;     //计数清0，准备重新组帧
                break;
            }
        }
    }     //switch_END
    return frameFlag;                 //返回组帧状态
}



void SPI0_ISR()
{
//    uint16_t effects;

    //关中断
    DISABLE_INTERRUPTS;

    if(1== spi_recieve1(SPI_0,&effects))
    {
    	_lwevent_set(&lwevent_group,EVENT_EFFECTS);
//        SPI_send1_slave(SPI_0, SPIRecData+1);
        printf("[K64]:接收数据：%d\n", effects);
    }
    else
    {
        printf("接收数据失败！\n");
    }

    //开中断
    ENABLE_INTERRUPTS;
}

