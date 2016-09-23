/*====================================================================
ģ����  �� COMLIB
�ļ���  �� comlib.c
����ļ��� comlib.h
�ļ�ʵ�ֹ��ܣ�ʵ��ͨ��֡����
����    ������
�汾    ��1.0.0.0.0
----------------------------------------------------------------------
�޸ļ�¼    ��
��  ��         �汾        �޸���        �޸�����
2012/07/03       1.0              ����                 ����
====================================================================*/
/* ͷ�ļ� */

#include "comlib_server.h"

//static TComInfo* s_ptcomserverinfo;
TComInfo* s_ptcomserverinfo;
static bool s_bmhcnodeidchange = FALSE;       //��������������ַ�仯
bool s_bpollack = FALSE;
bool s_bLinkStart = TRUE;                //�ս�������
bool b_sd = false;                        //�ļ�δ����
bool g_bSDWaitEvent = FALSE;
bool g_bDMXWaitEvent = FALSE;
bool g_bNodeRet = TRUE;
bool b_enet = true;                        //�ļ�δ����
static bool s_bPollStart = FALSE;        //��ʼ��ѯ������㲥�������״̬
static bool s_bAckFlash = FALSE;
static uint_32 s_AckRet;
//static uint_8 s_AckCh;
//static bool s_bAckOnOff = 0xFF;
//static uint_8 s_AckPower = 0xFF;
static uint_8 s_AckData[APP_DATALEN];


//20121130�㲥���ò������ж�β�������ֹ��©�����ĵ���
static uint_8 s_broadsetcount = 0;           //�㲥���ò�������
static uint_8 s_broadsetbuf[50];             //�㲥���ò�������
static uint_8 s_broadsetlen;                 //�㲥���ò������ݳ���

uint_8 g_gsmsendbuf[200];
uint_8 g_gsmsendlen = 0;

extern uint_8 CompileDate[];
extern uint_8 CompileTime[];

extern struct FileInfo fileinfo1;
extern uint_8 detect_receive[8];
//ϵͳ��λ
//extern bool bresetset;
//extern MQX_TICK_STRUCT resetticks;
//extern uint_32 resettimeout;

//extern TGSM_PARAM g_tgsmParam;

//============================================================================
//������: ServerStructAddr
//��  ��: �ṹ��ַ����
//��  ��: ��
//��  ��: ��
//============================================================================
static inline void ServerStructAddr(TServerCOMFrame* ptframe, uint_8* pbuf, uint_16 cmdlen)
{
	//��ַ����
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
//������: NodeAddrCheck
//��  ��: �ն˽ڵ��ַ����
//��  ��: ��
//��  ��: ��
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
//������: AddrReqProc
//��  ��: eSERVERCMD_ADDRREQ�����  ��ַ��ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerAddrReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 buf[6];
    uint_16* pnetid = (uint_16*)buf;
	uint_8* proute = (uint_8*)(pnetid + 1);
	uint_8* pgroupid = proute + 1;
	uint_16* pnodeid = (uint_16*)(pgroupid + 1);
	uint_8 ret = eRET_NONE;

	//��������ַ��ѯ
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
	//�ն˿�������ַ��ѯ
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//�ն���֡����
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_ADDR, NULL, 0, *ptcmd->pret);
	}
	//�㲥��ַ����
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
//������: ServerAddrSetProc
//��  ��: eSERVERCMD_ADDRSET�����  ��ַ����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerAddrSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;
	
	//���������ն˿�������ַ����
	if(eLANFRAMETYPE_ME == frametype || eLANFRAMETYPE_NODE_NET == frametype)
	{
		//��������Ϣ����
		if(eLANFRAMETYPE_ME == frametype)
		{
			s_ptcomserverinfo->tcomlaninfo.netid = (ptcmd->pcmdbuf[0] << 8) + ptcmd->pcmdbuf[1];
			s_ptcomserverinfo->tcomlaninfo.route = ptcmd->pcmdbuf[2];
			s_ptcomserverinfo->tcomlaninfo.groupid = ptcmd->pcmdbuf[3];
			s_ptcomserverinfo->tcomlaninfo.nodeid = (ptcmd->pcmdbuf[4] << 8) + ptcmd->pcmdbuf[5];
			s_bmhcnodeidchange = TRUE;  //�ȵ�Ӧ������
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
		
		//�ն���֡����
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_SET_ADDR, ptcmd->pcmdbuf, 6, *ptcmd->pret);
	}
	//�㲥��ַ����������
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
//������: ServerRFChReqProc
//��  ��: eSERVERCMD_CHREQ�����  ͨ����ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerRFChReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;

	//������ͨ����ѯ
	if(eLANFRAMETYPE_ME == frametype)
	{
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
			               eSERVERCMD_ACK_RFCH, &s_ptcomserverinfo->tcomlaninfo.rfch, 1, *ptcmd->pret);
	}
	//�ն˿�����ͨ����ѯ
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//�ն���֡����
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_RFCH, NULL, 0, *ptcmd->pret);
	}
	//�㲥��ַ����
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
//������: ServerRFChSetProc
//��  ��: eSERVERCMD_RFCHRSET�����  ͨ������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerRFChSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_8 ret = eRET_NONE;

	//��������Ϣ����
	if(eLANFRAMETYPE_ME ==  frametype && ptcmd->pcmdbuf[0] != s_ptcomserverinfo->tcomlaninfo.rfch)
	{
		s_ptcomserverinfo->tcomlaninfo.rfch = ptcmd->pcmdbuf[0];
		s_bmhcnodeidchange = TRUE;  //�ȵ�Ӧ������
	}
	
	//�ն���֡����
	ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
	                         eTERMINALCOM_SET_RFCH, ptcmd->pcmdbuf, 1, *ptcmd->pret);

	//�㲥��ַ����ֱ����������Ӧ��
	if(frametype < eLANFRAMETYPE_NODE_NET && frametype != eLANFRAMETYPE_ME)
	{
		//������������Ϣ
		s_ptcomserverinfo->tcomlaninfo.rfch = ptcmd->pcmdbuf[0];
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);

		//����Ӧ��
		if(ret != eRET_NONE)
			ack = eSERVERACK_ERR_CHSET;
		
		//��С��ת��
		ack = REVERSE16(ack);
		
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//������: ServerRouteReqProc
//��  ��: eSERVERCMD_ROUTREQ�����   ·��������ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerRouteReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;

	//������ͨ����ѯ
	if(eLANFRAMETYPE_ME == frametype)
	{
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
			               eSERVERCMD_ACK_ROUTE, &s_ptcomserverinfo->tcomlaninfo.brouter, 1, *ptcmd->pret);
	}
	//�ն˿�������ѯ
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//�ն���֡����
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_ROUTE, NULL, 0, *ptcmd->pret);
	}
	//�㲥��ַ����
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
//������: ServerRouteSetProc
//��  ��: eSERVERCMD_RFCHRSET�����  ·����������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerRouteSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_8 ret = eRET_NONE;

	//��������Ϣ����
	if(eLANFRAMETYPE_ME == frametype &&	ptcmd->pcmdbuf[0] != s_ptcomserverinfo->tcomlaninfo.brouter)
	{
		s_ptcomserverinfo->tcomlaninfo.brouter = ptcmd->pcmdbuf[0];
		s_bmhcnodeidchange = TRUE;  //�ȵ�Ӧ������
	}

	//�ն���֡����
	ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
	                         eTERMINALCOM_SET_ROUTE, ptcmd->pcmdbuf, 1, *ptcmd->pret);

	//�㲥��ַ����ֱ����������Ӧ��
	if(frametype < eLANFRAMETYPE_NODE_NET && frametype != eLANFRAMETYPE_ME)
	{
		//������������Ϣ
		s_ptcomserverinfo->tcomlaninfo.brouter = ptcmd->pcmdbuf[0];
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);

		//����Ӧ��
		if(ret != eRET_NONE)
			ack = eSERVERACK_ERR_ROUTSET;

		//��С��ת��
		ack = REVERSE16(ack);
		
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//������: ServerTranReqProc
//��  ��: eSERVERCMD_TRANUMREQ�����  ת��������ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerTranReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;

	//������ͨ����ѯ
	if(eLANFRAMETYPE_ME == frametype)
	{
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
			               eSERVERCMD_ACK_TRANUM, &s_ptcomserverinfo->tcomlaninfo.transnum, 1, *ptcmd->pret);
	}
	//�ն˿�������ѯ
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//�ն���֡����
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_TRANUM, NULL, 0, *ptcmd->pret);
	}
	//�㲥��ַ����
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
//������: ServerTranSetProc
//��  ��: eSERVERCMD_TRANUMRSET�����   ת����������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerTranSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_8 ret = eRET_NONE;

	//��������Ϣ����
	if(eLANFRAMETYPE_ME == frametype &&	ptcmd->pcmdbuf[0] != s_ptcomserverinfo->tcomlaninfo.transnum)
	{
		s_ptcomserverinfo->tcomlaninfo.transnum = ptcmd->pcmdbuf[0];
		s_bmhcnodeidchange = TRUE;  //�ȵ�Ӧ������
	}

	//�ն���֡����
	ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
	                         eTERMINALCOM_SET_TRANUM, ptcmd->pcmdbuf, 1, *ptcmd->pret);

	//�㲥��ַ����ֱ����������Ӧ��
	if(frametype < eLANFRAMETYPE_NODE_NET && frametype != eLANFRAMETYPE_ME)
	{
		//������������Ϣ
		s_ptcomserverinfo->tcomlaninfo.transnum = ptcmd->pcmdbuf[0];
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);

		//����Ӧ��
		if(ret != eRET_NONE)
			ack = eSERVERACK_ERR_TRANUMSET;

		//��С��ת��
		ack = REVERSE16(ack);
		
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}


