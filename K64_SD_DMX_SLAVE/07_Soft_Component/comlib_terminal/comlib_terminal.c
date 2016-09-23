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
//#include "buflist.h"
//#include "comlib.h"
//#include "comlib_server.h"
#include "comlib_terminal.h"
//#include "isr.h"
#include "includes.h"

static TComInfo* s_ptcomterminalinfo;
extern bool s_bpollack;

//-------------------------------------------------------------------------
//������: LanFrameType                                                        
//��  ��: �жϾ�����֡����           
//��  ��: uint_16 netid��netid
//               uint_16 nodeid��nodeid
//��  ��: ��                                              
//˵  ��: TRUE�����Լ���֡��FALSE�������Լ���֡
//-------------------------------------------------------------------------
uint_8 LanFrameType(uint_16 netid, uint_16 nodeid, uint_8 groupid)
{
	uint_8 ret = eLANFRAMETYPE_MAX;
	
	//ȫ���㲥֡
	if(netid == BROADCAST_NETID && nodeid == BROADCAST_NODEID)
		ret = eLANFRAMETYPE_ALL;
	//����
	else if(netid == s_ptcomterminalinfo->tcomlaninfo.netid)
	{
		//���ڵ�֡
		if(nodeid == s_ptcomterminalinfo->tcomlaninfo.nodeid)
			ret = eLANFRAMETYPE_ME;
		//�ڵ��ַΪ�㲥��ַ
		else if(nodeid == BROADCAST_NODEID)
		{
			//���ڹ㲥֡
			if(groupid == BROADCAST_GROUPID)
				ret = eLANFRAMETYPE_NET;
			//����������㲥֡
			else if(groupid == BROADCAST_ODD_GROUPID)
				ret = eLANFRAMETYPE_ODDGROUP;
			//����ż����㲥֡
			else if(groupid == BROADCAST_EVEN_GROUPID)
				ret = eLANFRAMETYPE_EVENGROUP;
			//�������ڹ㲥֡
			else
				ret = eLANFRAMETYPE_GROUP;
		}
		//�ڵ��ַΪ���ַ
		else if(nodeid == BROADCAST_ODD_NODEID)
		{
			//�������ַ�㲥֡
			if(groupid == BROADCAST_GROUPID)
				ret = eLANFRAMETYPE_ODD_NET;
			//�������������ַ�㲥֡
			else if(groupid == BROADCAST_ODD_GROUPID)
				ret = eLANFRAMETYPE_ODD_ODD;
			//����ż�������ַ�㲥֡
			else if(groupid == BROADCAST_EVEN_GROUPID)
				ret = eLANFRAMETYPE_ODD_EVEN;
			//�����������ַ�㲥֡
			else
				ret = eLANFRAMETYPE_ODD_GROUP;
		}
		//�ڵ��ַΪż��ַ
		else if(nodeid == BROADCAST_EVEN_NODEID)
		{
			//����ż��ַ�㲥֡
			if(groupid == BROADCAST_GROUPID)
				ret = eLANFRAMETYPE_EVEN_NET;
			//����������ż��ַ�㲥֡
			else if(groupid == BROADCAST_ODD_GROUPID)
				ret = eLANFRAMETYPE_EVEN_ODD;
			//����ż����ż��ַ�㲥֡
			else if(groupid == BROADCAST_EVEN_GROUPID)
				ret = eLANFRAMETYPE_EVEN_EVEN;
			//��������ż��ַ�㲥֡
			else
				ret = eLANFRAMETYPE_EVEN_GROUP;
		}
		//���������ڵ�֡
		else
			ret = eLANFRAMETYPE_NODE_NET;
	}
	//��������
	else 
		ret = eLANFRAMETYPE_OTHEER_NET;
	
	return ret;
}


