#define GLOBLE_VAR

#include "01_app_include.h"

uint_8 DEBUGOUT_LEVEL = DEBUGOUT_LEVEL_INFO;
TBufNode g_tSmallBufNode[SMALL_BUF_NODE_NUM];
TBufNode g_tLargeBufNode[LARGE_BUF_NODE_NUM];

//G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_main_stack[TASK_MAIN_STACK_SIZE];
//G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_light_stack[TASK_LIGHT_STACK_SIZE];
G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_DMX512_stack[TASK_DMX512_STACK_SIZE];
G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_sdhc_stack[TASK_SDHC_STACK_SIZE];
G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_enet_stack[TASK_ENET_STACK_SIZE];
G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_detect_stack[TASK_DETECT_STACK_SIZE];
G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_effects_stack[TASK_EFFECTS_STACK_SIZE];
G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_waterlamp_stack[TASK_WATERLAMP_STACK_SIZE];
G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_horserace_stack[TASK_HORSERACE_STACK_SIZE];
G_VAR_PREFIX uint_8 __attribute__((section (".m_data_2"))) task_uartsend_stack[TASK_UARTSEND_STACK_SIZE];



uint_8 closeVTG[16]={0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,
                         0x05,0x00,0x00,0x00,0x00,0x00,0x01,
                         0x05,0x47};
uint_8 closeGGA[16]={0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,
						 0x00,0x00,0x00,0x00,0x00,0x00,0x01,
						 0x00,0x24};
uint_8 closeGSA[16]={0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,
						 0x02,0x00,0x00,0x00,0x00,0x00,0x01,
						 0x02,0x32};
uint_8 closeGSV[16]={0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,
						 0x03,0x00,0x00,0x00,0x00,0x00,0x00,
						 0x02,0x38};
uint_8 closeGLL[16]={0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,
						 0x01,0x00,0x00,0x00,0x00,0x00,0x00,
						 0x00,0x2A};
//��������1����
uint_8 chgMeaPeriod[14]={0xB5, 0x62, 0x06, 0x08, 0x06, 0x00,
		                 0x60, 0xEA, 0x01, 0x00, 0x01, 0x00,
						 0x60, 0x8C};

//������RMC��䣬�ɸ���u-center���������ܱ������ã�NEO-7��
void gps_set()   //����GPS������
{
	uart_sendN(UART_GPS, 16, closeVTG);
	delay_ms(1);
    uart_sendN(UART_GPS, 16, closeGGA);
    delay_ms(1);
    uart_sendN(UART_GPS, 16, closeGSA);
    delay_ms(1);
    uart_sendN(UART_GPS, 16, closeGSV);
    delay_ms(1);
    uart_sendN(UART_GPS, 16, closeGLL);
    delay_ms(1);
    uart_sendN(UART_GPS, 14, chgMeaPeriod);
}


void Reset_W5500()/*Ӳ����W5500*/
{
     spi_init(SPI_2,MASTER,CPOL_0,CPHA_0);
	 gpio_init(SPI_PSC,GPIO_OUTPUT,0);    //ES1 PSC������ΪIO�����
	 gpio_init(ENET_RESET,GPIO_OUTPUT,0);    //ES1 ģ�鸴λ����
	 delay_us(10);
	 gpio_set(ENET_RESET,1);
	 delay_us(2000000);
}

void set_default(void)
{
  uint_8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x15};
  uint_8 lip[4]={192,168,1,155};
  uint_8 sub[4]={255,255,255,0};
  uint_8 gw[4]={192,168,1,1};
  uint_8 dns[4]={8,8,8,8};
  memcpy(ConfigMsg.lip, lip, 4);
  memcpy(ConfigMsg.sub, sub, 4);
  memcpy(ConfigMsg.gw,  gw, 4);
  memcpy(ConfigMsg.mac, mac,6);
  memcpy(ConfigMsg.dns,dns,4);

  ConfigMsg.dhcp=0;
  ConfigMsg.debug=1;
  ConfigMsg.fw_len=0;

  ConfigMsg.state=NORMAL_STATE;
  ConfigMsg.sw_ver[0]=FW_VER_HIGH;
  ConfigMsg.sw_ver[1]=FW_VER_LOW;

  gpio_init(ENET_INT, GPIO_IN, 0);     //��ʼ��ENETģ��INTn����ΪGPIO���ܣ��������½���ʱ�����ж�
  gpio_enable_int(ENET_INT, FALLING_EDGE);