//============================================================================
//������: ServerCSMACAReqProc
//��  ��: eSERVERCMD_CSMAREQ�����  CSMACA������ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerCSMACAReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 ret = eRET_NONE;
	uint_16 ack;

	//������ͨ����ѯ
	if(eLANFRAMETYPE_ME == frametype)
	{
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK_CSMACA, &s_ptcomserverinfo->tcomlaninfo.bcsmaca, 1, *ptcmd->pret);
	}
	//�ն˿�������ѯ
	else if(eLANFRAMETYPE_NODE_NET == frametype)
	{
		//�ն���֡����
		ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                         eTERMINALCOM_REQ_CSMACA, NULL, 0, *ptcmd->pret);
	}
	//�㲥��ַ����
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
//������: ServerCSMACASetProc
//��  ��: eSERVERCMD_CSMASET�����  CSMACA��������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerCSMACASetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_NONE;
	uint_8 ret = eRET_NONE;

	//��������Ϣ����
	if(eLANFRAMETYPE_ME == frametype &&	ptcmd->pcmdbuf[0] != s_ptcomserverinfo->tcomlaninfo.bcsmaca)
	{
		s_ptcomserverinfo->tcomlaninfo.bcsmaca = ptcmd->pcmdbuf[0];
		s_bmhcnodeidchange = TRUE;  //�ȵ�Ӧ������
	}

	//�ն���֡����
	ret = ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
	                         eTERMINALCOM_SET_CSMACA, ptcmd->pcmdbuf, 1, *ptcmd->pret);

	//�㲥��ַ����ֱ����������Ӧ��
	if(frametype < eLANFRAMETYPE_NODE_NET && frametype != eLANFRAMETYPE_ME)
	{
		//������������Ϣ
		s_ptcomserverinfo->tcomlaninfo.bcsmaca = ptcmd->pcmdbuf[0];
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);

		//����Ӧ��
		if(ret != eRET_NONE)
			ack = eSERVERACK_ERR_CSMASET;

		//��С��ת��
		ack = REVERSE16(ack);
		
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
			               eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//������: ServerReqAllDevProc
//��  ��: eSERVERCMD_REQ_ALLADDR�����   ��ѯ�����豸
//��  ��: ��
//��  ��: ��
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
				//��С��ת��
				*(uint_32*)(&buf[buflen]) = REVERSE16(pnode->tnodeinfosave.nodeid); //��2�ֽ�Ϊ0
				buflen += 4;
				pnodenum = (uint_16*)&buf[buflen];
				buflen += 2;
				*pnodenum = 1;
			}
			else
				*pnodenum += 1;
			
			nodeid = pnode->tnodeinfosave.nodeid;
			plist = plist->next;
			//�豸����
			if(plist == s_ptcomserverinfo->pterminalnodedatalist)
			{
				//��С��ת��
				*pnodenum = REVERSE16(*pnodenum);
				
				ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			               s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,
					       eSERVERCMD_ACK_ALLADDR, buf, buflen, *ptcmd->pret);
				plist = NULL;
				break;
			}
			
			//buf��
			if(s_ptcomserverinfo->tcomgprsinfo.heartmax - buflen <= (s_ptcomserverinfo->tcomgprsinfo.heartmax >> 3))
			{
				//��С��ת��
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
//������: ServerAddDevProc
//��  ��: eSERVERCMD_ADDDEV�����   �����豸
//��  ��: ��
//��  ��: ��
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
	
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype && (buflen % 6) == 0)
	{
		groupnum = buflen / 6;

		for(i = 0; i < groupnum; i++)
		{
			nodeid = *((uint_16*)(ptcmd->pcmdbuf + i * 6 + 2));  //��2�ֽڲ�����
			devnum = *((uint_16*)(ptcmd->pcmdbuf + i * 6 + 4));
			//��С��ת��
			nodeid = REVERSE16(nodeid);
			devnum = REVERSE16(devnum);

			//���Ҳ����Ӧ�ն˿�����
			for(j = 0; j < devnum; j++, nodeid++)
			{
				if(NodeAddrCheck(nodeid) == NULL)
				{
					//��ȡ������ڵ�
					if(list_empty(s_ptcomserverinfo->pterminalnodefreelist))		
					{
						ack = eSERVERACK_ERR_ADDDEV;
						break;
					}

					plist = s_ptcomserverinfo->pterminalnodefreelist->next;
					list_del(plist);	
					pnode = list_entry(plist, TNodeInfo, list);
					pnode->tnodeinfosave.route = eZIGFRAMEROUTE_V1;  //Ĭ�ϼ���1.X��·�ɷ�ʽ
					pnode->tnodeinfosave.groupid = 0;                //Ĭ�������
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

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			         s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				     eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}


//============================================================================
//������: ServerDelDevProc
//��  ��: eSERVERCMD_DELDEV�����  ɾ���豸
//��  ��: ��
//��  ��: ��
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
	
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype && (buflen % 6) == 0)
	{
		groupnum = buflen / 6;

		for(i = 0; i < groupnum; i++)
		{
			nodeid = *((uint_16*)(ptcmd->pcmdbuf + i * 6 + 2));
			devnum = *((uint_16*)(ptcmd->pcmdbuf + i * 6 + 4));

			//��С��ת��
			nodeid = REVERSE16(nodeid);
			devnum = REVERSE16(devnum);

			//20130105ɾ�������豸
			if(nodeid == 0 && devnum == 0 && 
				list_empty(s_ptcomserverinfo->pterminalnodedatalist) == FALSE)
			{
				dellist = s_ptcomserverinfo->pterminalnodedatalist->next;
				
				_time_get_ticks(&ticks);
				while(dellist != NULL && is_timeout(ticks.TICKS[0], 30000) == FALSE)
				{
			        pnode = list_entry(dellist, TNodeInfo, list);

					//��һ���ڵ�
					dellist = dellist->next;

					//��datalist��ɾ��
					list_del(&pnode->list);	
					
					DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "[ServerDelDevProc]: del dev nodeid = ");
					DEBUGOUT_HEX(DEBUGOUT_LEVEL_ALARM, pnode->tnodeinfosave.nodeid);
					DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, "\r\n");
					
					//����freelist
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

					//�豸����
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
					//��datalist��ɾ��
					list_del(&pnode->list);	
					//����freelist
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

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			            s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				        eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//������: ServerIPPortReqProc
//��  ��: eSERVERCMD_IPPORTREQ�����   ��ѯ������IP Port
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerIPPortReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_IPPORTREQ);   //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
	uint_8 buf[6];
		
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_IPPORT;
		memcpy(buf, s_ptcomserverinfo->tcomgprsinfo.gprsdstip, 4);
		//memcpy(&buf[4], (int_8*)&s_ptcomserverinfo->tcomgprsinfo.gprsdstport, 2);
		*(uint_16*)&buf[4] = REVERSE16(s_ptcomserverinfo->tcomgprsinfo.gprsdstport);
		pack = buf;
		acklen = 6;
	}

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			            s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				        acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//������: ServerIPPortSetProc
//��  ��: eSERVERCMD_IPPORTSET�����  ���÷�����IP Port
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerIPPortSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_IPPORTSET;   //����Ӧ��
	
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype && (ptcmd->pchk - ptcmd->pcmdbuf) == 6)
	{
		memcpy(s_ptcomserverinfo->tcomgprsinfo.gprsdstip, ptcmd->pcmdbuf, 4);
		memcpy((int_8*)&s_ptcomserverinfo->tcomgprsinfo.gprsdstport, &ptcmd->pcmdbuf[4], 2);

		//��С��ת��
		s_ptcomserverinfo->tcomgprsinfo.gprsdstport = REVERSE16(s_ptcomserverinfo->tcomgprsinfo.gprsdstport);
		
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
		{
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomgprsinfo, sizeof(TComGprsInfo), 0);
			ack = eSERVERACK_ERR_NONE;
		}
	}

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			            s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				        eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//������: ServerVerReqProc
//��  ��: eSERVERCMD_REQ_VERSION�����  �汾��ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerVerReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 buf[6];
	uint_16 acktype = eSERVERCMD_ACK_VERSION;
	uint_16 acklen = 6;

	//��������ѯ
	if(eLANFRAMETYPE_ME == frametype)
	{
		buf[0] = VER_PROT_MAJOR;
		buf[1] = VER_PROT_MINOR;
		buf[2] = VER_SOFT_MAJOR;
		buf[3] = VER_SOFT_MINOR;
		buf[4] = VER_HARD_MAJOR;
		buf[5] = VER_HARD_MINOR;
	}
	//��ַ����
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
//������: ServerAlarmPhoneFlagReqProc
//��  ��: eSERVERCMD_REQ_MOBILEALARM�����   �ֻ��澯���ܲ�ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerAlarmPhoneFlagReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 buf[2];
	uint_16 acktype = eSERVERCMD_ACK_MOBILEALARM;
	uint_16 acklen = 1;

	//��������ѯ
	if(eLANFRAMETYPE_ME == frametype)
	{
		buf[0] = s_ptcomserverinfo->tcomgsminfo.bgsmalarm;
	}
	//��ַ����
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
//������: ServerAlarmPhoneFlagSetProc
//��  ��: eSERVERCMD_SET_MOBILEALARM�����  �ֻ��澯��������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerAlarmPhoneFlagSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_MOBILEALARMSET;   //����Ӧ��
	
	//��������ͨ��������
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

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
			            s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				        eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//������: ServerAlarmPhoneReqProc
