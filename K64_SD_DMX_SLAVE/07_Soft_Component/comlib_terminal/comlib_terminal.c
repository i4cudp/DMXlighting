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
//#include "buflist.h"
//#include "comlib.h"
//#include "comlib_server.h"
#include "comlib_terminal.h"
//#include "isr.h"
#include "includes.h"

static TComInfo* s_ptcomterminalinfo;
extern bool s_bpollack;

//-------------------------------------------------------------------------
//函数名: LanFrameType                                                        
//功  能: 判断局域网帧类型           
//参  数: uint_16 netid，netid
//               uint_16 nodeid，nodeid
//返  回: 无                                              
//说  明: TRUE，是自己的帧；FALSE，不是自己的帧
//-------------------------------------------------------------------------
uint_8 LanFrameType(uint_16 netid, uint_16 nodeid, uint_8 groupid)
{
	uint_8 ret = eLANFRAMETYPE_MAX;
	
	//全网广播帧
	if(netid == BROADCAST_NETID && nodeid == BROADCAST_NODEID)
		ret = eLANFRAMETYPE_ALL;
	//网内
	else if(netid == s_ptcomterminalinfo->tcomlaninfo.netid)
	{
		//本节点帧
		if(nodeid == s_ptcomterminalinfo->tcomlaninfo.nodeid)
			ret = eLANFRAMETYPE_ME;
		//节点地址为广播地址
		else if(nodeid == BROADCAST_NODEID)
		{
			//网内广播帧
			if(groupid == BROADCAST_GROUPID)
				ret = eLANFRAMETYPE_NET;
			//网内奇数组广播帧
			else if(groupid == BROADCAST_ODD_GROUPID)
				ret = eLANFRAMETYPE_ODDGROUP;
			//网内偶数组广播帧
			else if(groupid == BROADCAST_EVEN_GROUPID)
				ret = eLANFRAMETYPE_EVENGROUP;
			//网内组内广播帧
			else
				ret = eLANFRAMETYPE_GROUP;
		}
		//节点地址为奇地址
		else if(nodeid == BROADCAST_ODD_NODEID)
		{
			//网内奇地址广播帧
			if(groupid == BROADCAST_GROUPID)
				ret = eLANFRAMETYPE_ODD_NET;
			//网内奇数组奇地址广播帧
			else if(groupid == BROADCAST_ODD_GROUPID)
				ret = eLANFRAMETYPE_ODD_ODD;
			//网内偶数组奇地址广播帧
			else if(groupid == BROADCAST_EVEN_GROUPID)
				ret = eLANFRAMETYPE_ODD_EVEN;
			//网内组内奇地址广播帧
			else
				ret = eLANFRAMETYPE_ODD_GROUP;
		}
		//节点地址为偶地址
		else if(nodeid == BROADCAST_EVEN_NODEID)
		{
			//网内偶地址广播帧
			if(groupid == BROADCAST_GROUPID)
				ret = eLANFRAMETYPE_EVEN_NET;
			//网内奇数组偶地址广播帧
			else if(groupid == BROADCAST_ODD_GROUPID)
				ret = eLANFRAMETYPE_EVEN_ODD;
			//网内偶数组偶地址广播帧
			else if(groupid == BROADCAST_EVEN_GROUPID)
				ret = eLANFRAMETYPE_EVEN_EVEN;
			//网内组内偶地址广播帧
			else
				ret = eLANFRAMETYPE_EVEN_GROUP;
		}
		//网内其他节点帧
		else
			ret = eLANFRAMETYPE_NODE_NET;
	}
	//其他网段
	else 
		ret = eLANFRAMETYPE_OTHEER_NET;
	
	return ret;
}


//-------------------------------------------------------------------------
//函数名: MakeCheckInfo                                                        
//功  能: 计算帧校验信息                                
//参  数: uint_8* pbuf，校验数据首地址
//		     uint_16 buflen，校验数据长度
//返  回: 校验码                                                
//说  明: (1)采用累加和校验
//-------------------------------------------------------------------------
uint_8 MakeCheckInfo(uint_8* pbuf, uint_16 buflen)
{
    uint_8 check = 0;
	uint_16 i = 0;
	
	for(; i < buflen; i++)
		check += pbuf[i];
	
    return check;
}

