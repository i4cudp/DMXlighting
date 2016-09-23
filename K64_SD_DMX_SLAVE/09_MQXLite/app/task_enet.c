#include "01_app_include.h"    //Ӧ�����񹫹�ͷ�ļ�


//static TBufNode* s_pNodePtr = NULL;

static uint_32 ENETSendFailCount = 0; //ENET��������ʧ�ܴ���
//static TENETInfo s_tENETInfo;
static uint_8 enetstate = STATE_TCP_START;


//��Ӳ���汾��x.x
uint_8 HMC_KINETIS_K60_VER_PROT_MAJOR;
uint_8 HMC_KINETIS_K60_VER_PROT_MINOR;
uint_8 HMC_KINETIS_K60_VER_HARD_MAJOR;
uint_8 HMC_KINETIS_K60_VER_HARD_MINOR;
uint_8 HMC_KINETIS_K60_VER_SOFT_MAJOR;
uint_8 HMC_KINETIS_K60_VER_SOFT_MINOR;

//2.���ն���Ϣ
//TGSM_PARAM g_tgsmParam;

//3.������ʱ��
const uint_8 CompileDate[] = __DATE__;  //����Ԥ�����
const uint_8 CompileTime[] = __TIME__;  //ʱ��Ԥ�����



struct list_head s_comserversenddatalist;     //ͨ�Ų���server������������
struct list_head s_comterminalsenddatalist;   //ͨ�Ų���terminal������������
struct list_head s_comserversendfreelist;     //ͨ�Ų���server���Ϳ�������
static struct list_head s_comterminalsendfreelist;   //ͨ�Ų���terminal���Ϳ�������
static struct list_head s_comterminalrecvdatalist;   //ͨ�Ų���terminal������������
static struct list_head s_comterminalrecvfreelist;   //ͨ�Ų���terminal���տ�������
static struct list_head s_mtnodedatalist;            //�ն˽ڵ�洢��������

static struct list_head s_mtnodefreelist;   //�ն��豸�洢��������

//Zigbeeģ�����flag
bool g_bZigbeeInsert = TRUE;

extern bool b_enet;       //�ļ�δ����
extern bool g_bNodeRet;

//uint_8 ENETRecvDataJuge(const TBufNode* const pnode);
void ENETRecvDataProc();
void ENETSendDataProc();
void AppInfoDump(void);
void VerDump(void);
void TerminalDevAddDelProc(void);
void ZigbeeSendDataProc(const TBufNode* const psndnode);
void AppRecordProc(const void* const pret, uint_8 retlen, uint_8 type);
uint_8 AppRecordWrite(void);
void TerminalMemRead(void);
bool AppRecordRead(void);