//��  ��: eSERVERCMD_REQ_MOBILEALARMNUM�����   �ֻ��澯�����ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerAlarmPhoneReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 buf[100];
    uint_8 i;
	uint_16 acktype = eSERVERCMD_ACK_MOBILEALARMNUM;
	uint_16 acklen;

	//��������ѯ
	if(eLANFRAMETYPE_ME == frametype)
	{
		for(i = 0; i < s_ptcomserverinfo->tcomgsminfo.phonenum; i++)
			memcpy(&buf[i * 11], s_ptcomserverinfo->tcomgsminfo.phone[i], 11);

		acklen = 11 * s_ptcomserverinfo->tcomgsminfo.phonenum;
		for(i = 0; i < acklen; i++)
			buf[i] -= '0';
	}
	//��ַ����
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
//������: ServerAlarmPhoneSetProc
//��  ��: eSERVERCMD_SET_MOBILEALARM�����   �ֻ��澯��������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerAlarmPhoneSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_MOBILEALARMNUMSET;   //����Ӧ��
	uint_8 i, j;

	i = (ptcmd->pchk - ptcmd->pcmdbuf) / 11;
	//��������ͨ��������
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

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//������: ServerPollTimeReqProc
//��  ��: eSERVERCMD_POLLTIMEREQ�����   ��ѯ��������ѯʱ��
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerPollTimeReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = eSERVERACK_ERR_POLLTIMEREQ;   //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
		
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_POLLTIME;
		ack = (s_ptcomserverinfo->tcomgprsinfo.polltime / 1000) ;
	}

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//������: ServerPollTimeSetProc
//��  ��: eSERVERCMD_POLLTIMESET�����   ������������ѯʱ��
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerPollTimeSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;	 
	uint_16 ack = eSERVERACK_ERR_POLLTIMESET;	 //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
	
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype && (ptcmd->pchk - ptcmd->pcmdbuf) == 2)
	{
		s_ptcomserverinfo->tcomgprsinfo.polltime = REVERSE16(*(uint_16*)ptcmd->pcmdbuf) * 1000;
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
		{
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomgprsinfo, sizeof(TComGprsInfo), 0);
			ack = eSERVERACK_ERR_NONE;
		}
	}

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//������: PhoneAlarmProc
//��  ��: eSERVERCMD_PHONEALARM_DATA�����  �ֻ��澯���ݷ���
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 PhoneAlarmProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_8 len = ptcmd->pchk - ptcmd->pcmdbuf;
	uint_8 i, tmp;

	//����У��
	if(len > 100 || (len % 2) != 0)
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[PhoneAlarmProc]: len err!\r\n");
		return eRET_LEN;
	}

	//�ֻ����Ÿ澯
	if(len < 100)
	{
		g_gsmsendlen = 0;
		
		//�ַ�ת��
		for(i = 0; i < len; i++)
		{
			//��4λ
			tmp = (ptcmd->pcmdbuf[i] & 0xF0) >> 4;
			g_gsmsendbuf[g_gsmsendlen++] = (tmp < 10) ? ('0' + tmp) : ('A' + tmp - 10);

			//��4λ
			tmp = ptcmd->pcmdbuf[i] & 0x0F;
			g_gsmsendbuf[g_gsmsendlen++] = (tmp < 10) ? ('0' + tmp) : ('A' + tmp - 10);
		}
	}
	
	return eRET_NONE;
}

//============================================================================
//������: ServerBroadSetTimeReqProc
//��  ��: eSERVERCMD_REQ_BROADSETTIME�����  ��ѯ�������㲥��������ִ�д���
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerBroadSetTimeReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK_BROADSETTIME;
	uint_8 buf[2];
	uint_8 acklen;

	//��������ѯ
	if(eLANFRAMETYPE_ME == frametype)
	{
		buf[0] = s_ptcomserverinfo->tcomgprsinfo.broadsettime;
		acklen = 1;
	}
	//��ַ����
	else
	{
		//��С��ת��
		*((uint_16*)buf) = REVERSE16(eSERVERACK_ERR_ADDR);
		acklen = 2;
	}
	
	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, buf, acklen, *ptcmd->pret);
	
}

//============================================================================
//������: ServerBroadSetTimeSetProc
//��  ��: eSERVERCMD_SET_MOBILEALARM�����  �����������㲥��������ִ�д���
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerBroadSetTimeSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 ack = eSERVERACK_ERR_BROADSETTIMESET;   //����Ӧ��
	
	//��������ͨ��������
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

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    eSERVERCMD_ACK, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//������: ServerIPReqMACIP
//��  ��: eSERVERCMD_REQ_MAC�����  ͨ��ZIG_IP��ַ��ѯZIG_MACIP��ַ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerIPReqMACIP(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, 
		                      eTERMINALCOM_REQ_MAC, NULL, 0, *ptcmd->pret);
}

//============================================================================
//������: ServerMACReqMACIP
//��  ��: eSERVERCMD_REQ_MACIP�����   ͨ��ZIG_MAC��ַ��ѯZIG_MACIP��ַ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerMACReqMACIP(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
    uint_8 ret = eRET_NONE;
	uint_16 errack = eSERVERACK_ERR_ADDR;

	//��������ѯ��MAC��ַ��ת��Ϊȫ���㲥
	if(eLANFRAMETYPE_ME == frametype && ptcmd->pchk == ptcmd->pcmdbuf + 8)
		ret = ComTerminalSendCmd(BROADCAST_NETID, *ptcmd->proute, 0, BROADCAST_NODEID, eTERMINALCOM_REQ_MACIP, ptcmd->pcmdbuf, 8, *ptcmd->pret);
	//��ַ����
	else
	{
		//��С��ת��
		errack = REVERSE16(errack);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
					eSERVERCMD_ACK, (uint_8*)&errack, 2, *ptcmd->pret);
	}
	
	return ret;
}

//============================================================================
//������: ServerMACSetIP
//��  ��: eSERVERCMD_SET_MACIP�����  ͨ��ZIG_MAC��ַ����ZIG_IP��ַ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerMACSetIP(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
    uint_8 ret = eRET_NONE;
	uint_16 errack = eSERVERACK_ERR_ADDR;

	//��������ѯ��MAC��ַ��ת��Ϊȫ���㲥
	if(eLANFRAMETYPE_ME == frametype && ptcmd->pchk == ptcmd->pcmdbuf + 14)
	{
		ret = ComTerminalSendCmd(BROADCAST_NETID, *ptcmd->proute, 0, BROADCAST_NODEID, eTERMINALCOM_SET_MACIP, ptcmd->pcmdbuf, 14, *ptcmd->pret);
	}
	//��ַ����
	else
	{
		//��С��ת��
		errack = REVERSE16(errack);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
					eSERVERCMD_ACK, (uint_8*)&errack, 2, *ptcmd->pret);
	}
	
	return ret;
}

//============================================================================
//������: ServerResetTimeReqProc
//��  ��: eSERVERCMD_REQ_RESETIME�����   ��ѯ���������ն˸�λʱ��
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerResetTimeReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = eSERVERACK_ERR_RESETIMEREQ;   //����Ӧ��
	
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_RESETIME;
		ack = s_ptcomserverinfo->tcomgprsinfo.cpuresettime / 1000;
	}

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMCarrierReqProc
//��  ��: eSERVERCMD_REQ_WSMCARRIER�����   ��������Ӫ�̲�ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMCarrierReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_WSMCARRIERREQ);   //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
	
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_WSMCARRIER;
		*pack = s_ptcomserverinfo->tcomgprsinfo.gprslinktype;
		acklen = 1;
	}

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMCarrierSetProc
//��  ��: eSERVERCMD_SET_WSMCARRIER�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMCarrierSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	

	return 0;
}

//============================================================================
//������: ServerWSMVolReqProc
//��  ��: eSERVERCMD_REQ_WSMVOL�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMVolReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMCurReqProc
//��  ��: eSERVERCMD_REQ_WSMCUR�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMCurReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMSwitchInReqProc
//��  ��: eSERVERCMD_REQ_WSMSWITCHIN�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMSwitchInReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMSwitchOutReqProc
//��  ��: eSERVERCMD_REQ_WSMSWITCHOUT�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMSwitchOutReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMSwitchOutSetProc
//��  ��: eSERVERCMD_SET_WSMSWITCHOUT�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMSwitchOutSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMCompileTimeReqProc
//��  ��: eSERVERCMD_REQ_WSMCOMPILETIME�����  ����������ʱ���ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMCompileTimeReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_WSMCOMPILETIMEREQ);   //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 buf[50];
	uint_8 acklen = 2;
	
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_WSMCOMPILETIME;
		pack = buf;
		acklen = strlen((int_8*)CompileDate);
		memcpy(buf, CompileDate, acklen);
		memcpy(buf + acklen, CompileTime, strlen((int_8*)CompileTime));
		acklen += strlen((int_8*)CompileTime);
	}

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMSDDataReqProc
//��  ��: eSERVERCMD_REQ_SDDATA�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMSDDataReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMHeartAttrReqProc
//��  ��: eSERVERCMD_REQ_WSMHEARTATTR�����  ���������������Բ�ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartAttrReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_8 buf[11];
	uint_8 acklen = 2;

	*((uint_16*)buf) = REVERSE16(eSERVERACK_ERR_WSMHEARTATTRREQ);  //����Ӧ��
	
	//��������ͨ��������
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

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, buf, acklen, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMHeartAttrSetProc
//��  ��: eSERVERCMD_SET_WSMHEARTATTR�����  ��������������������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartAttrSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;	 
	uint_16 ack = eSERVERACK_ERR_WSMHEARTATTRSET;	 //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 acklen = 2;
	uint_8 hearttype, devinfonum, devinfotype;
	uint_16 hearttime;
	uint_16 heartmax;
	
	//��������ͨ��������
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

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMSysTimeSetProc
//��  ��: eSERVERCMD_SET_SYSTIME�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMSysTimeSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	

	return 0;
}