//============================================================================
//函数名: TerminalStructAddr
//功  能: 结构地址分配
//参  数: 无
//返  回: 无
//============================================================================
static inline void TerminalStructAddr(TTerminalCOMFrame* ptframe, uint_8* pbuf, uint_8 cmdlen)
{
	//地址分配
	ptframe->plen = pbuf;
	ptframe->pret = (uint_32*)(ptframe->plen + 1);
	ptframe->pnetid = (uint_16*)(ptframe->pret + 1);
	ptframe->proute = (uint_8*)(ptframe->pnetid + 1);
	ptframe->pgroupid = ptframe->proute + 1;
	ptframe->pnodeid = (uint_16*)(ptframe->pgroupid + 1);
	ptframe->pcmd = (uint_8*)(ptframe->pnodeid + 1);
	ptframe->pbuf = (cmdlen > 0) ? (ptframe->pcmd + 1) : NULL;
	ptframe->pchk = (cmdlen > 0) ? (ptframe->pbuf + cmdlen) : (ptframe->pcmd + 1);
}

//============================================================================
//函数名: TerminalStructAddr
//功  能: 结构地址分配
//参  数: 无
//返  回: 无
//============================================================================
void TerminalAppStructAddr(TTerminalAppHead* ptapp, uint_8* pbuf)
{
	//地址分配
	ptapp->plen = pbuf;
	ptapp->pcmd = ptapp->plen + 1;
	ptapp->pbuf = ptapp->pcmd + 1;
}

//============================================================================
//函数名: MakeTerminalDataNode
//功  能: 终端控制组帧
//参  数: uint_16 netid, uint_32 nodeid，局域网地址
//               uint_8 cmdtypet，命令类型
//               uint_8* pcmd, uint_8 cmdlen，命令内容
//返  回: TBufNode*，命令buf
//============================================================================
static inline TBufNode* MakeTerminalDataNode(uint_16 netid, uint_8 route, uint_8 groupid, 
	uint_16 nodeid, uint_8 cmdtype, uint_8* pcmd, uint_8 cmdlen, uint_32 serverret)
{
	TBufNode* pnode = NULL;
	TTerminalCOMFrame tlanframe;
	uint_8 len = TERMINALCOMFRAME_MINLEN + cmdlen;
	
	//参数校验
	if(NULL == s_ptcomterminalinfo || NULL == s_ptcomterminalinfo->pterminalsendfreelist)
		return NULL;

	listnode_get(s_ptcomterminalinfo->pterminalsendfreelist, &pnode);
	if(NULL != pnode && pnode->size >= len)
	{
		//设置ram空间位置
		//查询命令，无命令内容
		TerminalStructAddr(&tlanframe, pnode->pbuf, cmdlen);

		//组帧
		*tlanframe.plen = len;
		*tlanframe.pret = serverret;
		*tlanframe.pnetid = netid;
		*tlanframe.proute = route;
		*tlanframe.pgroupid = groupid;
		*tlanframe.pnodeid = nodeid;
		*tlanframe.pcmd = cmdtype;
		memcpy(tlanframe.pbuf, pcmd, cmdlen);
		*tlanframe.pchk = MakeCheckInfo(pnode->pbuf, len - 1);

		pnode->len = len;
	}

	return pnode;
}