//===========================================================================
//�������ƣ�task_enet
//���ܸ�Ҫ��socket����
//����˵����δʹ��
//===========================================================================
void task_enet(uint32_t initial_data)
{
	uint_8 znlen=0;
//	uint_8 enetstate = STATE_TCP_START;
	uint_8 sd_state = 0, NodeNum;
	uint_8 dhcpret = DHCP_RET_NONE;
	uint_16 serverlen,anyport=3000;/*����һ������˿ڲ���ʼ��*/
	uint_8 ip[8];
	uint_16 i;
	uint_8* ptmp = null;  //��ʱ����,���ڳ�ʼ���������
	TBufNode* pnode = NULL;

	//uint_8 ENETRecvBuf[BUF_NODE_NUM_ENET_RECV * BUF_NODE_SIZE_ENET_RECV];
	//TBufNode tENETRecvNode[BUF_NODE_NUM_ENET_RECV];
	uint_8 COMServerSendBuf[BUF_NODE_NUM_COMSERVER_SEND * BUF_NODE_SIZE_COMSERVER_SEND];
	TBufNode tCOMServerSendNode[BUF_NODE_NUM_COMSERVER_SEND];
	TBufNode tCOMTerminalSendNode[BUF_NODE_NUM_COMTERMINAL_SEND];
	uint_8 COMTerminalSendBuf[BUF_NODE_NUM_COMTERMINAL_SEND * BUF_NODE_SIZE_COMTERMINAL_SEND];
	uint_8 COMTerminalRecvBuf[BUF_NODE_NUM_COMTERMINAL_RECV * BUF_NODE_SIZE_COMTERMINAL_RECV];
	TBufNode tCOMTerminalRecvNode[BUF_NODE_NUM_COMTERMINAL_RECV];
	TComInfo tcominfo;
	TNodeInfo tMtNode[NODE_NUM_MAX];



	//ENET���շ��������ݻ����������ʼ��
	//ptmp = ENETRecvBuf;
	//for(i = 0; i < BUF_NODE_NUM_ENET_RECV; i++)
	//{
	//	tENETRecvNode[i].pbuf = ptmp;	//���ݻ���ռ��ַ
	//	tENETRecvNode[i].size = BUF_NODE_SIZE_ENET_RECV;//��������С
	//	tENETRecvNode[i].len = 0;		//ʵ�����ݳ��ȣ�����ʱȷ��
	//	tENETRecvNode[i].type = ENETFRAME_MAX;	    //node���ͣ���Ӧ�ò���ȷ
	//	ptmp += BUF_NODE_SIZE_ENET_RECV;
	//}
	//buflist_init(&enetrecvdatalist, &enetrecvfreelist, tENETRecvNode, BUF_NODE_NUM_ENET_RECV);

	//listnodenum_get(&enetrecvdatalist, &NodeNum);
	//if(0 != NodeNum)
	//{
	//	printf("enetrecvdatalist nodenum = %d", NodeNum);
	//}

	//listnodenum_get(&enetrecvfreelist, &NodeNum);
	//if(0 != NodeNum)
	//{
	//	printf("enetrecvfreelist nodenum = %d", NodeNum);
	//}

	//4.COM�㷢�ͷ��������ݻ����������ʼ��
	ptmp = COMServerSendBuf;
	for(i = 0; i < BUF_NODE_NUM_COMSERVER_SEND; i++)
	{
		tCOMServerSendNode[i].pbuf = ptmp;
		tCOMServerSendNode[i].size = BUF_NODE_SIZE_COMSERVER_SEND;
		tCOMServerSendNode[i].len = 0;
		ptmp += BUF_NODE_SIZE_COMSERVER_SEND;
	}
	buflist_init(&s_comserversenddatalist, &s_comserversendfreelist, tCOMServerSendNode, BUF_NODE_NUM_COMSERVER_SEND);


	//5.COM�㷢���ն����ݻ����������ʼ��
	ptmp = COMTerminalSendBuf;
	for(i = 0; i < BUF_NODE_NUM_COMTERMINAL_SEND; i++)
	{
		tCOMTerminalSendNode[i].pbuf = ptmp;
		tCOMTerminalSendNode[i].size = BUF_NODE_SIZE_COMTERMINAL_SEND;
		tCOMTerminalSendNode[i].len = 0;
		ptmp += BUF_NODE_SIZE_COMTERMINAL_SEND;
	}
	buflist_init(&s_comterminalsenddatalist, &s_comterminalsendfreelist, tCOMTerminalSendNode, BUF_NODE_NUM_COMTERMINAL_SEND);


	//6.COM������ն����ݻ����������ʼ��
	ptmp = COMTerminalRecvBuf;
	for(i = 0; i < BUF_NODE_NUM_COMTERMINAL_RECV; i++)
	{
		tCOMTerminalRecvNode[i].pbuf = ptmp;
		tCOMTerminalRecvNode[i].size = BUF_NODE_SIZE_COMTERMINAL_RECV;
		tCOMTerminalRecvNode[i].len = 0;
		ptmp += BUF_NODE_SIZE_COMTERMINAL_RECV;
	}
	buflist_init(&s_comterminalrecvdatalist, &s_comterminalrecvfreelist, tCOMTerminalRecvNode, BUF_NODE_NUM_COMTERMINAL_RECV);

	//7.�ն��豸�洢��Ϣ�����ʼ��
	s_mtnodedatalist.next = &s_mtnodedatalist;
	s_mtnodedatalist.prev = &s_mtnodedatalist;
	s_mtnodefreelist.next = &s_mtnodefreelist;
	s_mtnodefreelist.prev = &s_mtnodefreelist;
	for(i = 0; i < NODE_NUM_MAX; i++)
		list_add_tail(&tMtNode[i].list, &s_mtnodefreelist);

	//11.�汾��Ϣ��ӡ
	VerDump();

	//12.��ȡ�ڴ��е��ն˽ڵ���Ϣ
	TerminalMemRead();
	//13.ͨ�Ų����ݳ�ʼ��
	tcominfo.pserversenddatalist = &s_comserversenddatalist;
	tcominfo.pserversendfreelist = &s_comserversendfreelist;
	tcominfo.pterminalsenddatalist = &s_comterminalsenddatalist;
	tcominfo.pterminalsendfreelist = &s_comterminalsendfreelist;
	tcominfo.pterminalrecvdatalist = &s_comterminalrecvdatalist;
	tcominfo.pterminalrecvfreelist = &s_comterminalrecvfreelist;
	tcominfo.pterminalnodedatalist = &s_mtnodedatalist;
	tcominfo.pterminalnodefreelist = &s_mtnodefreelist;

	memcpy(&tcominfo.tcomenetinfo, &s_tappinfo.tenetinfo, sizeof(TComENETInfo));
	memcpy(&tcominfo.tcomgprsinfo, &s_tappinfo.tgprsinfo, sizeof(TComGprsInfo));
	memcpy(&tcominfo.tcomlaninfo, &s_tappinfo.tlaninfo, sizeof(TComLanInfo));
	memcpy(&tcominfo.tcomgsminfo, &s_tappinfo.tgsminfo, sizeof(TComGSMInfo));
	tcominfo.pcominfochcallback = AppRecordProc;
	tcominfo.pcomterminalsndcallbak = ZigbeeSendDataProc;
	tcominfo.pcomterminaldevchcallbak = TerminalDevAddDelProc;
	ComInit(&tcominfo);

	//17.��ȡ��������Ӧ����Ϣ(��·�š����ڵ�ַ��·�� ��ʽ)
	if(AppRecordRead())
	{
		//����������Ӧ����ϢFlash
		if(AppRecordWrite())
			DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[main]: AppRecordWrite failed!\r\n");
	}

	memcpy(&tcominfo.tcomenetinfo, &s_tappinfo.tenetinfo, sizeof(TComENETInfo));
	memcpy(&tcominfo.tcomgprsinfo, &s_tappinfo.tgprsinfo, sizeof(TComGprsInfo));
	memcpy(&tcominfo.tcomlaninfo, &s_tappinfo.tlaninfo, sizeof(TComLanInfo));
	memcpy(&tcominfo.tcomgsminfo, &s_tappinfo.tgsminfo, sizeof(TComGSMInfo));

	//18.������Ӧ����Ϣ��ӡ
	AppInfoDump();

	//19.ͨ�Ų��������³�ʼ��
	ComInit(&tcominfo);

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


//	uint_8 pc_ip[4]={10, 10, 65, 100};   //������ַ
//	uint_8 pc_ip[4]={218, 4, 189, 26};   //������ַ
//	uint_8 pc_ip[4]={120, 25, 243, 73};//���˷�����
//	uint_8 pc_ip[4]={121, 40, 158, 140};//����


    while (TRUE)
    {
		switch(enetstate)
		{
			//��ʼdhcp
			case STATE_TCP_START:
				dhcpret = check_DHCP_state(SOCK_DHCP);
				switch(dhcpret)
				{
				  case DHCP_RET_NONE:
					break;
				  case DHCP_RET_TIMEOUT:
					break;
				  case DHCP_RET_UPDATE:
				  	setSHAR(ConfigMsg.mac);//����Mac��ַ
				    setSUBR(ConfigMsg.sub);//������������
				    setGAR(ConfigMsg.gw);//����Ĭ������
				    setSIPR(ConfigMsg.lip);//����Ip��ַ
				    
					getSIPR(ip);
					printf("[task_enet]: dhcp complete, get ip: %d.%d.%d.%d, ", ip[0],ip[1],ip[2],ip[3]);
					getSUBR(ip);
					printf("SN : %d.%d.%d.%d, ", ip[0],ip[1],ip[2],ip[3]);
					getGAR(ip);
					printf("GW : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);

					enetstate = STATE_TCP_LINK; 
					break;
				  case DHCP_RET_CONFLICT:
					while(1);
				  default:
					break;
				}
				break;

			//DHCP��ɣ���ʼtcp����
			case STATE_TCP_LINK:
				switch(getSn_SR(tcominfo.tcomenetinfo.enetsock))   //��ȡsocket1��״̬
				{
				   case SOCK_INIT:    //socket��ʼ�����
					 printf("[task_enet]: start tcp link��server: %d.%d.%d.%d\r\n",
							 tcominfo.tcomenetinfo.enetdstip[0],tcominfo.tcomenetinfo.enetdstip[1],
							 tcominfo.tcomenetinfo.enetdstip[2],tcominfo.tcomenetinfo.enetdstip[3]);

					 connect(tcominfo.tcomenetinfo.enetsock, tcominfo.tcomenetinfo.enetdstip,
							 tcominfo.tcomenetinfo.enetdstport); //��TCPģʽ���������������������
					 break;
					 
				   case SOCK_ESTABLISHED:   //socket���ӽ���
					 printf("[task_enet]: start tcp link ok!\r\n");
					 enetstate = STATE_TCP_FINISH;
					 break;
				   case SOCK_CLOSE_WAIT:   //socket�ȴ��ر�״̬
					 break;
				   case SOCK_CLOSED:       //socket�ر�
				   	 printf("[task_enet]: restart tcp link\r\n");
					 socket(tcominfo.tcomenetinfo.enetsock,Sn_MR_TCP,anyport++,Sn_MR_ND);   //��socket1��һ���˿�
					 break;
				 }
				break;

			//�������
			case STATE_TCP_FINISH:
				 //printf("[task_enet]: hhl 111111\r\n");

				 //�� �����ж�
				 if(getSn_IR(tcominfo.tcomenetinfo.enetsock) & Sn_IR_CON)
				 {
					setSn_IR(tcominfo.tcomenetinfo.enetsock, Sn_IR_CON);
				 }

				 //printf("[task_enet]: hhl 22222222222\r\n");
				 
				 setSIMR(0x02);   //SOCKET1���ж����룬�����ж�

				 //printf("[task_enet]: hhl 3333333333\r\n");
				 setSn_IMR(tcominfo.tcomenetinfo.enetsock,0x04);//�رշ�������ж�

				 //printf("[task_enet]: hhl 4444444444\r\n");

				 //����������
				 ComHeartProc();
				 //printf("[task_enet]: hhl 5555555555\r\n");

                 //����ENET�յ������ݰ�
			     ENETRecvDataProc();
				 //printf("[task_enet]: hhl 6666666666\r\n");

				 //sd�������Ѹ���
				 if( true == b_enet )
				 {
					 //ENET�������ݴ���
					 ENETSendDataProc();
				 }

				 break;
		}
		//printf("[task_enet]: enetstate=%d\r\n", enetstate);

	    _time_delay_ticks(10);			          //��ʱ1s��(����CPU����Ȩ)
    }
}

////============================================================================
////������: ENETRecvDataJuge
////��  ��: ENET���������ж�
////��  ��:��
////��  ��: ��
////============================================================================
//uint_8 ENETRecvDataJuge(const TBufNode* const pnode)
//{
//	uint_8 ret = ENET_RECVDATA_MAX;
//	if(pnode != NULL)
//	{
//		if(pnode->len > 2 && SERVERCOM_END_FLAG == pnode->pbuf[pnode->len - 1]
//			&& pnode->pbuf[pnode->len - 2] != SERVERCOM_ESC_FLAG)
//			ret = ENET_RECVDATA_OK;
//		else
//			ret = ENET_RECVDATA_ERR;
//	}
//
//	return ret;
//}


//============================================================================
//������: ENETRecvDataProc
//��  ��: enet�������ݴ���
//��  ��:��
//��  ��: ��
//============================================================================
void ENETRecvDataProc()
{
	TBufNode* pnode = NULL;
	uint_8 buf[SMALL_BUF_NODE_SIZE];
	uint_16 len, relen, ret = 0;
	uint_8* psrc;

	while(1)
	{
		//�������
		DISABLE_INTERRUPTS;
		listnode_get(&g_enetrecvdatalist, &pnode);
		//printf("[task_enet]: pnode=0x%X\r\n", pnode);
		ENABLE_INTERRUPTS;
		if(pnode != NULL)
		{
			printf("[task_enet]: pnode->len=%d\r\n", pnode->len);
			if(pnode->len > 0 && pnode->len <= LARGE_BUF_NODE_SIZE)
			{
				psrc = pnode->pbuf;
				relen = pnode->len;
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR,psrc);
				//��ת��
				while(relen > 0)
				{
					len = ServerFrameDelEsc(psrc, relen, &ret, buf, SMALL_BUF_NODE_SIZE);
					if(len <= 0 || ret > relen)
					{
						//DEBUGOUT_TIME(DEBUGOUT_LEVEL_ERR);
						DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ENETRecvDataProc]: ServerFrameDelEsc failed, relen=");
						DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, relen);
						DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", ret=");
						DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, ret);
						DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", len=");
						DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, len);
						DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", data is: ");
						for(len = 0; len < relen; len++)
						{
							DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR, psrc[len]);
							DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", ");
						}
						DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
						break;
					}
					//ͨ�Ų㴦��
					else
					{
						ComProc(eCOMTYPE_SERVER, buf, len);
						//ʣ��֡�����ƴ������
						psrc += ret;
						relen -= ret;
	//					g_psdrecord->gprsrcvtime = read_second_time();
					}
				}
			}

			//�黹freelist
			pnode->len = 0;
			//�������
			DISABLE_INTERRUPTS;
			if(pnode->size == LARGE_BUF_NODE_SIZE)
			{
				listnode_put(&g_largerecvfreelist, pnode);
			}
			else if(g_bNodeRet)
			{
				listnode_put(&g_smallrecvfreelist, pnode);
			}
			ENABLE_INTERRUPTS;
		}
		else
		{
			break;
		}
		
		pnode = null;
	}
}

