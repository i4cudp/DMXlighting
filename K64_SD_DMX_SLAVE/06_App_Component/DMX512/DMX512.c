//===========================================================================
//�ļ����ƣ�DMX512.h
//���ܸ�Ҫ��DMX512�ź��������
//��Ȩ���У����ݴ�ѧǶ��ʽ����(sumcu.suda.edu.cn)
//�汾���£�2016-04-24
//===========================================================================

#include "DMX512.h"

void ret_zero_send(uint16_t port_pin,uint_8 num);

//===========================================================================
//�������ƣ�Chip_DMX512APN_WriteAddr
//�������أ�0:дַ�ɹ�
//       1:ͨ��������
//       2:����Ŀǰ����дַ����
//����˵����channelNum��ͨ���� 0-8,0ͨ������������;
//       startAddr����ʼ��ַ��12λ����ַ��4096
//       stepChannel��ͨ��������������3����4
//	     chipNum:оƬ��Ŀ��ָ������оƬ��Ŀ
//���ܸ�Ҫ����DMX512APNдַ�����ô��ڷ��ͷ�ʽ��������Ϊ250Kbps,�ȷ��͵͵�ַ�ֽڣ��ٷ��͸��ֽڵ�ַ�������У���롣
//       ����DMX512Э�飬һ��д���оƬ�ĵ�ַ��дַʱ��ο��ĵ�
//===========================================================================
uint8_t Chip_DMX512APN_WriteAddr(uint8_t channelNum, uint16_t startAddr,
		uint8_t stepChannel,uint16_t chipNum)
{
	uint32_t i;
	uint8_t HA,LA,h;
	uint_8 uartnum;
	uint_16 data_tx;

	//ͨ��������,����1
	if(stepChannel !=3 && stepChannel !=4)
		return 1;
	//����Ŀǰ����дַ����������2
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

	gpio_init(data_tx,1,0);   //�ƿش���
	//reset 500ms
	delay_us(500000);
	// after break
	gpio_set(data_tx,1);
	delay_us(10);

	uart_init_Reverse(uartnum, DMX_BOUND);//�������ݴӵ�λ��ʼ����
	uart_send1(uartnum,0);   //start code

	//DMX512AP-NоƬ��дַ�淶
	for(i=0;i<chipNum;i++)  //дchipNum��оƬ�ĵ�ַ
	{
		LA=(uint8_t)startAddr;
		HA=(uint8_t)(startAddr>>8);
		h=~HA;
		h=h<<4;
		HA=HA+h;
		startAddr=startAddr+stepChannel;   //��һ��оƬ��ַ
		uart_send1(uartnum,LA);    //���͵͵�ַ
		uart_send1(uartnum,HA);    //���͸ߵ�ַ
		uart_send1(uartnum,0xD2);  //����У���ֽ�
	}
	return 0;
}