//============================================================================
//函数名: ComTerminalSysAckProc
//功  能: 终端应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalSysAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_8 ret = eRET_NONE;
	uint_16 serverack = eSERVERACK_ERR_SYSMAX;

	//ack有效长度
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 1) 
	{
		switch(ptcmd->pbuf[0])
		{
			case eTERMINALACK_ERR_NONE:
				serverack = eSERVERACK_ERR_NONE;
				break;
				
			case eTERMINALACK_ERR_CHK:
				serverack = eSERVERACK_ERR_CHK;
				break;
				
			case eTERMINALACK_ERR_LEN:
				serverack = eSERVERACK_ERR_LEN;
				break;
				
			case eTERMINALACK_ERR_CMD:
				serverack = eSERVERACK_ERR_CMD;
				break;
				
			case eTERMINALACK_ERR_ADDREQ:
				serverack = eSERVERACK_ERR_ADDREQ;
				break;
				
			case eTERMINALACK_ERR_ADDSET:
				serverack = eSERVERACK_ERR_ADDSET;
				break;
				
			case eTERMINALACK_ERR_CHREQ:
				serverack = eSERVERACK_ERR_CHREQ;
				break;
				
			case eTERMINALACK_ERR_CHSET:
				serverack = eSERVERACK_ERR_CHSET;
				break;
				
			case eTERMINALACK_ERR_ROUTREQ:
				serverack = eSERVERACK_ERR_ROUTREQ;
				break;
				
			case eTERMINALACK_ERR_ROUTSET:
				serverack = eSERVERACK_ERR_ROUTSET;
				break;
				
			case eTERMINALACK_ERR_CSMAREQ:
				serverack = eSERVERACK_ERR_CSMAREQ;
				break;
				
			case eTERMINALACK_ERR_CSMASET:
				serverack = eSERVERACK_ERR_CSMASET;
				break;
				
			case eTERMINALACK_ERR_MACREQ:
				serverack = eSERVERACK_ERR_MACREQ;
				break;

			case eTERMINALACK_ERR_MACIPREQ:
				serverack = eSERVERACK_ERR_MACIPREQ;
				break;
				
			case eTERMINALACK_ERR_MACIPSET:
				serverack = eSERVERACK_ERR_MACIPSET;
				break;
				
			case eTERMINALACK_ERR_MACFLAGREQ:
				serverack = eSERVERACK_ERR_MACFLAGREQ;
				break;
				
			case eTERMINALACK_ERR_MACFLAGSET:
				serverack = eSERVERACK_ERR_MACFLAGSET;
				break;
				
			default:
				ret = eRET_PARAM;
				break;
		}
	}

	//server组帧应答
	if(serverack < eSERVERACK_ERR_SYSMAX)
	{
		//大小端转序
		serverack = REVERSE16(serverack);
		ret = ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			REVERSE16(*ptcmd->pnodeid), eSERVERCMD_ACK, (uint_8*)&serverack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ComTerminalSysAckProc
//功  能: 终端应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalAddrAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_ADDREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //默认出错应答
	uint_8 acklen = 2;
	
	//正确应答
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 6) 
	{
		acktype = eSERVERCMD_ACK_ADDR;
		packbuf = ptcmd->pbuf;
		acklen = 6;
	}
	
	//server组帧应答
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ComTerminalRFChAckProc
//功  能: 终端应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalRFChAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_CHREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //默认出错应答
	uint_8 acklen = 2;
	
	//正确应答
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 1) 
	{
		acktype = eSERVERCMD_ACK_RFCH;
		packbuf = ptcmd->pbuf;
		acklen = 1;
	}
	
	//server组帧应答
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ComTerminalRouteAckProc
//功  能: 终端应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalRouteAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_ROUTREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //默认出错应答
	uint_8 acklen = 2;
	
	//正确应答
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 1) 
	{
		acktype = eSERVERCMD_ACK_ROUTE;
		packbuf = ptcmd->pbuf;
		acklen = 1;
	}
	
	//server组帧应答
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ComTerminalTranAckProc
//功  能:最大转发跳数应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalTranAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_TRANUMREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //默认出错应答
	uint_8 acklen = 2;
	
	//正确应答
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 1) 
	{
		acktype = eSERVERCMD_ACK_TRANUM;
		packbuf = ptcmd->pbuf;
		acklen = 1;
	}
	
	//server组帧应答
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ComTerminalCamaAckProc
//功  能: 终端应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalCamaAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_CSMAREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //默认出错应答
	uint_8 acklen = 2;
	
	//正确应答
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 1) 
	{
		acktype = eSERVERCMD_ACK_CSMACA;
		packbuf = ptcmd->pbuf;
		acklen = 1;
	}
	
	//server组帧应答
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ComTerminalMACIPAckProc
//功  能: 终端应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalMACIPAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_CSMAREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //默认出错应答
	uint_8 acklen = 2;
		
	//正确应答
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 14) 
	{
		acktype = eSERVERCMD_ACK_MACIP;
		packbuf = ptcmd->pbuf;
		acklen = 14;
	}
	
	//server组帧应答
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ComTerminalMacFlagAckProc
//功  能: 终端应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalMacFlagAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_MACFLAGREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //默认出错应答
	uint_8 acklen = 2;
	
	//正确应答
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 9) 
	{
		acktype = eSERVERCMD_ACK_MACFLAG;
		packbuf = ptcmd->pbuf;
		acklen = 9;
	}
	
	//server组帧应答
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//函数名: ComTerminalAPPPollProc
//功  能: 终端轮询应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static void ComTerminalAPPPollProc(const TTerminalCOMFrame* const ptcmd, const TTerminalAppHead* const ptapp)
{
	uint_8 tmp[30];
	TNodeInfo* pnode = NodeAddrCheck(REVERSE16(*ptcmd->pnodeid));

	//地址检查
	if(pnode == NULL)
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComTerminalAPPPollProc]: nodeid not exite!\r\n");
		return;
	}

	uint_8 len = *((*ptapp).plen) - 2;
	if(len > APP_DATALEN)
	{
		len = APP_DATALEN;
	}
	//状态更新
	if(*ptapp->pcmd == eTERMINALAPPCMD_ACK_BMM_STATE1_REQ)
	{
		memcpy(tmp, ptapp->pbuf, len);
		memcpy(tmp + len, pnode->data + len, APP_DATALEN - len);
		memcpy(pnode->data, tmp, APP_DATALEN);
		//memcpy(pnode->data, ptapp->pbuf, 18);
	}
	else if(*ptapp->pcmd == eTERMINALAPPCMD_ACK_BMM_STATE2_REQ)
	{
		memcpy(tmp, ptapp->pbuf, len);
		memcpy(tmp + len, pnode->data + len, APP_DATALEN - len);
		memcpy(pnode->data, tmp, APP_DATALEN);
		//memcpy(pnode->data + 18, ptapp->pbuf, 12);
	}
	//长度错误
	else
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComTerminalAPPPollProc]: data len err!\r\n");
	}
	
	return;
}