//============================================================================
//������: ENETSendDataProc
//��  ��: ͨ��SPI��ENET���ݷ��͵�ENETģ�顣
//��  ��:��
//��  ��: ��
//============================================================================
void ENETSendDataProc()
{
    TBufNode* pnode = NULL;
	uint_8 buf[BUF_NODE_SIZE_COMSERVER_SEND];  //��ʱ���������ڴ�ż�ת��֮���֡
	uint_16 i, len;
	uint_8 ret;
	//��ȡenet�������������е�һ����㵽pnode
	listnode_get(&s_comserversenddatalist, &pnode);
	//��ȡgprs�������ݵĽ��ʧ��
	if(pnode==NULL)
		goto ENETSendDataProc_EXIT;  //�˳�
	//��ȡgprs�������ݵĽ��ɹ�
	else
	{
		//��Ӽ�ת���ַ�
		len = ServerFrameAddEsc(pnode->pbuf, pnode->len, buf, s_tappinfo.tgprsinfo.heartmax);
	    //���ת���ַ��ɹ�
		if(len>0)
		{
			//��������(���ĵ��Եȼ��ɴ�ӡ���˴����������ݵ�g_psdlog�У�֮��д�뵽SD����)
			//DEBUGOUT_TIME(DEBUGOUT_LEVEL_GPRS);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_GPRS, "[ENETSendDataProc]: senddata is: ");
			for(i = 0; i < len; i++)
			{
				DEBUGOUT_HEX(DEBUGOUT_LEVEL_GPRS, buf[i]);
				DEBUGOUT_STR(DEBUGOUT_LEVEL_GPRS, ", ");
			}
			DEBUGOUT_STR(DEBUGOUT_LEVEL_GPRS, "\r\n");

			buf[2]=0x0A;
			//���ж�
			DISABLE_INTERRUPTS;
			ret = send(1,buf,len);
		    //���ж�
		    ENABLE_INTERRUPTS;
//			ret = gprs_send(buf, len);  //��������
			if(ret == 0)  //����ʧ��
			{
//				printf("\r\n\r\n\r\n%d\r\n",ret);
				ENETSendFailCount++;  //ENET��������ʧ�ܴ�����1
				if(ENETSendFailCount>2) //ʧ�ܴ����ﵽ3�Σ���ENET�����¼�λ��λ��ִ��ENET��������
				{
					enetstate = STATE_TCP_LINK;

					printf("[ENETSendDataProc]:please take care of your enet connection!\r\n");
					//GPRS�����¼�λ��λ������GPRS��������
//					_lwevent_set(&lwevent_group, EVENT_GPRS_RESTART);
					//ENET��������ʧ�ܴ�����0
					ENETSendFailCount = 0;
				}
				//��������
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ENETSendDataProc]: enet_send failed, ret=");
				DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR,ret);
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
			}
			else
			{
				printf("[ENETSendDataProc]:Send to server success!\r\n");
				//����gprs����tick
//				s_gprssendbreakticks = sys_tick;

//				g_psdrecord->gsmsndtime = read_second_time();
			}
		}
		//�黹pnode��s_comserversendfreelist
		pnode->len = 0;
		listnode_put(&s_comserversendfreelist, pnode);
	}
    ENETSendDataProc_EXIT:
	__asm("NOP");
}

