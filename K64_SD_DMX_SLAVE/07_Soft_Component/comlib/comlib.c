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
#include "comlib.h"
#include "comlib_server.h"
#include "comlib_terminal.h"
//#include "includes.h"

static TComInfo s_tcominfo;


//============================================================================
//������: ComInit
//��  ��: ͨ�Ų������ģ���ʼ��
//��  ��: const TComInfo* const ptcominfo��ͨ�Ų���Ϣ
//��  ��: ��
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
//������: ComGetLanInfo
//��  ��: ��ȡ��������Ϣ
//��  ��: TComLanInfo* ptcomlaninfo����������Ϣ
//��  ��: ��
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
//������: ComProc
//��  ��: ͨ�������
//��  ��: uint_8 comtype������server����terminal��ͨ��֡
//               const uint_8* const pframe��ͨ��֡�׵�ַ
//               uint_16 framelen��pframe��Ч����
//��  ��: ������
//============================================================================
uint_8 ComProc(uint_8 comtype, const uint_8* const pframe, uint_16 framelen)
{
	//�����ж�
	if(comtype >= eCOMTYPE_MAX || NULL == pframe)
		return eRET_PARAM;
	
	//serverͨ��֡����
	if(eCOMTYPE_SERVER == comtype)
		return ComServerProc(pframe, framelen);

	//terminalͨ��֡����
	return ComTerminalProc(pframe, framelen);
}

//============================================================================
//������: ComHeartProc
//��  ��: ͨ�Ų���������
//��  ��: ��
//��  ��: ��
//============================================================================
void ComHeartProc(void)
{
	ComServerHeartProc();
}

//============================================================================
//������: ServerFrameAddEsc
//��  ��: ������ͨ��֡����ת���
//��  ��: const uint_8* const psrc��ͨ��֡
//               uint_16 srclen��psrc��Ч����
//               uint_8* pdst������ת����ŵ�ַ
//               uint_16 dstsize��pdst�����С
//��  ��: pdst��Ч����
//============================================================================
uint_16 ServerFrameAddEsc(const uint_8* const psrc, uint_16 srclen, uint_8* pdst, uint_16 dstsize)
{
	uint_16 i, j;
	
	//������֡ͷ֡β����ȷ
	if(NULL == psrc || NULL == pdst || dstsize < srclen || 
		psrc[0] != SERVERCOM_START_FLAG || psrc[srclen - 1] != SERVERCOM_END_FLAG)
		return 0;

	//֡ͷ
	pdst[0] = psrc[0];

	//֡����ת��
	for(i = 1, j = 1; i < srclen - 1; i++)
	{
		//��ת��
		if(psrc[i] == SERVERCOM_START_FLAG || psrc[i] == SERVERCOM_END_FLAG || psrc[i] == SERVERCOM_ESC_FLAG)
			pdst[j++] = SERVERCOM_ESC_FLAG;

		//ԭ����
		pdst[j++] = psrc[i];

		//pdst��
		if(j >= dstsize)
		{
			j = 0;
			break;
		}
	}

	//֡β
	if(j > 0)
		pdst[j++] = psrc[srclen - 1];

	return j;
}

//============================================================================
//������: ServerFrameDelEsc
//��  ��: ������ͨ��֡ɾ��ת���
//��  ��: const uint_8* const psrc��ͨ��֡
//               uint_16 srclen��psrc��Ч����
//               uint_16* psrcretlen��psrc���ص�ǰ�����ȣ����ƴ������
//               uint_8* pdst������ת����ŵ�ַ
//               uint_16 dstsize��pdst�����С
//��  ��: pdst��Ч����
//============================================================================
uint_16 ServerFrameDelEsc(const uint_8* const psrc, uint_16 srclen, uint_16* psrcretlen, uint_8* pdst, uint_16 dstsize)
{
	uint_16 i, j;
	
	//������֡ͷ����ȷ
	if(NULL == psrc || NULL == pdst || psrc[0] != SERVERCOM_START_FLAG)
		return 0;

	//֡ͷ
	pdst[0] = psrc[0];

	//֡��������ȥת��
	for(i = 1, j = 1; i < srclen; i++)
	{
		//ת���ַ�
		if(psrc[i] == SERVERCOM_ESC_FLAG)
		{
			if(psrc[i + 1] == SERVERCOM_START_FLAG || psrc[i + 1] == SERVERCOM_END_FLAG || psrc[i + 1] == SERVERCOM_ESC_FLAG)
				pdst[j++] = psrc[++i];
			//����
			else
			{
				j = 0;
				break;
			}
		}
		else
		{
			pdst[j++] = psrc[i];
			//������
			if(psrc[i] == SERVERCOM_END_FLAG)
			{
				break;
			}
		}
		
		//pdst��
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
//������: ZigbeeWDIFeed
//��  ��: zigbeeģ�鿴�Ź�ι��
//��  ��: ��
//��  ��: ��
//============================================================================
void ZigbeeWDIFeed(void)
{
	uint_8 buf[2];       

	//zigbeeģ��ι������֡
	buf[0] = 2;
	buf[1] = eTERMINALAPPCMD_WDIFEED;
	
	//�ն���֡����
	ComTerminalSendCmd(REVERSE16(s_tcominfo.tcomlaninfo.netid), s_tcominfo.tcomlaninfo.route,
	                   s_tcominfo.tcomlaninfo.groupid, REVERSE16(s_tcominfo.tcomlaninfo.nodeid), 
	                   eTERMINALCOM_DATA, buf, 2, 0);
}

//============================================================================
//������: BroadSetBulk
//��  ��: �㲥���ص�����
//��  ��: ��
//��  ��: ��
//============================================================================
void BroadSetBulk(uint_8 groupid, bool bFlag)
{
	ComServerBroadSetBulk(groupid, bFlag);
}