//============================================================================
//函数名: ComTerminalSysCmdProc
//功  能: 系统命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalSysCmdProc(const TTerminalCOMFrame* const ptcmd)
{
    uint_8 ret;
	
	switch(*ptcmd->pcmd)
	{
		//应答
		case eTERMINALCOM_ACK:
			ret = ComTerminalSysAckProc(ptcmd);
			break;
			
		//地址应答
		case eTERMINALCOM_ACK_ADDR:
			ret = ComTerminalAddrAckProc(ptcmd);
			break;

		//通道应答
		case eTERMINALCOM_ACK_RFCH:
			ret = ComTerminalRFChAckProc(ptcmd);
			break;
		
		//路由应答
		case eTERMINALCOM_ACK_ROUTE:
			ret = ComTerminalRouteAckProc(ptcmd);
			break;
			
		//最大转发跳数应答
		case eTERMINALCOM_ACK_TRANUM:
			ret = ComTerminalTranAckProc(ptcmd);
			break;
		
		//CSMACA应答
		case eTERMINALCOM_ACK_CSMACA:
			ret = ComTerminalCamaAckProc(ptcmd);
			break;
			
		//MACIP地址应答
		case eTERMINALCOM_ACK_MACIP:
			ret = ComTerminalMACIPAckProc(ptcmd);
			break;
			
		//广播查询ZIG_MAC地址应答标志应答
		case eTERMINALCOM_ACK_MACFLAG:
			ret = ComTerminalMacFlagAckProc(ptcmd);
			break;
			
		default:
			break;
	}

	return ret;
}

