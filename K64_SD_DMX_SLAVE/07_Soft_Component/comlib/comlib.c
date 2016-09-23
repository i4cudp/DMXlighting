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
#include "comlib.h"
#include "comlib_server.h"
#include "comlib_terminal.h"
//#include "includes.h"

static TComInfo s_tcominfo;


//============================================================================
//函数名: ComInit
//功  能: 通信层命令处理模块初始化
//参  数: const TComInfo* const ptcominfo，通信层信息
//返  回: 无
//============================================================================
void ComInit(const TComInfo* const ptcominfo)
{
	s_tcominfo.pserversenddatalist = ptcominfo->pserversenddatalist;
	s_tcominfo.pserversendfreelist = ptcominfo->pserversendfreelist;
	s_tcominfo.pterminalsenddatalist = ptcominfo->pterminalsenddatalist;
	s_tcominfo.pterminalsendfreelist = ptcominfo->pterminalsendfreelist;
	s_tcominfo.pterminalrecvdatalist = ptcominfo->pterminalrecvdatalist;
	s_tcominfo.pterminalrecvfreelist = ptcominfo->pterminalrecvfreelist;
	s_tcominfo.pterminalnodedatalist = ptcominfo->pterminalnodedatalist;
	s_tcominfo.pterminalnodefreelist = ptcominfo->pterminalnodefreelist;
	memcpy(&s_tcominfo.tcomenetinfo, &ptcominfo->tcomenetinfo, sizeof(TComENETInfo));
	memcpy(&s_tcominfo.tcomgprsinfo, &ptcominfo->tcomgprsinfo, sizeof(TComGprsInfo));
	memcpy(&s_tcominfo.tcomlaninfo, &ptcominfo->tcomlaninfo, sizeof(TComLanInfo));
	memcpy(&s_tcominfo.tcomgsminfo, &ptcominfo->tcomgsminfo, sizeof(TComGSMInfo));
	s_tcominfo.pcominfochcallback = ptcominfo->pcominfochcallback;
	s_tcominfo.pcomterminalsndcallbak = ptcominfo->pcomterminalsndcallbak;
	s_tcominfo.pcomterminaldevchcallbak = ptcominfo->pcomterminaldevchcallbak;
	ComServerInit(&s_tcominfo);
	ComTerminalInit(&s_tcominfo);
}

//============================================================================
//函数名: ComGetLanInfo
//功  能: 获取局域网信息
//参  数: TComLanInfo* ptcomlaninfo，局域网信息
//返  回: 无
//============================================================================
uint_8 ComGetLanInfo(TComLanInfo* ptcomlaninfo, uint_32 timeout)
{
	uint_8 ret;
	
//	ret = ComTerminalGetLanAddr(&ptcomlaninfo->netid, &ptcomlaninfo->route, &ptcomlaninfo->groupid, &ptcomlaninfo->nodeid, timeout);
	if(ret != eRET_NONE)
		return ret;
	
	ret = ComTerminalGetbRouter(&ptcomlaninfo->brouter, timeout);
	if(ret != eRET_NONE)
		return ret;
	
	ret = ComTerminalGetTranNum(&ptcomlaninfo->transnum, timeout);
	if(ret != eRET_NONE)
		return ret;

	ret = ComTerminalGetbCsmaca(&ptcomlaninfo->bcsmaca, timeout);
	if(ret != eRET_NONE)
		return ret;
	
	return ComTerminalGetRfCh(&ptcomlaninfo->rfch, timeout);
}

//============================================================================
//函数名: ComProc
//功  能: 通信命令处理
//参  数: uint_8 comtype，来自server还是terminal的通信帧
//               const uint_8* const pframe，通信帧首地址
//               uint_16 framelen，pframe有效长度
//返  回: 错误码
//============================================================================
uint_8 ComProc(uint_8 comtype, const uint_8* const pframe, uint_16 framelen)
{
	//参数判断
	if(comtype >= eCOMTYPE_MAX || NULL == pframe)
		return eRET_PARAM;
	
	//server通信帧处理
	if(eCOMTYPE_SERVER == comtype)
		return ComServerProc(pframe, framelen);

	//terminal通信帧处理
	return ComTerminalProc(pframe, framelen);
}

//============================================================================
//函数名: ComHeartProc
//功  能: 通信层心跳处理
//参  数: 无
//返  回: 无
//============================================================================
void ComHeartProc(void)
{
	ComServerHeartProc();
}

