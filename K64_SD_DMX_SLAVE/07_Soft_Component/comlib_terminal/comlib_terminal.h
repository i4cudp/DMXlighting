/*********************************************************************
ģ����  �� COMLIB
�ļ���  �� comlib.h
����ļ��� comlib.c
�ļ�ʵ�ֹ��ܣ�ʵ��ͨ�Ų㹦��
����    ������
�汾    ��1.0.0.0.0
----------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2012/07/25   1.0         ����          ����
*********************************************************************/
#ifndef __COM_TERMINAL_LIB_H__
#define __COM_TERMINAL_LIB_H__

// ͷ�ļ�
#include "comlib.h"
#include "common.h"

//��������
enum eTERMINALCOMType
{
    eTERMINALCOM_DATA = 0,      //0x00. ����֡
	eTERMINALCOM_REQ_ADDR,		//0x01. ��ַ��ѯ
	eTERMINALCOM_SET_ADDR,		//0x02. ��ַ����
	eTERMINALCOM_REQ_RFCH,		//0x03. ͨ����ѯ
	eTERMINALCOM_SET_RFCH, 		//0x04. ͨ������
	eTERMINALCOM_REQ_ROUTE, 	//0x05. ·�ɲ�ѯ
	eTERMINALCOM_SET_ROUTE,		//0x06. ·������
	eTERMINALCOM_REQ_TRANUM,	//0x07. ���ת��������ѯ
	eTERMINALCOM_SET_TRANUM,	//0x08. ���ת����������
	eTERMINALCOM_SET_BAUD,		//0x09. ����������
	eTERMINALCOM_REQ_CSMACA,	//0x0A. CSMA/CA���Բ�ѯ
	eTERMINALCOM_SET_CSMACA,    //0x0B. CSMA/CA��������
	eTERMINALCOM_REQ_MAC,		//0x0C. ͨ��ZIG_IP��ַ��ѯZIG_MAC��ַ
	eTERMINALCOM_REQ_MACIP,		//0x0D. ͨ��ZIG_MAC��ַ��ѯZIG_MACIP��ַ
	eTERMINALCOM_SET_MACIP,		//0x0E. ͨ��ZIG_MAC��ַ����ZIG_IP��ַ
	eTERMINALCOM_SET_MAC,		//0x0F. ����ZIG_MAC��ַ
	eTERMINALCOM_REQ_MACFLAG,	//0x10. �㲥��ѯZIG_MAC��ַӦ���־��ѯ
	eTERMINALCOM_SET_MACFLAG,   //0x11. �㲥��ѯZIG_MAC��ַӦ���־����
	eTERMINALCOM_REQ_MACBROAD,  //0x12. �㲥��ѯZIG_MAC��ַ
	eTERMINALCOM_CMDMAX,				

	eTERMINALCOM_ACK = 0xA0,    //0xA0. Ӧ��֡
	eTERMINALCOM_ACK_ADDR,		//0xA1. ��ַӦ��
	eTERMINALCOM_ACK_RFCH, 		//0xA2. ͨ��Ӧ��
	eTERMINALCOM_ACK_ROUTE,		//0xA3. ·��Ӧ��
	eTERMINALCOM_ACK_TRANUM,	//0xA4. ���ת������Ӧ��
	eTERMINALCOM_ACK_CSMACA,	//0xA5. CSMA/CA����Ӧ��
	eTERMINALCOM_ACK_MACIP,		//0xA6. MACIP��ַӦ��
	eTERMINALCOM_ACK_MACFLAG,	//0xA7. �㲥��ѯZIG_MAC��ַӦ���־Ӧ��
    eTERMINALCOM_ACKMAX                
};