//============================================================================
//函数名: ComTerminalAppAckProc
//功  能: 终端应答命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalAppAckProc(const TTerminalCOMFrame* const ptcmd, const TTerminalAppHead* const ptapp)
{
	uint_8 ret = eRET_NONE;
	uint_16 serverack = eSERVERACK_ERR_APPMAX;

	//有效长度
	if(*ptapp->plen == 3)
	{
		switch(ptapp->pbuf[0])
		{
			case eTERMINALAppACK_NONE:
				serverack = eSERVERACK_ERR_NONE;
				break;
				
			case eTERMINALAppACK_ERR_BULKREQ:
				serverack = eSERVERACK_ERR_BULKREQ;
				break;
				
			case eTERMINALAppACK_ERR_BULKSET:
				serverack = eSERVERACK_ERR_BULKSET;
				break;
				
			case eTERMINALAppACK_ERR_POWERREQ:
				serverack = eSERVERACK_ERR_POWERREQ;
				break;
				
			case eTERMINALAppACK_ERR_POWERSET:
				serverack = eSERVERACK_ERR_POWERSET;
				break;
				
			case eTERMINALAppACK_ERR_PARAMREQ:
				serverack = eSERVERACK_ERR_PARAMREQ;
				break;
				
			case eTERMINALAppACK_ERR_PARAMSET:
				serverack = eSERVERACK_ERR_PARAMSET;
				break;
				
			case eTERMINALAppACK_ERR_VERREQ:
				serverack = eSERVERACK_ERR_VERREQ;
				break;
				
			case eTERMINALAppACK_ERR_BULKSTATEREQ:
				serverack = eSERVERACK_ERR_BULKSTATEREQ;
				break;
				
			case eTERMINALAppACK_ERR_INDIFLAGREQ:
				serverack = eSERVERACK_ERR_INDIFLAGREQ;
				break;
				
			case eTERMINALAppACK_ERR_INDIFLAGSET:
				serverack = eSERVERACK_ERR_INDIFLAGSET;
				break;
				
			case eTERMINALAppACK_ERR_RESET:
				serverack = eSERVERACK_ERR_RESET;
				break;
				
			case eTERMINALAppACK_ERR_ADVALUEREQ:
				serverack = eSERVERACK_ERR_ADVALUEREQ;
				break;
				
			case eTERMINALAppACK_ERR_RESETIMEREQ:
				serverack = eSERVERACK_ERR_RESETIMEREQ;
				break;
				
			case eTERMINALAppACK_ERR_WDITIMEREQ:
				serverack = eSERVERACK_ERR_WDITIMEREQ;
				break;
				
			case eTERMINALAppACK_ERR_WDITIMESET:
				serverack = eSERVERACK_ERR_WDITIMESET;
				break;
				
			case eTERMINALAppACK_ERR_STARTUPMODEREQ:
				serverack = eSERVERACK_ERR_STARTUPMODEREQ;
				break;
				
			case eTERMINALAppACK_ERR_STARTUPMODESET:
				serverack = eSERVERACK_ERR_STARTUPMODESET;
				break;
				
			case eTERMINALAppACK_ERR_BULKSETREQ:
				serverack = eSERVERACK_ERR_BULKSETREQ;
				break;
				
			case eTERMINALAppACK_ERR_BULKONCURREQ:
				serverack = eSERVERACK_ERR_BULKONCURREQ;
				break;
				
			case eTERMINALAppACK_ERR_BULKONCURSET:
				serverack = eSERVERACK_ERR_BULKONCURSET;
				break;
				
			case eTERMINALAppACK_ERR_BMM_SYSPARAM_REQ:
				serverack = eSERVERACK_ERR_BMM_SYSPARAM_REQ;
				break;
				
			case eTERMINALAppACK_ERR_BMM_SYSPARAM_SET:
				serverack = eSERVERACK_ERR_BMM_SYSPARAM_SET;
				break;
				
			case eTERMINALAppACK_ERR_BMM_STATE1_REQ:
				serverack = eSERVERACK_ERR_BMM_STATE1_REQ;
				break;
				
			case eTERMINALAppACK_ERR_BMM_STATE2_REQ:
				serverack = eSERVERACK_ERR_BMM_STATE2_REQ;
				break;
				
			default:
				ret = eRET_PARAM;
				break;
		}
	}

	//server组帧应答
	if(serverack < eSERVERACK_ERR_APPMAX)
	{
		//大小端转序
		serverack = REVERSE16(serverack);
		ret = ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			     REVERSE16(*ptcmd->pnodeid), eSERVERCMD_ACK, (uint_8*)&serverack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//函数名: ComTerminalAppCmdProc
//功  能: 用户命令处理
//参  数: const TTerminalCOMFrame* const ptcmd，数据命令
//返  回: 无
//============================================================================
static uint_8 ComTerminalAppCmdProc(const TTerminalCOMFrame* const ptcmd)
{
    uint_8 ret = eRET_NONE;
	uint_16 acktype = eSERVERCMD_ACK_APPMAX;
	uint_8* packbuf;
	uint_8 ackbuflen;
	TTerminalAppHead terminalapp;

	TerminalAppStructAddr(&terminalapp, ptcmd->pbuf);

	switch(*terminalapp.pcmd)
	{
		//应用应答
		case eTERMINALAPPCMD_ACK:
			//主控器轮询应答帧，不用应答服务器
			if(s_bpollack && *ptcmd->pret == 0xFFFFFFFF)
				s_bpollack = FALSE;
			else
				ret = ComTerminalAppAckProc(ptcmd, &terminalapp);
			break;
			
		//灯开关状态应答
		case eTERMINALAPPCMD_ACK_BULK:
			acktype = eSERVERCMD_ACK_BULK;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			//ComTerminalFlashBulkState(ptcmd, &terminalapp);  //更新状态
			break;

		//灯功率应答
		case eTERMINALAPPCMD_ACK_POWER:
			acktype = eSERVERCMD_ACK_POWER;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			//ComTerminalFlashBulkState(ptcmd, &terminalapp);   //更新状态
			break;
			
		//三一嵌入式模块参数应答
		case eTERMINALAPPCMD_ACK_PARAM:
			acktype = eSERVERCMD_ACK_PARAM;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			break;
			
		//版本信息应答
		case eTERMINALAPPCMD_ACK_VERSION:
			acktype = eSERVERCMD_ACK_VERSION;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			break;
			
		//灯状态应答
		case eTERMINALAPPCMD_ACK_BULKSTATE:
			//主控器轮询应答帧，不用应答服务器
			if(s_bpollack && *ptcmd->pret == 0xFFFFFFFF)
			{
				s_bpollack = FALSE;
			}
			else
			{
				acktype = eSERVERCMD_ACK_BULKSTATE;
				packbuf = terminalapp.pbuf;
				ackbuflen = *terminalapp.plen - 2;
			}
			//ComTerminalIndicationProc(ptcmd, &terminalapp);   //更新状态
			break;
			
		//主动上传标志应答
		case eTERMINALAPPCMD_ACK_INDIFLAG:
			acktype = eSERVERCMD_ACK_INDIFLAG;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			break;
			
		//灯状态主动上传包
		case eTERMINALAPPCMD_INDICATION:
			//ComTerminalIndicationProc(ptcmd, &terminalapp);
			break;

		//AD值查询
		case eTERMINALAPPCMD_ACK_ADVALUE:
			acktype = eSERVERCMD_ACK_ADVALUE;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			break;
			
		//Zigbee模块看门狗复位时间应当
		case eTERMINALAPPCMD_ACK_WIDTIME: 
		    acktype = eSERVERCMD_ACK_WIDTIME;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;   
			break;  
			
		//灯具启动方式应答帧
		case eTERMINALAPPCMD_ACK_STARTUPMODE: 
			acktype = eSERVERCMD_ACK_STARTUPMODE;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
			
		//灯具开关设置应答
		case eTERMINALAPPCMD_ACK_BULKSET: 
			acktype = eSERVERCMD_ACK_BULKSETACK;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;
			
		//开关灯电流阈值应答
		case eTERMINALAPPCMD_ACK_BULKONCUR: 
			acktype = eSERVERCMD_ACK_BULKONCURACK;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xAE. 电压采样应答
		case eTERMINALAPPCMD_ACK_VOLTAGE: 
			acktype = eSERVERCMD_ACK_VOLTAGE;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xAF. 电流采样应答
		case eTERMINALAPPCMD_ACK_CURRENT: 
			acktype = eSERVERCMD_ACK_CURRENT;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xB0. 温度采样应答
		case eTERMINALAPPCMD_ACK_TEMPERATURE: 
			acktype = eSERVERCMD_ACK_TEMPERATURE;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xB1. V2轮询命令应答
		case eTERMINALAPPCMD_ACK_V2POLL:
			//主控器轮询应答帧，不用应答服务器
			if(s_bpollack && *ptcmd->pret == 0xFFFFFFFF)
			{
				s_bpollack = FALSE;
			}
			else
			{
				acktype = eSERVERCMD_ACK_VOLCUR;
				packbuf = terminalapp.pbuf;
				ackbuflen = *terminalapp.plen - 2;
			}
			//ComTerminalV2PollProc(ptcmd, &terminalapp);   //更新状态
			break;
			
		//0XB2.硬件地址应答
		case eTERMINALAPPCMD_ACK_HDADDR: 
		    acktype = eSERVERCMD_ACK_HDADDR;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;   
			break;

		//0xD8. BMM参数应答帧
		case eTERMINALAPPCMD_ACK_BMM_SYSPARAM_REQ: 
			acktype = eSERVERCMD_ACK_BMM_SYSPARAM_REQ;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xD9. BMM状态1应答帧
		case eTERMINALAPPCMD_ACK_BMM_STATE1_REQ:
			//主控器轮询应答帧，不用应答服务器
			if(s_bpollack && *ptcmd->pret == 0xFFFFFFFF)
			{
				s_bpollack = FALSE;
			}
			else
			{
				acktype = eSERVERCMD_ACK_BMM_STATE1_REQ;
				packbuf = terminalapp.pbuf;
				ackbuflen = *terminalapp.plen - 2;
			}
			ComTerminalAPPPollProc(ptcmd, &terminalapp);   //更新状态
			break;
		//0xDA. BMM状态2应答帧
		case eTERMINALAPPCMD_ACK_BMM_STATE2_REQ:
			//主控器轮询应答帧，不用应答服务器
			if(s_bpollack && *ptcmd->pret == 0xFFFFFFFF)
			{
				s_bpollack = FALSE;
			}
			else
			{
				acktype = eSERVERCMD_ACK_BMM_STATE2_REQ;
				packbuf = terminalapp.pbuf;
				ackbuflen = *terminalapp.plen - 2;
			}
			ComTerminalAPPPollProc(ptcmd, &terminalapp);   //更新状态
			break;
			
		default:
			ret = eRET_PARAM;
			break;
	}

	//server组帧应答
	if(acktype < eSERVERCMD_ACK_APPMAX)
	{
		ret = ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			               REVERSE16(*ptcmd->pnodeid), acktype, packbuf, ackbuflen, *ptcmd->pret);
	}

	return ret;
}



//============================================================================
//函数名: ComInit
//功  能: 通信层命令处理模块初始化
//参  数: const TComInfo* const ptcominfo，通信层信息
//返  回: 无
//============================================================================
void ComTerminalInit(TComInfo* ptcominfo)
{
	s_ptcomterminalinfo = ptcominfo;
}

//============================================================================
//函数名: ComTerminalSendCmd
//功  能: 终端控制器发送命令
//参  数: uint_8 cmd, 命令字
//               uint_8* cmdbuf，命令字内容
//               uint_8 cmdlen，cmdbuf有效长度
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalSendCmd(uint_16 netid, uint_8 route, uint_8 groupid,  
	uint_16 nodeid, uint_8 cmdtype, uint_8* pcmd, uint_8 cmdlen, uint_32 serverret)
{
	uint_8 ret = eRET_PARAM;
	TBufNode* pnode = MakeTerminalDataNode(netid, route, groupid, nodeid, cmdtype, pcmd, cmdlen, serverret);
	if(pnode != NULL)
	{
		//发送
		listnode_put(s_ptcomterminalinfo->pterminalsenddatalist, pnode);
		ret = eRET_NONE;
	}
	return ret;	
}

//============================================================================
//函数名: ComTerminalGetLanInfo
//功  能: 获取局域网地址
//参  数: uint_16* pnetid, uint_32* pnodeid，返回局域网地址
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
static TTerminalCOMFrame* ComTerminalGetLanInfoPre(uint_8 reqtype, uint_8 acktype, uint_32 timeout)
{
	bool bsucc = FALSE;
	MQX_TICK_STRUCT ticks;
	TTerminalCOMFrame terminalframe;
	//查询帧
	TBufNode* pnode = MakeTerminalDataNode(0xFFFF, 0, 0, 0xFFFF, reqtype, NULL, 0, 0);
	
	//参数校验
	if(NULL == pnode)
		return NULL;

	//发送回调
	pnode->len = TERMINALCOMFRAME_MINLEN;
	if(s_ptcomterminalinfo->pcomterminalsndcallbak != NULL)
		s_ptcomterminalinfo->pcomterminalsndcallbak(pnode);

	//归还freelist
	pnode->len = 0;
	listnode_put(s_ptcomterminalinfo->pterminalsendfreelist, pnode);

	//超时等待应答
	pnode = NULL;

	while(is_timeout(ticks.TICKS[0],timeout) == FALSE)
	{
		listnode_get(s_ptcomterminalinfo->pterminalrecvdatalist, &pnode);
		if(NULL != pnode)
		{
			//地址分配
			TerminalStructAddr(&terminalframe, pnode->pbuf, pnode->len - TERMINALCOMFRAME_MINLEN);
			//校验和
			if(MakeCheckInfo(pnode->pbuf, pnode->len - 1) != *terminalframe.pchk)
			{				
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComTerminalGetLanInfoPre]: return chksum error!\r\n");
			}
			//命令错误返回
			else if(*terminalframe.pcmd != acktype)
			{				
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComTerminalGetLanInfoPre]: return cmd error!\r\n");
			}
			else
				bsucc = TRUE;

			listnode_put(s_ptcomterminalinfo->pterminalrecvfreelist,pnode);
			break;
		}
	}

	if(bsucc)
		return (TTerminalCOMFrame*)&terminalframe;
	else
		return NULL;
}


