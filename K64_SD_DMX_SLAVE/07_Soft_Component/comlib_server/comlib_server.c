/*====================================================================
模块名  ： COMLIB
文件名  ： comlib.c
相关文件： comlib.h
文件实现功能：实现通信帧处理
作者    ：刘辉
版本    ：1.0.0.0.0
----------------------------------------------------------------------
修改记录    ：
日  期         版本        修改人        修改内容
2012/07/03       1.0              刘辉                 创建
====================================================================*/
/* 头文件 */

#include "comlib_server.h"

//static TComInfo* s_ptcomserverinfo;
TComInfo* s_ptcomserverinfo;
static bool s_bmhcnodeidchange = FALSE;       //主控器局域网地址变化
bool s_bpollack = FALSE;
bool s_bLinkStart = TRUE;                //刚建立连接
bool b_sd = false;                        //文件未更新
bool g_bSDWaitEvent = FALSE;
bool g_bDMXWaitEvent = FALSE;
bool g_bNodeRet = TRUE;
bool b_enet = true;                        //文件未更新
static bool s_bPollStart = FALSE;        //开始轮询，解决广播控制命令，状态
static bool s_bAckFlash = FALSE;
static uint_32 s_AckRet;
//static uint_8 s_AckCh;
//static bool s_bAckOnOff = 0xFF;
//static uint_8 s_AckPower = 0xFF;
static uint_8 s_AckData[APP_DATALEN];


//20121130广播设置操作进行多次操作，防止有漏操作的单灯
static uint_8 s_broadsetcount = 0;           //广播设置操作次数
static uint_8 s_broadsetbuf[50];             //广播设置操作内容
static uint_8 s_broadsetlen;                 //广播设置操作内容长度

uint_8 g_gsmsendbuf[200];
uint_8 g_gsmsendlen = 0;

extern uint_8 CompileDate[];
extern uint_8 CompileTime[];

extern struct FileInfo fileinfo1;
extern uint_8 detect_receive[8];
//系统复位
//extern bool bresetset;
//extern MQX_TICK_STRUCT resetticks;
//extern uint_32 resettimeout;

//extern TGSM_PARAM g_tgsmParam;

//============================================================================
//函数名: ServerStructAddr
//功  能: 结构地址分配
//参  数: 无
//返  回: 无
//============================================================================
static inline void ServerStructAddr(TServerCOMFrame* ptframe, uint_8* pbuf, uint_16 cmdlen)
{
	//地址分配
	ptframe->phead = pbuf;
	ptframe->pret = (uint_32*)(ptframe->phead + 1);
	ptframe->pnetid = (uint_16*)(ptframe->pret + 1);
	ptframe->proute = (uint_8*)(ptframe->pnetid + 1);
	ptframe->pgroupid = ptframe->proute + 1;
	ptframe->pnodeid = (uint_16*)(ptframe->pgroupid + 1);
	ptframe->pcmd = ptframe->pnodeid + 1;
	ptframe->pcmdbuf = (uint_8*)(ptframe->pcmd + 1);
	ptframe->pchk = ptframe->pcmdbuf + cmdlen;
	ptframe->pend = ptframe->pchk + 1;
}

//============================================================================
//函数名: NodeAddrCheck
//功  能: 终端节点地址分配
//参  数: 无
//返  回: 无
//============================================================================
TNodeInfo* NodeAddrCheck(uint_16 nodeid)
{
	struct list_head* plist = NULL;
	TNodeInfo* pnode = NULL;
	
	if(NULL == s_ptcomserverinfo || NULL == s_ptcomserverinfo->pterminalnodedatalist)
		return NULL;
	
	list_for_each(plist, s_ptcomserverinfo->pterminalnodedatalist)
	{
        pnode = list_entry(plist, TNodeInfo, list);
		if(pnode->tnodeinfosave.nodeid == nodeid)
			break;
		pnode = NULL;
	}
        
   return pnode;
}

//============================================================================
//函数名: AddrReqProc
//功  能: eSERVERCMD_ADDRREQ命令处理  地址查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerAddrReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 buf[6];
    uint_16* pnetid = (uint_16*)buf;
	uint_8* proute = (uint_8*)(pnetid + 1);
	uint_8* pgroupid = proute + 1;
	uint_16* pnodeid = (uint_16*)(pgroupid + 1);
	uint_8 ret = eRET_NONE;

	//主控器地址查询
	if(eLANFRAMETYPE_ME == frametype)
	{
		*pnetid = REVERSE16(s_ptcomserverinfo->tcomlaninfo.netid);
		*proute = s_ptcomserverinfo->tcomlaninfo.route;
		*pgroupid = s_ptcomserverinfo->tcomlaninfo.groupid;
		*pnodeid = REVERSE16(s_ptcomserverinfo->tcomlaninfo.nodeid);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK_ADDR, buf, 6, *ptcmd->pret);
	}
	//终端控制器地址查询
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//终端组帧发送
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_ADDR, NULL, 0, *ptcmd->pret);
	}
	//广播地址错误
	else
	{
		*((uint_16*)buf) = REVERSE16(eSERVERACK_ERR_ADDR);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK, buf, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ServerAddrSetProc
//功  能: eSERVERCMD_ADDRSET命令处理  地址设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerAddrSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;
	
	//主控器或终端控制器地址设置
	if(eLANFRAMETYPE_ME == frametype || eLANFRAMETYPE_NODE_NET == frametype)
	{
		//主控器信息更新
		if(eLANFRAMETYPE_ME == frametype)
		{
			s_ptcomserverinfo->tcomlaninfo.netid = (ptcmd->pcmdbuf[0] << 8) + ptcmd->pcmdbuf[1];
			s_ptcomserverinfo->tcomlaninfo.route = ptcmd->pcmdbuf[2];
			s_ptcomserverinfo->tcomlaninfo.groupid = ptcmd->pcmdbuf[3];
			s_ptcomserverinfo->tcomlaninfo.nodeid = (ptcmd->pcmdbuf[4] << 8) + ptcmd->pcmdbuf[5];
			s_bmhcnodeidchange = TRUE;  //等到应答后更新
			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[ServerAddrSetProc], netid=");
			DEBUGOUT_HEX(DEBUGOUT_LEVEL_INFO, s_ptcomserverinfo->tcomlaninfo.netid);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, ", route=");
			DEBUGOUT_HEX(DEBUGOUT_LEVEL_INFO, s_ptcomserverinfo->tcomlaninfo.route);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, ", groupid=");
			DEBUGOUT_HEX(DEBUGOUT_LEVEL_INFO, s_ptcomserverinfo->tcomlaninfo.groupid);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, ", nodeid=");
			DEBUGOUT_HEX(DEBUGOUT_LEVEL_INFO, s_ptcomserverinfo->tcomlaninfo.nodeid);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, ")\r\n");
		}
		
		//终端组帧发送
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_SET_ADDR, ptcmd->pcmdbuf, 6, *ptcmd->pret);
	}
	//广播地址不允许设置
	else
	{
		ack = REVERSE16(eSERVERACK_ERR_ADDSET);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}
	
	return ret;
}

//============================================================================
//函数名: ServerRFChReqProc
//功  能: eSERVERCMD_CHREQ命令处理  通道查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerRFChReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;

	//主控器通道查询
	if(eLANFRAMETYPE_ME == frametype)
	{
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
			               eSERVERCMD_ACK_RFCH, &s_ptcomserverinfo->tcomlaninfo.rfch, 1, *ptcmd->pret);
	}
	//终端控制器通道查询
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//终端组帧发送
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_RFCH, NULL, 0, *ptcmd->pret);
	}
	//广播地址错误
	else
	{
		ack = REVERSE16(eSERVERACK_ERR_ADDR);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ServerRFChSetProc
//功  能: eSERVERCMD_RFCHRSET命令处理  通道设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerRFChSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_8 ret = eRET_NONE;

	//主控器信息更新
	if(eLANFRAMETYPE_ME ==  frametype && ptcmd->pcmdbuf[0] != s_ptcomserverinfo->tcomlaninfo.rfch)
	{
		s_ptcomserverinfo->tcomlaninfo.rfch = ptcmd->pcmdbuf[0];
		s_bmhcnodeidchange = TRUE;  //等到应答后更新
	}
	
	//终端组帧发送
	ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
	                         eTERMINALCOM_SET_RFCH, ptcmd->pcmdbuf, 1, *ptcmd->pret);

	//广播地址设置直接由主控器应答
	if(frametype < eLANFRAMETYPE_NODE_NET && frametype != eLANFRAMETYPE_ME)
	{
		//更新主控器信息
		s_ptcomserverinfo->tcomlaninfo.rfch = ptcmd->pcmdbuf[0];
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);

		//主动应答
		if(ret != eRET_NONE)
			ack = eSERVERACK_ERR_CHSET;
		
		//大小端转序
		ack = REVERSE16(ack);
		
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ServerRouteReqProc
//功  能: eSERVERCMD_ROUTREQ命令处理   路由能力查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerRouteReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;

	//主控器通道查询
	if(eLANFRAMETYPE_ME == frametype)
	{
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
			               eSERVERCMD_ACK_ROUTE, &s_ptcomserverinfo->tcomlaninfo.brouter, 1, *ptcmd->pret);
	}
	//终端控制器查询
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//终端组帧发送
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_ROUTE, NULL, 0, *ptcmd->pret);
	}
	//广播地址错误
	else
	{
		ack = REVERSE16(eSERVERACK_ERR_ADDR);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ServerRouteSetProc
//功  能: eSERVERCMD_RFCHRSET命令处理  路由能力设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerRouteSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_8 ret = eRET_NONE;

	//主控器信息更新
	if(eLANFRAMETYPE_ME == frametype &&	ptcmd->pcmdbuf[0] != s_ptcomserverinfo->tcomlaninfo.brouter)
	{
		s_ptcomserverinfo->tcomlaninfo.brouter = ptcmd->pcmdbuf[0];
		s_bmhcnodeidchange = TRUE;  //等到应答后更新
	}

	//终端组帧发送
	ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
	                         eTERMINALCOM_SET_ROUTE, ptcmd->pcmdbuf, 1, *ptcmd->pret);

	//广播地址设置直接由主控器应答
	if(frametype < eLANFRAMETYPE_NODE_NET && frametype != eLANFRAMETYPE_ME)
	{
		//更新主控器信息
		s_ptcomserverinfo->tcomlaninfo.brouter = ptcmd->pcmdbuf[0];
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);

		//主动应答
		if(ret != eRET_NONE)
			ack = eSERVERACK_ERR_ROUTSET;

		//大小端转序
		ack = REVERSE16(ack);
		
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ServerTranReqProc
//功  能: eSERVERCMD_TRANUMREQ命令处理  转发跳数查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerTranReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;

	//主控器通道查询
	if(eLANFRAMETYPE_ME == frametype)
	{
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
			               eSERVERCMD_ACK_TRANUM, &s_ptcomserverinfo->tcomlaninfo.transnum, 1, *ptcmd->pret);
	}
	//终端控制器查询
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//终端组帧发送
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_TRANUM, NULL, 0, *ptcmd->pret);
	}
	//广播地址错误
	else
	{
		ack = REVERSE16(eSERVERACK_ERR_ADDR);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ServerTranSetProc
//功  能: eSERVERCMD_TRANUMRSET命令处理   转发跳数设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerTranSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_8 ret = eRET_NONE;

	//主控器信息更新
	if(eLANFRAMETYPE_ME == frametype &&	ptcmd->pcmdbuf[0] != s_ptcomserverinfo->tcomlaninfo.transnum)
	{
		s_ptcomserverinfo->tcomlaninfo.transnum = ptcmd->pcmdbuf[0];
		s_bmhcnodeidchange = TRUE;  //等到应答后更新
	}

	//终端组帧发送
	ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
	                         eTERMINALCOM_SET_TRANUM, ptcmd->pcmdbuf, 1, *ptcmd->pret);

	//广播地址设置直接由主控器应答
	if(frametype < eLANFRAMETYPE_NODE_NET && frametype != eLANFRAMETYPE_ME)
	{
		//更新主控器信息
		s_ptcomserverinfo->tcomlaninfo.transnum = ptcmd->pcmdbuf[0];
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);

		//主动应答
		if(ret != eRET_NONE)
			ack = eSERVERACK_ERR_TRANUMSET;

		//大小端转序
		ack = REVERSE16(ack);
		
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}