//============================================================================
//������: ServerWSMTimeConReqProc
//��  ��: eSERVERCMD_REQ_TIMECONTROL�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMTimeConReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMTimeConSetProc
//��  ��: eSERVERCMD_SET_TIMECONTROL�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMTimeConSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMPollAttrReqProc
//��  ��: eSERVERCMD_REQ_WSMPOLLATTR�����  ��������ѯ���Բ�ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMPollAttrReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_WSMPOLLATTRREQ);   //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 buf[12];
	uint_16 acklen = 2;
	uint_8 groupid;
	uint_16 nodeid;
	TNodeInfo* pnode = NULL;
	
	//��������ͨ��������
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

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMPollAttrSetProc
//��  ��: eSERVERCMD_SET_WSMPOLLATTR�����   ��������ѯ��������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMPollAttrSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = eSERVERACK_ERR_WSMPOLLATTRSET;   //����Ӧ��
	uint_16 nodeid;
	TNodeInfo* pnode = NULL;
	struct list_head* settlist = NULL;
	MQX_TICK_STRUCT ticks;
	uint_8 groupid;

	//����У��
	if(eLANFRAMETYPE_ME == frametype && 12 == ptcmd->pchk - ptcmd->pcmdbuf)
	{
		groupid = ptcmd->pcmdbuf[0];
		nodeid = REVERSE16(*((uint_16*)&ptcmd->pcmdbuf[1]));
		//�ڵ���������
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
		//����������
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

	//���Ա仯�洢
	if(eSERVERACK_ERR_NONE == ack)
		s_ptcomserverinfo->pcomterminaldevchcallbak();

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMDevGroupReqProc
//��  ��: eSERVERCMD_REQ_WSMDEVGROUP�����  �������ն˷����ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMDevGroupReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERCMD_ERR_REQ_WSMDEVGROUP);   //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 buf[900];
	uint_16 acklen = 2;
	uint_8 i;
	TNodeInfo* pnode = NULL;
	struct list_head* reqlist = NULL;
	
	//��������ͨ��������
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

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMDevGroupSetProc
//��  ��: eSERVERCMD_SET_WSMDEVGROUP�����  �������ն˷�������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMDevGroupSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = eSERVERCMD_ERR_SET_WSMDEVGROUP;   //����Ӧ��
	uint_16 len = ptcmd->pchk - ptcmd->pcmdbuf;
	uint_16 devnum = len - 1;
	uint_16 i;
	uint_16* pdevid;
	TNodeInfo* pnode = NULL;
	bool bcallback = FALSE;

	//����У��
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
		
		//ʱ�ο���
		if(bcallback && s_ptcomserverinfo->pcomterminaldevchcallbak != NULL)
		{
			s_ptcomserverinfo->pcomterminaldevchcallbak();
			ack = eSERVERACK_ERR_NONE;
		}
	}

	//��С��ת��
	ack = REVERSE16(ack);

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, (uint_8*)&ack, 2, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMHeartVolNumReqProc
//��  ��: eSERVERCMD_REQ_WSMVOLNUM�����   ��������������ѹ��Ų�ѯ
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartVolNumReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;     
	uint_16 ack = REVERSE16(eSERVERACK_ERR_WSMVOLNUMREQ);   //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 tmp;
	uint_8 acklen = 2;
	
	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_WSMVOLNUM;
		tmp = s_ptcomserverinfo->tcomgprsinfo.heartvolnum;   
		pack = &tmp;
		acklen = 1;
	}

	//Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid,  
				    acktype, pack, acklen, *ptcmd->pret);
}

//============================================================================
//������: ServerWSMHeartVolNumSetProc
//��  ��: eSERVERCMD_SET_WSMVOLNUM�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartVolNumSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMHeartCurNumReqProc
//��  ��: eSERVERCMD_REQ_WSMCURNUM�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartCurNumReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMHeartCurNumSetProc
//��  ��: eSERVERCMD_SET_WSMCURNUM�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartCurNumSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMHeartSwitchInNumReqProc
//��  ��: eSERVERCMD_REQ_WSMSWITCHINNUM�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartSwitchInNumReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMHeartSwitchInNumSetProc
//��  ��: eSERVERCMD_SET_WSMSWITCHINNUM�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartSwitchInNumSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMHeartSwitchOutNumReqProc
//��  ��: eSERVERCMD_REQ_WSMSWITCHOUTNUM�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartSwitchOutNumReqProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerWSMHeartSwitchOutNumSetProc
//��  ��: eSERVERCMD_SET_WSMSWITCHOUTNUM�����
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerWSMHeartSwitchOutNumSetProc(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	return 0;
}

//============================================================================
//������: ServerTerminalAppCmdProc
//��  ��: �ն�Ӧ�������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerTerminalAppCmdProc(const TServerCOMFrame* const ptcmd,
	uint_8 terminalappcmd, uint_8 groupsize)
{
	uint_8 ret = eRET_NONE;
	uint_8 i, buf[50];       
	TTerminalAppHead terminalapp;
	uint_8 groupnum = 0;
	
	//������Ŵ���
	if(groupsize == 0 && ptcmd->pchk != ptcmd->pcmdbuf)
		goto ServerTerminalAppCmdProc_ERRRET;
	else if(groupsize > 0)
		groupnum = (ptcmd->pchk - ptcmd->pcmdbuf) / groupsize;
	if(groupnum > GROUPID_MAX)
		goto ServerTerminalAppCmdProc_ERRRET;

	//�޿������,���ж�
//	if(groupsize > 1)
//	{
//		for(i = 0; i < groupnum; i++)
//		{
//			if(ptcmd->pcmdbuf[i] > GROUPID_MAX)
//				goto ServerTerminalAppCmdProc_ERRRET;
//			i += (groupsize - 1);
//		}
//	}
	
	//��ַ����
	TerminalAppStructAddr(&terminalapp, buf);
	
	//���ն�����֡
	*terminalapp.plen = 2;
	*terminalapp.pcmd = terminalappcmd;
	if(groupnum > 0 && groupsize > 0)
	{
		groupnum *= groupsize;
		*terminalapp.plen += groupnum;
		for(i = 0; i < groupnum; i++)
			terminalapp.pbuf[i] = ptcmd->pcmdbuf[i];
	}
	
	//�ն���֡����
	return ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, eTERMINALCOM_DATA, buf, *terminalapp.plen, *ptcmd->pret);
	
ServerTerminalAppCmdProc_ERRRET:
	return eRET_PARAM;
}


//============================================================================
//������: ServerTerminalAppCmdProc_NoGroup
//��  ��: �ն�Ӧ�������
//��  ��: ��
//��  ��: ��
//============================================================================
static uint_8 ServerTerminalAppCmdProc_NoGroup(const TServerCOMFrame* const ptcmd,
	uint_8 terminalappcmd, uint_8 datasize)
{
	uint_8 ret = eRET_NONE;
	uint_8 i, buf[50];       
	TTerminalAppHead terminalapp;

	//����
	if(datasize == 0 && ptcmd->pchk != ptcmd->pcmdbuf)
		goto ServerTerminalAppCmdProc_NoGroup_ERRRET;
	
	//��ַ����
	TerminalAppStructAddr(&terminalapp, buf);
	
	//���ն�����֡
	*terminalapp.plen = 2;
	*terminalapp.pcmd = terminalappcmd;
	if(datasize > 0)
	{
		*terminalapp.plen += datasize;
		for(i = 0; i < datasize; i++)
			terminalapp.pbuf[i] = ptcmd->pcmdbuf[i];
	}
	
	//�ն���֡����
	return ComTerminalSendCmd(*ptcmd->pnetid, *ptcmd->proute, *ptcmd->pgroupid, *ptcmd->pnodeid, eTERMINALCOM_DATA, buf, *terminalapp.plen, *ptcmd->pret);
	
ServerTerminalAppCmdProc_NoGroup_ERRRET:
	return eRET_PARAM;
}

