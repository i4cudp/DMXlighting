#include "01_app_include.h"    //应用任务公共头文件


//static TBufNode* s_pNodePtr = NULL;

static uint_32 ENETSendFailCount = 0; //ENET发送数据失败次数
//static TENETInfo s_tENETInfo;
static uint_8 enetstate = STATE_TCP_START;


//软硬件版本号x.x
uint_8 HMC_KINETIS_K60_VER_PROT_MAJOR;
uint_8 HMC_KINETIS_K60_VER_PROT_MINOR;
uint_8 HMC_KINETIS_K60_VER_HARD_MAJOR;
uint_8 HMC_KINETIS_K60_VER_HARD_MINOR;
uint_8 HMC_KINETIS_K60_VER_SOFT_MAJOR;
uint_8 HMC_KINETIS_K60_VER_SOFT_MINOR;

//2.接收短消息
//TGSM_PARAM g_tgsmParam;

//3.日期与时间
const uint_8 CompileDate[] = __DATE__;  //日期预定义宏
const uint_8 CompileTime[] = __TIME__;  //时间预定义宏



struct list_head s_comserversenddatalist;     //通信层向server发送数据链表
struct list_head s_comterminalsenddatalist;   //通信层向terminal发送数据链表
struct list_head s_comserversendfreelist;     //通信层向server发送空闲链表
static struct list_head s_comterminalsendfreelist;   //通信层向terminal发送空闲链表
static struct list_head s_comterminalrecvdatalist;   //通信层向terminal接收数据链表
static struct list_head s_comterminalrecvfreelist;   //通信层向terminal接收空闲链表
static struct list_head s_mtnodedatalist;            //终端节点存储数据链表

static struct list_head s_mtnodefreelist;   //终端设备存储空闲链表

//Zigbee模块插入flag
bool g_bZigbeeInsert = TRUE;