//============================================================================
//函数名: ComTerminalGetLanAddr
//功  能: 获取局域网地址
//参  数: uint_16* pnetid, uint_32* pnodeid，返回局域网地址
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetLanAddr(uint_16* pnetid, uint_8* proute, uint_8* pgroupid, uint_16* pnodeid, uint_32 timeout)
{
	uint_8 ret = eRET_LANADDR;
	TTerminalCOMFrame* pterminalframe = 
		ComTerminalGetLanInfoPre(eTERMINALCOM_REQ_ADDR, eTERMINALCOM_ACK_ADDR, timeout);

  	if(NULL != pterminalframe)
	{
		*pnetid = (pterminalframe->pbuf[0] << 8) + pterminalframe->pbuf[1];
		*proute = pterminalframe->pbuf[2];
		*pgroupid = pterminalframe->pbuf[3];
		*pnodeid = (pterminalframe->pbuf[4] << 8) + pterminalframe->pbuf[5];
		ret = eRET_NONE;
	}
	
	return ret;
}

//============================================================================
//函数名: ComTerminalGetbRouter
//功  能: 获取主控器局域网路由功能
//参  数: bool* brouter，返回路由功能
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetbRouter(bool* brouter, uint_32 timeout)
{
	uint_8 ret = eRET_LANROUTER;
	TTerminalCOMFrame* pterminalframe = 
		ComTerminalGetLanInfoPre(eTERMINALCOM_REQ_ROUTE, eTERMINALCOM_ACK_ROUTE, timeout);

	if(NULL != pterminalframe)
	{
		*brouter = pterminalframe->pbuf[0];
		ret = eRET_NONE;
	}
	
	return ret;
}