//-------------------------------------------------------------------------
//������: MakeCheckInfo                                                        
//��  ��: ����֡У����Ϣ                                
//��  ��: uint_8* pbuf��У�������׵�ַ
//		     uint_16 buflen��У�����ݳ���
//��  ��: У����                                                
//˵  ��: (1)�����ۼӺ�У��
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
//������: TerminalStructAddr
//��  ��: �ṹ��ַ����
//��  ��: ��
//��  ��: ��
//============================================================================
static inline void TerminalStructAddr(TTerminalCOMFrame* ptframe, uint_8* pbuf, uint_8 cmdlen)
{
	//��ַ����
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
//������: TerminalStructAddr
//��  ��: �ṹ��ַ����
//��  ��: ��
//��  ��: ��
//============================================================================
void TerminalAppStructAddr(TTerminalAppHead* ptapp, uint_8* pbuf)
{
	//��ַ����
	ptapp->plen = pbuf;
	ptapp->pcmd = ptapp->plen + 1;
	ptapp->pbuf = ptapp->pcmd + 1;
}

//============================================================================
//������: MakeTerminalDataNode
//��  ��: �ն˿�����֡
//��  ��: uint_16 netid, uint_32 nodeid����������ַ
//               uint_8 cmdtypet����������
//               uint_8* pcmd, uint_8 cmdlen����������
//��  ��: TBufNode*������buf
//============================================================================
static inline TBufNode* MakeTerminalDataNode(uint_16 netid, uint_8 route, uint_8 groupid, 
	uint_16 nodeid, uint_8 cmdtype, uint_8* pcmd, uint_8 cmdlen, uint_32 serverret)
{
	TBufNode* pnode = NULL;
	TTerminalCOMFrame tlanframe;
	uint_8 len = TERMINALCOMFRAME_MINLEN + cmdlen;
	
	//����У��
	if(NULL == s_ptcomterminalinfo || NULL == s_ptcomterminalinfo->pterminalsendfreelist)
		return NULL;

	listnode_get(s_ptcomterminalinfo->pterminalsendfreelist, &pnode);
	if(NULL != pnode && pnode->size >= len)
	{
		//����ram�ռ�λ��
		//��ѯ�������������
		TerminalStructAddr(&tlanframe, pnode->pbuf, cmdlen);

		//��֡
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
//������: ComTerminalSysAckProc
//��  ��: �ն�Ӧ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalSysAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_8 ret = eRET_NONE;
	uint_16 serverack = eSERVERACK_ERR_SYSMAX;

	//ack��Ч����
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

	//server��֡Ӧ��
	if(serverack < eSERVERACK_ERR_SYSMAX)
	{
		//��С��ת��
		serverack = REVERSE16(serverack);
		ret = ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			REVERSE16(*ptcmd->pnodeid), eSERVERCMD_ACK, (uint_8*)&serverack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//������: ComTerminalSysAckProc
//��  ��: �ն�Ӧ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalAddrAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_ADDREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //Ĭ�ϳ���Ӧ��
	uint_8 acklen = 2;
	
	//��ȷӦ��
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 6) 
	{
		acktype = eSERVERCMD_ACK_ADDR;
		packbuf = ptcmd->pbuf;
		acklen = 6;
	}
	
	//server��֡Ӧ��
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//������: ComTerminalRFChAckProc
//��  ��: �ն�Ӧ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalRFChAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_CHREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //Ĭ�ϳ���Ӧ��
	uint_8 acklen = 2;
	
	//��ȷӦ��
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 1) 
	{
		acktype = eSERVERCMD_ACK_RFCH;
		packbuf = ptcmd->pbuf;
		acklen = 1;
	}
	
	//server��֡Ӧ��
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//������: ComTerminalRouteAckProc
//��  ��: �ն�Ӧ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalRouteAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_ROUTREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //Ĭ�ϳ���Ӧ��
	uint_8 acklen = 2;
	
	//��ȷӦ��
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 1) 
	{
		acktype = eSERVERCMD_ACK_ROUTE;
		packbuf = ptcmd->pbuf;
		acklen = 1;
	}
	
	//server��֡Ӧ��
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//������: ComTerminalTranAckProc
//��  ��:���ת������Ӧ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalTranAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_TRANUMREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //Ĭ�ϳ���Ӧ��
	uint_8 acklen = 2;
	
	//��ȷӦ��
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 1) 
	{
		acktype = eSERVERCMD_ACK_TRANUM;
		packbuf = ptcmd->pbuf;
		acklen = 1;
	}
	
	//server��֡Ӧ��
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//������: ComTerminalCamaAckProc
//��  ��: �ն�Ӧ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalCamaAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_CSMAREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //Ĭ�ϳ���Ӧ��
	uint_8 acklen = 2;
	
	//��ȷӦ��
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 1) 
	{
		acktype = eSERVERCMD_ACK_CSMACA;
		packbuf = ptcmd->pbuf;
		acklen = 1;
	}
	
	//server��֡Ӧ��
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//������: ComTerminalMACIPAckProc
//��  ��: �ն�Ӧ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalMACIPAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_CSMAREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //Ĭ�ϳ���Ӧ��
	uint_8 acklen = 2;
		
	//��ȷӦ��
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 14) 
	{
		acktype = eSERVERCMD_ACK_MACIP;
		packbuf = ptcmd->pbuf;
		acklen = 14;
	}
	
	//server��֡Ӧ��
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//������: ComTerminalMacFlagAckProc
//��  ��: �ն�Ӧ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalMacFlagAckProc(const TTerminalCOMFrame* const ptcmd)
{
	uint_16 acktype = eSERVERCMD_ACK;
	uint_16 serverack = REVERSE16(eSERVERACK_ERR_MACFLAGREQ);
	uint_8* packbuf = (uint_8*)&serverack;         //Ĭ�ϳ���Ӧ��
	uint_8 acklen = 2;
	
	//��ȷӦ��
	if(*ptcmd->plen == TERMINALCOMFRAME_MINLEN + 9) 
	{
		acktype = eSERVERCMD_ACK_MACFLAG;
		packbuf = ptcmd->pbuf;
		acklen = 9;
	}
	
	//server��֡Ӧ��
	return ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			            REVERSE16(*ptcmd->pnodeid), acktype, packbuf, acklen, *ptcmd->pret);
}