//============================================================================
//函数名: ServerCSMACAReqProc
//功  能: eSERVERCMD_CSMAREQ命令处理  CSMACA能力查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerCSMACAReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;

	//主控器通道查询
	if(eLANFRAMETYPE_ME == frametype)
	{
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK_CSMACA, &s_ptcomserverinfo->tcomlaninfo.bcsmaca, 1, *ptcmd->pret);
	}
	//终端控制器查询
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//终端组帧发送
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_CSMACA, NULL, 0, *ptcmd->pret);
	}
	//广播地址错误
	else
	{
		ack = REVERSE16(eSERVERACK_ERR_ADDR);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ServerCSMACASetProc
//功  能: eSERVERCMD_CSMASET命令处理  CSMACA能力设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerCSMACASetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_8 ret = eRET_NONE;

	//主控器信息更新
	if(eLANFRAMETYPE_ME == frametype &&	ptcmd->pcmdbuf[0] != s_ptcomserverinfo->tcomlaninfo.bcsmaca)
	{
		s_ptcomserverinfo->tcomlaninfo.bcsmaca = ptcmd->pcmdbuf[0];
		s_bmhcnodeidchange = TRUE;  //等到应答后更新
	}

	//终端组帧发送
	ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
	                         eTERMINALCOM_SET_CSMACA, ptcmd->pcmdbuf, 1, *ptcmd->pret);

	//广播地址设置直接由主控器应答
	if(frametype < eLANFRAMETYPE_NODE_NET && frametype != eLANFRAMETYPE_ME)
	{
		//更新主控器信息
		s_ptcomserverinfo->tcomlaninfo.bcsmaca = ptcmd->pcmdbuf[0];
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);

		//主动应答
		if(ret != eRET_NONE)
			ack = eSERVERACK_ERR_CSMASET;

		//大小端转序
		ack = REVERSE16(ack);
		
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ServerReqAllDevProc
//功  能: eSERVERCMD_REQ_ALLADDR命令处理   查询所有设备
//参  数: 无
//返  回: 无
//============================================================================
static void ServerReqAllDevProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	struct list_head* plist = NULL;
	TNodeInfo* pnode = NULL;
	uint_8 buf[BUF_NODE_SIZE_COMSERVER_SEND];
	uint_16 buflen = 0;
	uint_16 nodeid = 0;
	uint_16* pnodenum;
	MQX_TICK_STRUCT ticks;
			
	if(list_empty(s_ptcomserverinfo->pterminalnodedatalist) == FALSE)
	{
		plist = s_ptcomserverinfo->pterminalnodedatalist->next;

		_time_get_ticks(&ticks);
		while(plist != NULL && is_timeout(ticks.TICKS[0], 30000) == FALSE)
		{
			pnode = list_entry(plist, TNodeInfo, list);

			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[ServerReqAllDevProc]: nodeid=");
			DEBUGOUT_DEC(DEBUGOUT_LEVEL_INFO, nodeid);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, ", pnode->nodeid=");
			DEBUGOUT_DEC(DEBUGOUT_LEVEL_INFO, pnode->tnodeinfosave.nodeid);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "\r\n");
			
			//nodeid
			if(nodeid == 0 || pnode->tnodeinfosave.nodeid != nodeid + 1)
			{
				//大小端转序
				*(uint_32*)(&buf[buflen]) = REVERSE16(pnode->tnodeinfosave.nodeid); //高2字节为0
				buflen += 4;
				pnodenum = (uint_16*)&buf[buflen];
				buflen += 2;
				*pnodenum = 1;
			}
			else
				*pnodenum += 1;
			
			nodeid = pnode->tnodeinfosave.nodeid;
			plist = plist->next;
			//设备结束
			if(plist == s_ptcomserverinfo->pterminalnodedatalist)
			{
				//大小端转序
				*pnodenum = REVERSE16(*pnodenum);
				
				ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
					       eSERVERCMD_ACK_ALLADDR, buf, buflen, *ptcmd->pret);
				plist = NULL;
				break;
			}
			
			//buf满
			if(s_ptcomserverinfo->tcomgprsinfo.heartmax - buflen <= (s_ptcomserverinfo->tcomgprsinfo.heartmax >> 3))
			{
				//大小端转序
				*pnodenum = REVERSE16(*pnodenum);

				ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
					       eSERVERCMD_ACK_ALLADDR, buf, buflen, *ptcmd->pret);
				buflen = 0;
				nodeid = 0;
			}
		}
	}
	else
		ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			         s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			         eSERVERCMD_ACK_ALLADDR, NULL, 0, *ptcmd->pret);
}


//============================================================================
//函数名: ServerAddDevProc
//功  能: eSERVERCMD_ADDDEV命令处理   增加设备
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerAddDevProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_16 buflen = ptcmd->pchk - ptcmd->pcmdbuf;
	uint_8 i, groupnum;
	uint_16 nodeid;
	uint_16 j, devnum;
	TNodeInfo* pnode = NULL;
	bool bcallback = FALSE;
	struct list_head* plist = NULL;
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && (buflen % 6) == 0)
	{
		groupnum = buflen / 6;

		for(i = 0; i < groupnum; i++)
		{
			nodeid = *((uint_16*)(ptcmd->pcmdbuf + i * 6 + 2));  //高2字节不考虑
			devnum = *((uint_16*)(ptcmd->pcmdbuf + i * 6 + 4));
			//大小端转序
			nodeid = REVERSE16(nodeid);
			devnum = REVERSE16(devnum);

			//查找插入对应终端控制器
			for(j = 0; j < devnum; j++, nodeid++)
			{
				if(NodeAddrCheck(nodeid) == NULL)
				{
					//获取空链表节点
					if(list_empty(s_ptcomserverinfo->pterminalnodefreelist))		
					{
						ack = eSERVERACK_ERR_ADDDEV;
						break;
					}

					plist = s_ptcomserverinfo->pterminalnodefreelist->next;
					list_del(plist);	
					pnode = list_entry(plist, TNodeInfo, list);
					pnode->tnodeinfosave.route = eZIGFRAMEROUTE_V1;  //默认兼容1.X版路由方式
					pnode->tnodeinfosave.groupid = 0;                //默认无组号
					pnode->tnodeinfosave.nodeid = nodeid;
					pnode->tnodeinfosave.polltimeout = GPRS_DEFAULT_POLL_TIMEOUT;
					pnode->tnodeinfosave.pollcount1 = GPRS_DEFAULT_POLL_COUNT1;
					pnode->tnodeinfosave.pollcount2 = GPRS_DEFAULT_POLL_COUNT2;
					pnode->failcount = 0;
					pnode->phytype = 0xFF;
					
					memset(pnode->data, 0xFF, APP_DATALEN);
					list_add_tail(&pnode->list, s_ptcomserverinfo->pterminalnodedatalist);
					bcallback = TRUE;
					
					DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "[ServerAddDevProc]: add dev nodeid = ");
					DEBUGOUT_HEX(DEBUGOUT_LEVEL_ALARM, nodeid);
					DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "\r\n");

//					//20160421
//					TerminalDevAddDelProc();
				}
			}
		}
	}
	else
		ack = eSERVERACK_ERR_ADDDEV;

	if(bcallback)
		s_ptcomserverinfo->pcomterminaldevchcallbak();   //TerminalDevAddDelProc();

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			         s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				     eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}


//============================================================================
//函数名: ServerDelDevProc
//功  能: eSERVERCMD_DELDEV命令处理  删除设备
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerDelDevProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_16 buflen = ptcmd->pchk - ptcmd->pcmdbuf;
	uint_8 i, groupnum;
	uint_16 nodeid;
	uint_16 j, devnum;
	TNodeInfo* pnode = NULL;
	bool bcallback = FALSE;
	struct list_head* dellist = NULL;
	MQX_TICK_STRUCT ticks;
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && (buflen % 6) == 0)
	{
		groupnum = buflen / 6;

		for(i = 0; i < groupnum; i++)
		{
			nodeid = *((uint_16*)(ptcmd->pcmdbuf + i * 6 + 2));
			devnum = *((uint_16*)(ptcmd->pcmdbuf + i * 6 + 4));

			//大小端转序
			nodeid = REVERSE16(nodeid);
			devnum = REVERSE16(devnum);

			//20130105删除所有设备
			if(nodeid == 0 && devnum == 0 && 
				list_empty(s_ptcomserverinfo->pterminalnodedatalist) == FALSE)
			{
				dellist = s_ptcomserverinfo->pterminalnodedatalist->next;
				
				_time_get_ticks(&ticks);
				while(dellist != NULL && is_timeout(ticks.TICKS[0], 30000) == FALSE)
				{
			        pnode = list_entry(dellist, TNodeInfo, list);

					//下一个节点
					dellist = dellist->next;

					//从datalist中删除
					list_del(&pnode->list);	
					
					DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "[ServerDelDevProc]: del dev nodeid = ");
					DEBUGOUT_HEX(DEBUGOUT_LEVEL_ALARM, pnode->tnodeinfosave.nodeid);
					DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "\r\n");
					
					//加入freelist
					pnode->tnodeinfosave.route = 0xFF;
					pnode->tnodeinfosave.groupid = 0xFF;
					pnode->tnodeinfosave.nodeid = 0xFFFF;
					pnode->tnodeinfosave.polltimeout = 0xFFFF;
					pnode->tnodeinfosave.pollcount1 = 0xFF;
					pnode->tnodeinfosave.pollcount2 = 0xFF;
					pnode->failcount = 0xFF;
					pnode->phytype = 0xFF;
					
					memset(pnode->data, 0xFF, APP_DATALEN);
					list_add_tail(&pnode->list, s_ptcomserverinfo->pterminalnodefreelist);
					bcallback = TRUE;

					//设备结束
					if(dellist == s_ptcomserverinfo->pterminalnodedatalist)
						dellist = NULL;
				}
				break;
			}
			
			for(j = 0; j < devnum; j++, nodeid++)
			{
				pnode = NodeAddrCheck(nodeid);
				if(pnode != NULL)
				{
					//从datalist中删除
					list_del(&pnode->list);	
					//加入freelist
					pnode->tnodeinfosave.route = 0xFF;
					pnode->tnodeinfosave.groupid = 0xFF;
					pnode->tnodeinfosave.nodeid = 0xFFFF;
					pnode->tnodeinfosave.polltimeout = 0xFFFF;
					pnode->tnodeinfosave.pollcount1 = 0xFF;
					pnode->tnodeinfosave.pollcount2 = 0xFF;
					pnode->failcount = 0xFF;
					pnode->phytype = 0xFF;
					
					memset(pnode->data, 0xFF, APP_DATALEN);
					list_add_tail(&pnode->list, s_ptcomserverinfo->pterminalnodefreelist);
					bcallback = TRUE;
					
					DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[ServerDelDevProc]: del dev nodeid = ");
					DEBUGOUT_HEX(DEBUGOUT_LEVEL_INFO, nodeid);
					DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "\r\n");
				}
			}
		}
	}
	else
		ack = eSERVERACK_ERR_DELDEV;

	if(bcallback)
		s_ptcomserverinfo->pcomterminaldevchcallbak();

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			            s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				        eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//函数名: ServerIPPortReqProc
//功  能: eSERVERCMD_IPPORTREQ命令处理   查询服务器IP Port
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerIPPortReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_IPPORTREQ);   //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
	uint_8 buf[6];
		
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_IPPORT;
		memcpy(buf, s_ptcomserverinfo->tcomgprsinfo.gprsdstip, 4);
		//memcpy(&buf[4], (int_8*)&s_ptcomserverinfo->tcomgprsinfo.gprsdstport, 2);
		*(uint_16*)&buf[4] = REVERSE16(s_ptcomserverinfo->tcomgprsinfo.gprsdstport);
		pack = buf;
		acklen = 6;
	}

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			            s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				        acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerIPPortSetProc
//功  能: eSERVERCMD_IPPORTSET命令处理  设置服务器IP Port
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerIPPortSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_IPPORTSET;   //错误应答
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && (ptcmd->pchk - ptcmd->pcmdbuf) == 6)
	{
		memcpy(s_ptcomserverinfo->tcomgprsinfo.gprsdstip, ptcmd->pcmdbuf, 4);
		memcpy((int_8*)&s_ptcomserverinfo->tcomgprsinfo.gprsdstport, &ptcmd->pcmdbuf[4], 2);

		//大小端转序
		s_ptcomserverinfo->tcomgprsinfo.gprsdstport = REVERSE16(s_ptcomserverinfo->tcomgprsinfo.gprsdstport);
		
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
		{
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomgprsinfo, sizeof(TComGprsInfo), 0);
			ack = eSERVERACK_ERR_NONE;
		}
	}

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			            s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				        eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//函数名: ServerVerReqProc
//功  能: eSERVERCMD_REQ_VERSION命令处理  版本查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerVerReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 buf[6];
	uint_16 acktype = eSERVERCMD_ACK_VERSION;
	uint_16 acklen = 6;

	//主控器查询
	if(eLANFRAMETYPE_ME == frametype)
	{
		buf[0] = VER_PROT_MAJOR;
		buf[1] = VER_PROT_MINOR;
		buf[2] = VER_SOFT_MAJOR;
		buf[3] = VER_SOFT_MINOR;
		buf[4] = VER_HARD_MAJOR;
		buf[5] = VER_HARD_MINOR;
	}
	//地址错误
	else
	{
		acktype = eSERVERCMD_ACK;
		*((uint_16*)buf) = REVERSE16(eSERVERACK_ERR_ADDR);
		acklen = 2;
	}
	
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
					acktype, buf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerAlarmPhoneFlagReqProc
//功  能: eSERVERCMD_REQ_MOBILEALARM命令处理   手机告警功能查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerAlarmPhoneFlagReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 buf[2];
	uint_16 acktype = eSERVERCMD_ACK_MOBILEALARM;
	uint_16 acklen = 1;

	//主控器查询
	if(eLANFRAMETYPE_ME == frametype)
	{
		buf[0] = s_ptcomserverinfo->tcomgsminfo.bgsmalarm;
	}
	//地址错误
	else
	{
		acktype = eSERVERCMD_ACK;
		*((uint_16*)buf) = REVERSE16(eSERVERACK_ERR_ADDR);
		acklen = 2;
	}
	
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
					acktype, buf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerAlarmPhoneFlagSetProc
//功  能: eSERVERCMD_SET_MOBILEALARM命令处理  手机告警功能设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerAlarmPhoneFlagSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_MOBILEALARMSET;   //错误应答
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && (ptcmd->pchk - ptcmd->pcmdbuf)== 1)
	{
		if(s_ptcomserverinfo->tcomgsminfo.bgsmalarm == ptcmd->pcmdbuf[0]
			|| s_ptcomserverinfo->tcomgsminfo.bgsmalarm && ptcmd->pcmdbuf[0] > 0)
			ack = REVERSE16(eSERVERACK_ERR_NONE);
		else
		{
			s_ptcomserverinfo->tcomgsminfo.bgsmalarm = ptcmd->pcmdbuf[0];
			if(s_ptcomserverinfo->pcominfochcallback != NULL)
			{
				s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomgsminfo, sizeof(TComGSMInfo), 2);
				ack = eSERVERACK_ERR_NONE;
			}
		}
	}

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			            s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				        eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//函数名: ServerAlarmPhoneReqProc
//功  能: eSERVERCMD_REQ_MOBILEALARMNUM命令处理   手机告警号码查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerAlarmPhoneReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 buf[100];
    uint_8 i;
	uint_16 acktype = eSERVERCMD_ACK_MOBILEALARMNUM;
	uint_16 acklen;

	//主控器查询
	if(eLANFRAMETYPE_ME == frametype)
	{
		for(i = 0; i < s_ptcomserverinfo->tcomgsminfo.phonenum; i++)
			memcpy(&buf[i * 11], s_ptcomserverinfo->tcomgsminfo.phone[i], 11);

		acklen = 11 * s_ptcomserverinfo->tcomgsminfo.phonenum;
		for(i = 0; i < acklen; i++)
			buf[i] -= '0';
	}
	//地址错误
	else
	{
		acktype = eSERVERCMD_ACK;
		*((uint_16*)buf) = REVERSE16(eSERVERACK_ERR_ADDR);
		acklen = 2;
	}
	
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
					acktype, buf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerAlarmPhoneSetProc
//功  能: eSERVERCMD_SET_MOBILEALARM命令处理   手机告警号码设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerAlarmPhoneSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_MOBILEALARMNUMSET;   //错误应答
	uint_8 i, j;

	i = (ptcmd->pchk - ptcmd->pcmdbuf) / 11;
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && i <= PHONENUM_MAX
		&& ((ptcmd->pchk - ptcmd->pcmdbuf) % 11) == 0)
	{
		s_ptcomserverinfo->tcomgsminfo.phonenum = i;
		for(i = 0; i < s_ptcomserverinfo->tcomgsminfo.phonenum; i++)
		{
			memcpy(s_ptcomserverinfo->tcomgsminfo.phone[i], &ptcmd->pcmdbuf[i * 11], 11);
			for(j = 0; j < 11; j++)
				s_ptcomserverinfo->tcomgsminfo.phone[i][j] += '0';
		}
		
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
		{
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomgsminfo, sizeof(TComGSMInfo), 2);
			ack = eSERVERACK_ERR_NONE;
		}
	}

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//函数名: ServerPollTimeReqProc
//功  能: eSERVERCMD_POLLTIMEREQ命令处理   查询主控器轮询时间
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerPollTimeReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = eSERVERACK_ERR_POLLTIMEREQ;   //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
		
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_POLLTIME;
		ack = (s_ptcomserverinfo->tcomgprsinfo.polltime / 1000) ;
	}

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerPollTimeSetProc
//功  能: eSERVERCMD_POLLTIMESET命令处理   设置主控器轮询时间
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerPollTimeSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;	 
	uint_16 ack = eSERVERACK_ERR_POLLTIMESET;	 //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && (ptcmd->pchk - ptcmd->pcmdbuf) == 2)
	{
		s_ptcomserverinfo->tcomgprsinfo.polltime = REVERSE16(*(uint_16*)ptcmd->pcmdbuf) * 1000;
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
		{
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomgprsinfo, sizeof(TComGprsInfo), 0);
			ack = eSERVERACK_ERR_NONE;
		}
	}

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: PhoneAlarmProc
//功  能: eSERVERCMD_PHONEALARM_DATA命令处理  手机告警内容发送
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 PhoneAlarmProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 len = ptcmd->pchk - ptcmd->pcmdbuf;
	uint_8 i, tmp;

	//长度校验
	if(len > 100 || (len % 2) != 0)
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[PhoneAlarmProc]: len err!\r\n");
		return eRET_LEN;
	}

	//手机短信告警
	if(len < 100)
	{
		g_gsmsendlen = 0;
		
		//字符转换
		for(i = 0; i < len; i++)
		{
			//高4位
			tmp = (ptcmd->pcmdbuf[i] & 0xF0) >> 4;
			g_gsmsendbuf[g_gsmsendlen++] = (tmp < 10) ? ('0' + tmp) : ('A' + tmp - 10);

			//低4位
			tmp = ptcmd->pcmdbuf[i] & 0x0F;
			g_gsmsendbuf[g_gsmsendlen++] = (tmp < 10) ? ('0' + tmp) : ('A' + tmp - 10);
		}
	}
	
	return eRET_NONE;
}

//============================================================================
//函数名: ServerBroadSetTimeReqProc
//功  能: eSERVERCMD_REQ_BROADSETTIME命令处理  查询主控器广播设置命令执行次数
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerBroadSetTimeReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK_BROADSETTIME;
	uint_8 buf[2];
	uint_8 acklen;

	//主控器查询
	if(eLANFRAMETYPE_ME == frametype)
	{
		buf[0] = s_ptcomserverinfo->tcomgprsinfo.broadsettime;
		acklen = 1;
	}
	//地址错误
	else
	{
		//大小端转序
		*((uint_16*)buf) = REVERSE16(eSERVERACK_ERR_ADDR);
		acklen = 2;
	}
	
	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, buf, acklen, *ptcmd->pret);
	
}

//============================================================================
//函数名: ServerBroadSetTimeSetProc
//功  能: eSERVERCMD_SET_MOBILEALARM命令处理  设置主控器广播设置命令执行次数
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerBroadSetTimeSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_BROADSETTIMESET;   //错误应答
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && (ptcmd->pchk - ptcmd->pcmdbuf)== 1 && ptcmd->pcmdbuf[0] > 0)
	{
		if(s_ptcomserverinfo->tcomgprsinfo.broadsettime == ptcmd->pcmdbuf[0])
			ack = eSERVERACK_ERR_NONE;
		else
		{
			s_ptcomserverinfo->tcomgprsinfo.broadsettime = ptcmd->pcmdbuf[0];
			if(s_ptcomserverinfo->pcominfochcallback != NULL)
			{
				s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomgprsinfo, sizeof(TComGprsInfo), 0);
				ack = eSERVERACK_ERR_NONE;
			}
		}
	}

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//函数名: ServerIPReqMACIP
//功  能: eSERVERCMD_REQ_MAC命令处理  通过ZIG_IP地址查询ZIG_MACIP地址
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerIPReqMACIP(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                      eTERMINALCOM_REQ_MAC, NULL, 0, *ptcmd->pret);
}

//============================================================================
//函数名: ServerMACReqMACIP
//功  能: eSERVERCMD_REQ_MACIP命令处理   通过ZIG_MAC地址查询ZIG_MACIP地址
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerMACReqMACIP(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
    uint_8 ret = eRET_NONE;
	uint_16 errack = eSERVERACK_ERR_ADDR;

	//主控器查询，MAC地址须转换为全网广播
	if(eLANFRAMETYPE_ME == frametype && ptcmd->pchk == ptcmd->pcmdbuf + 8)
		ret = ComTerminalSendCmd(BROADCAST_NETID, *ptcmd->proute, 0, BROADCAST_NODEID, eTERMINALCOM_REQ_MACIP, ptcmd->pcmdbuf, 8, *ptcmd->pret);
	//地址错误
	else
	{
		//大小端转序
		errack = REVERSE16(errack);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
					eSERVERCMD_ACK, (uint_8*)&errack, 2, *ptcmd->pret);
	}
	
	return ret;
}

//============================================================================
//函数名: ServerMACSetIP
//功  能: eSERVERCMD_SET_MACIP命令处理  通过ZIG_MAC地址设置ZIG_IP地址
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerMACSetIP(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
    uint_8 ret = eRET_NONE;
	uint_16 errack = eSERVERACK_ERR_ADDR;

	//主控器查询，MAC地址须转换为全网广播
	if(eLANFRAMETYPE_ME == frametype && ptcmd->pchk == ptcmd->pcmdbuf + 14)
	{
		ret = ComTerminalSendCmd(BROADCAST_NETID, *ptcmd->proute, 0, BROADCAST_NODEID, eTERMINALCOM_SET_MACIP, ptcmd->pcmdbuf, 14, *ptcmd->pret);
	}
	//地址错误
	else
	{
		//大小端转序
		errack = REVERSE16(errack);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
					eSERVERCMD_ACK, (uint_8*)&errack, 2, *ptcmd->pret);
	}
	
	return ret;
}

//============================================================================
//函数名: ServerResetTimeReqProc
//功  能: eSERVERCMD_REQ_RESETIME命令处理   查询主控器或终端复位时间
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerResetTimeReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = eSERVERACK_ERR_RESETIMEREQ;   //错误应答
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_RESETIME;
		ack = s_ptcomserverinfo->tcomgprsinfo.cpuresettime / 1000;
	}

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMCarrierReqProc
//功  能: eSERVERCMD_REQ_WSMCARRIER命令处理   主控器运营商查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMCarrierReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_WSMCARRIERREQ);   //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_WSMCARRIER;
		*pack = s_ptcomserverinfo->tcomgprsinfo.gprslinktype;
		acklen = 1;
	}

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMCarrierSetProc
//功  能: eSERVERCMD_SET_WSMCARRIER命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMCarrierSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	

	return 0;
}

