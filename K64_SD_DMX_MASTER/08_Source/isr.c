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
    //���ж�
    DISABLE_INTERRUPTS;
    ch = uart_re1(UART_1, &flag);
//    uart_send1(UART_4, ch);
    //��֡�ɹ�,�ô���0������������֡�¼�λ
	if(CreateFrame(ch,uart_recvBuf)!=0)    //�����ڲ�����CreateFrame������֡
	{
		gpio_set(K64_DIR, 1); //���ڱ�Ϊ����ģʽ
		uart_send1(UART_4, 0xCC);
		if(uart_recvBuf[0]==0x53&&uart_recvBuf[1]==0x01&&uart_recvBuf[2]==0x00&&uart_recvBuf[3]==0x55)
		{
			uart_send1(UART_4, 0xDD);
			_lwevent_set(&lwevent_group, EVENT_UARTSENDISR);
		}
	}

    //���ж�
    ENABLE_INTERRUPTS;
}

void UART2_RX_ISR(void)
{
	uint8_t ch;
	uint8_t flag = 1;
	uart_disable_re_int(UART_2);             //�رմ��ڽ����ж�
	ch = uart_re1(UART_2, &flag);
	if(0 == flag)
	{
        uart_send1(UART_4, ch);
	}

    uart_enable_re_int(UART_2);              //�򿪴��ڽ����ж�
}


void UART3_RX_ISR(pointer user_isr_ptr )
{
    uint8_t ch;
    uint8_t flag = 1;
    static uint_8 index;
    //���ж�
    DISABLE_INTERRUPTS;
    ch = uart_re1(UART_3, &flag);
    if (0 == flag)
    {
//    	uart_send1(UART_4, ch);
    	detect_receive[index++] = ch;
    	if(8 == index)
    	{
    		 gpio_set(CH1_DIR, 1);   //������ɣ��л��ɷ���״̬
             index = 0;
    	}
//        uart_send1(UART_4, ch);
    }
    //���ж�
    ENABLE_INTERRUPTS;
}

void UART4_RX_ISR(pointer user_isr_ptr )
{
    uint8_t ch;
    uint8_t flag = 1;
    //���ж�
    DISABLE_INTERRUPTS;
    ch = uart_re1(UART_4, &flag);
    if (0 == flag)
    {
        uart_send1(UART_4, ch);
    }
    //���ж�
    ENABLE_INTERRUPTS;
}

void GPIO_ISR(pointer user_isr_ptr )
{
	//MQX_TICK_STRUCT mqx_tick1,mqx_tick2;
    //���ж�
    DISABLE_INTERRUPTS;

	if(getSn_IR(1) & Sn_IR_RECV)
	{
		//printf("Start enet receive!!\r\n");
		//_time_get_ticks(&mqx_tick1);
		enet_rec(1);
		//_time_get_ticks(&mqx_tick2);
		//printf("Enet receive time = %d\r\n",mqx_tick2.TICKS[0]-mqx_tick1.TICKS[0]);
		//printf("End enet receive!!\r\n");
		setSn_IR(1, Sn_IR_RECV);    //�������ɱ�־
	}
//	PORTE_PCR25 |= PORT_PCR_ISF_MASK; 	                 //���־λ
	PORTB_PCR19 |= PORT_PCR_ISF_MASK;
    //���ж�
    ENABLE_INTERRUPTS;
}


//===========================================================================
//���ƣ�CreateFrame
//���ܸ�Ҫ���齨����֡��������֡���ݼ��뵽����֡��
//����˵����Data��                            ����֡����
//          buffer:        ����֡����
//�������أ���֡״̬    0-��֡δ�ɹ���1-��֡�ɹ�
//��ע��ʮ����������֡��ʽ
//               ֡ͷ       + ���ݳ���         + ��Ч����    +  ֡β
//        FrameHead   +   len   + ��Ч����     +  FrameTail
//===========================================================================
//uint_8 CreateFrame(uint_8 Data,uint_8 * buffer)
//{
//    static uint_8 frameCount=0,chknum=0;    //��֡������
//    uint_8 frameFlag;              //��֡״̬
//    frameFlag=0;            //��֡״̬ ��ʼ��
//    //���ݾ�̬����frameCount��֡
//    if(frameCount==0)
//    {
//    	buffer[0]=Data;    //���
//    	frameCount++;
//    	chknum^=Data;
//    }
//    else
//    {
//    	if(frameCount>=1 && frameCount<buffer[0])
//		{
//			buffer[frameCount]=Data;
//			if(frameCount<(buffer[0]-1))   //����У��ֵʱ��������У��ֵ����
//			{
//	    	    chknum^=Data;
//			}
//			frameCount++;
//		}
//    	else
//		{
//    		if(frameCount>=buffer[0] && chknum == buffer[frameCount-1])
//    		{
//    			frameFlag=1;    //��֡�ɹ�
//    			frameCount=0;   //������0��׼��������֡
//    			chknum=0;       //У��ֵ����
//    		}
//    		else    //��֡ʧ��
//    		{
//    			frameCount=0;   //������0��׼��������֡
//    			chknum=0;       //У��ֵ����
//    		}
//		}
//    }
//    return frameFlag;                 //������֡״̬
//}
#define FrameHead    (0x53)         //֡ͷ
#define FrameTail    (0x55)         //֡β
uint8_t CreateFrame(uint8_t Data,uint8_t * buffer)
{
    static uint8_t frameCount=0;    //��֡������
    uint8_t frameFlag;              //��֡״̬

    frameFlag=0;            //��֡״̬ ��ʼ��
    //���ݾ�̬����frameCount��֡
    switch(frameCount)
    {
        case 0:    //��һ������
        {
            if (Data==FrameHead)    //�յ�������֡ͷFrameHead
            {
                buffer[0]=Data;
                frameCount++;
                frameFlag=0;        //��֡��ʼ
            }
            break;
        }
        case 1:    //�ڶ������ݣ��������������յ����ݸ���
        {
            buffer[1]=Data;
            frameCount++;
            break;
        }
        default:    //�������
        {
            //�ڶ�λ��������Ч���ݳ���,�������������µ�����ֱ��֡βǰһλ
            if(frameCount>=2 && frameCount<=(buffer[1] + 1))
            {
                buffer[frameCount]=Data;
                frameCount++;
                break;
            }

            //����ĩβ������ִ��
            if(frameCount>=(buffer[1]+2))
            {
                if (Data==FrameTail)    //����֡β
                {
                    buffer[frameCount]=Data;     //��֡β0x44���뻺����
                    frameFlag=1;    //��֡�ɹ�
                }
                frameCount=0;     //������0��׼��������֡
                break;
            }
        }
    }     //switch_END
    return frameFlag;                 //������֡״̬
}



void SPI0_ISR()
{
//    uint16_t effects;

    //���ж�
    DISABLE_INTERRUPTS;

    if(1== spi_recieve1(SPI_0,&effects))
    {
    	_lwevent_set(&lwevent_group,EVENT_EFFECTS);
//        SPI_send1_slave(SPI_0, SPIRecData+1);
        printf("[K64]:�������ݣ�%d\n", effects);
    }
    else
    {
        printf("��������ʧ�ܣ�\n");
    }

    //���ж�
    ENABLE_INTERRUPTS;
}