extern bool b_enet;       //文件未更新
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
//任务名称：task_enet
//功能概要：socket建立
//参数说明：未使用
//===========================================================================
void task_enet(uint32_t initial_data)
{
	uint_8 znlen=0;
//	uint_8 enetstate = STATE_TCP_START;
	uint_8 sd_state = 0, NodeNum;
	uint_8 dhcpret = DHCP_RET_NONE;
	uint_16 serverlen,anyport=3000;/*定义一个任意端口并初始化*/
	uint_8 ip[8];
	uint_16 i;
	uint_8* ptmp = null;  //临时变量,用于初始化相关链表
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



	//ENET接收服务器数据缓冲区链表初始化
	//ptmp = ENETRecvBuf;
	//for(i = 0; i < BUF_NODE_NUM_ENET_RECV; i++)
	//{
	//	tENETRecvNode[i].pbuf = ptmp;	//数据缓冲空间地址
	//	tENETRecvNode[i].size = BUF_NODE_SIZE_ENET_RECV;//缓冲区大小
	//	tENETRecvNode[i].len = 0;		//实际数据长度，操作时确定
	//	tENETRecvNode[i].type = ENETFRAME_MAX;	    //node类型，由应用层明确
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

	//4.COM层发送服务器数据缓冲区链表初始化
	ptmp = COMServerSendBuf;
	for(i = 0; i < BUF_NODE_NUM_COMSERVER_SEND; i++)
	{
		tCOMServerSendNode[i].pbuf = ptmp;
		tCOMServerSendNode[i].size = BUF_NODE_SIZE_COMSERVER_SEND;
		tCOMServerSendNode[i].len = 0;
		ptmp += BUF_NODE_SIZE_COMSERVER_SEND;
	}
	buflist_init(&s_comserversenddatalist, &s_comserversendfreelist, tCOMServerSendNode, BUF_NODE_NUM_COMSERVER_SEND);


	//5.COM层发送终端数据缓冲区链表初始化
	ptmp = COMTerminalSendBuf;
	for(i = 0; i < BUF_NODE_NUM_COMTERMINAL_SEND; i++)
	{
		tCOMTerminalSendNode[i].pbuf = ptmp;
		tCOMTerminalSendNode[i].size = BUF_NODE_SIZE_COMTERMINAL_SEND;
		tCOMTerminalSendNode[i].len = 0;
		ptmp += BUF_NODE_SIZE_COMTERMINAL_SEND;
	}
	buflist_init(&s_comterminalsenddatalist, &s_comterminalsendfreelist, tCOMTerminalSendNode, BUF_NODE_NUM_COMTERMINAL_SEND);


	//6.COM层接收终端数据缓冲区链表初始化
	ptmp = COMTerminalRecvBuf;
	for(i = 0; i < BUF_NODE_NUM_COMTERMINAL_RECV; i++)
	{
		tCOMTerminalRecvNode[i].pbuf = ptmp;
		tCOMTerminalRecvNode[i].size = BUF_NODE_SIZE_COMTERMINAL_RECV;
		tCOMTerminalRecvNode[i].len = 0;
		ptmp += BUF_NODE_SIZE_COMTERMINAL_RECV;
	}
	buflist_init(&s_comterminalrecvdatalist, &s_comterminalrecvfreelist, tCOMTerminalRecvNode, BUF_NODE_NUM_COMTERMINAL_RECV);

	//7.终端设备存储信息链表初始化
	s_mtnodedatalist.next = &s_mtnodedatalist;
	s_mtnodedatalist.prev = &s_mtnodedatalist;
	s_mtnodefreelist.next = &s_mtnodefreelist;
	s_mtnodefreelist.prev = &s_mtnodefreelist;
	for(i = 0; i < NODE_NUM_MAX; i++)
		list_add_tail(&tMtNode[i].list, &s_mtnodefreelist);

	//11.版本信息打印
	VerDump();

	//12.读取内存中的终端节点信息
	TerminalMemRead();
	//13.通信层数据初始化
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

	//17.获取主控制器应用信息(网路号、网内地址、路由 方式)
	if(AppRecordRead())
	{
		//更新主控器应用信息Flash
		if(AppRecordWrite())
			DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[main]: AppRecordWrite failed!\r\n");
	}

	memcpy(&tcominfo.tcomenetinfo, &s_tappinfo.tenetinfo, sizeof(TComENETInfo));
	memcpy(&tcominfo.tcomgprsinfo, &s_tappinfo.tgprsinfo, sizeof(TComGprsInfo));
	memcpy(&tcominfo.tcomlaninfo, &s_tappinfo.tlaninfo, sizeof(TComLanInfo));
	memcpy(&tcominfo.tcomgsminfo, &s_tappinfo.tgsminfo, sizeof(TComGSMInfo));

	//18.主控器应用信息打印
	AppInfoDump();

	//19.通信层数据重新初始化
	ComInit(&tcominfo);

	//SDHC初始化及znFAT文件系统初始化
	sd_state = znFAT_Device_Init();   //SD卡系统初始化
	//串口发送初始化提示
	if (RES_OK == sd_state)
	{
		printf("SD初始化完成！\n");
	}
	else
	{
		printf("SD初始化失败！\n");
	}
	znFAT_Select_Device(0, &sdInitArgs);
	sd_state = znFAT_Init();   //znFAT文件系统初始化
	//串口发送初始化提示
	if (RES_OK == sd_state)
	{
		printf("文件系统初始化完成！\n");
	}
	else
	{
		printf("文件系统初始化失败！\n");
	}


//	uint_8 pc_ip[4]={10, 10, 65, 100};   //内网地址
//	uint_8 pc_ip[4]={218, 4, 189, 26};   //外网地址
//	uint_8 pc_ip[4]={120, 25, 243, 73};//友盟服务器
//	uint_8 pc_ip[4]={121, 40, 158, 140};//华祥


    while (TRUE)
    {
		switch(enetstate)
		{
			//开始dhcp
			case STATE_TCP_START:
				dhcpret = check_DHCP_state(SOCK_DHCP);
				switch(dhcpret)
				{
				  case DHCP_RET_NONE:
					break;
				  case DHCP_RET_TIMEOUT:
					break;
				  case DHCP_RET_UPDATE:
				  	setSHAR(ConfigMsg.mac);//配置Mac地址
				    setSUBR(ConfigMsg.sub);//配置子网掩码
				    setGAR(ConfigMsg.gw);//配置默认网关
				    setSIPR(ConfigMsg.lip);//配置Ip地址
				    
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

			//DHCP完成，开始tcp链接
			case STATE_TCP_LINK:
				switch(getSn_SR(tcominfo.tcomenetinfo.enetsock))   //获取socket1的状态
				{
				   case SOCK_INIT:    //socket初始化完成
					 printf("[task_enet]: start tcp link，server: %d.%d.%d.%d\r\n",
							 tcominfo.tcomenetinfo.enetdstip[0],tcominfo.tcomenetinfo.enetdstip[1],
							 tcominfo.tcomenetinfo.enetdstip[2],tcominfo.tcomenetinfo.enetdstip[3]);

					 connect(tcominfo.tcomenetinfo.enetsock, tcominfo.tcomenetinfo.enetdstip,
							 tcominfo.tcomenetinfo.enetdstport); //在TCP模式下向服务器发送连接请求
					 break;
					 
				   case SOCK_ESTABLISHED:   //socket连接建立
					 printf("[task_enet]: start tcp link ok!\r\n");
					 enetstate = STATE_TCP_FINISH;
					 break;
				   case SOCK_CLOSE_WAIT:   //socket等待关闭状态
					 break;
				   case SOCK_CLOSED:       //socket关闭
				   	 printf("[task_enet]: restart tcp link\r\n");
					 socket(tcominfo.tcomenetinfo.enetsock,Sn_MR_TCP,anyport++,Sn_MR_ND);   //打开socket1的一个端口
					 break;
				 }
				break;

			//链接完成
			case STATE_TCP_FINISH:
				 //printf("[task_enet]: hhl 111111\r\n");

				 //清 连接中断
				 if(getSn_IR(tcominfo.tcomenetinfo.enetsock) & Sn_IR_CON)
				 {
					setSn_IR(tcominfo.tcomenetinfo.enetsock, Sn_IR_CON);
				 }

				 //printf("[task_enet]: hhl 22222222222\r\n");
				 
				 setSIMR(0x02);   //SOCKET1的中断掩码，开启中断

				 //printf("[task_enet]: hhl 3333333333\r\n");
				 setSn_IMR(tcominfo.tcomenetinfo.enetsock,0x04);//关闭发送完成中断

				 //printf("[task_enet]: hhl 4444444444\r\n");

				 //心跳包处理
				 ComHeartProc();
				 //printf("[task_enet]: hhl 5555555555\r\n");

                 //处理ENET收到的数据包
			     ENETRecvDataProc();
				 //printf("[task_enet]: hhl 6666666666\r\n");

				 //sd卡内容已更新
				 if( true == b_enet )
				 {
					 //ENET发送数据处理
					 ENETSendDataProc();
				 }

				 break;
		}
		//printf("[task_enet]: enetstate=%d\r\n", enetstate);

	    _time_delay_ticks(10);			          //延时1s，(放弃CPU控制权)
    }
}

////============================================================================
////函数名: ENETRecvDataJuge
////功  能: ENET接收数据判断
////参  数:无
////返  回: 无
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
//函数名: ENETRecvDataProc
//功  能: enet接收数据处理
//参  数:无
//返  回: 无
//============================================================================
void ENETRecvDataProc()
{
	TBufNode* pnode = NULL;
	uint_8 buf[SMALL_BUF_NODE_SIZE];
	uint_16 len, relen, ret = 0;
	uint_8* psrc;

	while(1)
	{
		//互斥操作
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
				//解转义
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
					//通信层处理
					else
					{
						ComProc(eCOMTYPE_SERVER, buf, len);
						//剩余帧，解决拼包问题
						psrc += ret;
						relen -= ret;
	//					g_psdrecord->gprsrcvtime = read_second_time();
					}
				}
			}

			//归还freelist
			pnode->len = 0;
			//互斥操作
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
//函数名: ENETSendDataProc
//功  能: 通过SPI将ENET数据发送到ENET模块。
//参  数:无
//返  回: 无
//============================================================================
void ENETSendDataProc()
{
    TBufNode* pnode = NULL;
	uint_8 buf[BUF_NODE_SIZE_COMSERVER_SEND];  //临时变量，用于存放加转义之后的帧
	uint_16 i, len;
	uint_8 ret;
	//获取enet发送数据链表中的一个结点到pnode
	listnode_get(&s_comserversenddatalist, &pnode);
	//获取gprs发送数据的结点失败
	if(pnode==NULL)
		goto ENETSendDataProc_EXIT;  //退出
	//获取gprs发送数据的结点成功
	else
	{
		//添加加转义字符
		len = ServerFrameAddEsc(pnode->pbuf, pnode->len, buf, s_tappinfo.tgprsinfo.heartmax);
	    //添加转义字符成功
		if(len>0)
		{
			//保存数据(更改调试等级可打印，此处仅保存数据到g_psdlog中，之后写入到SD卡中)
			//DEBUGOUT_TIME(DEBUGOUT_LEVEL_GPRS);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_GPRS, "[ENETSendDataProc]: senddata is: ");
			for(i = 0; i < len; i++)
			{
				DEBUGOUT_HEX(DEBUGOUT_LEVEL_GPRS, buf[i]);
				DEBUGOUT_STR(DEBUGOUT_LEVEL_GPRS, ", ");
			}
			DEBUGOUT_STR(DEBUGOUT_LEVEL_GPRS, "\r\n");

			buf[2]=0x0A;
			//关中断
			DISABLE_INTERRUPTS;
			ret = send(1,buf,len);
		    //开中断
		    ENABLE_INTERRUPTS;
//			ret = gprs_send(buf, len);  //发送数据
			if(ret == 0)  //发送失败
			{
//				printf("\r\n\r\n\r\n%d\r\n",ret);
				ENETSendFailCount++;  //ENET发送数据失败次数加1
				if(ENETSendFailCount>2) //失败次数达到3次，将ENET重启事件位置位，执行ENET重启任务
				{
					enetstate = STATE_TCP_LINK;

					printf("[ENETSendDataProc]:please take care of your enet connection!\r\n");
					//GPRS重启事件位置位，启动GPRS重启任务
//					_lwevent_set(&lwevent_group, EVENT_GPRS_RESTART);
					//ENET发送数据失败次数清0
					ENETSendFailCount = 0;
				}
				//保存数据
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ENETSendDataProc]: enet_send failed, ret=");
				DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR,ret);
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
			}
			else
			{
				printf("[ENETSendDataProc]:Send to server success!\r\n");
				//更新gprs断链tick
//				s_gprssendbreakticks = sys_tick;

//				g_psdrecord->gsmsndtime = read_second_time();
			}
		}
		//归还pnode到s_comserversendfreelist
		pnode->len = 0;
		listnode_put(&s_comserversendfreelist, pnode);
	}
    ENETSendDataProc_EXIT:
	__asm("NOP");
}