//============================================================================
//函数名: ServerWSMVolReqProc
//功  能: eSERVERCMD_REQ_WSMVOL命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMVolReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMCurReqProc
//功  能: eSERVERCMD_REQ_WSMCUR命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMCurReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMSwitchInReqProc
//功  能: eSERVERCMD_REQ_WSMSWITCHIN命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMSwitchInReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMSwitchOutReqProc
//功  能: eSERVERCMD_REQ_WSMSWITCHOUT命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMSwitchOutReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMSwitchOutSetProc
//功  能: eSERVERCMD_SET_WSMSWITCHOUT命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMSwitchOutSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMCompileTimeReqProc
//功  能: eSERVERCMD_REQ_WSMCOMPILETIME命令处理  主控器编译时间查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMCompileTimeReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_WSMCOMPILETIMEREQ);   //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 buf[50];
	uint_8 acklen = 2;
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_WSMCOMPILETIME;
		pack = buf;
		acklen = strlen((int_8*)CompileDate);
		memcpy(buf, CompileDate, acklen);
		memcpy(buf + acklen, CompileTime, strlen((int_8*)CompileTime));
		acklen += strlen((int_8*)CompileTime);
	}

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMSDDataReqProc
//功  能: eSERVERCMD_REQ_SDDATA命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMSDDataReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMHeartAttrReqProc
//功  能: eSERVERCMD_REQ_WSMHEARTATTR命令处理  主控器心跳包属性查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartAttrReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_8 buf[11];
	uint_8 acklen = 2;

	*((uint_16*)buf) = REVERSE16(eSERVERACK_ERR_WSMHEARTATTRREQ);  //错误应答
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_WSMHEARTATTR;
		buf[0] = s_ptcomserverinfo->tcomgprsinfo.hearttype;
		*((uint_16*)(buf+1)) = REVERSE16(s_ptcomserverinfo->tcomgprsinfo.hearttime / 1000);
		*((uint_16*)(buf+3))  = REVERSE16(s_ptcomserverinfo->tcomgprsinfo.heartmax);
		buf[5] = s_ptcomserverinfo->tcomgprsinfo.devinfonum;
		buf[6] = s_ptcomserverinfo->tcomgprsinfo.devinfotype;
		acklen = 11;
	}

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, buf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMHeartAttrSetProc
//功  能: eSERVERCMD_SET_WSMHEARTATTR命令处理  主控器心跳包属性设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartAttrSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;	 
	uint_16 ack = eSERVERACK_ERR_WSMHEARTATTRSET;	 //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
	uint_8 hearttype, devinfonum, devinfotype;
	uint_16 hearttime;
	uint_16 heartmax;
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && 11 == (ptcmd->pchk - ptcmd->pcmdbuf))
	{
		hearttype = ptcmd->pcmdbuf[0];
		hearttime = REVERSE16(*((uint_16*)(ptcmd->pcmdbuf+1)));
		heartmax = REVERSE16(*((uint_16*)(ptcmd->pcmdbuf+3)));
		devinfonum = ptcmd->pcmdbuf[5];
		devinfotype = ptcmd->pcmdbuf[6];
		if(hearttype < eHEARTTYPE_MAX && heartmax >= 100 && heartmax <= 1000 && devinfonum < 4)
		{
			s_ptcomserverinfo->tcomgprsinfo.hearttype = hearttype;
			s_ptcomserverinfo->tcomgprsinfo.hearttime = hearttime * 1000;
			s_ptcomserverinfo->tcomgprsinfo.heartmax = heartmax;
			s_ptcomserverinfo->tcomgprsinfo.devinfonum = devinfonum;
			s_ptcomserverinfo->tcomgprsinfo.devinfotype = devinfotype;
			if(s_ptcomserverinfo->pcominfochcallback != NULL)
			{
				s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomgprsinfo, sizeof(TComGprsInfo), 0);
				ack = eSERVERACK_ERR_NONE;
			}
		}
	}

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMSysTimeSetProc
//功  能: eSERVERCMD_SET_SYSTIME命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMSysTimeSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	

	return 0;
}

//============================================================================
//函数名: ServerWSMTimeConReqProc
//功  能: eSERVERCMD_REQ_TIMECONTROL命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMTimeConReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMTimeConSetProc
//功  能: eSERVERCMD_SET_TIMECONTROL命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMTimeConSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMPollAttrReqProc
//功  能: eSERVERCMD_REQ_WSMPOLLATTR命令处理  主控器轮询属性查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMPollAttrReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_WSMPOLLATTRREQ);   //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 buf[12];
	uint_16 acklen = 2;
	uint_8 groupid;
	uint_16 nodeid;
	TNodeInfo* pnode = NULL;
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && 3 == ptcmd->pchk - ptcmd->pcmdbuf)
	{
		groupid = ptcmd->pcmdbuf[0];
		nodeid = REVERSE16(*((uint_16*)&ptcmd->pcmdbuf[1]));
		pnode = NodeAddrCheck(nodeid);
		if(pnode != NULL && pnode->tnodeinfosave.groupid == groupid)
		{
			acktype = eSERVERCMD_ACK_WSMPOLLATTR;
			pack = buf;
			acklen = 0;
			buf[acklen++] = groupid;
			*((uint_16*)&buf[acklen]) = REVERSE16(nodeid);
			acklen += 2;
			buf[acklen++] = pnode->tnodeinfosave.route;
			*((uint_16*)&buf[acklen]) = REVERSE16(pnode->tnodeinfosave.polltimeout);
			acklen += 2;
			buf[acklen++] = pnode->tnodeinfosave.pollcount1;
			buf[acklen++] = pnode->tnodeinfosave.pollcount2;
			acklen += 4;   //reverse
		}
	}

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMPollAttrSetProc
//功  能: eSERVERCMD_SET_WSMPOLLATTR命令处理   主控器轮询属性设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMPollAttrSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = eSERVERACK_ERR_WSMPOLLATTRSET;   //错误应答
	uint_16 nodeid;
	TNodeInfo* pnode = NULL;
	struct list_head* settlist = NULL;
	MQX_TICK_STRUCT ticks;
	uint_8 groupid;

	//长度校验
	if(eLANFRAMETYPE_ME == frametype && 12 == ptcmd->pchk - ptcmd->pcmdbuf)
	{
		groupid = ptcmd->pcmdbuf[0];
		nodeid = REVERSE16(*((uint_16*)&ptcmd->pcmdbuf[1]));
		//节点属性设置
		if(nodeid > 10)
		{
			pnode = NodeAddrCheck(nodeid);
            if(pnode != NULL)
            {
				if(groupid != 0xFF)
					pnode->tnodeinfosave.groupid = groupid;
				if(ptcmd->pcmdbuf[3] != 0xFF)
					pnode->tnodeinfosave.route = ptcmd->pcmdbuf[3];
				if(*((uint_16*)&ptcmd->pcmdbuf[4]) <= 10000)
					pnode->tnodeinfosave.polltimeout = REVERSE16(*((uint_16*)&ptcmd->pcmdbuf[4]));
				if(ptcmd->pcmdbuf[6] <= 100)
					pnode->tnodeinfosave.pollcount1 = ptcmd->pcmdbuf[6];
				if(ptcmd->pcmdbuf[7] <= pnode->tnodeinfosave.pollcount1)
					pnode->tnodeinfosave.pollcount2 = ptcmd->pcmdbuf[7];
				ack = eSERVERACK_ERR_NONE;
            }
		}
		//组属性设置
		else if(BROADCAST_NODEID == nodeid)
		{
			settlist = s_ptcomserverinfo->pterminalnodedatalist->next;
//			ticks = sys_tick;
			_time_get_ticks(&ticks);
			while(settlist != NULL && is_timeout(ticks.TICKS[0], 5000) == FALSE)
			{
				pnode = list_entry(settlist, TNodeInfo, list);
				if(pnode != NULL && (BROADCAST_GROUPID == groupid || groupid == pnode->tnodeinfosave.groupid))
				{
					if(ptcmd->pcmdbuf[3] != 0xFF)
						pnode->tnodeinfosave.route = ptcmd->pcmdbuf[3];
					if(REVERSE16(*((uint_16*)&ptcmd->pcmdbuf[4])) <= 10000)
						pnode->tnodeinfosave.polltimeout = REVERSE16(*((uint_16*)&ptcmd->pcmdbuf[4]));
					if(ptcmd->pcmdbuf[6] <= 100)
						pnode->tnodeinfosave.pollcount1 = ptcmd->pcmdbuf[6];
					if(ptcmd->pcmdbuf[7] <= pnode->tnodeinfosave.pollcount1)
						pnode->tnodeinfosave.pollcount2 = ptcmd->pcmdbuf[7];
					ack = eSERVERACK_ERR_NONE;
				}
				settlist = settlist->next;
			}
		}
	}

	//属性变化存储
	if(eSERVERACK_ERR_NONE == ack)
		s_ptcomserverinfo->pcomterminaldevchcallbak();

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMDevGroupReqProc
//功  能: eSERVERCMD_REQ_WSMDEVGROUP命令处理  主控器终端分组查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMDevGroupReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERCMD_ERR_REQ_WSMDEVGROUP);   //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 buf[900];
	uint_16 acklen = 2;
	uint_8 i;
	TNodeInfo* pnode = NULL;
	struct list_head* reqlist = NULL;
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype && 1 == ptcmd->pchk - ptcmd->pcmdbuf)
	{
		acktype = eSERVERCMD_ACK_WSMDEVGROUP;
		pack = buf;
		buf[0] = ptcmd->pcmdbuf[0];
		acklen = 1;

		list_for_each(reqlist, s_ptcomserverinfo->pterminalnodedatalist)
		{
	        pnode = list_entry(reqlist, TNodeInfo, list);
			if(pnode != NULL && ptcmd->pcmdbuf[0] == pnode->tnodeinfosave.groupid)
			{
				*((uint_16*)&buf[acklen]) = REVERSE16(pnode->tnodeinfosave.nodeid);
				acklen += 2;
			}
			pnode = NULL;
		}
	}

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMDevGroupSetProc
//功  能: eSERVERCMD_SET_WSMDEVGROUP命令处理  主控器终端分组设置
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMDevGroupSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = eSERVERCMD_ERR_SET_WSMDEVGROUP;   //错误应答
	uint_16 len = ptcmd->pchk - ptcmd->pcmdbuf;
	uint_16 devnum = len - 1;
	uint_16 i;
	uint_16* pdevid;
	TNodeInfo* pnode = NULL;
	bool bcallback = FALSE;

	//长度校验
	if(eLANFRAMETYPE_ME == frametype && len > 1 && 0 == (devnum % 2))
	{
		devnum >>= 1;
		pdevid = (uint_16*)(ptcmd->pcmdbuf + 1);
		for(i = 0; i < devnum; i++)
		{
			pnode = NodeAddrCheck(REVERSE16(*pdevid));
			if(pnode != NULL && pnode->tnodeinfosave.groupid != ptcmd->pcmdbuf[0])
			{
				pnode->tnodeinfosave.groupid = ptcmd->pcmdbuf[0];
				bcallback = TRUE;
			}
			pdevid++;
		}
		
		//时段控制
		if(bcallback && s_ptcomserverinfo->pcomterminaldevchcallbak != NULL)
		{
			s_ptcomserverinfo->pcomterminaldevchcallbak();
			ack = eSERVERACK_ERR_NONE;
		}
	}

	//大小端转序
	ack = REVERSE16(ack);

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMHeartVolNumReqProc
//功  能: eSERVERCMD_REQ_WSMVOLNUM命令处理   主控器心跳包电压组号查询
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartVolNumReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_WSMVOLNUMREQ);   //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 tmp;
	uint_8 acklen = 2;
	
	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_WSMVOLNUM;
		tmp = s_ptcomserverinfo->tcomgprsinfo.heartvolnum;   
		pack = &tmp;
		acklen = 1;
	}

	//应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ServerWSMHeartVolNumSetProc
//功  能: eSERVERCMD_SET_WSMVOLNUM命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartVolNumSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMHeartCurNumReqProc
//功  能: eSERVERCMD_REQ_WSMCURNUM命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartCurNumReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMHeartCurNumSetProc
//功  能: eSERVERCMD_SET_WSMCURNUM命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartCurNumSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMHeartSwitchInNumReqProc
//功  能: eSERVERCMD_REQ_WSMSWITCHINNUM命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartSwitchInNumReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMHeartSwitchInNumSetProc
//功  能: eSERVERCMD_SET_WSMSWITCHINNUM命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartSwitchInNumSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMHeartSwitchOutNumReqProc
//功  能: eSERVERCMD_REQ_WSMSWITCHOUTNUM命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartSwitchOutNumReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerWSMHeartSwitchOutNumSetProc
//功  能: eSERVERCMD_SET_WSMSWITCHOUTNUM命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerWSMHeartSwitchOutNumSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//函数名: ServerTerminalAppCmdProc
//功  能: 终端应用命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerTerminalAppCmdProc(const TServerCOMFrame* const ptcmd,
	uint_8 terminalappcmd, uint_8 groupsize)
{
	uint_8 ret = eRET_NONE;
	uint_8 i, buf[50];       
	TTerminalAppHead terminalapp;
	uint_8 groupnum = 0;
	
	//控制组号错误
	if(groupsize == 0 && ptcmd->pchk != ptcmd->pcmdbuf)
		goto ServerTerminalAppCmdProc_ERRRET;
	else if(groupsize > 0)
		groupnum = (ptcmd->pchk - ptcmd->pcmdbuf) / groupsize;
	if(groupnum > GROUPID_MAX)
		goto ServerTerminalAppCmdProc_ERRRET;

	//无控制组号,不判断
//	if(groupsize > 1)
//	{
//		for(i = 0; i < groupnum; i++)
//		{
//			if(ptcmd->pcmdbuf[i] > GROUPID_MAX)
//				goto ServerTerminalAppCmdProc_ERRRET;
//			i += (groupsize - 1);
//		}
//	}
	
	//地址分配
	TerminalAppStructAddr(&terminalapp, buf);
	
	//组终端命令帧
	*terminalapp.plen = 2;
	*terminalapp.pcmd = terminalappcmd;
	if(groupnum > 0 && groupsize > 0)
	{
		groupnum *= groupsize;
		*terminalapp.plen += groupnum;
		for(i = 0; i < groupnum; i++)
			terminalapp.pbuf[i] = ptcmd->pcmdbuf[i];
	}
	
	//终端组帧发送
	return ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, eTERMINALCOM_DATA, buf, *terminalapp.plen, *ptcmd->pret);
	