/*====================================================================
    ������      : VerDump
    ��  ��        ����ӡ�汾��
    ��  ��        ����
    ����          ����
====================================================================*/
void VerDump(void)
{
	//���ð汾��Ϣ
    HMC_KINETIS_K60_VER_PROT_MAJOR = VER_PROT_MAJOR;  //2
    HMC_KINETIS_K60_VER_PROT_MINOR = VER_PROT_MINOR;  //3
    HMC_KINETIS_K60_VER_SOFT_MAJOR = VER_SOFT_MAJOR; //3
    HMC_KINETIS_K60_VER_SOFT_MINOR = VER_SOFT_MINOR;  //0
    HMC_KINETIS_K60_VER_HARD_MAJOR = VER_HARD_MAJOR;   //2
    HMC_KINETIS_K60_VER_HARD_MINOR = VER_HARD_MINOR;   //0

	//�汾��Ϣ��ӡ
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "*************************************************************************\r\n");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "* SuZhou HuaXiang Infomation Technology Co.,Ltd                         *\r\n");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "* HMC_KINETIS_K60 Protocol VER:");
    DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, HMC_KINETIS_K60_VER_PROT_MAJOR);
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ".");
    DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, HMC_KINETIS_K60_VER_PROT_MINOR);
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "                                      *\r\n");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "* HMC_KINETIS_K60 Hardware VER:");
    DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, HMC_KINETIS_K60_VER_HARD_MAJOR);
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ".");
    DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, HMC_KINETIS_K60_VER_HARD_MINOR);
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "                                      *\r\n");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "* HMC_KINETIS_K60 Software VER:");
    DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, HMC_KINETIS_K60_VER_SOFT_MAJOR);
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ".");
    DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, HMC_KINETIS_K60_VER_SOFT_MINOR);
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "                                      *\r\n");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "* HMC_KINETIS_K60 Compile Time:");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, (uint_8*)CompileDate);   //��������
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "-");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, (uint_8*)CompileTime);   //����ʱ��
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "                     *\r\n");
    //DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "* HMC_KINETIS_K60 System Time:");
    //DEBUGOUT_TIME(DEBUGOUT_LEVEL_ERR);   //ϵͳ����ʱ��
    //DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "                                 *\r\n");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "*************************************************************************\r\n");
}