//============================================================================
//函数名: ComTerminalGetTranNum
//功  能: 获取主控器局域网转发跳数
//参  数: uint_8* trannum，返回转发跳数
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetTranNum(uint_8* trannum, uint_32 timeout)
{
	uint_8 ret = eRET_LANTRANNUM;
	TTerminalCOMFrame* pterminalframe = 
		ComTerminalGetLanInfoPre(eTERMINALCOM_REQ_TRANUM, eTERMINALCOM_ACK_TRANUM, timeout);

	if(NULL != pterminalframe)
	{
		*trannum = pterminalframe->pbuf[0];
		ret = eRET_NONE;
	}
	
	return ret;
}

//============================================================================
//函数名: ComTerminalGetTranNum
//功  能: 获取主控器局域网csma功能
//参  数: uint_8* trannum，返回转发跳数
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetbCsmaca(bool* bcsmaca, uint_32 timeout)
{
	uint_8 ret = eRET_LANBCSMA;
	TTerminalCOMFrame* pterminalframe = 
		ComTerminalGetLanInfoPre(eTERMINALCOM_REQ_CSMACA, eTERMINALCOM_ACK_CSMACA, timeout);

	if(NULL != pterminalframe)
	{
		*bcsmaca = pterminalframe->pbuf[0];
		ret = eRET_NONE;
	}
	
	return ret;
}