//============================================================================
//������: ComTerminalAPPPollProc
//��  ��: �ն���ѯӦ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static void ComTerminalAPPPollProc(const TTerminalCOMFrame* const ptcmd, const TTerminalAppHead* const ptapp)
{
	uint_8 tmp[30];
	TNodeInfo* pnode = NodeAddrCheck(REVERSE16(*ptcmd->pnodeid));

	//��ַ���
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
	//״̬����
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
	//���ȴ���
	else
	{
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComTerminalAPPPollProc]: data len err!\r\n");
	}
	
	return;
}

//============================================================================
//������: ComTerminalSysCmdProc
//��  ��: ϵͳ�����
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalSysCmdProc(const TTerminalCOMFrame* const ptcmd)
{
    uint_8 ret;
	
	switch(*ptcmd->pcmd)
	{
		//Ӧ��
		case eTERMINALCOM_ACK:
			ret = ComTerminalSysAckProc(ptcmd);
			break;
			
		//��ַӦ��
		case eTERMINALCOM_ACK_ADDR:
			ret = ComTerminalAddrAckProc(ptcmd);
			break;

		//ͨ��Ӧ��
		case eTERMINALCOM_ACK_RFCH:
			ret = ComTerminalRFChAckProc(ptcmd);
			break;
		
		//·��Ӧ��
		case eTERMINALCOM_ACK_ROUTE:
			ret = ComTerminalRouteAckProc(ptcmd);
			break;
			
		//���ת������Ӧ��
		case eTERMINALCOM_ACK_TRANUM:
			ret = ComTerminalTranAckProc(ptcmd);
			break;
		
		//CSMACAӦ��
		case eTERMINALCOM_ACK_CSMACA:
			ret = ComTerminalCamaAckProc(ptcmd);
			break;
			
		//MACIP��ַӦ��
		case eTERMINALCOM_ACK_MACIP:
			ret = ComTerminalMACIPAckProc(ptcmd);
			break;
			
		//�㲥��ѯZIG_MAC��ַӦ���־Ӧ��
		case eTERMINALCOM_ACK_MACFLAG:
			ret = ComTerminalMacFlagAckProc(ptcmd);
			break;
			
		default:
			break;
	}

	return ret;
}