//ACK����
enum eTERMINALACKType
{
    eTERMINALACK_ERR_NONE = 0,      //0x00. ��ȷ����
    eTERMINALACK_ERR_CHK,           //0x01. У��ʹ���
	eTERMINALACK_ERR_LEN,			//0x02. ���ȴ���
	eTERMINALACK_ERR_CMD,			//0x03, �����ִ���
	eTERMINALACK_ERR_ADDREQ,	    //0x04, ��ѯ��ַʧ��
	eTERMINALACK_ERR_ADDSET,		//0x05, ���õ�ַʧ��
	eTERMINALACK_ERR_CHREQ,		    //0x06, ��ѯͨ��ʧ��
	eTERMINALACK_ERR_CHSET,		    //0x07, ����ͨ��ʧ��
	eTERMINALACK_ERR_ROUTREQ, 		//0x08, ��ѯ·�ɹ���ʧ��
	eTERMINALACK_ERR_ROUTSET, 		//0x09, ����·�ɹ���ʧ��
	eTERMINALACK_ERR_BRTSET,		//0x0A, ���ò�����ʧ��
	eTERMINALACK_ERR_CSMAREQ,		//0x0B, ��ѯCSMA/CA����ʧ��
	eTERMINALACK_ERR_CSMASET,		//0x0C, ����CSMA/CA����ʧ��
	eTERMINALACK_ERR_MACREQ,		//0x0D, ͨ��ZIG_IP��ַ��ѯZIG_MAC��ַʧ��
	eTERMINALACK_ERR_MACIPREQ,		//0x0E, ͨ��ZIG_MAC��ַ��ѯZIG_IP��ַʧ��
	eTERMINALACK_ERR_MACIPSET,		//0x0F, ͨ��ZIG_MAC��ַ����ZIG_IP��ַʧ��
	eTERMINALACK_ERR_MACSET,		//0x10, ZIG_MAC��ַ����ʧ��
	eTERMINALACK_ERR_MACFLAGREQ,	//0x11, �㲥��ѯZIG_MAC��ַӦ�𿪹ز�ѯʧ��
	eTERMINALACK_ERR_MACFLAGSET,	//0x12, �㲥��ѯZIG_MAC��ַӦ�𿪹�����ʧ��
	
    eTERMINALACK_ERR_MAX                
};

//���������ն˿�����ͨ��֡��ʽ
typedef struct 
{
    uint_8*    plen;       //�ܳ���
    uint_32*   pret;       //�ϲ㷵��
    uint_16*   pnetid;     //�����
    uint_8*    proute;     //·�ɷ�ʽ
    uint_8*    pgroupid;   //���
    uint_16*   pnodeid;    //�ڵ��
    uint_8*    pcmd;	   //��������
	uint_8*    pbuf;       //��������
    uint_8*    pchk;       //�ۼӺ�У��
}TTerminalCOMFrame;
#define TERMINALCOMFRAME_MINLEN    13

//��������
enum eTerminalAppCmd
{
    eTERMINALAPPCMD_REQ_BULK = 0,      //0x00. �ƿ���״̬��ѯ
	eTERMINALAPPCMD_SET_BULK,		   //0x01. ���ص�����
	eTERMINALAPPCMD_REQ_POWER,		   //0x02. ����������ѯ
	eTERMINALAPPCMD_SET_POWER,		   //0x03. �ƹ��ʵ���
	eTERMINALAPPCMD_REQ_PARAM,		   //0x04. ��һǶ��ʽģ�������ѯ
	eTERMINALAPPCMD_SET_PARAM,		   //0x05. ��һǶ��ʽģ���������
	eTERMINALAPPCMD_REQ_VERSION, 	   //0x06.�汾��Ϣ��ѯ		
	eTERMINALAPPCMD_REQ_BULKSTATE, 	   //0x07.����״̬��ѯ		
	eTERMINALAPPCMD_REQ_INDIFLAG,	   //0x08.�����ϴ���־��ѯ		
	eTERMINALAPPCMD_SET_INDIFLAG,	   //0x09.�����ϴ���־����	
	eTERMINALAPPCMD_RESET,	           //0x0A.�ն˿�������λ����
	eTERMINALAPPCMD_ADVALUE,		   //0x0B.�ն˿�������ȡADֵ
	eTERMINALAPPCMD_REQ_RESETIME,      //0x0C.�ն˿�������λʱ���ѯ
	eTERMINALAPPCMD_WDIFEED,		   //0x0D. Zigbeeģ�鿴�Ź�ι������
	eTERMINALAPPCMD_REQ_WDITIME,       //0x0E. Zigbeeģ�鿴�Ź���λʱ���ѯ
	eTERMINALAPPCMD_SET_WDITIME,	   //0x0F. Zigbeeģ�鿴�Ź���λʱ������
	eTERMINALAPPCMD_MACSETBULK, 	   //0x10. ͨ��MAC��ַ���п��ص�
	eTERMINALAPPCMD_REQ_STARTUPMODE,   //0x11. �ƾ�������ʽ��ѯ
	eTERMINALAPPCMD_SET_STARTUPMODE,   //0x12. �ƾ�������ʽ����
	eTERMINALAPPCMD_REQ_BULKSET,	   //0x13. ���ص����ò�ѯ
	eTERMINALAPPCMD_REQ_BULKONCUR, 	   //0x14. ���صƵ�����ֵ��ѯ
	eTERMINALAPPCMD_SET_BULKONCUR,	   //0x15. ���صƵ�����ֵ����
	eTERMINALAPPCMD_VOLTAGEREQ,		   //0x16. ��ѹ������ѯ
	eTERMINALAPPCMD_CURRENTREQ, 	   //0x17. ����������ѯ
	eTERMINALAPPCMD_TEMPERATUREREQ,    //0x18. �¶Ȳ�����ѯ
	eTERMINALAPPCMD_V2POLL, 	       //0x19. V2��ѯ�����ѯ
	eTERMINALAPPCMD_HDADDRREQ,         //0x1A. Ӳ����ַ��ѯ
	eTERMINALAPPCMD_HDADDRESET, 	   //0x1B. Ӳ����ַ����

	
	//���Ӿ�������Ŀ����
	eTERMINALCOM_PURIFIERREQ = 0x50,   //0x50. ��������Ŀ��ȡ����
	eTERMINALCOM_PURIFIERSET,		   //0x51. ��������Ŀ���ò���

	eTERMINALCOM_BMM_SYSPARAM_REQ = 0x58,     //0x58. BMM������ѯ����
	eTERMINALCOM_BMM_SYSPARAM_SET,            //0x59. BMM������������
	eTERMINALCOM_BMM_STATE1_REQ,		      //0x5A. BMM״̬1��ѯ����
	eTERMINALCOM_BMM_STATE2_REQ,		      //0x5B. BMM״̬2��ѯ����
	
	eTERMINALAPPCMD_MAX,				

	eTERMINALAPPCMD_ACK = 0xA0,        //0xA0. Ӧ��֡
	eTERMINALAPPCMD_ACK_BULK,		   //0xA1. ��ַӦ��
	eTERMINALAPPCMD_ACK_POWER, 		   //0xA2. ͨ��Ӧ��
	eTERMINALAPPCMD_ACK_PARAM,		   //0xA3. ��һǶ��ʽģ�����Ӧ��
	eTERMINALAPPCMD_ACK_VERSION,	   //0xA4. �汾��ϢӦ��
	eTERMINALAPPCMD_ACK_BULKSTATE,	   //0xA5. ��״̬Ӧ��
	eTERMINALAPPCMD_ACK_INDIFLAG,	   //0xA6. �����ϴ���־Ӧ��
	eTERMINALAPPCMD_INDICATION,	       //0xA7. ��״̬�����ϴ���
	eTERMINALAPPCMD_ACK_ADVALUE, 	   //0xA8. ADֵӦ��
	eTERMINALAPPCMD_ACK_RESETIME,      //0xA9. �ն˿�������λʱ��Ӧ��
	eTERMINALAPPCMD_ACK_WIDTIME,       //0XAA.���Ź���λʱ��Ӧ��
	eTERMINALAPPCMD_ACK_STARTUPMODE,   //0XAB.�ƾ�������ʽӦ��֡
	eTERMINALAPPCMD_ACK_BULKSET,	   //0xAC. ��������Ӧ��
	eTERMINALAPPCMD_ACK_BULKONCUR,     //0xAD. ���صƵ�����ֵӦ��
	eTERMINALAPPCMD_ACK_VOLTAGE,	   //0xAE. ��ѹ����Ӧ��
	eTERMINALAPPCMD_ACK_CURRENT,	   //0xAF. ��������Ӧ��
	eTERMINALAPPCMD_ACK_TEMPERATURE,   //0xB0. �¶Ȳ���Ӧ��
	eTERMINALAPPCMD_ACK_V2POLL,	       //0xB1. V2��ѯ����Ӧ��
	eTERMINALAPPCMD_ACK_HDADDR,		   //0XB2.Ӳ����ַӦ��

	eTERMINALAPPCMD_ACK_PURIFIERREQ = 0xD0,	 //0xD0. ��������Ŀ��ȡ����Ӧ��

	eTERMINALAPPCMD_ACK_BMM_SYSPARAM_REQ = 0xD8,    //0xD8. BMM����Ӧ��֡
	eTERMINALAPPCMD_ACK_BMM_STATE1_REQ,             //0xD9. BMM״̬1Ӧ��֡
	eTERMINALAPPCMD_ACK_BMM_STATE2_REQ,		        //0xDA. BMM״̬2Ӧ��֡
	
    eTERMINALAPPCMD_ACKMAX                
};

//ACK����
enum eTerminalAppACKType
{
    eTERMINALAppACK_NONE = 0,          //0x00. ��ȷ����
    eTERMINALAppACK_ERR_BULKREQ,       //0x01. ���صƲ�ѯ����
	eTERMINALAppACK_ERR_BULKSET,	   //0x02. ���ص����ô���
	eTERMINALAppACK_ERR_POWERREQ,      //0x03. �������ô���
	eTERMINALAppACK_ERR_POWERSET,	   //0x04. �������ô���
	eTERMINALAppACK_ERR_PARAMREQ,	   //0x05. ��һǶ��ʽģ�������ѯ����
	eTERMINALAppACK_ERR_PARAMSET,	   //0x06. ��һǶ��ʽģ��������ô���
	eTERMINALAppACK_ERR_VERREQ,	       //0x07. �汾��Ϣ��ѯ����
	eTERMINALAppACK_ERR_BULKSTATEREQ,  //0x08. ��״̬��ѯ����
	eTERMINALAppACK_ERR_INDIFLAGREQ,   //0x09. �����ϴ���־��ѯ����
	eTERMINALAppACK_ERR_INDIFLAGSET,   //0x0A. �����ϴ���־���ô���
	eTERMINALAppACK_ERR_RESET,         //0x0B. �ն˿�������λ�������
	eTERMINALAppACK_ERR_ADVALUEREQ,	   //0x0C. �ն˿�������ȡADֵ�������
	eTERMINALAppACK_ERR_RESETIMEREQ,   //0x0D. �ն˿�������λʱ���ѯ����
	eTERMINALAppACK_ERR_WDITIMEREQ,    //0x0E. WDI��ʱֵ��ѯ����
	eTERMINALAppACK_ERR_WDITIMESET,	   //0x0F. WDI��ʱֵ��ѯ����
	eTERMINALAppACK_ERR_STARTUPMODEREQ,  //0x10. �ƾ�������ʽ��ѯʧ��
	eTERMINALAppACK_ERR_STARTUPMODESET,  //0x11. �ƾ�������ʽ����ʧ��
	eTERMINALAppACK_ERR_BULKSETREQ,      //0x12. ���ص����ò�ѯʧ��
	eTERMINALAppACK_ERR_BULKONCURREQ, 	 //0x13. ���صƵ�����ֵ��ѯʧ��
	eTERMINALAppACK_ERR_BULKONCURSET,	 //0x14. ���صƵ�����ֵ����ʧ��
	eTERMINALAppACK_ERR_VOLTAGEREQ,	     //0x15. ��ѹ������ѯʧ��
	eTERMINALAppACK_ERR_CURRENTREQ, 	 //0x16. ����������ѯʧ��
	eTERMINALAppACK_ERR_TEMPERATUREREQ,  //0x17. �¶Ȳ�����ѯʧ��
	eTERMINALAppACK_ERR_V2POLL,          //0x18. V2��ѯ����ʧ��

	eTERMINALAppACK_ERR_HDADDRREQ = 0x22,//0x22. Ӳ����ַ��ѯ����
	eTERMINALAppACK_ERR_HDADDRSET, 	     //0x23. Ӳ����ַ��ѯ����

	eTERMINALAppACK_ERR_PURIFIERREQ = 0x50, //0x50. ��������Ŀ��ȡ����Ӧ��ʧ��
	eTERMINALAppACK_ERR_PURIFIERSET,    //0x51. ��������Ŀ���ò���Ӧ��ʧ��

	eTERMINALAppACK_ERR_BMM_SYSPARAM_REQ = 0x58,  //0x58. BMM������ѯӦ��ʧ��
	eTERMINALAppACK_ERR_BMM_SYSPARAM_SET,         //0x59. BMM��������Ӧ��ʧ��
	eTERMINALAppACK_ERR_BMM_STATE1_REQ,           //0x5A. BMM״̬1��ѯӦ��ʧ��
	eTERMINALAppACK_ERR_BMM_STATE2_REQ,	          //0x5B. BMM״̬2��ѯӦ��ʧ��
	
    eTERMINALAppACK_MAX                
};

//�ṹ���� 
typedef struct 
{
    uint_8*      plen;
	uint_8*      pcmd;
	uint_8*      pbuf;
}TTerminalAppHead;
#define TERMINALAPPFRAME_MINLEN    (TERMINALCOMFRAME_MINLEN + 2)


//�㲥��ַ
//0.0Ϊ��������㲥��ַ
//X.0Ϊ�����X��ȫ��ַ�㲥
//X.1Ϊ�����X�����ַ�㲥
//X.2Ϊ�����X��ż��ַ�㲥
#define BROADCAST_NETID        0
#define BROADCAST_NODEID       0
#define BROADCAST_ODD_NODEID   1
#define BROADCAST_EVEN_NODEID  2
#define BROADCAST_GROUPID      0
#define BROADCAST_ODD_GROUPID  1
#define BROADCAST_EVEN_GROUPID 2

enum eLanFrameType
{                                                          //netid      nodeid      groupid
	eLANFRAMETYPE_ALL = 0,	   //0x00. ȫ���㲥֡                                     0             0         
    eLANFRAMETYPE_ME,         //0x01. ���ڵ�֡                                          X              Y  
    eLANFRAMETYPE_NET,        //0x02.���ڹ㲥֡                                     X              0            0
	eLANFRAMETYPE_ODDGROUP,   //0x03.����������㲥֡	               	   X		   0		1
	eLANFRAMETYPE_EVENGROUP,  //0x04.����ż����㲥֡		           X		   0		2
	eLANFRAMETYPE_GROUP,	   //0x05.�������ڹ㲥֡		           X		   0		Z
	eLANFRAMETYPE_ODD_NET,	   //0x06.�������ַ�㲥֡			   X		   1		0
	eLANFRAMETYPE_ODD_ODD,	   //0x07.�������������ַ�㲥֡	   X		   1		1
	eLANFRAMETYPE_ODD_EVEN,   //0x08.����ż�������ַ�㲥֡	   X		   1		2
	eLANFRAMETYPE_ODD_GROUP,  //0x09.�����������ַ�㲥֡	   X		   1		Z
	eLANFRAMETYPE_EVEN_NET,   //0x0A.����ż��ַ�㲥֡		           X 		   2        	0
	eLANFRAMETYPE_EVEN_ODD,   //0x0B.����������ż��ַ�㲥֡    X 		   2 		1
	eLANFRAMETYPE_EVEN_EVEN,  //0x0C.����ż����ż��ַ�㲥֡    X 		   2 		2
	eLANFRAMETYPE_EVEN_GROUP, //0x0D.��������ż��ַ�㲥֡	   X 		   2		Z
	eLANFRAMETYPE_OTHER_GROUP,//0x0E. �����ڲ�������㲥֡        X             0/1/2    Z1(!=Z)
	eLANFRAMETYPE_NODE_NET,   //0x0F. �����ڲ������ڵ�֡		  X	       Y1(!=Y)
	eLANFRAMETYPE_OTHEER_NET, //0x10.��������֡                                     X1(!=X)

    eLANFRAMETYPE_MAX                
};	

//eZigFrameRouteType
enum eZigFrameRouteType
{
    eZIGFRAMEROUTE_V1 = 0,         //0. 1.X��·�ɷ�ʽ
	eZIGFRAMEROUTE_NONE = 10,	   //10. ��·��
	eZIGFRAMEROUTE_FLOOD,	       //11. ����·��
	eZIGFRAMEROUTE_UP,             //12. ����·��
	eZIGFRAMEROUTE_DOWN,           //13. �ݼ�·��
	
    eZIGFRAMEROUTE_MAX                
};