//===========================================================================
//�������ƣ�Chip_SM16512_WriteAddr
//�������أ�0:дַ�ɹ�
//       1:ͨ��������
//       2:����Ŀǰ����дַ����
//����˵����channelNum��ͨ���� 0-8,0ͨ������������;
//       startAddr����ʼ��ַ��12λ����ַ��4096
//       stepChannel��ͨ��������������3����4
//	     chipNum:оƬ��Ŀ��ָ������оƬ��Ŀ
//���ܸ�Ҫ����SM16512дַ�����ô��ڷ��ͷ�ʽ��������Ϊ250Kbps,�ȷ��͵͵�ַ�ֽڣ��ٷ��͸��ֽڵ�ַ��
//       �����У���롣����DMX512Э�飬һ��д���оƬ�ĵ�ַ��дַʱ��ο��ĵ�
//===========================================================================
uint8_t Chip_SM16512_WriteAddr(uint8_t channelNum, uint16_t startAddr,
		uint8_t stepChannel,uint16_t chipNum)
{
	uint32_t i,j;
	uint8_t HA,LA,h;
	uint_8 uartnum;
	uint_16 data_tx;

	uint16_t startAddrtemp=startAddr;
	//ͨ��������,����1
	if(stepChannel !=3 && stepChannel !=4)
		return 1;
	//����Ŀǰ����дַ����������2
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
		gpio_init(data_tx,1,0);   //�ƿش���
		//reset 100ms
		delay_us(100000);
		// after break
		gpio_set(data_tx,1);
		delay_us(12);
		uart_init(uartnum, DMX_BOUND);//�������ݴӵ�λ��ʼ����
		uart_send1(uartnum,0);   //start code
		for(j=0;j<10;j++)
		{
			uart_send1(uartnum,0);   //10�ֽ�ȫ0����
		}
		delay_us(82);
	}
	gpio_init(data_tx,1,0);
	//�����벿��
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
			startAddr=startAddr+stepChannel;   //��һ��оƬ��ַ
			ret_zero_send(data_tx,LA);    //���͵͵�ַ
			ret_zero_send(data_tx,HA);    //���͸ߵ�ַ
			ret_zero_send(data_tx,0xD2);  //����У���ֽ�
	    }
	    startAddr=startAddrtemp;
	}
	return 0;
}
//===========================================================================
//�������ƣ�DMX512_WriteData
//�������أ�0:дַ�ɹ�
//       1:ͨ��������
//       2:����Ŀǰ����дַ����
//       3:ָ��Ϊ�գ�д����ʧ��
//����˵����channelNum��ͨ���� 0-8,0ͨ������������;
//       startAddr����ʼ��ַ��12λ����ַ��4096
//       stepChannel��ͨ��������������3����4
//	     chipNum:оƬ��Ŀ��ָ������оƬ��Ŀ
//       Red,Green,Blue,White:RGB(W)����(����)��ɫ��һ�����ص㣬���ȵȼ���0-255
//       ��stepChannelΪ4��ʱ�򣬷��Ͱ�ɫ����
//���ܸ�Ҫ�����ô��ڷ��ͷ�ʽ��������Ϊ250-750Kbps,ÿ��������RGB�������ص����(����ɫ)
//===========================================================================
uint8_t DMX512_WriteData(uint8_t channelNum, int16_t startAddr,
		uint8_t stepChannel, uint16_t chipNum, uint8_t *pColor)
{
	uint32_t i;
	uint_8 uartnum;
	uint_16 data_tx, rs485dir;

	//ͨ��������,����1
	if(stepChannel != 3 && stepChannel != 4)
		return 1;
	//����Ŀǰ����дַ����������2
	if(chipNum > 683)
		return 2;
	//ָ��Ϊ�գ�д����ʧ�ܣ�����3
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

	gpio_init(rs485dir, 1, 1);   //485���
	// break >88us
	gpio_init(data_tx, 1, 0);   //�ƿش���
	delay_us(92);
	// after break >8us
	gpio_set(data_tx, 1);
	delay_us(10);

	uart_init(uartnum, DMX_BOUND);//��ʼ�����ڣ�����ֹͣλ��250K������
	uart_send1(uartnum, 0);   //start code
	//������ʼ��֮ǰ������
	for(i = 0; i < startAddr - 1; i++)
	{
		uart_send1(uartnum, 0);
	}
    //�����û�����
	for(i = 0; i < chipNum; i++)
	{
		uart_send1(uartnum,pColor[i*stepChannel]);
		uart_send1(uartnum,pColor[i*stepChannel+1]);
		uart_send1(uartnum,pColor[i*stepChannel+2]);
//		if(4 == stepChannel)
//			uart_send1(uartnum, pColor[i * stepChannel + 3]);
	}
	delay_us(1000);
	gpio_set(rs485dir, 0);    //485�л��ɽ���״̬
	return 0;
}

//===========================================================================
//�������ƣ�ret_zero_send
//�������أ���
//����˵����port_pin��port_pin��(�˿ں�)|(���ź�)���磺(PORTB)|(5) ��ʾΪB��5�Žţ�
//       num�����������
//���ܸ�Ҫ�����������
//===========================================================================
void ret_zero_send(uint16_t port_pin,uint_8 num)
{
	uint_8 i;
	gpio_init(port_pin,1,0);   //�ƿش���
	for(i=0;i<8;i++)
	{
		if((num&0x80)!=0)  //�жϸ�λ�Ƿ�Ϊ0����1
		{
			gpio_set(port_pin,1);
			delay_us(30);
			gpio_set(port_pin,0);
			delay_us(111);
		}
		//�ж�λΪ0
		else
		{
			//�ߵ�ƽ��8us
			gpio_set(port_pin,1);
			delay_us(6);
			//�͵�ƽ139us
			gpio_set(port_pin,0);
			delay_us(136);
		}
		num=num<<1;
	}
}