//============================================================================
//函数名: ServerFrameAddEsc
//功  能: 服务器通信帧增加转义符
//参  数: const uint_8* const psrc，通信帧
//               uint_16 srclen，psrc有效长度
//               uint_8* pdst，增加转义后存放地址
//               uint_16 dstsize，pdst缓冲大小
//返  回: pdst有效长度
//============================================================================
uint_16 ServerFrameAddEsc(const uint_8* const psrc, uint_16 srclen, uint_8* pdst, uint_16 dstsize)
{
	uint_16 i, j;
	
	//参数或帧头帧尾不正确
	if(NULL == psrc || NULL == pdst || dstsize < srclen || 
		psrc[0] != SERVERCOM_START_FLAG || psrc[srclen - 1] != SERVERCOM_END_FLAG)
		return 0;

	//帧头
	pdst[0] = psrc[0];

	//帧数据转义
	for(i = 1, j = 1; i < srclen - 1; i++)
	{
		//加转义
		if(psrc[i] == SERVERCOM_START_FLAG || psrc[i] == SERVERCOM_END_FLAG || psrc[i] == SERVERCOM_ESC_FLAG)
			pdst[j++] = SERVERCOM_ESC_FLAG;

		//原数据
		pdst[j++] = psrc[i];

		//pdst满
		if(j >= dstsize)
		{
			j = 0;
			break;
		}
	}

	//帧尾
	if(j > 0)
		pdst[j++] = psrc[srclen - 1];

	return j;
}

//============================================================================
//函数名: ServerFrameDelEsc
//功  能: 服务器通信帧删除转义符
//参  数: const uint_8* const psrc，通信帧
//               uint_16 srclen，psrc有效长度
//               uint_16* psrcretlen，psrc返回当前包长度，解决拼包问题
//               uint_8* pdst，增加转义后存放地址
//               uint_16 dstsize，pdst缓冲大小
//返  回: pdst有效长度
//============================================================================
uint_16 ServerFrameDelEsc(const uint_8* const psrc, uint_16 srclen, uint_16* psrcretlen, uint_8* pdst, uint_16 dstsize)
{
	uint_16 i, j;
	
	//参数或帧头不正确
	if(NULL == psrc || NULL == pdst || psrc[0] != SERVERCOM_START_FLAG)
		return 0;

	//帧头
	pdst[0] = psrc[0];

	//帧数据内容去转义
	for(i = 1, j = 1; i < srclen; i++)
	{
		//转义字符
		if(psrc[i] == SERVERCOM_ESC_FLAG)
		{
			if(psrc[i + 1] == SERVERCOM_START_FLAG || psrc[i + 1] == SERVERCOM_END_FLAG || psrc[i + 1] == SERVERCOM_ESC_FLAG)
				pdst[j++] = psrc[++i];
			//出错
			else
			{
				j = 0;
				break;
			}
		}
		else
		{
			pdst[j++] = psrc[i];
			//结束符
			if(psrc[i] == SERVERCOM_END_FLAG)
			{
				break;
			}
		}
		
		//pdst满
		if(j >= dstsize)
		{
			j = 0;
			break;
		}
	}
	
	*psrcretlen = i + 1;

	return j;
	
}

//============================================================================
//函数名: ZigbeeWDIFeed
//功  能: zigbee模块看门狗喂狗
//参  数: 无
//返  回: 无
//============================================================================
void ZigbeeWDIFeed(void)
{
	uint_8 buf[2];       

	//zigbee模块喂狗命令帧
	buf[0] = 2;
	buf[1] = eTERMINALAPPCMD_WDIFEED;
	
	//终端组帧发送
	ComTerminalSendCmd(REVERSE16(s_tcominfo.tcomlaninfo.netid), s_tcominfo.tcomlaninfo.route,
	                   s_tcominfo.tcomlaninfo.groupid, REVERSE16(s_tcominfo.tcomlaninfo.nodeid), 
	                   eTERMINALCOM_DATA, buf, 2, 0);
}

//============================================================================
//函数名: BroadSetBulk
//功  能: 广播开关灯设置
//参  数: 无
//返  回: 无
//============================================================================
void BroadSetBulk(uint_8 groupid, bool bFlag)
{
	ComServerBroadSetBulk(groupid, bFlag);
}