//����������
enum ePHYType
{
    ePHYType_AD = 0,             //0. ADֵ
    ePHYType_PHY,                //1. ʵ������ֵ
	
    ePHYType_MAX               
};


//============================================================================
//������: ComTerminalInit
//��  ��: ͨ�Ų������ģ���ʼ��
//��  ��: const TComInfo* const ptcominfo��ͨ�Ų���Ϣ
//��  ��: ��
//============================================================================
void ComTerminalInit(TComInfo* ptcominfo);

//============================================================================
//������: ComTerminalSendCmd
//��  ��: �ն˿�������������
//��  ��: uint_8 cmd, ������
//               uint_8* cmdbuf������������
//               uint_8 cmdlen��cmdbuf��Ч����
//��  ��: uint8��������
//============================================================================
uint_8 ComTerminalSendCmd(uint_16 netid, uint_8 route, uint_8 groupid,  
	uint_16 nodeid, uint_8 cmdtype, uint_8* pcmd, uint_8 cmdlen, uint_32 serverret);

//============================================================================
//������: ComTerminalGetLanAddr
//��  ��: ��ȡ��������������ַ
//��  ��: uint_16* pnetid, uint_32* pnodeid�����ؾ�������ַ
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
//============================================================================
uint_8 ComTerminalGetLanAddr(uint_16* pnetid, uint_8* proute, uint_8* pgroupid, uint_16* pnodeid, uint_32 timeout);

//============================================================================
//������: ComTerminalGetbRouter
//��  ��: ��ȡ������������·�ɹ���
//��  ��: bool* brouter������·�ɹ���
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
//============================================================================
uint_8 ComTerminalGetbRouter(bool* brouter, uint_32 timeout);

//============================================================================
//������: ComTerminalGetTranNum
//��  ��: ��ȡ������������ת������
//��  ��: uint_8* trannum������ת������
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
//============================================================================
uint_8 ComTerminalGetTranNum(uint_8* trannum, uint_32 timeout);

//============================================================================
//������: ComTerminalGetTranNum
//��  ��: ��ȡ������������csma����
//��  ��: uint_8* trannum������ת������
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
//============================================================================
uint_8 ComTerminalGetbCsmaca(bool* bcsmaca, uint_32 timeout);

//============================================================================
//������: ComTerminalGetRfCh
//��  ��: ��ȡ������������ת������
//��  ��: uint_8* trannum������ת������
//               uint_32 timeout����ʱʱ��
//��  ��: uint8��������
//============================================================================
uint_8 ComTerminalGetRfCh(uint_8* rfch, uint_32 timeout);

//============================================================================
//������: ComTerminalProc
//��  ��: ͨ�������
//��  ��: uint_8 comtype������server����terminal��ͨ��֡
//               const uint_8* const pframe��ͨ��֡�׵�ַ
//               uint_16 framelen��pframe��Ч����
//��  ��: ������
//============================================================================
uint_8 ComTerminalProc( const uint_8* const pframe, uint_16 framelen);

//-------------------------------------------------------------------------
//������: LanFrameType                                                        
//��  ��: �жϾ�����֡����           
//��  ��: uint_16 netid��netid
//               uint_16 nodeid��nodeid
//��  ��: ��                                              
//˵  ��: TRUE�����Լ���֡��FALSE�������Լ���֡
//-------------------------------------------------------------------------
uint_8 LanFrameType(uint_16 netid, uint_16 nodeid, uint_8 groupid);

//-------------------------------------------------------------------------
//������: MakeCheckInfo                                                        
//��  ��: ����֡У����Ϣ                                
//��  ��: uint_8* pbuf��У�������׵�ַ
//		     uint_16 buflen��У�����ݳ���
//��  ��: У����                                                
//˵  ��: (1)�����ۼӺ�У��
//-------------------------------------------------------------------------
uint_8 MakeCheckInfo(uint_8* pbuf, uint_16 buflen);

//============================================================================
//������: TerminalStructAddr
//��  ��: �ṹ��ַ����
//��  ��: ��
//��  ��: ��
//============================================================================
void TerminalAppStructAddr(TTerminalAppHead* ptapp, uint_8* pbuf);

#endif // __COM_TERMINAL_LIB_H__