/*====================================================================
    ������      : AppInfoDump
    ����        ����ӡӦ����Ϣ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵������
    ����ֵ˵��  ����
====================================================================*/
void AppInfoDump(void)
{
	uint_8 i,j;

	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppInfoDump]: GPRS Dst IP is: ");
	for(i = 0; i < 4; i++)
	{
		DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.gprsdstip[i]);
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ".");
	}
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", Dst Port is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.gprsdstport);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppInfoDump]: GPRS LinkType is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.gprslinktype);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", hearttype is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.hearttype);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", hearttime is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.hearttime);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", heartmax is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.heartmax);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", devinfonum is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.devinfonum);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", devinfotype is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.devinfotype);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", polltime is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.polltime);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", broadsettime is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.broadsettime);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", cpuresettime is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.cpuresettime);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", heartvolnum is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.heartvolnum);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", heartcurnum is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.heartcurnum);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", heartswitchinnum is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.heartswitchinnum);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", heartswitchoutnum is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgprsinfo.heartswitchoutnum);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppInfoDump]: Lan netid is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tlaninfo.netid);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", route is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tlaninfo.route);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", groupid is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tlaninfo.groupid);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", nodeid is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tlaninfo.nodeid);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", brouter is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tlaninfo.brouter);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", bcsmaca is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tlaninfo.bcsmaca);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", rfch is: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tlaninfo.rfch);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppInfoDump]: Alarm OnOff: ");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgsminfo.bgsmalarm);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ", Phone is: ");
	for(i = 0; i < s_tappinfo.tgsminfo.phonenum; i++)
	{
		for(j = 0; j < 11; j++)
			DEBUGOUT_CHAR(DEBUGOUT_LEVEL_ERR, s_tappinfo.tgsminfo.phone[i][j]);
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, ",");
	}
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
}