//============================================================================
//������: ComTerminalAppAckProc
//��  ��: �ն�Ӧ�������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
//============================================================================
static uint_8 ComTerminalAppAckProc(const TTerminalCOMFrame* const ptcmd, const TTerminalAppHead* const ptapp)
{
	uint_8 ret = eRET_NONE;
	uint_16 serverack = eSERVERACK_ERR_APPMAX;

	//��Ч����
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

	//server��֡Ӧ��
	if(serverack < eSERVERACK_ERR_APPMAX)
	{
		//��С��ת��
		serverack = REVERSE16(serverack);
		ret = ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			     REVERSE16(*ptcmd->pnodeid), eSERVERCMD_ACK, (uint_8*)&serverack, 2, *ptcmd->pret);
	}

	return ret;
}

//============================================================================
//������: ComTerminalAppCmdProc
//��  ��: �û������
//��  ��: const TTerminalCOMFrame* const ptcmd����������
//��  ��: ��
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
		//Ӧ��Ӧ��
		case eTERMINALAPPCMD_ACK:
			//��������ѯӦ��֡������Ӧ�������
			if(s_bpollack && *ptcmd->pret == 0xFFFFFFFF)
				s_bpollack = FALSE;
			else
				ret = ComTerminalAppAckProc(ptcmd, &terminalapp);
			break;
			
		//�ƿ���״̬Ӧ��
		case eTERMINALAPPCMD_ACK_BULK:
			acktype = eSERVERCMD_ACK_BULK;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			//ComTerminalFlashBulkState(ptcmd, &terminalapp);  //����״̬
			break;

		//�ƹ���Ӧ��
		case eTERMINALAPPCMD_ACK_POWER:
			acktype = eSERVERCMD_ACK_POWER;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			//ComTerminalFlashBulkState(ptcmd, &terminalapp);   //����״̬
			break;
			
		//��һǶ��ʽģ�����Ӧ��
		case eTERMINALAPPCMD_ACK_PARAM:
			acktype = eSERVERCMD_ACK_PARAM;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			break;
			
		//�汾��ϢӦ��
		case eTERMINALAPPCMD_ACK_VERSION:
			acktype = eSERVERCMD_ACK_VERSION;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			break;
			
		//��״̬Ӧ��
		case eTERMINALAPPCMD_ACK_BULKSTATE:
			//��������ѯӦ��֡������Ӧ�������
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
			//ComTerminalIndicationProc(ptcmd, &terminalapp);   //����״̬
			break;
			
		//�����ϴ���־Ӧ��
		case eTERMINALAPPCMD_ACK_INDIFLAG:
			acktype = eSERVERCMD_ACK_INDIFLAG;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			break;
			
		//��״̬�����ϴ���
		case eTERMINALAPPCMD_INDICATION:
			//ComTerminalIndicationProc(ptcmd, &terminalapp);
			break;

		//ADֵ��ѯ
		case eTERMINALAPPCMD_ACK_ADVALUE:
			acktype = eSERVERCMD_ACK_ADVALUE;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;
			break;
			
		//Zigbeeģ�鿴�Ź���λʱ��Ӧ��
		case eTERMINALAPPCMD_ACK_WIDTIME: 
		    acktype = eSERVERCMD_ACK_WIDTIME;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;   
			break;  
			
		//�ƾ�������ʽӦ��֡
		case eTERMINALAPPCMD_ACK_STARTUPMODE: 
			acktype = eSERVERCMD_ACK_STARTUPMODE;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
			
		//�ƾ߿�������Ӧ��
		case eTERMINALAPPCMD_ACK_BULKSET: 
			acktype = eSERVERCMD_ACK_BULKSETACK;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;
			
		//���صƵ�����ֵӦ��
		case eTERMINALAPPCMD_ACK_BULKONCUR: 
			acktype = eSERVERCMD_ACK_BULKONCURACK;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xAE. ��ѹ����Ӧ��
		case eTERMINALAPPCMD_ACK_VOLTAGE: 
			acktype = eSERVERCMD_ACK_VOLTAGE;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xAF. ��������Ӧ��
		case eTERMINALAPPCMD_ACK_CURRENT: 
			acktype = eSERVERCMD_ACK_CURRENT;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xB0. �¶Ȳ���Ӧ��
		case eTERMINALAPPCMD_ACK_TEMPERATURE: 
			acktype = eSERVERCMD_ACK_TEMPERATURE;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xB1. V2��ѯ����Ӧ��
		case eTERMINALAPPCMD_ACK_V2POLL:
			//��������ѯӦ��֡������Ӧ�������
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
			//ComTerminalV2PollProc(ptcmd, &terminalapp);   //����״̬
			break;
			
		//0XB2.Ӳ����ַӦ��
		case eTERMINALAPPCMD_ACK_HDADDR: 
		    acktype = eSERVERCMD_ACK_HDADDR;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;   
			break;

		//0xD8. BMM����Ӧ��֡
		case eTERMINALAPPCMD_ACK_BMM_SYSPARAM_REQ: 
			acktype = eSERVERCMD_ACK_BMM_SYSPARAM_REQ;
			packbuf = terminalapp.pbuf;
			ackbuflen = *terminalapp.plen - 2;	 
			break;	
		//0xD9. BMM״̬1Ӧ��֡
		case eTERMINALAPPCMD_ACK_BMM_STATE1_REQ:
			//��������ѯӦ��֡������Ӧ�������
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
			ComTerminalAPPPollProc(ptcmd, &terminalapp);   //����״̬
			break;
		//0xDA. BMM״̬2Ӧ��֡
		case eTERMINALAPPCMD_ACK_BMM_STATE2_REQ:
			//��������ѯӦ��֡������Ӧ�������
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
			ComTerminalAPPPollProc(ptcmd, &terminalapp);   //����״̬
			break;
			
		default:
			ret = eRET_PARAM;
			break;
	}

	//server��֡Ӧ��
	if(acktype < eSERVERCMD_ACK_APPMAX)
	{
		ret = ComServerAck(s_ptcomterminalinfo->tcomlaninfo.netid, *ptcmd->proute, *ptcmd->pgroupid, 
			               REVERSE16(*ptcmd->pnodeid), acktype, packbuf, ackbuflen, *ptcmd->pret);
	}

	return ret;
}