ServerTerminalAppCmdProc_ERRRET:
	return eRET_PARAM;
}


//============================================================================
//函数名: ServerTerminalAppCmdProc_NoGroup
//功  能: 终端应用命令处理
//参  数: 无
//返  回: 无
//============================================================================
static uint_8 ServerTerminalAppCmdProc_NoGroup(const TServerCOMFrame* const ptcmd,
	uint_8 terminalappcmd, uint_8 datasize)
{
	uint_8 ret = eRET_NONE;
	uint_8 i, buf[50];       
	TTerminalAppHead terminalapp;

	//错误
	if(datasize == 0 && ptcmd->pchk != ptcmd->pcmdbuf)
		goto ServerTerminalAppCmdProc_NoGroup_ERRRET;
	
	//地址分配
	TerminalAppStructAddr(&terminalapp, buf);
	
	//组终端命令帧
	*terminalapp.plen = 2;
	*terminalapp.pcmd = terminalappcmd;
	if(datasize > 0)
	{
		*terminalapp.plen += datasize;
		for(i = 0; i < datasize; i++)
			terminalapp.pbuf[i] = ptcmd->pcmdbuf[i];
	}
	
	//终端组帧发送
	return ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, eTERMINALCOM_DATA, buf, *terminalapp.plen, *ptcmd->pret);
	
ServerTerminalAppCmdProc_NoGroup_ERRRET:
	return eRET_PARAM;
}

//============================================================================
//函数名: ServerDetecDev
//功  能: 检测设备
//参  数: 无
//返  回: 无
//检测设备 命令格式： 地址：2B  + 命令字：1B  +  数据： 4B   + 检验和： 1B
//============================================================================
static uint_8 ServerDetecDev(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 ack = REVERSE16(eSERVERACK_ERR_DMX_DETEC_DEV);   //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 tmp;
	uint_8 ret = eRET_PARAM;
	uint_8 acklen = 4;

	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_DMX_DETEC_DEV;

//		memcpy(detec_sendcmd, &ptcmd->pcmdbuf[0], 8);
		_lwevent_clear(&lwevent_group, EVENT_DMX512);    //禁止DMX512信号输出
        //切换为发送状态
		gpio_set(CH1_DIR, 1);
		delay_ms(100);
		uart_init(CH1_UART, 9600);
		delay_ms(150);
		uart_sendN(CH1_UART, 8, &(ptcmd->pcmdbuf[0]));
		delay_ms(5);
		gpio_set(CH1_DIR, 0);           //切换为接收状态

        delay_ms(100);                   //等待中断接收数据
		//置时间位，令高优先级的task_DMX512任务得到执行
		_lwevent_set(&lwevent_group,EVENT_DMX512);
//		ack = REVERSE16(eSERVERACK_ERR_NONE);
        memcpy(pack, &detect_receive[3], 4);
//		pack = &detect_receive[3];


		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route,
										s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
										acktype, pack, acklen, *ptcmd->pret);
        return ret;
	}

	ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route,
						s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
					    acktype, pack, acklen, *ptcmd->pret);
	return ret;
}


//============================================================================
//函数名: ServerWriteSDHC
//功  能: 更新SD卡内容处理
//参  数: 无
//返  回: 无
//写SD卡命令内容 前6个字节 分别是 总帧数 帧号 本帧长 每个字段占两个字节
//============================================================================
static uint_8 ServerWriteSDHC(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 ack = REVERSE16(eSERVERACK_ERR_DMX_WRITE_SDHC);   //错误应答
	uint_8* pack = (uint_8*)&ack;
	uint_8 tmp;
	uint_8 ret = eRET_PARAM;
	uint_8 acklen = 2;
	MQX_TICK_STRUCT mqx_tick1,mqx_tick2;
	uint_16 sd_fnum, sd_fsum, sd_flen, len;
	uint_16 i,j;
    uint_8* ptemp;
	TBufNode* pnode = NULL;
	uint_8  chNodeNum = SMALL_BUF_NODE_NUM / CHNUM_DMX512;
	struct list_head* plist = NULL;
	
	sd_fsum = ((ptcmd->pcmdbuf[0])<<8) | (ptcmd->pcmdbuf[1]);   //总帧数
	sd_fnum = ((ptcmd->pcmdbuf[2])<<8) | (ptcmd->pcmdbuf[3]);   //帧号
    sd_flen = ((ptcmd->pcmdbuf[4])<<8) | (ptcmd->pcmdbuf[5]);   //本帧长

	//非主控器通道，出错
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_DMX_WRITE_SDHC;

//		len = znFAT_Open_File(&fileinfo1,"/f0_2CH.txt",0,1);
//
//		if(!len)
//			printf("[ServerWriteSDHC]: Old File open correct!\r\n");

		printf("[ServerWriteSDHC]:sd_fsum = %d, sd_fnum = %d, sd_flen = %d\r\n", sd_fsum, sd_fnum, sd_flen);

		//如果本帧是第0帧并且文件不是空文件,那么重新生成数据文件，只有第0帧做ACK
		if(sd_fnum == 0)
		{
			 g_bSDWaitEvent = FALSE;
			 g_bDMXWaitEvent = FALSE;
			 _lwevent_clear(&lwevent_group, EVENT_SDHC);      //禁止读SD卡
			 _lwevent_clear(&lwevent_group, EVENT_DMX512);    //禁止DMX512信号输出

			 //第0帧，内存切换到g_smallrecvfreelist
			 //切换前必须等待sd、dmx线程阻塞
			 while(!g_bSDWaitEvent || !g_bDMXWaitEvent)
			 {
				 _time_delay_ticks(2);	 //放弃CPU由SD任务执行
			 }
			 
			 b_enet = false;  //停止发送功能

			 //删掉原文件，重新建立一个同名文件
			 if(fileinfo1.File_Size > 0)
			 {
				dt1.date=fileinfo1.File_CDate;
				dt1.time=fileinfo1.File_CTime;
				znFAT_Close_File(&fileinfo1);
				znFAT_Delete_File("/f0_2CH.txt");
				znFAT_Create_File(&fileinfo1,"/f0_2CH.txt",&dt1);
				len = znFAT_Open_File(&fileinfo1,"/f0_2CH.txt",0,1);
				/*if(!len)
				{
					ack = REVERSE16(eSERVERACK_ERR_NONE);
					pack = (uint_8 *)&ack;
					printf("[ServerWriteSDHC]:New File open correct!\r\n");

					//只在收到第0帧的情况下进行ACK
					ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route,
						s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
						acktype, pack, acklen, *ptcmd->pret);
				}*/
			 }
				
			for(i = 0; i < CHNUM_DMX512; i++)
		    {
				g_tfdatalist[i].next = &g_tfdatalist[i];
				g_tfdatalist[i].prev = &g_tfdatalist[i];
				g_tffreelist[i].next = &g_tffreelist[i];
				g_tffreelist[i].prev = &g_tffreelist[i];
		    }
			g_smallrecvfreelist.next = &g_smallrecvfreelist;
			g_smallrecvfreelist.prev = &g_smallrecvfreelist;
			for(i = 0; i < SMALL_BUF_NODE_NUM; i++)
			{
				g_tSmallBufNode[i].len = 0;
				list_add_tail(&g_tSmallBufNode[i].list, &g_smallrecvfreelist);
			}
		}

        //printf("Start Write SD!!!\r\n");

	    //关中断
	    DISABLE_INTERRUPTS;
		//_time_get_ticks(&mqx_tick1);
		znFAT_WriteData(&fileinfo1, sd_flen, &(ptcmd->pcmdbuf[6]));
		//_time_get_ticks(&mqx_tick2);
		//printf("Write SD time = %d\r\n",mqx_tick2.TICKS[0]-mqx_tick1.TICKS[0]);
        //开中断
        ENABLE_INTERRUPTS;
		//printf("End Write SD!!!\r\n");
        znFAT_Close_File(&fileinfo1);

		//文件更新完毕，帧号=总帧数-1，置标志位，
		if(sd_fnum == (sd_fsum - 1))
		{
			b_enet = true;
			b_sd = true;      //更新完成，称为新文件
//			znFAT_Close_File(&fileinfo1);
			znFAT_Flush_FS();

	        //最后一帧，内存切换到g_tffreelist
			pnode = g_tSmallBufNode;
		    for(i = 0, j = 0; i < SMALL_BUF_NODE_NUM; )
		    {
				//给各个通道上的节点赋部分初始值
				g_tSmallBufNode[i].CHNum = 0;
				g_tSmallBufNode[i].CHSeleted = 0;
				g_tSmallBufNode[i].len = 0;

				if(++i % chNodeNum == 0)
				{
					buflist_init(&g_tfdatalist[j], &g_tffreelist[j], pnode, chNodeNum);
					pnode = &g_tSmallBufNode[i];
			    	j++;
				}
		    }
			
			g_smallrecvfreelist.next = &g_smallrecvfreelist;
			g_smallrecvfreelist.prev = &g_smallrecvfreelist;
			g_bNodeRet = FALSE;
			
			_lwevent_set(&lwevent_group, EVENT_SDHC);    //允许执行读SD卡任务
			printf("SD file finished!\r\n");
		}
		else
		{
			g_bNodeRet = TRUE;
		}

       //return ret;
	}

	//ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route,
	//					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
	//				    acktype, pack, acklen, *ptcmd->pret);
	
	return ret;
}

//============================================================================
//函数名: ServerCmdProc
//功  能: 命令处理
//参  数: const TServerCOMFrame* const ptcmd，数据命令  就是发下来的整帧
//返  回: 无
//============================================================================
static uint_8 ServerCmdProc(TServerCOMFrame* ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 errack = eSERVERACK_ERR_NONE;
	bool back = FALSE;

	
	DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[ServerCmdProc]: cmd =");
	DEBUGOUT_HEX(DEBUGOUT_LEVEL_INFO, REVERSE16(*ptcmd->pcmd));
	DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, ", frametype =");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_INFO, frametype);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "\r\n");
	
	switch(REVERSE16(*ptcmd->pcmd))
	{
	     //侦测模块任务
		case eSERVERCMD_DMX_DETEC_DEV:
			ret = ServerDetecDev(ptcmd, frametype);
			break;
	    //写SD卡任务
	    case eSERVERCMD_DMX_WRITE_SDHC:
	    	ret = ServerWriteSDHC(ptcmd, frametype);
		    break;
		//地址查询
		case eSERVERCMD_REQ_ADDR:
			ret = ServerAddrReqProc(ptcmd, frametype);
			break;
		//地址设置
		case eSERVERCMD_SET_ADDR:
			ret = ServerAddrSetProc(ptcmd, frametype);
			break;
		//通道查询
		case eSERVERCMD_REQ_RFCH:
			ret = ServerRFChReqProc(ptcmd, frametype);
			break;
		//通道设置
		case eSERVERCMD_SET_RFCH:
			ret = ServerRFChSetProc(ptcmd, frametype);
			break;
		//路由能力查询
		case eSERVERCMD_REQ_ROUTE:
			ret = ServerRouteReqProc(ptcmd, frametype);
			break;
		//路由能力设置
		case eSERVERCMD_SET_ROUTE:
			ret = ServerRouteSetProc(ptcmd, frametype);
			break;
		//转发跳数查询
		case eSERVERCMD_REQ_TRANUM:
			ret = ServerTranReqProc(ptcmd, frametype);
			break;
		//转发跳数设置
		case eSERVERCMD_SET_TRANUM:
			ret = ServerTranSetProc(ptcmd, frametype);
			break;
		//CSMACA能力查询
		case eSERVERCMD_REQ_CSMACA:
			ret = ServerCSMACAReqProc(ptcmd, frametype);
			break;
		//CSMACA能力设置
		case eSERVERCMD_SET_CSMACA:
			ret = ServerCSMACASetProc(ptcmd, frametype);
			break;
				
		//查询主控器轮询时间
		case eSERVERCMD_REQ_POLLTIME:
			ret = ServerPollTimeReqProc(ptcmd, frametype);
			break;
			
		//设置主控器轮询时间
		case eSERVERCMD_SET_POLLTIME:
			ret = ServerPollTimeSetProc(ptcmd, frametype);
			break;
			
		//查询服务器IP Port
		case eSERVERCMD_REQ_IPPORT:
			ret = ServerIPPortReqProc(ptcmd, frametype);
			break;
			
		//设置服务器IP Port
		case eSERVERCMD_SET_IPPORT:
			ret = ServerIPPortSetProc(ptcmd, frametype);
			break;
			
		//版本查询
		case eSERVERCMD_REQ_VERSION:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_VERSION, 0);
				if(ret != eRET_NONE)
				{
					errack = eTERMINALAppACK_ERR_VERREQ;
					back = TRUE;
				}
			}
			else
				ret = ServerVerReqProc(ptcmd, frametype);
			break;
			
		//主控器手机卡号查询
		case eSERVERCMD_REQ_SIMNUM:
			break;
			
		//手机告警功能查询
		case eSERVERCMD_REQ_MOBILEALARM:
			ret = ServerAlarmPhoneFlagReqProc(ptcmd, frametype);
			break;
			
		//手机告警功能设置
		case eSERVERCMD_SET_MOBILEALARM:
			ret = ServerAlarmPhoneFlagSetProc(ptcmd, frametype);
			break;
			
		//手机告警号码查询
		case eSERVERCMD_REQ_MOBILEALARMNUM:
			ret = ServerAlarmPhoneReqProc(ptcmd, frametype);
			break;
			
		//手机告警号码设置
		case eSERVERCMD_SET_MOBILEALARMNUM:
			ret = ServerAlarmPhoneSetProc(ptcmd, frametype);
			break;
			
		//设备复位
		case eSERVERCMD_RESET:
			//终端控制器