//============================================================================
//������: ServerDetecDev
//��  ��: ����豸
//��  ��: ��
//��  ��: ��
//����豸 �����ʽ�� ��ַ��2B  + �����֣�1B  +  ���ݣ� 4B   + ����ͣ� 1B
//============================================================================
static uint_8 ServerDetecDev(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 ack = REVERSE16(eSERVERACK_ERR_DMX_DETEC_DEV);   //����Ӧ��
	uint_8* pack = (uint_8*)&ack;
	uint_8 tmp;
	uint_8 ret = eRET_PARAM;
	uint_8 acklen = 4;

	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_DMX_DETEC_DEV;

//		memcpy(detec_sendcmd, &ptcmd->pcmdbuf[0], 8);
		_lwevent_clear(&lwevent_group, EVENT_DMX512);    //��ֹDMX512�ź����
        //�л�Ϊ����״̬
		gpio_set(CH1_DIR, 1);
		delay_ms(100);
		uart_init(CH1_UART, 9600);
		delay_ms(150);
		uart_sendN(CH1_UART, 8, &(ptcmd->pcmdbuf[0]));
		delay_ms(5);
		gpio_set(CH1_DIR, 0);           //�л�Ϊ����״̬

        delay_ms(100);                   //�ȴ��жϽ�������
		//��ʱ��λ��������ȼ���task_DMX512����õ�ִ��
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
//������: ServerWriteSDHC
//��  ��: ����SD�����ݴ���
//��  ��: ��
//��  ��: ��
//дSD���������� ǰ6���ֽ� �ֱ��� ��֡�� ֡�� ��֡�� ÿ���ֶ�ռ�����ֽ�
//============================================================================
static uint_8 ServerWriteSDHC(const TServerCOMFrame* const ptcmd, uint_8 frametype)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 ack = REVERSE16(eSERVERACK_ERR_DMX_WRITE_SDHC);   //����Ӧ��
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
	
	sd_fsum = ((ptcmd->pcmdbuf[0])<<8) | (ptcmd->pcmdbuf[1]);   //��֡��
	sd_fnum = ((ptcmd->pcmdbuf[2])<<8) | (ptcmd->pcmdbuf[3]);   //֡��
    sd_flen = ((ptcmd->pcmdbuf[4])<<8) | (ptcmd->pcmdbuf[5]);   //��֡��

	//��������ͨ��������
	if(eLANFRAMETYPE_ME == frametype)
	{
		acktype = eSERVERCMD_ACK_DMX_WRITE_SDHC;

//		len = znFAT_Open_File(&fileinfo1,"/f0_2CH.txt",0,1);
//
//		if(!len)
//			printf("[ServerWriteSDHC]: Old File open correct!\r\n");

		printf("[ServerWriteSDHC]:sd_fsum = %d, sd_fnum = %d, sd_flen = %d\r\n", sd_fsum, sd_fnum, sd_flen);

		//�����֡�ǵ�0֡�����ļ����ǿ��ļ�,��ô�������������ļ���ֻ�е�0֡��ACK
		if(sd_fnum == 0)
		{
			 g_bSDWaitEvent = FALSE;
			 g_bDMXWaitEvent = FALSE;
			 _lwevent_clear(&lwevent_group, EVENT_SDHC);      //��ֹ��SD��
			 _lwevent_clear(&lwevent_group, EVENT_DMX512);    //��ֹDMX512�ź����

			 //��0֡���ڴ��л���g_smallrecvfreelist
			 //�л�ǰ����ȴ�sd��dmx�߳�����
			 while(!g_bSDWaitEvent || !g_bDMXWaitEvent)
			 {
				 _time_delay_ticks(2);	 //����CPU��SD����ִ��
			 }
			 
			 b_enet = false;  //ֹͣ���͹���

			 //ɾ��ԭ�ļ������½���һ��ͬ���ļ�
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

					//ֻ���յ���0֡������½���ACK
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

	    //���ж�
	    DISABLE_INTERRUPTS;
		//_time_get_ticks(&mqx_tick1);
		znFAT_WriteData(&fileinfo1, sd_flen, &(ptcmd->pcmdbuf[6]));
		//_time_get_ticks(&mqx_tick2);
		//printf("Write SD time = %d\r\n",mqx_tick2.TICKS[0]-mqx_tick1.TICKS[0]);
        //���ж�
        ENABLE_INTERRUPTS;
		//printf("End Write SD!!!\r\n");
        znFAT_Close_File(&fileinfo1);

		//�ļ�������ϣ�֡��=��֡��-1���ñ�־λ��
		if(sd_fnum == (sd_fsum - 1))
		{
			b_enet = true;
			b_sd = true;      //������ɣ���Ϊ���ļ�
//			znFAT_Close_File(&fileinfo1);
			znFAT_Flush_FS();

	        //���һ֡���ڴ��л���g_tffreelist
			pnode = g_tSmallBufNode;
		    for(i = 0, j = 0; i < SMALL_BUF_NODE_NUM; )
		    {
				//������ͨ���ϵĽڵ㸳���ֳ�ʼֵ
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
			
			_lwevent_set(&lwevent_group, EVENT_SDHC);    //����ִ�ж�SD������
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
//������: ServerCmdProc
//��  ��: �����
//��  ��: const TServerCOMFrame* const ptcmd����������  ���Ƿ���������֡
//��  ��: ��
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
	     //���ģ������
		case eSERVERCMD_DMX_DETEC_DEV:
			ret = ServerDetecDev(ptcmd, frametype);
			break;
	    //дSD������
	    case eSERVERCMD_DMX_WRITE_SDHC:
	    	ret = ServerWriteSDHC(ptcmd, frametype);
		    break;
		//��ַ��ѯ
		case eSERVERCMD_REQ_ADDR:
			ret = ServerAddrReqProc(ptcmd, frametype);
			break;
		//��ַ����
		case eSERVERCMD_SET_ADDR:
			ret = ServerAddrSetProc(ptcmd, frametype);
			break;
		//ͨ����ѯ
		case eSERVERCMD_REQ_RFCH:
			ret = ServerRFChReqProc(ptcmd, frametype);
			break;
		//ͨ������
		case eSERVERCMD_SET_RFCH:
			ret = ServerRFChSetProc(ptcmd, frametype);
			break;
		//·��������ѯ
		case eSERVERCMD_REQ_ROUTE:
			ret = ServerRouteReqProc(ptcmd, frametype);
			break;
		//·����������
		case eSERVERCMD_SET_ROUTE:
			ret = ServerRouteSetProc(ptcmd, frametype);
			break;
		//ת��������ѯ
		case eSERVERCMD_REQ_TRANUM:
			ret = ServerTranReqProc(ptcmd, frametype);
			break;
		//ת����������
		case eSERVERCMD_SET_TRANUM:
			ret = ServerTranSetProc(ptcmd, frametype);
			break;
		//CSMACA������ѯ
		case eSERVERCMD_REQ_CSMACA:
			ret = ServerCSMACAReqProc(ptcmd, frametype);
			break;
		//CSMACA��������
		case eSERVERCMD_SET_CSMACA:
			ret = ServerCSMACASetProc(ptcmd, frametype);
			break;
				
		//��ѯ��������ѯʱ��
		case eSERVERCMD_REQ_POLLTIME:
			ret = ServerPollTimeReqProc(ptcmd, frametype);
			break;
			
		//������������ѯʱ��
		case eSERVERCMD_SET_POLLTIME:
			ret = ServerPollTimeSetProc(ptcmd, frametype);
			break;
			
		//��ѯ������IP Port
		case eSERVERCMD_REQ_IPPORT:
			ret = ServerIPPortReqProc(ptcmd, frametype);
			break;
			
		//���÷�����IP Port
		case eSERVERCMD_SET_IPPORT:
			ret = ServerIPPortSetProc(ptcmd, frametype);
			break;
			
		//�汾��ѯ
		case eSERVERCMD_REQ_VERSION:
			//�ն˿�����
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
			
		//�������ֻ����Ų�ѯ
		case eSERVERCMD_REQ_SIMNUM:
			break;
			
		//�ֻ��澯���ܲ�ѯ
		case eSERVERCMD_REQ_MOBILEALARM:
			ret = ServerAlarmPhoneFlagReqProc(ptcmd, frametype);
			break;
			
		//�ֻ��澯��������
		case eSERVERCMD_SET_MOBILEALARM:
			ret = ServerAlarmPhoneFlagSetProc(ptcmd, frametype);
			break;
			
		//�ֻ��澯�����ѯ
		case eSERVERCMD_REQ_MOBILEALARMNUM:
			ret = ServerAlarmPhoneReqProc(ptcmd, frametype);
			break;
			
		//�ֻ��澯��������
		case eSERVERCMD_SET_MOBILEALARMNUM:
			ret = ServerAlarmPhoneSetProc(ptcmd, frametype);
			break;
			
		//�豸��λ
		case eSERVERCMD_RESET:
			//�ն˿�����
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
			//������
			else if(eLANFRAMETYPE_ME == frametype)
#else
			if(eLANFRAMETYPE_ME == frametype)
#endif
			{
				
				back = TRUE;
			}
            //��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//�ն˿����������ϴ����ܲ�ѯ
		case eSERVERCMD_REQ_INDIFLAG:
			//�ն˿�����
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_INDIFLAG, 0);
				if(ret != eRET_NONE)
				{
					errack = eTERMINALAppACK_ERR_INDIFLAGREQ;
					back = TRUE;
				}
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//�ն˿����������ϴ���������
		case eSERVERCMD_SET_INDIFLAG:
			//�ն˿�������㲥����
			if(frametype != eLANFRAMETYPE_ME && frametype < eLANFRAMETYPE_OTHEER_NET)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_SET_INDIFLAG, 1);
				if(ret != eRET_NONE)
				{
					errack = eTERMINALAppACK_ERR_INDIFLAGSET;
					back = TRUE;
				}
				//�㲥���ã�������������ȷӦ��
				else if(frametype < eLANFRAMETYPE_NODE_NET)
					back = TRUE;
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;

		//�ֻ��澯���ݷ���
		case eSERVERCMD_PHONEALARM_DATA:
			if(eLANFRAMETYPE_ME == frametype)
			{
				if(PhoneAlarmProc(ptcmd, frametype) != eRET_NONE)
					errack = eSERVERACK_ERR_PHONEALARM;
				
			}
			//��ַ����
			else
				errack = eSERVERACK_ERR_ADDR;
			back = TRUE;
			break;
			
		//��ѯ�������㲥��������ִ�д���
		case eSERVERCMD_REQ_BROADSETTIME:
			ret = ServerBroadSetTimeReqProc(ptcmd, frametype);
			break;
			
		//�����������㲥��������ִ�д���
		case eSERVERCMD_SET_BROADSETTIME:
			ret = ServerBroadSetTimeSetProc(ptcmd, frametype);
			break;
			
		//ͨ��ZIG_IP��ַ��ѯZIG_MACIP��ַ
		case eSERVERCMD_REQ_MAC:
			ret = ServerIPReqMACIP(ptcmd, frametype);
			break;
			
		//ͨ��ZIG_MAC��ַ��ѯZIG_MACIP��ַ
		case eSERVERCMD_REQ_MACIP:
			ret = ServerMACReqMACIP(ptcmd, frametype);
			break;
			
		//ͨ��ZIG_MAC��ַ����ZIG_IP��ַ
		case eSERVERCMD_SET_MACIP:
			ret = ServerMACSetIP(ptcmd, frametype);
			break;
			
		//��ѯ���������ն˸�λʱ��
		case eSERVERCMD_REQ_RESETIME:
			//���ڵ�
			if(eLANFRAMETYPE_ME == frametype)
				ret = ServerResetTimeReqProc(ptcmd, frametype);
			//�ն˲�ѯ
			else if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_RESETIME, 0);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_RESETIMEREQ;
					back = TRUE;
				}
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//Zigbeeģ�鿴�Ź���λʱ���ѯ
		case eSERVERCMD_REQ_WDITIME:
			//���ڵ�
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_REQ_WDITIME, 0);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_WDITIMEREQ;
					back = TRUE;
				}
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
		
		//Zigbeeģ�鿴�Ź���λʱ������
		case eSERVERCMD_SET_WDITIME:
			//���ڵ�
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_SET_WDITIME, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_WDITIMESET;
					back = TRUE;
				}
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//�㲥��ѯZIG_MAC��ַӦ���־��ѯ
		case eSERVERCMD_REQ_MACFLAG:
			//�㲥��ѯZIG_MAC��ַӦ���־��ѯ��ת��Ϊȫ���㲥
			if(eLANFRAMETYPE_ME == frametype)
			{
				//�ն���֡����
				ret = ComTerminalSendCmd(BROADCAST_NETID, eZIGFRAMEROUTE_FLOOD, 0, BROADCAST_NODEID, 
				                eTERMINALCOM_REQ_MACFLAG, ptcmd->pcmdbuf, 8, *ptcmd->pret);
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//�㲥��ѯZIG_MAC��ַӦ���־����
		case eSERVERCMD_SET_MACFLAG:
			//�㲥��ѯZIG_MAC��ַӦ���־������ת��Ϊȫ���㲥
			if(eLANFRAMETYPE_ME == frametype)
			{
				//�ն���֡����
				ret = ComTerminalSendCmd(BROADCAST_NETID, eZIGFRAMEROUTE_FLOOD, 0, BROADCAST_NODEID, 
				                 eTERMINALCOM_SET_MACFLAG, ptcmd->pcmdbuf, 9, *ptcmd->pret);
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		// �㲥��ѯZIG_MAC��ַ
		case eSERVERCMD_MACBROADREQ:
			//��������ѯ��MAC��ַ��ת��Ϊȫ���㲥
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ComTerminalSendCmd(BROADCAST_NETID, eZIGFRAMEROUTE_FLOOD, 0, BROADCAST_NODEID, 
					             eTERMINALCOM_REQ_MACBROAD, NULL, 0, *ptcmd->pret);
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;

		//Ӳ����ַ��ѯ
		case eSERVERCMD_REQ_HDADDR:
			//���ڵ�
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_HDADDRREQ, 0);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_HDADDRREQ;
					back = TRUE;
				}
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//Ӳ����ַ����
		case eSERVERCMD_SET_HDADDR:
			//���ڵ�
			if(eLANFRAMETYPE_ME == frametype)
			{
				ret = ServerTerminalAppCmdProc(ptcmd, eTERMINALAPPCMD_HDADDRESET, 1);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_HDADDRSET;
					back = TRUE;
				}
			}
			//��ַ����
			else
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}
			break;
			
		//��ѯ�����豸
		case eSERVERCMD_REQ_ALLADDR:
			ServerReqAllDevProc(ptcmd, frametype);
			break;
			
		//�����豸
		case eSERVERCMD_ADD_DEV:
			ret = ServerAddDevProc(ptcmd, frametype);
			break;
			
		//ɾ���豸
		case eSERVERCMD_DEL_DEV:
			ret = ServerDelDevProc(ptcmd, frametype);
			break;

		//���ص�״̬��ѯ
		case eSERVERCMD_REQ_BULK:
			//�ն˿�����
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
			
		//���ص�״̬����
		case eSERVERCMD_SET_BULK:
#ifdef ZIGBEE_CTRL
			//����
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}	
#else
			errack = eSERVERACK_ERR_BULKSET;
			back = TRUE;
#endif
			break;
			
		//�ƹ��ʲ�ѯ
		case eSERVERCMD_REQ_POWER:
			//�ն˿�����
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
			
		//�ƹ�������
		case eSERVERCMD_SET_POWER:
#ifdef ZIGBEE_CTRL
			//�ն˿�����
			
			{
				errack = eSERVERACK_ERR_ADDR;
				back = TRUE;
			}	
#else
			errack = eSERVERACK_ERR_POWERSET;
			back = TRUE;
#endif
			break;

		//��һǶ��ʽģ�������ѯ
		case eSERVERCMD_REQ_PARAM:
			//�ն˿�����
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
			
		//��һǶ��ʽģ���������
		case eSERVERCMD_SET_PARAM:
			//�ն˿�����
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
			
		//��״̬��ѯ
		case eSERVERCMD_REQ_BULKSTATE:
			//���ȴ���
			if(ptcmd->pchk - ptcmd->pcmdbuf != 1)
			{
				errack = eSERVERACK_ERR_LEN;
				back = TRUE;
			}
			//�ն˿�����
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
		
		//ADֵ��ѯ
		case eSERVERCMD_REQ_ADVALUE:
			//�ն˿�����
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
		
		//ͨ��MAC��ַ���п��ص�
		case eSERVERCMD_MACSETBULK:
#ifdef ZIGBEE_CTRL
			//��������ѯ��IP��ַ��ת��Ϊȫ���㲥
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
        //�ƾ�������ʽ��ѯ
		case eSERVERCMD_REQ_STARTUPMODE:
			//�ն˿�����
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
		//�ƾ�������ʽ����
		case eSERVERCMD_SET_STARTUPMODE:
			//�ն˿�����
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
		//���ص����ò�ѯ
		case eSERVERCMD_REQ_BULKSET:
			//�ն˿�����
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
		//���صƵ�����ֵ��ѯ
		case eSERVERCMD_REQ_BULKONCUR:
			//�ն˿�����
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
		//���صƵ�����ֵ����
		case eSERVERCMD_SET_BULKONCUR:
			//�ն˿�����
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
		//0x1012. �ն˵�ѹ������ѯ
		case eSERVERCMD_REQ_VOLTAGE:
			//�ն˿�����
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
		//0x1013. �ն˵���������ѯ
		case eSERVERCMD_REQ_CURRENT:
			//�ն˿�����
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
		//0x1014. �ն��¶Ȳ�����ѯ
		case eSERVERCMD_REQ_TEMPERATURE:
			//�ն˿�����
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
		//0x1015. �ն˵�ѹ����������ѯ
		case eSERVERCMD_REQ_VOLCUR:
			//�ն˿�����
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
			
		//0x1200. BMM������ѯ����
		case eSERVERCMD_BMM_SYSPARAM_REQ:
			//�ն˿�����
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
		//0x1201. BMM������������
		case eSERVERCMD_BMM_SYSPARAM_SET:
			//�ն˿�����
			if(eLANFRAMETYPE_NODE_NET == frametype)
			{
				ret = ServerTerminalAppCmdProc_NoGroup(ptcmd, eTERMINALCOM_BMM_SYSPARAM_SET, 13);
				if(ret != eRET_NONE)
				{
					errack = eSERVERACK_ERR_BMM_SYSPARAM_SET;
					back = TRUE;
				}
			}
			//�㲥���������
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
		//0x1202. BMM״̬1��ѯ����
		case eSERVERCMD_BMM_STATE1_REQ:
			//�ն˿�����
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
		//0x1202. BMM״̬2��ѯ
		case eSERVERCMD_BMM_STATE2_REQ:
			//�ն˿�����
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
			
		//��������Ӫ�̲�ѯ
		case eSERVERCMD_REQ_WSMCARRIER:
			ret = ServerWSMCarrierReqProc(ptcmd, frametype);
			break;
		//��������Ӫ������
		case eSERVERCMD_SET_WSMCARRIER:
			ret = ServerWSMCarrierSetProc(ptcmd, frametype);
			break;
		//��������ѹ������ѯ
		case eSERVERCMD_REQ_WSMVOL:
			ret = ServerWSMVolReqProc(ptcmd, frametype);
			break;
		//����������������ѯ
		case eSERVERCMD_REQ_WSMCUR:
			ret = ServerWSMCurReqProc(ptcmd, frametype);
			break;
		//�����������������ѯ
		case eSERVERCMD_REQ_WSMSWITCHIN:
			ret = ServerWSMSwitchInReqProc(ptcmd, frametype);
			break;
		//�����������������ѯ
		case eSERVERCMD_REQ_WSMSWITCHOUT:
			ret = ServerWSMSwitchOutReqProc(ptcmd, frametype);
			break;
		//�������������������
		case eSERVERCMD_SET_WSMSWITCHOUT:
			ret = ServerWSMSwitchOutSetProc(ptcmd, frametype);
			break;
		//����������ʱ���ѯ
		case eSERVERCMD_REQ_WSMCOMPILETIME:
			ret = ServerWSMCompileTimeReqProc(ptcmd, frametype);
			break;
		//������SD�����ݲ�ѯ
		case eSERVERCMD_REQ_WSMSDDATA:
			ret = ServerWSMSDDataReqProc(ptcmd, frametype);
			break;
		//���������������Բ�ѯ
		case eSERVERCMD_REQ_WSMHEARTATTR:
			ret = ServerWSMHeartAttrReqProc(ptcmd, frametype);
			break;
		//��������������������
		case eSERVERCMD_SET_WSMHEARTATTR:
			ret = ServerWSMHeartAttrSetProc(ptcmd, frametype);
			break;
		//��������ʱ
		case eSERVERCMD_SET_SYSTIME:
			ret = ServerWSMSysTimeSetProc(ptcmd, frametype);
			break;
		//������ʱ�ο��Ʋ�ѯ
		case eSERVERCMD_REQ_TIMECONTROL:
			ret = ServerWSMTimeConReqProc(ptcmd, frametype);
			break;
		//������ʱ�ο�������
		case eSERVERCMD_SET_TIMECONTROL:
			ret = ServerWSMTimeConSetProc(ptcmd, frametype);
			break;
		//��������ѯ���Բ�ѯ
		case eSERVERCMD_REQ_WSMPOLLATTR:
			ret = ServerWSMPollAttrReqProc(ptcmd, frametype);
			break;
		//��������ѯ��������
		case eSERVERCMD_SET_WSMPOLLATTR:
			ret = ServerWSMPollAttrSetProc(ptcmd, frametype);
			break;
		//�������ն˷����ѯ
		case eSERVERCMD_REQ_WSMDEVGROUP:
			ret = ServerWSMDevGroupReqProc(ptcmd, frametype);
			break;
		//�������ն˷�������
		case eSERVERCMD_SET_WSMDEVGROUP:
			ret = ServerWSMDevGroupSetProc(ptcmd, frametype);
			break;
		//��������������ѹ��Ų�ѯ
		case eSERVERCMD_REQ_WSMVOLNUM:
			ret = ServerWSMHeartVolNumReqProc(ptcmd, frametype);
			break;
		//��������������ѹ�������
		case eSERVERCMD_SET_WSMVOLNUM:
			ret = ServerWSMHeartVolNumSetProc(ptcmd, frametype);
			break;
		//������������������Ų�ѯ
		case eSERVERCMD_REQ_WSMCURNUM:
			ret = ServerWSMHeartCurNumReqProc(ptcmd, frametype);
			break;
		//�����������������������
		case eSERVERCMD_SET_WSMCURNUM:
			ret = ServerWSMHeartCurNumSetProc(ptcmd, frametype);
			break;
		//������������������������Ų�ѯ
		case eSERVERCMD_REQ_WSMSWITCHINNUM:
			ret = ServerWSMHeartSwitchInNumReqProc(ptcmd, frametype);
			break;
		//�����������������������������
		case eSERVERCMD_SET_WSMSWITCHINNUM:
			ret = ServerWSMHeartSwitchInNumSetProc(ptcmd, frametype);
			break;
		//�����������������������Ų�ѯ
		case eSERVERCMD_REQ_WSMSWITCHOUTNUM:
			ret = ServerWSMHeartSwitchOutNumReqProc(ptcmd, frametype);
			break;
		//����������������������������
		case eSERVERCMD_SET_WSMSWITCHOUTNUM:
			ret = ServerWSMHeartSwitchOutNumSetProc(ptcmd, frametype);
			break;

		default:
			errack = eSERVERACK_ERR_CMD;
			back = TRUE;
			ret = eRET_CMD;
			break;
	}

	//Ӧ��
	if(back)
	{
		//��С��ת��
		errack = REVERSE16(errack);
		ret = ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
			        eSERVERCMD_ACK, (uint_8*)&errack, 2, *ptcmd->pret);
	}
	
	return ret;
}

//============================================================================
//������: ServerBroadSetProc
//��  ��: �㲥���ò�������
//��  ��: ��
//��  ��: ��
//============================================================================
static void ServerBroadSetProc(void)
{
	TServerCOMFrame tserverframe;
	uint_8 terminalappcmd = eTERMINALAPPCMD_MAX;
	static uint_8 s_errtime = 0;
	uint_8 groupsize;
	
	//�㲥���ò���
	if(s_broadsetcount > 0)
	{
		//��ַ����
		ServerStructAddr(&tserverframe, s_broadsetbuf, s_broadsetlen - SERVERCOMFRAME_MINLEN);

		switch(REVERSE16(*tserverframe.pcmd))
		{
			//���ص�״̬����
			case eSERVERCMD_SET_BULK:
				terminalappcmd = eTERMINALAPPCMD_SET_BULK;
				groupsize = 2;
				break;
				
			//�ƹ�������
			case eSERVERCMD_SET_POWER:
				terminalappcmd = eTERMINALAPPCMD_SET_POWER;
				groupsize = 2;
				break;
				
			//0x1201. BMM������������
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
				s_bPollStart = TRUE;     //��ʼ��ѯ������㲥���ƣ�״̬��ѯ����
				s_errtime = 0;
			}
			else
			{
				if(++s_errtime >= 3)
				{
					s_broadsetcount--;
					s_bPollStart = TRUE;     //��ʼ��ѯ������㲥���ƣ�״̬��ѯ����
					s_errtime = 0;
				}
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ServerBroadOptProc]: ServerTerminalAppCmdProc failed!\r\n");
			}
		}
	}
}


//============================================================================
//������: ComInit
//��  ��: ͨ�Ų������ģ���ʼ��
//��  ��: const TComInfo* const ptcominfo��ͨ�Ų���Ϣ
//��  ��: ��
//============================================================================
void ComServerInit(TComInfo* ptcominfo)
{
	s_ptcomserverinfo = ptcominfo;
}


//============================================================================
//������: ComServerProc
//��  ��: ͨ�������
//��  ��: uint_8 comtype������server����terminal��ͨ��֡
//               const uint_8* const pframe��ͨ��֡�׵�ַ
//               uint_16 framelen��pframe��Ч����
//��  ��: ������
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


	//��ַ����
	ServerStructAddr(&tserverframe, (uint_8*)pframe, framelen - SERVERCOMFRAME_MINLEN);

	//����У��
	if(framelen < SERVERCOMFRAME_MINLEN)
	{
		errack = eSERVERACK_ERR_LEN;
		ret = eRET_LEN;
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]:ERR, framelen error!\r\n");
		goto ComServerProc_ERRRET;
	}

	//�����ж�
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
	
	//У���,������֡ͷ֡β��У����
//	if(MakeCheckInfo((uint_8*)tserverframe.pret, framelen - 3) != *tserverframe.pchk)
//	{
//		errack = eSERVERACK_ERR_CHK;
//		ret = eRET_CHK;
//		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]:ERR, chk error!\r\n");
//		goto ComServerProc_ERRRET;
//	}

	//��������ַƥ��
	i = LanFrameType(REVERSE16(*tserverframe.pnetid), REVERSE16(*tserverframe.pnodeid), *tserverframe.pgroupid);
	//��������֡������
	if(i >= eLANFRAMETYPE_OTHEER_NET)
	{
		errack = eSERVERACK_ERR_ADDR;
		ret = eRET_ADDR;
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]: ERR, other net frame!\r\n");
		goto ComServerProc_ERRRET;
	}
	//�ն˿�����֡,��·��û�д˵�ַ������
	else if(eLANFRAMETYPE_NODE_NET == i && NodeAddrCheck(REVERSE16(*tserverframe.pnodeid)) == NULL)
	{
		errack = eSERVERACK_ERR_ADDR;
		ret = eRET_ADDR;
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerProc]: ERR, the dev is not exite!\r\n");
		goto ComServerProc_ERRRET;
	}

	//������֡����
	return ServerCmdProc(&tserverframe, i);

ComServerProc_ERRRET:
	//��С��ת��
	errack = REVERSE16(errack);
	//����Ӧ��
	return ComServerAck(s_ptcomserverinfo->tcomlaninfo.netid, s_ptcomserverinfo->tcomlaninfo.route, 
					s_ptcomserverinfo->tcomlaninfo.groupid, s_ptcomserverinfo->tcomlaninfo.nodeid, 
					eSERVERCMD_ACK, (uint_8*)&errack, 2, *tserverframe.pret);
}