//============================================================================
//函数名: ComTerminalGetRfCh
//功  能: 获取主控器局域网转发跳数
//参  数: uint_8* trannum，返回转发跳数
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetRfCh(uint_8* rfch, uint_32 timeout)
{
	uint_8 ret = eRET_LANRFCH;
	TTerminalCOMFrame* pterminalframe = 
		ComTerminalGetLanInfoPre(eTERMINALCOM_REQ_RFCH, eTERMINALCOM_ACK_RFCH, timeout);

	if(NULL != pterminalframe)
	{
		*rfch = pterminalframe->pbuf[0];
		ret = eRET_NONE;
	}
	
	return ret;
}


//============================================================================
//函数名: ComTerminalProc
//功  能: 通信命令处理
//参  数: uint_8 comtype，来自server还是terminal的通信帧
//               const uint_8* const pframe，通信帧首地址
//               uint_16 framelen，pframe有效长度
//返  回: 错误码
//============================================================================
uint_8 ComTerminalProc(const uint_8* const pframe, uint_16 framelen)
{
	uint_8 i;
	TTerminalCOMFrame terminalframe;
	uint_8 ret = eRET_NONE;

	//DEBUGOUT_TIME(DEBUGOUT_LEVEL_INFO);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[ComTerminalProc]: pframe is: ");
	for(i = 0; i < framelen; i++)
	{
		DEBUGOUT_HEX(DEBUGOUT_LEVEL_INFO, pframe[i]);
		DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, ", ");
	}
	DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "\r\n");
		
	//地址分配
	TerminalStructAddr(&terminalframe, (uint_8*)pframe, framelen - TERMINALCOMFRAME_MINLEN);

	//长度错误返回
	if(*terminalframe.plen < TERMINALCOMFRAME_MINLEN || framelen < TERMINALCOMFRAME_MINLEN)
	{
		ret = eRET_LEN;
		goto ComTerminalProc_ERRRET;
	}
		
	//命令错误返回
	if((*terminalframe.pcmd >= eTERMINALCOM_CMDMAX && *terminalframe.pcmd < eTERMINALCOM_ACK)
		|| *terminalframe.pcmd >= eTERMINALCOM_ACKMAX)
	{
		ret = eRET_CMD;
		goto ComTerminalProc_ERRRET;
	}
	
	//校验和
	if(MakeCheckInfo(terminalframe.plen, framelen - 1) != *terminalframe.pchk)
	{
		ret = eRET_CHK;
		goto ComTerminalProc_ERRRET;
	}
	
	//用户命令
	if(eTERMINALCOM_DATA == *terminalframe.pcmd)
	{
		ret = ComTerminalAppCmdProc(&terminalframe);
	}
	//系统命令
	else
	{
		ret = ComTerminalSysCmdProc(&terminalframe);
	}
	return ret;
				
ComTerminalProc_ERRRET:
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComTerminalProc]: ret=");
	DEBUGOUT_DEC(DEBUGOUT_LEVEL_ERR, ret);
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
	return ret;
}