#ifdef ZIGBEE_CTRL
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_RESET, 2);
				if(ret != eRET_NONE)
				{
					errack = eTERMINALAppACK_ERR_RESET;
					back = TRUE;
				}
			}
			//主控器
			else if(eLANFRAMETYPE_ME == frametype)
#else
			if(eLANFRAMETYPE_ME == frametype)
#endif
			{
				
				back = TRUE;
			}
            //地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//终端控制器主动上传功能查询
		case eSERVERCMD_REQ_INDIFLAG:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_INDIFLAG, 0);
				if(ret != eRET_NONE)
				{
					errack = eTERMINALAppACK_ERR_INDIFLAGREQ;
					back = TRUE;
				}
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//终端控制器主动上传功能设置
		case eSERVERCMD_SET_INDIFLAG:
			//终端控制器或广播设置
			if(frametype != eLANFRAMETYPE_ME && frametype < eLANFRAMETYPE_OTHEER_NET)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_SET_INDIFLAG, 1);
				if(ret != eRET_NONE)
				{
					errack = eTERMINALAppACK_ERR_INDIFLAGSET;
					back = TRUE;
				}
				//广播设置，主控器主动正确应答
				else if(frametype < eLANFRAMETYPE_NODE_NET)
					back = TRUE;
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;

		//手机告警内容发送
		case eSERVERCMD_PHONEALARM_DATA:
			if(eLANFRAMETYPE_ME == frametype)
			{
				if(PhoneAlarmProc(ptcmd, frametype) != eRET_NONE)
					errack = eSERVERACK_ERR_PHONEALARM;
				
			}
			//地址错误
			else
				errack = eSERVERACK_ERR_ADDR;
			back = TRUE;
			break;
			
		//查询主控器广播设置命令执行次数
		case eSERVERCMD_REQ_BROADSETTIME:
			ret = ServerBroadSetTimeReqProc(ptcmd, frametype);
			break;
			
		//设置主控器广播设置命令执行次数
		case eSERVERCMD_SET_BROADSETTIME:
			ret = ServerBroadSetTimeSetProc(ptcmd, frametype);
			break;
			
		//通过ZIG_IP地址查询ZIG_MACIP地址
		case eSERVERCMD_REQ_MAC:
			ret = ServerIPReqMACIP(ptcmd, frametype);
			break;
			
		//通过ZIG_MAC地址查询ZIG_MACIP地址
		case eSERVERCMD_REQ_MACIP:
			ret = ServerMACReqMACIP(ptcmd, frametype);
			break;
			
		//通过ZIG_MAC地址设置ZIG_IP地址
		case eSERVERCMD_SET_MACIP:
			ret = ServerMACSetIP(ptcmd, frametype);
			break;
			
		//查询主控器或终端复位时间
		case eSERVERCMD_REQ_RESETIME:
			//本节点
			if(eLANFRAMETYPE_ME == frametype)
				ret = ServerResetTimeReqProc(ptcmd, frametype);
			//终端查询
			else if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_RESETIME, 0);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_RESETIMEREQ;
					back = TRUE;
				}
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//Zigbee模块看门狗复位时间查询
		case eSERVERCMD_REQ_WDITIME:
			//本节点
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_WDITIME, 0);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_WDITIMEREQ;
					back = TRUE;
				}
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		
		//Zigbee模块看门狗复位时间设置
		case eSERVERCMD_SET_WDITIME:
			//本节点
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_SET_WDITIME, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_WDITIMESET;
					back = TRUE;
				}
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//广播查询ZIG_MAC地址应答标志查询
		case eSERVERCMD_REQ_MACFLAG:
			//广播查询ZIG_MAC地址应答标志查询须转换为全网广播
			if(eLANFRAMETYPE_ME == frametype)
			{
				//终端组帧发送
				ret = ComTerminalSendCmd(BROADCAST_NETID, eZIGFRAMEROUTE_FLOOD, 0, BROADCAST_NODEID, 
				                eTERMINALCOM_REQ_MACFLAG, ptcmd->pcmdbuf, 8, *ptcmd->pret);
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//广播查询ZIG_MAC地址应答标志设置
		case eSERVERCMD_SET_MACFLAG:
			//广播查询ZIG_MAC地址应答标志设置须转换为全网广播
			if(eLANFRAMETYPE_ME == frametype)
			{
				//终端组帧发送
				ret = ComTerminalSendCmd(BROADCAST_NETID, eZIGFRAMEROUTE_FLOOD, 0, BROADCAST_NODEID, 
				                 eTERMINALCOM_SET_MACFLAG, ptcmd->pcmdbuf, 9, *ptcmd->pret);
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		// 广播查询ZIG_MAC地址
		case eSERVERCMD_MACBROADREQ:
			//主控器查询，MAC地址须转换为全网广播
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ComTerminalSendCmd(BROADCAST_NETID, eZIGFRAMEROUTE_FLOOD, 0, BROADCAST_NODEID, 
					             eTERMINALCOM_REQ_MACBROAD, NULL, 0, *ptcmd->pret);
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;

		//硬件地址查询
		case eSERVERCMD_REQ_HDADDR:
			//本节点
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_HDADDRREQ, 0);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_HDADDRREQ;
					back = TRUE;
				}
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//硬件地址设置
		case eSERVERCMD_SET_HDADDR:
			//本节点
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_HDADDRESET, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_HDADDRSET;
					back = TRUE;
				}
			}
			//地址错误
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//查询所有设备
		case eSERVERCMD_REQ_ALLADDR:
			ServerReqAllDevProc(ptcmd, frametype);
			break;
			
		//增加设备
		case eSERVERCMD_ADD_DEV:
			ret = ServerAddDevProc(ptcmd, frametype);
			break;
			
		//删除设备
		case eSERVERCMD_DEL_DEV:
			ret = ServerDelDevProc(ptcmd, frametype);
			break;

		//开关灯状态查询
		case eSERVERCMD_REQ_BULK:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_BULK, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BULKREQ;
					back = TRUE;
				}
				
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}	
			break;
			
		//开关灯状态设置
		case eSERVERCMD_SET_BULK:
#ifdef ZIGBEE_CTRL
			//单灯
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}	
#else
			errack = eSERVERACK_ERR_BULKSET;
			back = TRUE;
#endif
			break;
			
		//灯功率查询
		case eSERVERCMD_REQ_POWER:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_POWER, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_POWERREQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}	
			break;
			
		//灯功率设置
		case eSERVERCMD_SET_POWER:
#ifdef ZIGBEE_CTRL
			//终端控制器
			
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}	
#else
			errack = eSERVERACK_ERR_POWERSET;
			back = TRUE;
#endif
			break;

		//三一嵌入式模块参数查询
		case eSERVERCMD_REQ_PARAM:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_PARAM, 0);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_PARAMREQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//三一嵌入式模块参数设置
		case eSERVERCMD_SET_PARAM:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_SET_PARAM, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_PARAMSET;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//灯状态查询
		case eSERVERCMD_REQ_BULKSTATE:
			//长度错误
			if(ptcmd->pchk - ptcmd->pcmdbuf != 1)
			{
				errack = eSERVERACK_ERR_LEN;
				back = TRUE;
			}
			//终端控制器
			else if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_BULKSTATE, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BULKSTATEREQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		
		//AD值查询
		case eSERVERCMD_REQ_ADVALUE:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_ADVALUE, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_ADVALUEREQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		
		//通过MAC地址进行开关灯
		case eSERVERCMD_MACSETBULK:
#ifdef ZIGBEE_CTRL
			//主控器查询，IP地址须转换为全网广播
			if(eLANFRAMETYPE_ME == frametype)
			{
				*ptcmd->pnetid = BROADCAST_NETID;
				*ptcmd->pnodeid = REVERSE32(eZIGFRAMEROUTE_FLOOD << 24);
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_MACSETBULK, 10);
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}	
#else
			errack = eSERVERACK_ERR_BULKSET;
			back = TRUE;