//============================================================================
//������: ComServerAck
//��  ��: Server ACKӦ��
//��  ��: uint_16 acktype��Ӧ������
//               uint_8* packbuf��Ӧ������
//		     uint_16 ackbuflen��Ӧ�����ݳ���
//��  ��: ��
//��  ע:�������������Ӧ�𣬲�������øýӿڣ������ݿ���ʱ�䳤
//============================================================================
uint_8 ComServerAck(uint_16 netid, uint_8 route, uint_8 groupid, uint_16 nodeid, uint_16 acktype, uint_8* packbuf, uint_16 ackbuflen, uint_32 serverret)
{
    TBufNode* pnode = NULL;
	TServerCOMFrame tackframe;
	uint_16 len = ackbuflen + SERVERCOMFRAME_MINLEN;
	uint_8 ret = eRET_PARAM;
	TNodeInfo* pdevnode = NULL;
	
	//����У��
	if(NULL == s_ptcomserverinfo->pserversenddatalist || NULL == s_ptcomserverinfo->pserversendfreelist)
		return ret;

	listnode_get(s_ptcomserverinfo->pserversendfreelist,&pnode);
    if(NULL != pnode && pnode->size >= len)
    {
		//��ַ����
		ServerStructAddr(&tackframe, pnode->pbuf, ackbuflen);

		//��֡
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
		
    	//�黹datalist
    	pnode->len = len;
		listnode_put(s_ptcomserverinfo->pserversenddatalist, pnode);

		ret = eRET_NONE;
	}

	//��������Ϣ����
	if(netid == s_ptcomserverinfo->tcomlaninfo.netid && nodeid == s_ptcomserverinfo->tcomlaninfo.nodeid &&
		(acktype == eSERVERCMD_ACK_ADDR || acktype == eSERVERCMD_ACK_RFCH || acktype == eSERVERCMD_ACK_ROUTE || acktype == eSERVERCMD_ACK_CSMACA)
		&& s_bmhcnodeidchange)
	{
		if(s_ptcomserverinfo->pcominfochcallback != NULL)
			s_ptcomserverinfo->pcominfochcallback(&s_ptcomserverinfo->tcomlaninfo, sizeof(TComLanInfo), 1);
		s_bmhcnodeidchange = FALSE;
	}

	//�ն��豸״̬���óɹ������´洢״̬
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
//������: ComServerPoll_App
//��  ��: ������Ӧ����ѯ����
//��  ��: ��
//��  ��: ��
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

	//������ѯ������£��㲥����
	//����ѯ�����еȴ�Ӧ���ͬʱ���������
	if(s_bPollStart == FALSE && 
		(bpollstart == FALSE || (s_bpollack && is_timeout(polltimeoutticks.TICKS[0], 50))))
		ServerBroadSetProc();
			
	//��������ѯ
	//�㲥���ú��ѯ����ʱ1s
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

	//��ѯÿ���նˣ���ȴ�Ӧ���ʱ
	if(bpollstart && (s_bpollack == FALSE || is_timeout(polltimeoutticks.TICKS[0], pollnode->tnodeinfosave.polltimeout)))
	{
		//Ӧ��ʧ�ܣ����γ�ʱ��������ѯ��һ�ն�
		if(s_bpollack && pollnode != NULL && pollnode->tnodeinfosave.pollcount1 > 0)
		{
			//��ʱ����
			if(pollnode->failcount++ >= pollnode->tnodeinfosave.pollcount1)
			{
				memset(pollnode->data, 0xFF, APP_DATALEN);
				polllist = polllist->next;         //��һ�ն�
				//����ȫ���㣬��֤�豸���������£������ڲ�ѯʱ�����
				pollnode->failcount -= pollnode->tnodeinfosave.pollcount2;          
			}
		}
		//�ɹ�Ӧ��
		else if(s_bpollack == FALSE)
			polllist = polllist->next;             //��һ�ն�
		
		//��ѯ����
		if(polllist == s_ptcomserverinfo->pterminalnodedatalist)
			bpollstart = FALSE;
		else
		{
			//��֡��ѯ��״̬
			pollnode = list_entry(polllist, TNodeInfo, list);

			//��ֹ�ڵ���ʴ���,�ڵ㱻ɾ��
			if(NULL == pollnode || NULL == NodeAddrCheck(pollnode->tnodeinfosave.nodeid))
			{
				polllist = NULL;
				pollnode = NULL;
				s_bpollack = FALSE;
				bpollstart = FALSE;
			}
			//�ýڵ�������ѯ
			else if(pollnode->tnodeinfosave.pollcount1 > 0)
			{
				//��ַ����
				TerminalAppStructAddr(&terminalapp, buf);
				*terminalapp.plen = 2;
				//��������ѯ����
				if((bpolltime % 10) == 0)
				{
					*terminalapp.pcmd = eTERMINALCOM_BMM_STATE2_REQ;
				}
				else
				{
					*terminalapp.pcmd = eTERMINALCOM_BMM_STATE1_REQ;
				}
				
				//����ret=0xFFFFFFFF�ж��Ƿ�Ϊ��������ѯ������Ƿ�������
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
//������: ComServerHeart_App
//��  ��: ��ĿӦ������������
//��  ��: ��
//��  ��: ��
//============================================================================
static void ComServerHeart_App()
{
	static MQX_TICK_STRUCT mqx_tick_heartticks;
//	static uint_32 heartticks = 0;
	static MQX_TICK_STRUCT allheartticks;    //�����ϱ�ʱ��
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

	//����������
	if(s_ptcomserverinfo->tcomgprsinfo.hearttime > 0 && (s_bLinkStart || 
//	    is_timeout(heartticks, sys_tick, s_ptcomserverinfo->tcomgprsinfo.hearttime)))
		is_timeout(mqx_tick_heartticks.TICKS[0], s_ptcomserverinfo->tcomgprsinfo.hearttime)))
	{
		_time_get_ticks(&mqx_tick_heartticks);
//		heartticks = sys_tick;
		s_bLinkStart = FALSE;

		//�˳�͸��ģʽ
		/*if(gprs_set_iptrans(FALSE) > 0)
		{
			DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerHeart_App]: quit iptrans failed!\r\n");
			return;
		}*/
		
		

		if(FALSE == gprs_isbroken())
		{		
			/*//��ȡCSQ
			if(gprs_get_csq(buf) != eGPRSRET_NONE)
			{
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerHeart_App]: GPRSGetLinkCSQ failed!\r\n");
				buf[0] = 0xFF;
			}

			//���ŷ���
			if(s_ptcomserverinfo->tcomgsminfo.bgsmalarm && g_gsmsendlen > 0)
			{
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerHeart_App]: gsm send!\r\n");
				for(i = 0; i < s_ptcomserverinfo->tcomgsminfo.phonenum; i++)
				{
					gsm_send(s_ptcomserverinfo->tcomgsminfo.phone[i], g_gsmsendbuf, g_gsmsendlen);
				}

				g_gsmsendlen = 0;
			}

			//����͸��ģʽ
			if(gprs_set_iptrans(TRUE) > 0)
			{
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComServerHeart_App]: enter iptrans failed!\r\n");
				return;
			}*/
			
			//��ǰϵͳʱ��
			//get_daytime(&buf[1], &buf[2], &buf[3]);
			buflen = 4;

			/*//�ϱ��豸��Ϣ
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
					//ι��
					FeedTheWatchDog();

					pnode = list_entry(heartlist, TNodeInfo, list);

					//��ֹ�ڵ���ʴ���,��ɾ��
					if(NULL == pnode || NULL == NodeAddrCheck(pnode->tnodeinfosave.nodeid))
					{
						heartlist = NULL;
						break;
					}

					//��ַ����������ֹ������
					if(lastnodeid != 0 && pnode != NULL && pnode->tnodeinfosave.nodeid != (lastnodeid + 1))
						break;

					DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "[ComServerHeart_App]: lastnodeid=");
					DEBUGOUT_DEC(DEBUGOUT_LEVEL_INFO, lastnodeid);
					DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, ", pnode->tnodeinfosave.nodeid=");
					DEBUGOUT_DEC(DEBUGOUT_LEVEL_INFO, pnode->tnodeinfosave.nodeid);
					DEBUGOUT_STR(DEBUGOUT_LEVEL_INFO, "\r\n");
					
					//��ʼ��ַ
					if(lastnodeid == 0)
					{
						//nodeid
						*(uint_16*)(&buf[buflen]) = REVERSE16(pnode->tnodeinfosave.nodeid);
						buflen += 2;
						pnodenum = (uint_16*)&buf[buflen];
						*pnodenum = 0;
						buflen += 2;
					}

					//����
					memcpy(&buf[buflen], pnode->data, APP_DATALEN);
					buflen += APP_DATALEN;
					
					lastnodeid = pnode->tnodeinfosave.nodeid;
					*pnodenum += 1;
					heartlist = heartlist->next;
					//�豸����
					if(heartlist == s_ptcomserverinfo->pterminalnodedatalist)
					{
						heartlist = NULL;
						if(s_ptcomserverinfo->tcomgprsinfo.devinfotype >= eDEVINFOTYPE_CHANGE_SEND &&
							is_timeout(allheartticks.TICKS[0], s_ptcomserverinfo->tcomgprsinfo.devinfotype * 60000))
						{
							_time_get_ticks(&ticks);
							allheartticks = ticks;   //���������ϴ�ʱ��
						}
					}
					
					//buf��
					if(s_ptcomserverinfo->tcomgprsinfo.heartmax - buflen <= (s_ptcomserverinfo->tcomgprsinfo.heartmax >> 3))
						br0eak;
				}	
			}

			//��С��ת��
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
//������: ComServerHeartProc
//��  ��: ͨ�Ų����������
//��  ��: ��
//��  ��: ��
//============================================================================
void ComServerHeartProc()
{
	//�ն���ѯ
	//ComServerPoll_App();
	
	//������
	ComServerHeart_App();
}