//============================================================================
//������: TerminalMemRead
//��  ��: ��ȡ�洢���ն˿�������Ϣ
//��  ��:��
//��  ��: ��
//============================================================================
void TerminalMemRead(void)
{
    uint_32 address =  TERMINALINFOFLASHPAGE * SIZE_FLASHPAGE;
	uint_16 i, terminalnum;
	TNodeInfo* pnode = null;
	TNodeInfoSave* pnodesave = null;
	struct list_head* plist = null;
	uint_8 pagenum;

	for(pagenum = 0; pagenum < 3; pagenum++)
	{
		memcpy((int_8*)&terminalnum, (int_8*)address, 2);
		pnodesave = (TNodeInfoSave*)(address + 2);

		//��1ҳ
		//������Ч
		if(terminalnum <= NODE_NUM_MAX)
		{
			//˳���ȡ
			for(i = 0; i < terminalnum; i++, pnodesave++)
			{
				plist = s_mtnodefreelist.next;
				list_del(plist);


				pnode = list_entry(plist, TNodeInfo, list);

				//��ȡ�ڵ��ַ����ʼ��״̬��Ϣ
				memcpy(&pnode->tnodeinfosave, pnodesave, sizeof(TNodeInfoSave));
				pnode->failcount = 0;
				pnode->phytype = 0xFF;
				memset(pnode->data, 0xFF, APP_DATALEN);

				list_add_tail(&pnode->list, &s_mtnodedatalist);

				DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "[TerminalMemRead]: terminal: route=");
				DEBUGOUT_DEC(DEBUGOUT_LEVEL_ALARM, pnode->tnodeinfosave.route);
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, ", groupid=");
				DEBUGOUT_DEC(DEBUGOUT_LEVEL_ALARM, pnode->tnodeinfosave.groupid);
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, ", nodeid=");
				DEBUGOUT_HEX(DEBUGOUT_LEVEL_ALARM, pnode->tnodeinfosave.nodeid);
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "\r\n");
			}
		}

		address += SIZE_FLASHPAGE;
	}
}