/*====================================================================
    函数名      : VerDump
    功  能        ：打印版本号
    参  数        ：无
    返回          ：无
====================================================================*/
void VerDump(void)
{
	//配置版本信息
    HMC_KINETIS_K60_VER_PROT_MAJOR = VER_PROT_MAJOR;  //2
    HMC_KINETIS_K60_VER_PROT_MINOR = VER_PROT_MINOR;  //3
    HMC_KINETIS_K60_VER_SOFT_MAJOR = VER_SOFT_MAJOR; //3
    HMC_KINETIS_K60_VER_SOFT_MINOR = VER_SOFT_MINOR;  //0
    HMC_KINETIS_K60_VER_HARD_MAJOR = VER_HARD_MAJOR;   //2
    HMC_KINETIS_K60_VER_HARD_MINOR = VER_HARD_MINOR;   //0

	//版本信息打印
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
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, (uint_8*)CompileDate);   //编译日期
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "-");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, (uint_8*)CompileTime);   //编译时间
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "                     *\r\n");
    //DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "* HMC_KINETIS_K60 System Time:");
    //DEBUGOUT_TIME(DEBUGOUT_LEVEL_ERR);   //系统运行时间
    //DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "                                 *\r\n");
    DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "*************************************************************************\r\n");
}


/*====================================================================
    函数名      : AppInfoDump
    功能        ：打印应用信息
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：无
    返回值说明  ：无
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
//函数名: TerminalMemRead
//功  能: 读取存储的终端控制器信息
//参  数:无
//返  回: 无
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

		//第1页
		//长度有效
		if(terminalnum <= NODE_NUM_MAX)
		{
			//顺序读取
			for(i = 0; i < terminalnum; i++, pnodesave++)
			{
				plist = s_mtnodefreelist.next;
				list_del(plist);


				pnode = list_entry(plist, TNodeInfo, list);

				//获取节点地址，初始化状态信息
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
//函数名: AppRecordRead
//功  能: 读取主控制器应用记录信息
//参  数:无
//返  回: 无
//============================================================================
bool AppRecordRead(void)
{
    uint_32 address =  APPINFOFLASHPAGE * SIZE_FLASHPAGE;   //存储GPRS信息的扇区
	TComLanInfo tcomlaninfo;  //局域网信息
	bool bflash = FALSE;
	uint_8 zigcount = 0;
	uint_8 i;

	memcpy((int_8*)&s_tappinfo, (int_8*)address, sizeof(TMHCAppRecord));

	//GPRS记录信息不正确，采用默认信息
	if(s_tappinfo.tgprsinfo.gprslinktype >= GPRSLINK_MAX)
	{
		//enet方式
		s_tappinfo.tenetinfo.enetdstip[0] = ENET_DEFAULT_DSTIP1;
		s_tappinfo.tenetinfo.enetdstip[1] = ENET_DEFAULT_DSTIP2;
		s_tappinfo.tenetinfo.enetdstip[2] = ENET_DEFAULT_DSTIP3;
		s_tappinfo.tenetinfo.enetdstip[3] = ENET_DEFAULT_DSTIP4;
		s_tappinfo.tenetinfo.enetdstport = ENET_DEFAULT_DSTPORT;

		s_tappinfo.tenetinfo.enetsock = SOCKNUM;
		//目的ip、port
		s_tappinfo.tgprsinfo.gprsdstip[0] = GPRS_DEFAULT_DSTIP1;
		s_tappinfo.tgprsinfo.gprsdstip[1] = GPRS_DEFAULT_DSTIP2;
		s_tappinfo.tgprsinfo.gprsdstip[2] = GPRS_DEFAULT_DSTIP3;
		s_tappinfo.tgprsinfo.gprsdstip[3] = GPRS_DEFAULT_DSTIP4;
		s_tappinfo.tgprsinfo.gprsdstport = GPRS_DEFAULT_DSTPORT;
		//默认采用GPRSLINK_CMNET
		s_tappinfo.tgprsinfo.gprslinktype = GPRSLINK_CMNET;
		s_tappinfo.tgprsinfo.hearttype = eHEARTTYPE_V0;
		s_tappinfo.tgprsinfo.hearttime = GPRS_DEFAULT_HEART;   //心跳包默认周期60S
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

	//cpu复位时间
	if(s_tappinfo.tgprsinfo.cpuresettime > 0)
	{
		//reset = TRUE;
		//resetticks = sys_tick;
		//resettimeout = s_tappinfo.tgprsinfo.cpuresettime;
	}

	DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[AppRecordRead]: ComGetLanInfo!\r\n");

	//获取局域网信息
	while(ComGetLanInfo(&tcomlaninfo, 1000) != eRET_NONE)
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppRecordRead]: ComGetLanInfo failed!\r\n");
		//连续10次获取失败，复位zigbee模块
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
	//Flash存储信息无效,采用默认值
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


	//GSM手机号码信息
	if(s_tappinfo.tgsminfo.phonenum > PHONENUM_MAX)
	{
		s_tappinfo.tgsminfo.bgsmalarm = GSM_ALRAM_ON;
		s_tappinfo.tgsminfo.phonenum = GSM_PHONE_NUM;
		memcpy(s_tappinfo.tgsminfo.phone[0], (int_8*)GSM_PHONE_DEF, 11);
	}

	return bflash;
}

//============================================================================
//函数名: AppRecordWrite
//功  能: 保存应用记录信息
//参  数:无
//返  回: 无
//============================================================================
uint_8 AppRecordWrite(void)
{
    uint_8 ret = 0;
	uint_8 buf[SIZE_FLASHPAGE];
	uint_8* ptmp = buf;
	uint_16 len = sizeof(TMHCAppRecord);
	memcpy(buf, (uint_8*)&s_tappinfo, len);

	//擦除
	if(flash_erase_sector(APPINFOFLASHPAGE) != 0)
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppRecordWrite]: flash_erase_sector failed!\r\n");
		ret = 1;
	}
	//写入
	else if(flash_write(APPINFOFLASHPAGE, 0, len, buf) != 0)
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[AppRecordWrite]: flash_write failed!\r\n");
		ret = 2;
	}

	//AppInfoDump();

	return ret;
}


//============================================================================
//函数名: AppRecordProc
//功  能: 保存应用记录信息
//参  数:uint_8 type: 0，gprsinfo；1，laninfo，2，gsminfo，3，timecontrol
//返  回: 无
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

	//记录信息保存
	if(bflash)
	{
		AppRecordWrite();
		DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[AppRecordProc]: write ok!\r\n");
	}
}

//============================================================================
//函数名: ZigbeeSendDataProc
//功  能: zigbee发送数据处理,将数据通过串口发送到Zigbee模块
//参  数: psndnode:Zigbee链表结点
//返  回: 无
//============================================================================
void ZigbeeSendDataProc(const TBufNode* const psndnode)
{
	TBufNode* pnode = (TBufNode*)psndnode;  //临时变量，用于获取zigbee发送数据链表中的一个结点
	uint_8 i;

	//获取zigbee发送数据链表中的一个结点到pnode
	if(null == pnode)
		listnode_get(&s_comterminalsenddatalist, &pnode);

	//获取zigbee发送数据的结点失败，退出
	if(pnode!=null && pnode->len > 0)
	{
	    //成功获取一个zigbee发送数据结点
		//DEBUGOUT_TIME(DEBUGOUT_LEVEL_ZIG);
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ZIG, "[ZigbeeSendDataProc]: senddata is: ");

		//相关数据保存并打印
		for(i = 0; i < pnode->len; i++)
		{
			DEBUGOUT_HEX(DEBUGOUT_LEVEL_ZIG, pnode->pbuf[i]);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_ZIG, ", ");
		}
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ZIG, "\r\n");

		//将数据发送出去
//		uart_sendN(UART_ID_ZIGBEE, pnode->len, pnode->pbuf);

		//归还结点到s_comterminalsendfreelist
		pnode->len = 0;
		listnode_put(&s_comterminalsendfreelist, pnode);
	}
}

//============================================================================
//函数名: TerminalDevAddDelProc
//功  能: 路段终端节点变化
//参  数:无
//返  回: 无
//============================================================================
void TerminalDevAddDelProc(void)
{
	//置终端控制器变化事件位，启动TerminalChange任务
	//_lwevent_set(&lwevent_group, EVENT_TERMINAL_CHANGE);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\n---1\r\n");
	//更新终端节点全局时间ticks
	//_time_get_ticks(&terminal_change_ticks);
}