//  setSIMR(0x02);                       //Socket1�����ж�
//  setSn_IMR(1,0x0F);//�رշ�������ж�

}






//===========================================================================
//�������ƣ�task_main
//���ܸ�Ҫ��MQX������������Ҫʵ��ȫ�ֱ�����ʼ���������������񡢰�װ�û�ISR
//����˵����δʹ��
//===========================================================================
void task_main(uint32_t initial_data)
{
    //1.ȫ�ֱ�����ʼ��
    _lwevent_create(&lwevent_group,0);  //�����������¼���

    //������ʱ����
	uint_16 i,j;
    uint_8* ptemp;

    uint_8  tmpSmallBuf[SMALL_BUF_NODE_SIZE * SMALL_BUF_NODE_NUM];
    uint_8  tmpLargeBuf[LARGE_BUF_NODE_SIZE * LARGE_BUF_NODE_NUM];
	TBufNode* pnode = NULL;
	uint_8  sd_state;
	uint_8  chNodeNum = SMALL_BUF_NODE_NUM / CHNUM_DMX512;

	
    //2. �����ж�
    DISABLE_INTERRUPTS;
    //3.�����ʼ��
    light_init(LIGHT_GREEN,LIGHT_ON);     //��Ƴ�ʼ��

    //ע�⣺����ֹͣλ
    uart_init(UART_Debug,115200);          //��ʼ��"���Դ���"
//    uart_init(UART_ID_ZIGBEE, 115200);
    uart_init(UART_GPS,9600);          //��ʼ��"���Դ���"
    uart_init(UART_1,750000);
    gpio_init(K64_DIR, 1, 1);

    gpio_init(CH1_DIR, 1, 1);

    gpio_init(ENET_POWER, 1, 1);     //������̫����Դ
    delay_us(2000);

	//��������ɫ��ֵ
//	memset(tempColor, 0xFF, buflen);
	
//  printf("Before W5500 initialized !\r\n");

	Reset_W5500();   //Ӳ����W5500

	printf("W5500 initialized!\r\n");

	set_default();

	init_dhcp_client();

	spi_init(SPI_0,SLAVE,CPOL_0,CPHA_0);   //K64��Ϊ�ӻ�ͬKW01ͨ��SPI0����ͨ��

//	gpio_init(GPS_POWER, 1, 1);     //����GPS��Դ
//	delay_ms(2000);

//	gps_set();

	//SDHC��ʼ����znFAT�ļ�ϵͳ��ʼ��
	sd_state = znFAT_Device_Init();   //SD��ϵͳ��ʼ��
	//���ڷ��ͳ�ʼ����ʾ
	if (RES_OK == sd_state)
	{
		printf("SD��ʼ����ɣ�\n");
	}
	else
	{
		printf("SD��ʼ��ʧ�ܣ�\n");
	}
	znFAT_Select_Device(0, &sdInitArgs);
	sd_state = znFAT_Init();   //znFAT�ļ�ϵͳ��ʼ��
	//���ڷ��ͳ�ʼ����ʾ
	if (RES_OK == sd_state)
	{
		printf("�ļ�ϵͳ��ʼ����ɣ�\n");
	}
	else
	{
		printf("�ļ�ϵͳ��ʼ��ʧ�ܣ�\n");
	}

    //��ʼ��ͨ����Ϣ
	ptemp = tmpSmallBuf;
	pnode = g_tSmallBufNode;
    for(i = 0, j = 0; i < SMALL_BUF_NODE_NUM; )
    {
    	//������ͨ���ϵĽڵ㸳���ֳ�ʼֵ
		g_tSmallBufNode[i].pbuf = ptemp;
		g_tSmallBufNode[i].CHSeleted = 0;
		g_tSmallBufNode[i].size = SMALL_BUF_NODE_SIZE;
		g_tSmallBufNode[i].len = 0;
		ptemp = ptemp + SMALL_BUF_NODE_SIZE;

		if(++i % chNodeNum == 0)
		{
			buflist_init(&g_tfdatalist[j], &g_tffreelist[j], pnode, chNodeNum);
			pnode += chNodeNum;
	    	j++;
		}
    }

	ptemp = tmpLargeBuf;
	pnode = g_tLargeBufNode;
    for(i = 0; i < LARGE_BUF_NODE_NUM; i++)
    {
    	//������ͨ���ϵĽڵ㸳���ֳ�ʼֵ
		g_tLargeBufNode[i].pbuf = ptemp;
		g_tLargeBufNode[i].CHSeleted = 0;
		g_tLargeBufNode[i].size = LARGE_BUF_NODE_SIZE;
		g_tLargeBufNode[i].len = 0;
		ptemp = ptemp + LARGE_BUF_NODE_SIZE;
    }
	buflist_init(&g_enetrecvdatalist, &g_largerecvfreelist, g_tLargeBufNode, LARGE_BUF_NODE_NUM);
	g_smallrecvfreelist.next = &g_smallrecvfreelist;
	g_smallrecvfreelist.prev = &g_smallrecvfreelist;


	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[main]: start to create task!");
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");

    //4.������������
    _task_create_at(0, TASK_LIGHT,     0, task_light_stack,     TASK_LIGHT_STACK_SIZE);
	_task_create_at(0, TASK_DMX512,    0, task_DMX512_stack,    TASK_DMX512_STACK_SIZE);
	_task_create_at(0, TASK_SDHC,      0, task_sdhc_stack,      TASK_SDHC_STACK_SIZE);
    _task_create_at(0, TASK_ENET,      0, task_enet_stack,      TASK_ENET_STACK_SIZE);
//    _task_create_at(0, TASK_DETECT,  0, task_detect_stack,    TASK_DETECT_STACK_SIZE);
    _task_create_at(0, TASK_EFFECTS,   0, task_effects_stack,   TASK_EFFECTS_STACK_SIZE);
    _task_create_at(0, TASK_WATERLAMP, 0, task_waterlamp_stack, TASK_WATERLAMP_STACK_SIZE);
    _task_create_at(0, TASK_HORSERACE, 0, task_horserace_stack, TASK_HORSERACE_STACK_SIZE);
    _task_create_at(0, TASK_UARTSEND, 0, task_uartsend_stack, TASK_UARTSEND_STACK_SIZE);

    //5.��װ�û�ISR
    _int_install_isr(UART1_RX_TX_IRQn+16,UART1_RX_ISR,NULL); //ע����Դ��ڵ�ISR
    _int_install_isr(UART2_RX_TX_IRQn+16,UART2_RX_ISR,NULL); //ע��GPS���ڵ�ISR
//    _int_install_isr(UART3_RX_TX_IRQn+16,UART3_RX_ISR,NULL); //ע����Դ��ڵ�ISR
    _int_install_isr(UART4_RX_TX_IRQn+16,UART4_RX_ISR,NULL); //ע����Դ��ڵ�ISR

    _int_install_isr(PORTB_IRQn+16,GPIO_ISR,NULL); //ENET�ж�
    _int_install_isr(SPI0_IRQn+16,SPI0_ISR,NULL); //ע��SPI0��ISR��ͬKW01ͨ��



    //6.ʹ��ģ���жϼ����ж�
    uart_enable_re_int(UART_GPS);     //ʹ�ܵ��Դ��ڽ����ж�
    uart_enable_re_int(UART_Debug);   //ʹ�ܵ��Դ��ڽ����ж�
    uart_enable_re_int(UART_1);   //ʹ�ܼ������ڽ����ж�
    spi_enable_re_int(SPI_0);          //SPI0



    //7.�����ж�
    ENABLE_INTERRUPTS;
    



    //------------ִ����ϣ��������������̬-----------------------------------
    _task_block();
}