//============================================================================
//������: AppRecordRead
//��  ��: ��ȡ��������Ӧ�ü�¼��Ϣ
//��  ��:��
//��  ��: ��
//============================================================================
bool AppRecordRead(void)
{
    uint_32 address =  APPINFOFLASHPAGE * SIZE_FLASHPAGE;   //�洢GPRS��Ϣ������
	TComLanInfo tcomlaninfo;  //��������Ϣ
	bool bflash = FALSE;
	uint_8 zigcount = 0;
	uint_8 i;

	memcpy((int_8*)&s_tappinfo, (int_8*)address, sizeof(TMHCAppRecord));

	//GPRS��¼��Ϣ����ȷ������Ĭ����Ϣ
	if(s_tappinfo.tgprsinfo.gprslinktype >= GPRSLINK_MAX)
	{
		//enet��ʽ
		s_tappinfo.tenetinfo.enetdstip[0] = ENET_DEFAULT_DSTIP1;
		s_tappinfo.tenetinfo.enetdstip[1] = ENET_DEFAULT_DSTIP2;
		s_tappinfo.tenetinfo.enetdstip[2] = ENET_DEFAULT_DSTIP3;
		s_tappinfo.tenetinfo.enetdstip[3] = ENET_DEFAULT_DSTIP4;
		s_tappinfo.tenetinfo.enetdstport = ENET_DEFAULT_DSTPORT;

		s_tappinfo.tenetinfo.enetsock = SOCKNUM;
		//Ŀ��ip��port
		s_tappinfo.tgprsinfo.gprsdstip[0] = GPRS_DEFAULT_DSTIP1;
		s_tappinfo.tgprsinfo.gprsdstip[1] = GPRS_DEFAULT_DSTIP2;
		s_tappinfo.tgprsinfo.gprsdstip[2] = GPRS_DEFAULT_DSTIP3;
		s_tappinfo.tgprsinfo.gprsdstip[3] = GPRS_DEFAULT_DSTIP4;
		s_tappinfo.tgprsinfo.gprsdstport = GPRS_DEFAULT_DSTPORT;
		//Ĭ�ϲ���GPRSLINK_CMNET
		s_tappinfo.tgprsinfo.gprslinktype = GPRSLINK_CMNET;
		s_tappinfo.tgprsinfo.hearttype = eHEARTTYPE_V0;
		s_tappinfo.tgprsinfo.hearttime = GPRS_DEFAULT_HEART;   //������Ĭ������60S
		s_tappinfo.tgprsinfo.heartmax = 600;
		s_tappinfo.tgprsinfo.devinfonum = 3;
		s_tappinfo.tgprsinfo.devinfotype = eDEVINFOTYPE_ALL_SEND;
		s_tappinfo.tgprsinfo.polltime = GPRS_DEFAULT_POLL_TIME;
		s_tappinfo.tgprsinfo.broadsettime = GPRS_DEFAULT_BROAD_SET_TIME;
		s_tappinfo.tgprsinfo.cpuresettime = GPRS_DEFAULT_CPU_RESET_TIME;
		s_tappinfo.tgprsinfo.heartvolnum = 1;
		s_tappinfo.tgprsinfo.heartcurnum = 1;
		s_tappinfo.tgprsinfo.heartswitchinnum = 0;
		s_tappinfo.tgprsinfo.heartswitchoutnum = 0;
	}

	//cpu��λʱ��
	if(s_tappinfo.tgprsinfo.cpuresettime > 0)
	{
		//reset = TRUE;
		//resetticks = sys_tick;
		//resettimeout = s_tappinfo.tgprsinfo.cpuresettime;
	}

	DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[AppRecordRead]: ComGetLanInfo!\r\n");

	//��ȡ��������Ϣ
	while(ComGetLanInfo(&tcomlaninfo, 1000) != eRET_NONE)
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppRecordRead]: ComGetLanInfo failed!\r\n");
		//����10�λ�ȡʧ�ܣ���λzigbeeģ��
		if(++zigcount > 10)
		{
			DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppRecordRead]: Reset Zigbee!\r\n");
			//gpio_set(PORT_ZIG_RST, PIN_ZIG_RST, ZIG_RST_OFF);
			//delay_ms(1000);
			//gpio_set(PORT_ZIG_RST, PIN_ZIG_RST, ZIG_RST_ON);
			//delay_ms(1000);
			//zigcount = 0;
			DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppRecordRead]: g_bZigbeeInsert = FALSE!\r\n");
			g_bZigbeeInsert = FALSE;
			break;
		}
	}
	if(g_bZigbeeInsert)
	{
		memcpy(&s_tappinfo.tlaninfo, &tcomlaninfo, sizeof(TComLanInfo));
	}
	//Flash�洢��Ϣ��Ч,����Ĭ��ֵ
	else if(s_tappinfo.tlaninfo.brouter > 1 || s_tappinfo.tlaninfo.bcsmaca > 1 || s_tappinfo.tlaninfo.rfch > 14)
	{
		s_tappinfo.tlaninfo.netid = LAN_DEFAULT_NETID;
		s_tappinfo.tlaninfo.route = 0;
		s_tappinfo.tlaninfo.groupid = 0;
		s_tappinfo.tlaninfo.nodeid = LAN_DEFAULT_NODEID;
		s_tappinfo.tlaninfo.brouter = 0;
		s_tappinfo.tlaninfo.transnum = 0;
		s_tappinfo.tlaninfo.bcsmaca = 0;
		s_tappinfo.tlaninfo.rfch = 0;
	}


	//GSM�ֻ�������Ϣ
	if(s_tappinfo.tgsminfo.phonenum > PHONENUM_MAX)
	{
		s_tappinfo.tgsminfo.bgsmalarm = GSM_ALRAM_ON;
		s_tappinfo.tgsminfo.phonenum = GSM_PHONE_NUM;
		memcpy(s_tappinfo.tgsminfo.phone[0], (int_8*)GSM_PHONE_DEF, 11);
	}

	return bflash;
}