#endif
			break;
        //灯具启动方式查询
		case eSERVERCMD_REQ_STARTUPMODE:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_STARTUPMODE, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_STARTUPMODEREQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//灯具启动方式设置
		case eSERVERCMD_SET_STARTUPMODE:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_SET_STARTUPMODE, 2);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_STARTUPMODESET;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//开关灯设置查询
		case eSERVERCMD_REQ_BULKSET:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_BULKSET, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BULKSETREQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//开关灯电流阈值查询
		case eSERVERCMD_REQ_BULKONCUR:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_BULKONCUR, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BULKONCURREQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//开关灯电流阈值设置
		case eSERVERCMD_SET_BULKONCUR:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_SET_BULKONCUR, 3);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BULKONCURSET;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//0x1012. 终端电压采样查询
		case eSERVERCMD_REQ_VOLTAGE:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_VOLTAGEREQ, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_VOLTAGE;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//0x1013. 终端电流采样查询
		case eSERVERCMD_REQ_CURRENT:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_CURRENTREQ, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_CURRENT;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//0x1014. 终端温度采样查询
		case eSERVERCMD_REQ_TEMPERATURE:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_TEMPERATUREREQ, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_TEMPERATURE;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//0x1015. 终端电压电流采样查询
		case eSERVERCMD_REQ_VOLCUR:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_V2POLL, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_VOLCUR;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//0x1200. BMM参数查询命令
		case eSERVERCMD_BMM_SYSPARAM_REQ:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALCOM_BMM_SYSPARAM_REQ, ptcmd->pchk - ptcmd->pcmdbuf);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BMM_SYSPARAM_REQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//0x1201. BMM参数设置命令
		case eSERVERCMD_BMM_SYSPARAM_SET:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc_NoGroup(ptcmd, eTERMINALCOM_BMM_SYSPARAM_SET, 13);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BMM_SYSPARAM_SET;
					back = TRUE;
				}
			}
			//广播，多次设置
			else if(frametype != eLANFRAMETYPE_ME)
			{
				back = TRUE;
				s_broadsetlen = ptcmd->pend - ptcmd->phead + 1;
				memcpy(s_broadsetbuf, ptcmd->phead, s_broadsetlen);
				s_broadsetcount = s_ptcomserverinfo->tcomgprsinfo.broadsettime;
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//0x1202. BMM状态1查询命令
		case eSERVERCMD_BMM_STATE1_REQ:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALCOM_BMM_STATE1_REQ, 0);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BMM_STATE1_REQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		//0x1202. BMM状态2查询
		case eSERVERCMD_BMM_STATE2_REQ:
			//终端控制器
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALCOM_BMM_STATE2_REQ, 0);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BMM_STATE2_REQ;
					back = TRUE;
				}
			}
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//主控器运营商查询
		case eSERVERCMD_REQ_WSMCARRIER:
			ret = ServerWSMCarrierReqProc(ptcmd, frametype);
			break;
		//主控器运营商设置
		case eSERVERCMD_SET_WSMCARRIER:
			ret = ServerWSMCarrierSetProc(ptcmd, frametype);
			break;
		//主控器电压采样查询
		case eSERVERCMD_REQ_WSMVOL:
			ret = ServerWSMVolReqProc(ptcmd, frametype);
			break;
		//主控器电流采样查询
		case eSERVERCMD_REQ_WSMCUR:
			ret = ServerWSMCurReqProc(ptcmd, frametype);
			break;
		//主控器开关量输入查询
		case eSERVERCMD_REQ_WSMSWITCHIN:
			ret = ServerWSMSwitchInReqProc(ptcmd, frametype);
			break;
		//主控器开关量输出查询
		case eSERVERCMD_REQ_WSMSWITCHOUT:
			ret = ServerWSMSwitchOutReqProc(ptcmd, frametype);
			break;
		//主控器开关量输出设置
		case eSERVERCMD_SET_WSMSWITCHOUT:
			ret = ServerWSMSwitchOutSetProc(ptcmd, frametype);
			break;
		//主控器编译时间查询
		case eSERVERCMD_REQ_WSMCOMPILETIME:
			ret = ServerWSMCompileTimeReqProc(ptcmd, frametype);
			break;
		//主控器SD卡内容查询
		case eSERVERCMD_REQ_WSMSDDATA:
			ret = ServerWSMSDDataReqProc(ptcmd, frametype);
			break;
		//主控器心跳包属性查询
		case eSERVERCMD_REQ_WSMHEARTATTR:
			ret = ServerWSMHeartAttrReqProc(ptcmd, frametype);
			break;
		//主控器心跳包属性设置
		case eSERVERCMD_SET_WSMHEARTATTR:
			ret = ServerWSMHeartAttrSetProc(ptcmd, frametype);
			break;
		//主控器对时
		case eSERVERCMD_SET_SYSTIME:
			ret = ServerWSMSysTimeSetProc(ptcmd, frametype);
			break;
		//主控器时段控制查询
		case eSERVERCMD_REQ_TIMECONTROL:
			ret = ServerWSMTimeConReqProc(ptcmd, frametype);
			break;
		//主控器时段控制设置
		case eSERVERCMD_SET_TIMECONTROL:
			ret = ServerWSMTimeConSetProc(ptcmd, frametype);
			break;
		//主控器轮询属性查询
		case eSERVERCMD_REQ_WSMPOLLATTR:
			ret = ServerWSMPollAttrReqProc(ptcmd, frametype);
			break;
		//主控器轮询属性设置
		case eSERVERCMD_SET_WSMPOLLATTR:
			ret = ServerWSMPollAttrSetProc(ptcmd, frametype);
			break;
		//主控器终端分组查询
		case eSERVERCMD_REQ_WSMDEVGROUP:
			ret = ServerWSMDevGroupReqProc(ptcmd, frametype);
			break;
		//主控器终端分组设置
		case eSERVERCMD_SET_WSMDEVGROUP:
			ret = ServerWSMDevGroupSetProc(ptcmd, frametype);
			break;
		//主控器心跳包电压组号查询
		case eSERVERCMD_REQ_WSMVOLNUM:
			ret = ServerWSMHeartVolNumReqProc(ptcmd, frametype);
			break;
		//主控器心跳包电压组号设置
		case eSERVERCMD_SET_WSMVOLNUM:
			ret = ServerWSMHeartVolNumSetProc(ptcmd, frametype);
			break;
		//主控器心跳包电流组号查询
		case eSERVERCMD_REQ_WSMCURNUM:
			ret = ServerWSMHeartCurNumReqProc(ptcmd, frametype);
			break;
		//主控器心跳包电流组号设置
		case eSERVERCMD_SET_WSMCURNUM:
			ret = ServerWSMHeartCurNumSetProc(ptcmd, frametype);
			break;
		//主控器心跳包开关量输入组号查询
		case eSERVERCMD_REQ_WSMSWITCHINNUM:
			ret = ServerWSMHeartSwitchInNumReqProc(ptcmd, frametype);
			break;
		//主控器心跳包开关量输入组号设置
		case eSERVERCMD_SET_WSMSWITCHINNUM:
			ret = ServerWSMHeartSwitchInNumSetProc(ptcmd, frametype);
			break;
		//主控器心跳包开关量输出组号查询
		case eSERVERCMD_REQ_WSMSWITCHOUTNUM:
			ret = ServerWSMHeartSwitchOutNumReqProc(ptcmd, frametype);
			break;
		//主控器心跳包开关量输出组号设置
		case eSERVERCMD_SET_WSMSWITCHOUTNUM:
			ret = ServerWSMHeartSwitchOutNumSetProc(ptcmd, frametype);
			break;

		default:
			errack = eSERVERACK_ERR_CMD;
			back = TRUE;
			ret = eRET_CMD;
			break;
	}

	//应答
	if(back)
	{
		//大小端转序
		errack = REVERSE16(errack);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
			        eSERVERCMD_ACK, (uint_8*)&errack, 2, *ptcmd->pret);
	}
	
	return ret;
}

//============================================================================
//函数名: ServerBroadSetProc
//功  能: 广播设置操作处理
//参  数: 无
//返  回: 无
//============================================================================
static void ServerBroadSetProc(void)
{
	TServerCOMFrame tserverframe;
	uint_8 terminalappcmd = eTERMINALAPPCMD_MAX;
	static uint_8 s_errtime = 0;
	uint_8 groupsize;
	
	//广播设置操作
	if(s_broadsetcount > 0)
	{
		//地址分配
		ServerStructAddr(&tserverframe, s_broadsetbuf, s_broadsetlen - SERVERCOMFRAME_MINLEN);

		switch(REVERSE16(*tserverframe.pcmd))
		{
			//开关灯状态设置
			case eSERVERCMD_SET_BULK:
				terminalappcmd = eTERMINALAPPCMD_SET_BULK;
				groupsize = 2;
				break;
				
			//灯功率设置
			case eSERVERCMD_SET_POWER:
				terminalappcmd = eTERMINALAPPCMD_SET_POWER;
				groupsize = 2;
				break;
				
			//0x1201. BMM参数设置命令
			case eSERVERCMD_BMM_SYSPARAM_SET:
				terminalappcmd = eTERMINALCOM_BMM_SYSPARAM_SET;
				groupsize = 11;
				break;

			default:
				break;
		}

		if(terminalappcmd < eTERMINALAPPCMD_MAX)
		{
			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[ServerBroadOptProc]: s_broadsetcount = ");
			DEBUGOUT_DEC(DEBUGOUT_LEVEL_INFO, s_broadsetcount);
			DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "\r\n");

			if(ServerTerminalAppCmdProc(&tserverframe, terminalappcmd, groupsize) == eRET_NONE)
			{
				s_broadsetcount--;
				s_bPollStart = TRUE;     //开始轮询，解决广播控制，状态查询问题
				s_errtime = 0;
			}
			else
			{
				if(++s_errtime >= 3)
				{
					s_broadsetcount--;
					s_bPollStart = TRUE;     //开始轮询，解决广播控制，状态查询问题
					s_errtime = 0;
				}
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ServerBroadOptProc]: ServerTerminalAppCmdProc failed!\r\n");
			}
		}
	}
}


//============================================================================
//函数名: ComInit
//功  能: 通信层命令处理模块初始化
//参  数: const TComInfo* const ptcominfo，通信层信息
//返  回: 无
//============================================================================
void ComServerInit(TComInfo* ptcominfo)
{
	s_ptcomserverinfo = ptcominfo;
}


//============================================================================
//函数名: ComServerProc
//功  能: 通信命令处理
//参  数: uint_8 comtype，来自server还是terminal的通信帧
//               const uint_8* const pframe，通信帧首地址
//               uint_16 framelen，pframe有效长度
//返  回: 错误码
//============================================================================
uint_8 ComServerProc(const uint_8* const pframe, uint_16 framelen)
{
	TServerCOMFrame tserverframe;
	uint_8 i;
	uint_16 errack;
	uint_8 ret = eRET_NONE; 

	//DEBUGOUT_TIME(DEBUGOUT_LEVEL_ALARM);
//	DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "[ComServerProc]: pframe is: ");
//	for(i = 0; i < (framelen - 980); i++)
//	{
//		DEBUGOUT_HEX(DEBUGOUT_LEVEL_ALARM, pframe[i]);
//		DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, ", ");
//	}
//	DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "\r\n");
	printf("[ComServerProc]: framelen=%d\r\n", framelen);


	//地址分配
	ServerStructAddr(&tserverframe, (uint_8*)pframe, framelen - SERVERCOMFRAME_MINLEN);

	//长度校验
	if(framelen < SERVERCOMFRAME_MINLEN)
	{
		errack = eSERVERACK_ERR_LEN;
		ret = eRET_LEN;
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]:ERR, framelen error!\r\n");
		goto ComServerProc_ERRRET;
	}

	//参数判断
	if(*tserverframe.phead != SERVERCOM_START_FLAG)
	{
		errack = eSERVERACK_ERR_HEAD;
		ret = eRET_PARAM;
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]:ERR, framehead error!\r\n");
		goto ComServerProc_ERRRET;
	}
	if(*tserverframe.pend != SERVERCOM_END_FLAG)
	{
		errack = eSERVERACK_ERR_END;
		ret = eRET_PARAM;
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]:ERR, frameend error!\r\n");
		goto ComServerProc_ERRRET;
	}
	
	//校验和,不包括帧头帧尾和校验字
//	if(MakeCheckInfo((uint_8*)tserverframe.pret, framelen - 3) != *tserverframe.pchk)
//	{
//		errack = eSERVERACK_ERR_CHK;
//		ret = eRET_CHK;
//		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]:ERR, chk error!\r\n");
//		goto ComServerProc_ERRRET;
//	}

	//局域网地址匹配
	i = LanFrameType(REVERSE16(*tserverframe.pnetid), REVERSE16(*tserverframe.pnodeid), *tserverframe.pgroupid);
	//其他网段帧，丢弃
	if(i >= eLANFRAMETYPE_OTHEER_NET)
	{
		errack = eSERVERACK_ERR_ADDR;
		ret = eRET_ADDR;
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]: ERR, other net frame!\r\n");
		goto ComServerProc_ERRRET;
	}
	//终端控制器帧,该路段没有此地址，丢弃
	else if(eLANFRAMETYPE_NODE_NET == i && NodeAddrCheck(REVERSE16(*tserverframe.pnodeid)) == NULL)
	{
		errack = eSERVERACK_ERR_ADDR;
		ret = eRET_ADDR;
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]: ERR, the dev is not exite!\r\n");
		goto ComServerProc_ERRRET;
	}

	//主控制帧处理
	return ServerCmdProc(&tserverframe, i);

ComServerProc_ERRRET:
	//大小端转序
	errack = REVERSE16(errack);
	//错误应答
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
					eSERVERCMD_ACK, (uint_8*)&errack, 2, *tserverframe.pret);
}