//============================================================================
//������: ComInit
//��  ��: ͨ�Ų������ģ���ʼ��
//��  ��: const TComInfo* const ptcominfo��ͨ�Ų���Ϣ
//��  ��: ��
//============================================================================
void ComTerminalInit(TComInfo* ptcominfo)
{
	s_ptcomterminalinfo = ptcominfo;
}

//============================================================================
//������: ComTerminalSendCmd
//��  ��: �ն˿�������������
//��  ��: uint_8 cmd, ������
//               uint_8* cmdbuf������������
//               uint_8 cmdlen��cmdbuf��Ч����
//��  ��: uint8��������
//============================================================================
uint_8 ComTerminalSendCmd(uint_16 netid, uint_8 route, uint_8 groupid,  
	uint_16 nodeid, uint_8 cmdtype, uint_8* pcmd, uint_8 cmdlen, uint_32 serverret)
{
	uint_8 ret = eRET_PARAM;
	TBufNode* pnode = MakeTerminalDataNode(netid, route, groupid, nodeid, cmdtype, pcmd, cmdlen, serverret);
	if(pnode != NULL)
	{
		//����
		listnode_put(s_ptcomterminalinfo->pterminalsenddatalist, pnode);
		ret = eRET_NONE;
	}
	return ret;	
}