//============================================================================
//������: AppRecordWrite
//��  ��: ����Ӧ�ü�¼��Ϣ
//��  ��:��
//��  ��: ��
//============================================================================
uint_8 AppRecordWrite(void)
{
    uint_8 ret = 0;
	uint_8 buf[SIZE_FLASHPAGE];
	uint_8* ptmp = buf;
	uint_16 len = sizeof(TMHCAppRecord);
	memcpy(buf, (uint_8*)&s_tappinfo, len);

	//����
	if(flash_erase_sector(APPINFOFLASHPAGE) != 0)
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppRecordWrite]: flash_erase_sector failed!\r\n");
		ret = 1;
	}
	//д��
	else if(flash_write(APPINFOFLASHPAGE, 0, len, buf) != 0)
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppRecordWrite]: flash_write failed!\r\n");
		ret = 2;
	}

	//AppInfoDump();

	return ret;
}


//============================================================================
//������: AppRecordProc
//��  ��: ����Ӧ�ü�¼��Ϣ
//��  ��:uint_8 type: 0��gprsinfo��1��laninfo��2��gsminfo��3��timecontrol
//��  ��: ��
//============================================================================
void AppRecordProc(const void* const pret, uint_8 retlen, uint_8 type)
{
	bool bflash = FALSE;
	uint_8 timeconnum;

	switch(type)
	{
		case 0:
			if(retlen == sizeof(TComGprsInfo))
			{
				memcpy(&s_tappinfo.tgprsinfo, pret, retlen);
				bflash = TRUE;
			}
			break;
		case 1:
			if(retlen == sizeof(TComLanInfo))
			{
				memcpy(&s_tappinfo.tlaninfo, pret, retlen);
				bflash = TRUE;
			}
			break;

		case 2:
			if(retlen == sizeof(TComGSMInfo))
			{
				memcpy(&s_tappinfo.tgsminfo, pret, retlen);
				bflash = TRUE;
			}
			break;

		/*case 3:
			if(NULL == pret && retlen <= TMHCTIMECON_NUM_MAX)
			{
				s_apptimecontrolnum = retlen;
				bflash = TRUE;
			}
			break;*/

		default:
			break;
	}

	//��¼��Ϣ����
	if(bflash)
	{
		AppRecordWrite();
		DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[AppRecordProc]: write ok!\r\n");
	}
}

//============================================================================
//������: ZigbeeSendDataProc
//��  ��: zigbee�������ݴ���,������ͨ�����ڷ��͵�Zigbeeģ��
//��  ��: psndnode:Zigbee������
//��  ��: ��
//============================================================================
void ZigbeeSendDataProc(const TBufNode* const psndnode)
{
	TBufNode* pnode = (TBufNode*)psndnode;  //��ʱ���������ڻ�ȡzigbee�������������е�һ�����
	uint_8 i;

	//��ȡzigbee�������������е�һ����㵽pnode
	if(null == pnode)
		listnode_get(&s_comterminalsenddatalist, &pnode);

	//��ȡzigbee�������ݵĽ��ʧ�ܣ��˳�
	if(pnode!=null && pnode->len > 0)
	{
	    //�ɹ���ȡһ��zigbee�������ݽ��
		//DEBUGOUT_TIME(DEBUGOUT_LEVEL_ZIG);
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ZIG, "[ZigbeeSendDataProc]: senddata is: ");

		//������ݱ��沢��ӡ
		for(i = 0; i < pnode->len; i++)
		{
			DEBUGOUT_HEX(DEBUGOUT_LEVEL_ZIG, pnode->pbuf[i]);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_ZIG, ", ");
		}
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ZIG, "\r\n");

		//�����ݷ��ͳ�ȥ
//		uart_sendN(UART_ID_ZIGBEE, pnode->len, pnode->pbuf);

		//�黹��㵽s_comterminalsendfreelist
		pnode->len = 0;
		listnode_put(&s_comterminalsendfreelist, pnode);
	}
}

//============================================================================
//������: TerminalDevAddDelProc
//��  ��: ·���ն˽ڵ�仯
//��  ��:��
//��  ��: ��
//============================================================================
void TerminalDevAddDelProc(void)
{
	//���ն˿������仯�¼�λ������TerminalChange����
	//_lwevent_set(&lwevent_group, EVENT_TERMINAL_CHANGE);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\n---1\r\n");
	//�����ն˽ڵ�ȫ��ʱ��ticks
	//_time_get_ticks(&terminal_change_ticks);
}