//============================================================================
//函数名: ComServerAck
//功  能: Server ACK应答
//参  数: uint_16 acktype，应答类型
//               uint_8* packbuf，应答内容
//		     uint_16 ackbuflen，应答内容长度
//返  回: 无
//备  注:对于数据量大的应答，不建议采用该接口，大数据拷贝时间长
//============================================================================
uint_8 ComServerAck(uint_16 netid, uint_8 route, uint_8 groupid, uint_16 nodeid, uint_16 acktype, uint_8* packbuf, uint_16 ackbuflen, uint_32 serverret)
{
    TBufNode* pnode = NULL;
	TServerCOMFrame tackframe;
	uint_16 len = ackbuflen + SERVERCOMFRAME_MINLEN;
	uint_8 ret = eRET_PARAM;
	TNodeInfo* pdevnode = NULL;
	
	//参数校验
	if(NULL == s_ptcomserverinfo->pserversenddatalist || NULL == s_ptcomserverinfo->pserversendfreelist)
		return ret;

	listnode_get(s_ptcomserverinfo->pserversendfreelist,&pnode);
    if(NULL != pnode && pnode->size >= len)
    {
		//地址分配
		ServerStructAddr(&tackframe, pnode->pbuf, ackbuflen);

		//组帧
		*tackframe.phead = SERVERCOM_START_FLAG;
		*tackframe.pret = serverret;
		*tackframe.pnetid = REVERSE16(netid);

		*tackframe.proute = route;
		*tackframe.pgroupid = groupid;
		*tackframe.pnodeid = REVERSE16(nodeid);

		*tackframe.pcmd = REVERSE16(acktype);
		memcpy(tackframe.pcmdbuf, packbuf, ackbuflen);
		*tackframe.pchk = MakeCheckInfo((uint_8*)tackframe.pret, len - 3);
		*tackframe.pend = SERVERCOM_END_FLAG;
		
    	//归还datalist
    	pnode->len = len;
		listnode_put(s_ptcomserverinfo->pserversenddatalist, pnode);

		ret = eRET_NONE;
	}

	//主控器信息更新
	if(netid == s_ptcomserverinfo->tcomlaninfo.netid && nodeid == s_ptcomserverinfo->tcomlaninfo.nodeid &&
		(acktype == eSERVERCMD_ACK_ADDR || acktype == eSERVERCMD_ACK_RFCH || acktype == eSERVERCMD_ACK_ROUTE || acktype == eSERVERCMD_ACK_CSMACA)
		&& s_bmhcnodeidchange)
	{
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);
		s_bmhcnodeidchange = FALSE;
	}

	//终端设备状态设置成功，更新存储状态
	if(acktype == eSERVERCMD_ACK && packbuf[0] == eSERVERACK_ERR_NONE && packbuf[1] == eSERVERACK_ERR_NONE
		&& s_bAckFlash && serverret == s_AckRet)
	{
		pdevnode = NodeAddrCheck(nodeid);
		if(pdevnode != NULL)
		{
			memcpy(pdevnode->data, s_AckData, APP_DATALEN);
			
			pdevnode->failcount = 0;
		}

		s_bAckFlash = FALSE;
	}
	return ret;
}

//============================================================================
//函数名: ComServerPoll_App
//功  能: 主控器应用轮询处理
//参  数: 无
//返  回: 无
//============================================================================
static void ComServerPoll_App(void)
{
	static uint_8 bpolltime = 0;
	static bool bpollstart = FALSE;
	MQX_TICK_STRUCT nowticks;
	static MQX_TICK_STRUCT pollticks;
	static MQX_TICK_STRUCT polltimeoutticks;
	static struct list_head* polllist = NULL;
	static TNodeInfo* pollnode = NULL;
	TTerminalAppHead terminalapp;
	uint_8 buf[5];

	//不再轮询的情况下，广播操作
	//在轮询过程中等待应答的同时，允许控制
	if(s_bPollStart == FALSE && 
		(bpollstart == FALSE || (s_bpollack && is_timeout(polltimeoutticks.TICKS[0], 50))))
		ServerBroadSetProc();
			
	//主控器轮询
	//广播设置后查询，延时1s
	if(s_bPollStart)
	{
		_time_get_ticks(&nowticks);
		if(nowticks.TICKS[0]*5 >= s_ptcomserverinfo->tcomgprsinfo.polltime - 1000)
			pollticks.TICKS[0] = nowticks.TICKS[0]*5 + 1000 - s_ptcomserverinfo->tcomgprsinfo.polltime;
		else
			pollticks.TICKS[0] = 0xFFFFFFFF - s_ptcomserverinfo->tcomgprsinfo.polltime + nowticks.TICKS[0]*5 + 1000;
	}

	if(s_ptcomserverinfo->tcomgprsinfo.polltime > 0 && 
		is_timeout(pollticks.TICKS[0], s_ptcomserverinfo->tcomgprsinfo.polltime))
	{
		_time_get_ticks(&pollticks);
//		pollticks = sys_tick;
		if(list_empty(s_ptcomserverinfo->pterminalnodedatalist) == FALSE)
		{
			bpollstart = TRUE;
			bpolltime++;
			polllist = s_ptcomserverinfo->pterminalnodedatalist;
		}
		s_bPollStart = FALSE;
		pollnode = NULL;
		s_bpollack = FALSE;
		_time_get_ticks(&nowticks);
		polltimeoutticks = nowticks;
	}

	//轮询每个终端，需等待应答或超时
	if(bpollstart && (s_bpollack == FALSE || is_timeout(polltimeoutticks.TICKS[0], pollnode->tnodeinfosave.polltimeout)))
	{
		//应答失败，单次超时，重新轮询上一终端
		if(s_bpollack && pollnode != NULL && pollnode->tnodeinfosave.pollcount1 > 0)
		{
			//超时次数
			if(pollnode->failcount++ >= pollnode->tnodeinfosave.pollcount1)
			{
				memset(pollnode->data, 0xFF, APP_DATALEN);
				polllist = polllist->next;         //下一终端
				//不完全清零，保证设备掉电的情况下，不至于查询时间过长
				pollnode->failcount -= pollnode->tnodeinfosave.pollcount2;          
			}
		}
		//成功应答
		else if(s_bpollack == FALSE)
			polllist = polllist->next;             //下一终端
		
		//轮询结束
		if(polllist == s_ptcomserverinfo->pterminalnodedatalist)
			bpollstart = FALSE;
		else
		{
			//组帧查询灯状态
			pollnode = list_entry(polllist, TNodeInfo, list);

			//防止节点访问错误,节点被删除
			if(NULL == pollnode || NULL == NodeAddrCheck(pollnode->tnodeinfosave.nodeid))
			{
				polllist = NULL;
				pollnode = NULL;
				s_bpollack = FALSE;
				bpollstart = FALSE;
			}
			//该节点允许轮询
			else if(pollnode->tnodeinfosave.pollcount1 > 0)
			{
				//地址分配
				TerminalAppStructAddr(&terminalapp, buf);
				*terminalapp.plen = 2;
				//主控器轮询处理
				if((bpolltime % 10) == 0)
				{
					*terminalapp.pcmd = eTERMINALCOM_BMM_STATE2_REQ;
				}
				else
				{
					*terminalapp.pcmd = eTERMINALCOM_BMM_STATE1_REQ;
				}
				
				//利用ret=0xFFFFFFFF判断是否为主控器轮询命令，还是服务命令
				ComTerminalSendCmd(REVERSE16(s_ptcomserverinfo->tcomlaninfo.netid), pollnode->tnodeinfosave.route, 
				                   pollnode->tnodeinfosave.groupid, REVERSE16(pollnode->tnodeinfosave.nodeid),
					               eTERMINALCOM_DATA, buf, *terminalapp.plen, 0xFFFFFFFF);
				s_bpollack = TRUE;
				_time_get_ticks(&nowticks);
				polltimeoutticks = nowticks;
			}
		}
	}
}

//============================================================================
//函数名: ComServerHeart_App
//功  能: 项目应用心跳包处理
//参  数: 无
//返  回: 无
//============================================================================
static void ComServerHeart_App()
{
	static MQX_TICK_STRUCT mqx_tick_heartticks;
//	static uint_32 heartticks = 0;
	static MQX_TICK_STRUCT allheartticks;    //整体上报时间
	static struct list_head* heartlist = NULL;
	TNodeInfo* pnode = NULL;
	uint_8 buf[BUF_NODE_SIZE_COMSERVER_SEND];
	uint_16 buflen;
	uint_16 lastnodeid = 0;
	uint_16* pnodenum = NULL;
	uint_8 i;
	MQX_TICK_STRUCT ticks;
	static uint_8 s_gsmdelindex[3] = {0, 0xFF, 0xFF};
	static uint_8 s_gsmindex = 0;

	//心跳包发送
	if(s_ptcomserverinfo->tcomgprsinfo.hearttime > 0 && (s_bLinkStart || 
//	    is_timeout(heartticks, sys_tick, s_ptcomserverinfo->tcomgprsinfo.hearttime)))
		is_timeout(mqx_tick_heartticks.TICKS[0], s_ptcomserverinfo->tcomgprsinfo.hearttime)))
	{
		_time_get_ticks(&mqx_tick_heartticks);
//		heartticks = sys_tick;
		s_bLinkStart = FALSE;

		//退出透传模式
		/*if(gprs_set_iptrans(FALSE) > 0)
		{
			DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerHeart_App]: quit iptrans failed!\r\n");
			return;
		}*/
		
		

		if(FALSE == gprs_isbroken())
		{		
			/*//获取CSQ
			if(gprs_get_csq(buf) != eGPRSRET_NONE)
			{
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerHeart_App]: GPRSGetLinkCSQ failed!\r\n");
				buf[0] = 0xFF;
			}

			//短信发送
			if(s_ptcomserverinfo->tcomgsminfo.bgsmalarm && g_gsmsendlen > 0)
			{
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerHeart_App]: gsm send!\r\n");
				for(i = 0; i < s_ptcomserverinfo->tcomgsminfo.phonenum; i++)
				{
					gsm_send(s_ptcomserverinfo->tcomgsminfo.phone[i], g_gsmsendbuf, g_gsmsendlen);
				}

				g_gsmsendlen = 0;
			}

			//进入透传模式
			if(gprs_set_iptrans(TRUE) > 0)
			{
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerHeart_App]: enter iptrans failed!\r\n");
				return;
			}*/
			
			//当前系统时间
			//get_daytime(&buf[1], &buf[2], &buf[3]);
			buflen = 4;

			/*//上报设备信息
			if(s_ptcomserverinfo->tcomgprsinfo.devinfotype > eDEVINFOTYPE_NO_SEND)
			{
				if(heartlist == NULL && list_empty(s_ptcomserverinfo->pterminalnodedatalist) == FALSE)
				{
					heartlist = s_ptcomserverinfo->pterminalnodedatalist->next;
					lastnodeid = 0;
				}

				//?????
//				ticks = sys_tick;
				while(heartlist != NULL && is_timeout(allheartticks.TICKS[0], 5000) == FALSE)
				{
					//喂狗
					FeedTheWatchDog();

					pnode = list_entry(heartlist, TNodeInfo, list);

					//防止节点访问错误,被删除
					if(NULL == pnode || NULL == NodeAddrCheck(pnode->tnodeinfosave.nodeid))
					{
						heartlist = NULL;
						break;
					}

					//地址不连续，截止心跳包
					if(lastnodeid != 0 && pnode != NULL && pnode->tnodeinfosave.nodeid != (lastnodeid + 1))
						break;

					DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[ComServerHeart_App]: lastnodeid=");
					DEBUGOUT_DEC(DEBUGOUT_LEVEL_INFO, lastnodeid);
					DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, ", pnode->tnodeinfosave.nodeid=");
					DEBUGOUT_DEC(DEBUGOUT_LEVEL_INFO, pnode->tnodeinfosave.nodeid);
					DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "\r\n");
					
					//起始地址
					if(lastnodeid == 0)
					{
						//nodeid
						*(uint_16*)(&buf[buflen]) = REVERSE16(pnode->tnodeinfosave.nodeid);
						buflen += 2;
						pnodenum = (uint_16*)&buf[buflen];
						*pnodenum = 0;
						buflen += 2;
					}

					//数据
					memcpy(&buf[buflen], pnode->data, APP_DATALEN);
					buflen += APP_DATALEN;
					
					lastnodeid = pnode->tnodeinfosave.nodeid;
					*pnodenum += 1;
					heartlist = heartlist->next;
					//设备结束
					if(heartlist == s_ptcomserverinfo->pterminalnodedatalist)
					{
						heartlist = NULL;
						if(s_ptcomserverinfo->tcomgprsinfo.devinfotype >= eDEVINFOTYPE_CHANGE_SEND &&
							is_timeout(allheartticks.TICKS[0], s_ptcomserverinfo->tcomgprsinfo.devinfotype * 60000))
						{
							_time_get_ticks(&ticks);
							allheartticks = ticks;   //更新整体上传时间
						}
					}
					
					//buf满
					if(s_ptcomserverinfo->tcomgprsinfo.heartmax - buflen <= (s_ptcomserverinfo->tcomgprsinfo.heartmax >> 3))
						br0eak;
				}	
			}

			//大小端转序
			if(pnodenum != NULL)
			{
				*pnodenum = REVERSE16(*pnodenum);
			}
			*/

//			ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route,
//					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
//				    eSERVERCMD_HEART_BMM_V1, buf, buflen, 0);

			ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
								s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
								eSERVERCMD_HEART_DMX_V1, buf, buflen, 0);
		}
	}
}



//============================================================================
//函数名: ComServerHeartProc
//功  能: 通信层心跳包组包
//参  数: 无
//返  回: 无
//============================================================================
void ComServerHeartProc()
{
	//终端轮询
	//ComServerPoll_App();
	
	//心跳包
	ComServerHeart_App();
}