//============================================================================
//������: ComTerminalGetLanInfo
//��  ��: ��ȡ��������ַ
//��  ��: uint_16* pnetid, uint_32* pnodeid�����ؾ�������ַ
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
//============================================================================
static TTerminalCOMFrame* ComTerminalGetLanInfoPre(uint_8 reqtype, uint_8 acktype, uint_32 timeout)
{
	bool bsucc = FALSE;
	MQX_TICK_STRUCT ticks;
	TTerminalCOMFrame terminalframe;
	//��ѯ֡
	TBufNode* pnode = MakeTerminalDataNode(0xFFFF, 0, 0, 0xFFFF, reqtype, NULL, 0, 0);
	
	//����У��
	if(NULL == pnode)
		return NULL;

	//���ͻص�
	pnode->len = TERMINALCOMFRAME_MINLEN;
	if(s_ptcomterminalinfo->pcomterminalsndcallbak != NULL)
		s_ptcomterminalinfo->pcomterminalsndcallbak(pnode);

	//�黹freelist
	pnode->len = 0;
	listnode_put(s_ptcomterminalinfo->pterminalsendfreelist, pnode);

	//��ʱ�ȴ�Ӧ��
	pnode = NULL;

	while(is_timeout(ticks.TICKS[0],timeout) == FALSE)
	{
		listnode_get(s_ptcomterminalinfo->pterminalrecvdatalist, &pnode);
		if(NULL != pnode)
		{
			//��ַ����
			TerminalStructAddr(&terminalframe, pnode->pbuf, pnode->len - TERMINALCOMFRAME_MINLEN);
			//У���
			if(MakeCheckInfo(pnode->pbuf, pnode->len - 1) != *terminalframe.pchk)
			{				
				DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "[ComTerminalGetLanInfoPre]: return chksum error!\r\n");
			}
			//������󷵻�
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
//������: ComTerminalGetLanAddr
//��  ��: ��ȡ��������ַ
//��  ��: uint_16* pnetid, uint_32* pnodeid�����ؾ�������ַ
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
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
//������: ComTerminalGetbRouter
//��  ��: ��ȡ������������·�ɹ���
//��  ��: bool* brouter������·�ɹ���
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
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
//������: ComTerminalGetTranNum
//��  ��: ��ȡ������������ת������
//��  ��: uint_8* trannum������ת������
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
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
//������: ComTerminalGetTranNum
//��  ��: ��ȡ������������csma����
//��  ��: uint_8* trannum������ת������
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
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
//������: ComTerminalGetRfCh
//��  ��: ��ȡ������������ת������
//��  ��: uint_8* trannum������ת������
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
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
//������: ComTerminalProc
//��  ��: ͨ�������
//��  ��: uint_8 comtype������server����terminal��ͨ��֡
//               const uint_8* const pframe��ͨ��֡�׵�ַ
//               uint_16 framelen��pframe��Ч����
//��  ��: ������
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
		
	//��ַ����
	TerminalStructAddr(&terminalframe, (uint_8*)pframe, framelen - TERMINALCOMFRAME_MINLEN);

	//���ȴ��󷵻�
	if(*terminalframe.plen < TERMINALCOMFRAME_MINLEN || framelen < TERMINALCOMFRAME_MINLEN)
	{
		ret = eRET_LEN;
		goto ComTerminalProc_ERRRET;
	}
		
	//������󷵻�
	if((*terminalframe.pcmd >= eTERMINALCOM_CMDMAX && *terminalframe.pcmd < eTERMINALCOM_ACK)
		|| *terminalframe.pcmd >= eTERMINALCOM_ACKMAX)
	{
		ret = eRET_CMD;
		goto ComTerminalProc_ERRRET;
	}
	
	//У���
	if(MakeCheckInfo(terminalframe.plen, framelen - 1) != *terminalframe.pchk)
	{
		ret = eRET_CHK;
		goto ComTerminalProc_ERRRET;
	}
	
	//�û�����
	if(eTERMINALCOM_DATA == *terminalframe.pcmd)
	{
		ret = ComTerminalAppCmdProc(&terminalframe);
	}
	//ϵͳ����
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




