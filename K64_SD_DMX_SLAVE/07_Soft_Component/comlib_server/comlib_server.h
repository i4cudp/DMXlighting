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
#ifndef __COM_SERVER_LIB_H__
#define __COM_SERVER_LIB_H__

// ͷ�ļ�

#include "01_app_include.h"
/*****************�������������ͨ�����************************/
#define SERVERCOM_START_FLAG           0x53       //֡ͷ��ʾ
#define SERVERCOM_END_FLAG             0x54       //֡β��ʾ
#define SERVERCOM_ESC_FLAG             0x55       //ת���ַ�

//�������������ͨ��֡��������
enum eSERVERCMDType
{
    eSERVERCMD_REQ_ADDR = 0,               //0x0000. ��ַ��ѯ
	eSERVERCMD_SET_ADDR,		           //0x0001. ��ַ����
	eSERVERCMD_REQ_RFCH,		           //0x0002. ͨ����ѯ
	eSERVERCMD_SET_RFCH, 		           //0x0003. ͨ������
	eSERVERCMD_REQ_ROUTE, 		           //0x0004. ·�ɲ�ѯ
	eSERVERCMD_SET_ROUTE,		           //0x0005. ·������
	eSERVERCMD_REQ_TRANUM,		           //0x0006. ���ת��������ѯ
	eSERVERCMD_SET_TRANUM,		           //0x0007. ���ת����������
	eSERVERCMD_REQ_CSMACA,		           //0x0008. CSMA/CA���Բ�ѯ
	eSERVERCMD_SET_CSMACA,		           //0x0009. CSMA/CA��������
	eSERVERCMD_REQ_HEARTTIME,	           //0x000A. �������ϴ�ʱ���ѯ
	eSERVERCMD_SET_HEARTTIME,	           //0x000B. �������ϴ�ʱ������
	eSERVERCMD_REQ_POLLTIME,		       //0x000C. ��������ѯʱ���ѯ
	eSERVERCMD_SET_POLLTIME,		       //0x000D. ��������ѯʱ������
	eSERVERCMD_REQ_IPPORT,		           //0x000E. ������IP Port��ѯ
	eSERVERCMD_SET_IPPORT,		           //0x000F. ������IP Port����
	eSERVERCMD_REQ_VERSION,			       //0x0010. �汾��Ϣ��ѯ
	eSERVERCMD_REQ_SIMNUM, 		           //0x0011. ��ѯ������SIM����
	eSERVERCMD_REQ_MOBILEALARM, 		   //0x0012. �ֻ��澯���ܿ��ز�ѯ
	eSERVERCMD_SET_MOBILEALARM, 		   //0x0013. �ֻ��澯���ܿ�������
	eSERVERCMD_REQ_MOBILEALARMNUM, 		   //0x0014. �ֻ��澯�����ѯ
	eSERVERCMD_SET_MOBILEALARMNUM, 		   //0x0015. �ֻ��澯��������
	eSERVERCMD_RESET,				       //0x0016. �豸��λ����
	eSERVERCMD_REQ_INDIFLAG,	           //0x0017.�����ϴ���־��ѯ		
	eSERVERCMD_SET_INDIFLAG,	           //0x0018.�����ϴ���־����	
	eSERVERCMD_SET_TIMECON,			       //0x0019.ʱ�ο�������	
	eSERVERCMD_PHONEALARM_DATA, 		   //0x001A.�����ֻ��澯����	
	eSERVERCMD_REQ_BROADSETTIME, 		   //0x001B.��ѯ�������㲥��������ִ�д���	
	eSERVERCMD_SET_BROADSETTIME,		   //0x001C.�����������㲥��������ִ�д���	
	eSERVERCMD_REQ_MAC,		               //0x001D. ͨ��ZIG_IP��ַ��ѯZIG_MACIP��ַ
	eSERVERCMD_REQ_MACIP,		           //0x001E. ͨ��ZIG_MAC��ַ��ѯZIG_MACIP��ַ
	eSERVERCMD_SET_MACIP,		           //0x001F. ͨ��ZIG_MAC��ַ����ZIG_IP��ַ
	eSERVERCMD_REQ_POLLTIMEOUT,			   //0x0020. ��ѯ������ÿ����ѯ��ʱʱ��
	eSERVERCMD_SET_POLLTIMEOUT, 		   //0x0021. ����������ÿ����ѯ��ʱʱ��
	eSERVERCMD_REQ_POLLCOUNT1, 		       //0x0022. ��ѯ������ÿ���ڵ������ѯ����
	eSERVERCMD_SET_POLLCOUNT1, 		       //0x0023. ���������������ѯ����
	eSERVERCMD_REQ_POLLCOUNT2,			   //0x0024. ��ѯ����������ѯ����������£���ѯ����
	eSERVERCMD_SET_POLLCOUNT2,			   //0x0025. ��������������ѯ����������£���ѯ������ע��ӦС�ڵ��������ѯ����
	eSERVERCMD_REQ_RESETIME,			   //0x0026. ��ѯ���������ն˿�������λʱ��
	eSERVERCMD_REQ_WDITIME,                //0x0027. Zigbeeģ�鿴�Ź���λʱ���ѯ
	eSERVERCMD_SET_WDITIME,	               //0x0028. Zigbeeģ�鿴�Ź���λʱ������
	eSERVERCMD_REQ_MACFLAG,	               //0x0029. �㲥��ѯZIG_MAC��ַӦ���־��ѯ
	eSERVERCMD_SET_MACFLAG,                //0x002A. �㲥��ѯZIG_MAC��ַӦ���־����
	eSERVERCMD_MACBROADREQ,                //0x002B. �㲥��ѯZIG_MAC��ַ
	eSERVERCMD_REQ_HDADDR,	               //0x002C. Ӳ����ַ��ѯ
	eSERVERCMD_SET_HDADDR,                 //0x002D. Ӳ����ַ����
	eSERVERCMD_SYSMAX, 			   

	eSERVERCMD_REQ_ALLADDR = 0x1000,       //0x1000. ·�����е�ַ��ѯ
	eSERVERCMD_ADD_DEV,		               //0x1001. ����·���豸
	eSERVERCMD_DEL_DEV, 		           //0x1002. ɾ��·���豸
	eSERVERCMD_REQ_BULK,		           //0x1003. �ƿ���״̬��ѯ
	eSERVERCMD_SET_BULK,		           //0x1004. �ƿ���״̬����
	eSERVERCMD_REQ_POWER,		           //0x1005. �ƹ��ʲ�ѯ
	eSERVERCMD_SET_POWER,			       //0x1006. �ƹ�������
	eSERVERCMD_REQ_PARAM,		           //0x1007. ��һǶ��ʽģ�������ѯ
	eSERVERCMD_SET_PARAM,		           //0x1008. ��һǶ��ʽģ���������
	eSERVERCMD_REQ_BULKSTATE,			   //0x1009. ��״̬��ѯ
	eSERVERCMD_REQ_ADVALUE,			       //0x100A. ADֵ��ѯ
	eSERVERCMD_SET_FUPINLED, 			   //0x100B. ��ƽ��ĿLED��ʾ����
	eSERVERCMD_MACSETBULK, 		           //0x100C. ͨ��MAC��ַ���п��ص�
	eSERVERCMD_REQ_STARTUPMODE,            //0x100D. �ƾ�������ʽ��ѯ
	eSERVERCMD_SET_STARTUPMODE,            //0x100E. �ƾ�������ʽ����
	eSERVERCMD_REQ_BULKSET,		           //0x100F. ���ص����ò�ѯ
	eSERVERCMD_REQ_BULKONCUR, 	           //0x1010. ���صƵ�����ֵ��ѯ
	eSERVERCMD_SET_BULKONCUR,	           //0x1011. ���صƵ�����ֵ����
	eSERVERCMD_REQ_VOLTAGE,		           //0x1012. �ն˵�ѹ������ѯ
	eSERVERCMD_REQ_CURRENT, 		       //0x1013. �ն˵���������ѯ
	eSERVERCMD_REQ_TEMPERATURE, 	       //0x1014. �ն��¶Ȳ�����ѯ
	eSERVERCMD_REQ_VOLCUR, 		           //0x1015. �ն˵�ѹ������������ѯ

	//���Ӿ�������Ŀ����
	eSERVERCMD_REQ_PURIFIER = 0x1100,      //0x1100. ��������Ŀ��ȡ����
	eSERVERCMD_SET_PURIFIER,		       //0x1101. ��������Ŀ���ò���
	eSERVERCMD_SET_ELECTRIC,			   //0x1102. ����������ģ�����ò���

	
	//����BMM��Ŀ����
	eSERVERCMD_BMM_SYSPARAM_REQ = 0x1200,  //0x1200. BMM������ѯ����
	eSERVERCMD_BMM_SYSPARAM_SET,		   //0x1201. BMM������������
	eSERVERCMD_BMM_STATE1_REQ,		       //0x1202. BMM״̬1��ѯ����
	eSERVERCMD_BMM_STATE2_REQ,		       //0x1203. BMM״̬2��ѯ����
	
	//����DMX�ƹ���Ŀ����
	eSERVERCMD_DMX_WRITE_SDHC = 0x1300,    //0x1300. дSD������
	eSERVERCMD_DMX_DETEC_DEV,              //0x1301. ���ģ������

	eSERVERCMD_REQ_WSMCARRIER = 0x2000,	   //0x2000. ��������Ӫ�̲�ѯ
	eSERVERCMD_SET_WSMCARRIER, 	           //0x2001. ��������Ӫ������
	eSERVERCMD_REQ_WSMVOL,				   //0x2002. ��������ѹ������ѯ
	eSERVERCMD_REQ_WSMCUR,		           //0x2003. ����������������ѯ
	eSERVERCMD_REQ_WSMSWITCHIN,			   //0x2004. �����������������ѯ
	eSERVERCMD_REQ_WSMSWITCHOUT,		   //0x2005. �����������������ѯ
	eSERVERCMD_SET_WSMSWITCHOUT, 		   //0x2006. �������������������
	eSERVERCMD_REQ_WSMCOMPILETIME,		   //0x2007. ����������ʱ���ѯ
	eSERVERCMD_REQ_WSMSDDATA,		       //0x2008. ������SD���ݲ�ѯ
	eSERVERCMD_REQ_WSMHEARTATTR,		   //0x2009. ���������������Բ�ѯ
	eSERVERCMD_SET_WSMHEARTATTR,		   //0x200A. ��������������������
	eSERVERCMD_SET_SYSTIME, 		       //0x200B. ��������ʱ
	eSERVERCMD_REQ_TIMECONTROL, 		   //0x200C. ������ʱ�ο��Ʋ�ѯ
	eSERVERCMD_SET_TIMECONTROL, 		   //0x200D. ������ʱ�ο�������
	eSERVERCMD_REQ_WSMPOLLATTR,		       //0x200E. ��������ѯ���Բ�ѯ
	eSERVERCMD_SET_WSMPOLLATTR,		       //0x200F. ��������ѯ��������
	eSERVERCMD_REQ_WSMDEVGROUP, 		   //0x2010. �������ն˷����ѯ
	eSERVERCMD_SET_WSMDEVGROUP, 		   //0x2011. �������ն˷�������
	eSERVERCMD_REQ_WSMVOLNUM, 		       //0x2012. ��������������ѹ��Ų�ѯ
	eSERVERCMD_SET_WSMVOLNUM, 		       //0x2013. ��������������ѹ�������
	eSERVERCMD_REQ_WSMCURNUM,			   //0x2014. ������������������Ų�ѯ
	eSERVERCMD_SET_WSMCURNUM,			   //0x2015. �����������������������
	eSERVERCMD_REQ_WSMSWITCHINNUM,		   //0x2016. ������������������������Ų�ѯ
	eSERVERCMD_SET_WSMSWITCHINNUM,		   //0x2017. �����������������������������
	eSERVERCMD_REQ_WSMSWITCHOUTNUM,		   //0x2018. �����������������������Ų�ѯ
	eSERVERCMD_SET_WSMSWITCHOUTNUM,		   //0x2019. ����������������������������
	eSERVERCMD_REQ_METERPOW, 	           //0x201A. ��������ȡ������
	eSERVERCMD_APPMAX,

	eSERVERCMD_ACK = 0x8000,               //0x8000. Ӧ��֡
	eSERVERCMD_ACK_ADDR,		           //0x8001. ��ַӦ��
	eSERVERCMD_ACK_RFCH, 		           //0x8002. ͨ��Ӧ��
	eSERVERCMD_ACK_ROUTE,		           //0x8003. ·��Ӧ��
	eSERVERCMD_ACK_TRANUM,		           //0x8004. ���ת������Ӧ��
	eSERVERCMD_ACK_CSMACA,		           //0x8005. CSMA/CA����Ӧ��
	eSERVERCMD_ACK_HEARTTIME,		       //0x8006. �������ϴ�ʱ��Ӧ��
	eSERVERCMD_ACK_POLLTIME, 		       //0x8007. ��������ѯʱ��Ӧ��
	eSERVERCMD_ACK_IPPORT,		           //0x8008. ������IP PortӦ��
	eSERVERCMD_ACK_VERSION,	               //0x8009. �汾��ϢӦ��
	eSERVERCMD_ACK_SIMNUM,			       //0x800A. ������SIM����Ӧ��
	eSERVERCMD_ACK_MOBILEALARM,		       //0x800B. �������ֻ��澯���ع���Ӧ��
	eSERVERCMD_ACK_MOBILEALARMNUM, 	       //0x800C. �������ֻ��澯����Ӧ��
	eSERVERCMD_ACK_INDIFLAG,	           //0x800D. �ն˿����������ϴ���־Ӧ��
	eSERVERCMD_ACK_BROADSETTIME,		   //0x800E. �������㲥��������ִ�д���Ӧ��
	eSERVERCMD_ACK_MACIP,		           //0x800F. MACIP��ַӦ��
	eSERVERCMD_ACK_POOLTIMEOUT,			   //0x8010. ��ѯ��ʱʱ��Ӧ��
	eSERVERCMD_ACK_POOLCOUNT1, 	    	   //0x8011. �����������ѯ����Ӧ��֡
	eSERVERCMD_ACK_POOLCOUNT2,			   //0x8012. ����������ѯ����������£���ѯ����Ӧ��֡
	eSERVERCMD_ACK_RESETIME,			   //0x8013. ���������ն˿�������λʱ��Ӧ��֡
	eSERVERCMD_ACK_WIDTIME,                //0x8014.Zigbeeģ�鿴�Ź���λʱ��Ӧ��
	eSERVERCMD_ACK_MACFLAG,	               //0x8015. �㲥��ѯZIG_MAC��ַӦ���־Ӧ��
	eSERVERCMD_ACK_HDADDR, 			       //0x8016. Ӳ����ַӦ��֡
	eSERVERCMD_ACK_SYSMAX,
	
	eSERVERCMD_HEART_V0 = 0xA000,          //0xA000. V0����������
	eSERVERCMD_ACK_ALLADDR,		           //0xA001. ·�������豸��ַӦ��
	eSERVERCMD_ACK_BULK, 		           //0xA002. �ƿ���״̬Ӧ��
	eSERVERCMD_ACK_POWER,		           //0xA003. �ƹ���Ӧ��
	eSERVERCMD_ACK_PARAM,		           //0xA004. ��һǶ��ʽģ�����Ӧ��
	eSERVERCMD_ACK_BULKSTATE,			   //0xA005. ��״̬Ӧ��
	eSERVERCMD_ACK_ADVALUE,			       //0xA006. ADֵӦ��	
	eSERVERCMD_ACK_STARTUPMODE,            //0XA007.�ƾ�������ʽӦ��֡
	eSERVERCMD_ACK_BULKSETACK,		       //0xA008. ��������Ӧ��
	eSERVERCMD_ACK_BULKONCURACK, 	       //0xA009. ���صƵ�����ֵӦ��
	eSERVERCMD_ACK_VOLTAGE,		           //0xA00A. �ն˵�ѹ����Ӧ��
	eSERVERCMD_ACK_CURRENT,		           //0xA00B. �ն˵�������Ӧ��
	eSERVERCMD_ACK_TEMPERATURE,	           //0xA00C. �ն��¶Ȳ���Ӧ��
	eSERVERCMD_ACK_VOLCUR, 		           //0xA00D. �ն˵�ѹ����������Ӧ��

	eSERVERCMD_ACK_PURIFIERREQ = 0xA100,   //0xA100. ��������Ŀ��ȡ����Ӧ��
	
	eSERVERCMD_ACK_BMM_SYSPARAM_REQ = 0xA200,   //0xA200. BMM����Ӧ��֡
	eSERVERCMD_ACK_BMM_STATE1_REQ,              //0xA201. BMM״̬1Ӧ��֡
	eSERVERCMD_ACK_BMM_STATE2_REQ,		        //0xA202. BMM״̬2Ӧ��֡
	
	eSERVERCMD_ACK_DMX_WRITE_SDHC = 0xA300,
	eSERVERCMD_ACK_DMX_DETEC_DEV,


	eSERVERCMD_ACK_WSMCARRIER = 0xB000,	   //0xB000. ��������Ӫ��Ӧ��֡
	eSERVERCMD_ACK_WSMVOL, 	               //0xB001. ��������ѹ������ѯӦ��֡
	eSERVERCMD_ACK_WSMCUR,		           //0xB002. ����������������ѯӦ��֡
	eSERVERCMD_ACK_WSMSWITCHIN,			   //0xB003. �����������������ѯӦ��֡
	eSERVERCMD_ACK_WSMSWITCHOUT, 		   //0xB004. �����������������ѯӦ��֡
	eSERVERCMD_ACK_WSMCOMPILETIME,		   //0xB005. ����������ʱ���ѯӦ��֡
	eSERVERCMD_ACK_WSMSDDATA, 	           //0xB006. ������SD�����ݲ�ѯӦ��֡
	eSERVERCMD_ACK_WSMHEARTATTR,		   //0xB007. ���������������Բ�ѯӦ��֡
	eSERVERCMD_ACK_TIMECONTROL, 		   //0xB008. ������ʱ�ο��Ʋ�ѯӦ��֡
	eSERVERCMD_ACK_WSMPOLLATTR,		       //0xB009. ��������ѯ���Բ�ѯӦ��֡
	eSERVERCMD_ACK_WSMDEVGROUP, 		   //0xB00A. �������ն˷����ѯӦ��֡
	eSERVERCMD_ACK_WSMVOLNUM, 	           //0xB00B. ��������������ѹ��Ų�ѯӦ��֡
	eSERVERCMD_ACK_WSMCURNUM,			   //0xB00C. ������������������Ų�ѯӦ��֡
	eSERVERCMD_ACK_WSMSWITCHINNUM,		   //0xB00D. ������������������������Ų�ѯӦ��֡
	eSERVERCMD_ACK_WSMSWITCHOUTNUM,		   //0xB00E. �����������������������Ų�ѯӦ��֡
	eSERVERCMD_ACK_METERPOW, 	           //0xB00F. ��������ȡ������Ӧ��֡
	eSERVERCMD_ACK_APPMAX,

	eSERVERCMD_HEART_V1 = 0xC000,          //0xC000. V1����������
	eSERVERCMD_HEART_V2,		           //0xC001. V2����������
	
	eSERVERCMD_HEART_AIRCLEANER = 0xC100,  //0xC100. ����������������
	eSERVERCMD_HEART_PURIFIER_V1,          //0xC101. ���������豸Purifier V1������
	
	eSERVERCMD_HEART_BMM_V1 = 0xC200,      //0xC200. BMM�豸������V1
	
	eSERVERCMD_HEART_DMX_V1 = 0xC300,      //0xC300. DMX�豸������V1

	eSERVERCMD_APP_MAX
};

//�������������ͨ��֡ACK����
enum eSERVERACKType
{
    eSERVERACK_ERR_NONE = 0,               //0x0000. ��ȷ����
    eSERVERACK_ERR_CHK,                    //0x0001. У��ʹ���
	eSERVERACK_ERR_HEAD, 			       //0x0002. ֡ͷ����
	eSERVERACK_ERR_END,				       //0x0003. ֡β����
	eSERVERACK_ERR_LEN,			           //0x0004. ���ȴ���
	eSERVERACK_ERR_CMD,			           //0x0005, �����ִ���
	eSERVERACK_ERR_ADDR, 			       //0x0006, ��ַ����
	eSERVERACK_ERR_ADDREQ,	               //0x0007, ��ѯ��ַʧ��
	eSERVERACK_ERR_ADDSET,		           //0x0008, ���õ�ַʧ��
	eSERVERACK_ERR_CHREQ,		           //0x0009, ��ѯͨ��ʧ��
	eSERVERACK_ERR_CHSET,		           //0x000A, ����ͨ��ʧ��
	eSERVERACK_ERR_ROUTREQ, 		       //0x000B, ��ѯ·�ɹ���ʧ��
	eSERVERACK_ERR_ROUTSET, 		       //0x000C, ����·�ɹ���ʧ��
	eSERVERACK_ERR_TRANUMREQ,		       //0x000D. ���ת��������ѯʧ��
	eSERVERACK_ERR_TRANUMSET,		       //0x000E. ���ת����������ʧ��
	eSERVERACK_ERR_CSMAREQ,		           //0x000F, ��ѯCSMA/CA����ʧ��
	eSERVERACK_ERR_CSMASET,		           //0x0010, ����CSMA/CA����ʧ��
	eSERVERACK_ERR_HEARTTIMEREQ,	       //0x0011, ��ѯ�������ϴ�ʱ��ʧ��
	eSERVERACK_ERR_HEARTTIMESET,	       //0x0012, �����������ϴ�ʱ��ʧ��
	eSERVERACK_ERR_POLLTIMEREQ,		       //0x0013, ��ѯ�������ϴ�ʱ��ʧ��
	eSERVERACK_ERR_POLLTIMESET,		       //0x0014, �����������ϴ�ʱ��ʧ��
	eSERVERACK_ERR_IPPORTREQ,		       //0x0015, ��ѯ������IPPortʧ��
	eSERVERACK_ERR_IPPORTSET,		       //0x0016, ���÷�����IPPortʧ��
	eSERVERACK_ERR_SIMNUMREQ, 		       //0x0017. ��ѯ������SIM����ʧ��
	eSERVERACK_ERR_MOBILEALARMREQ, 	       //0x0018. �ֻ��澯���ܿ��ز�ѯʧ��
	eSERVERACK_ERR_MOBILEALARMSET,	       //0x0019. �ֻ��澯���ܿ�������ʧ��
	eSERVERACK_ERR_MOBILEALARMNUMREQ,	   //0x001A. �ֻ��澯���ܿ��ز�ѯʧ��
	eSERVERACK_ERR_MOBILEALARMNUMSET,	   //0x001B. �ֻ��澯���ܿ�������ʧ��
	eSERVERACK_ERR_VERREQ,			       //0x001C, �汾��Ϣ��ѯʧ��
	eSERVERACK_ERR_RESET,			       //0x001D, ��λ�豸ʧ��
	eSERVERACK_ERR_INDIFLAGREQ,            //0x001E. �����ϴ���־��ѯ����
	eSERVERACK_ERR_INDIFLAGSET,            //0x001F. �����ϴ���־���ô���
	eSERVERACK_ERR_TIMECONSET, 		       //0x0020. ʱ�ο������ô���
	eSERVERACK_ERR_PHONEALARM,			   //0x0021. �ֻ��澯���ʹ���
	eSERVERACK_ERR_BROADSETTIMEREQ,		   //0x0022. �������㲥��������ִ�д�����ѯ����
	eSERVERACK_ERR_BROADSETTIMESET, 	   //0x0023. �������㲥��������ִ�д������ô���
	eSERVERACK_ERR_MACREQ,		           //0x0024, ͨ��ZIG_IP��ַ��ѯZIG_MAC��ַʧ��
	eSERVERACK_ERR_MACIPREQ,		       //0x0025, ͨ��ZIG_MAC��ַ��ѯZIG_IP��ַʧ��
	eSERVERACK_ERR_MACIPSET,		       //0x0026, ͨ��ZIG_MAC��ַ����ZIG_IP��ַʧ��
	eSERVERACK_ERR_POLLTIMEOUTREQ,		   //0x0027, ��ѯ������ÿ����ѯ��ʱʱ��ʧ��
	eSERVERACK_ERR_POLLTIMEOUTSET,		   //0x0028, ����������ÿ����ѯ��ʱʱ��ʧ��
	eSERVERACK_ERR_POLLCOUNT1REQ,		   //0x0029, ��ѯ�����������ѯ����ʧ��
	eSERVERACK_ERR_POLLCOUNT1SET,		   //0x002A, ���������������ѯ����ʧ��
	eSERVERACK_ERR_POLLCOUNT2REQ,		   //0x002B, ��ѯ����������ѯ����������£���ѯ����ʧ��
	eSERVERACK_ERR_POLLCOUNT2SET,		   //0x002C, ��������������ѯ����������£���ѯ����ʧ��
	eSERVERACK_ERR_RESETIMEREQ, 		   //0x002D, ��ѯ��������λʱ��ʧ��
	eSERVERACK_ERR_WDITIMEREQ,             //0x002E. Zigbeeģ��WDI��ʱֵ��ѯ����
	eSERVERACK_ERR_WDITIMESET,	           //0x002F. Zigbeeģ��WDI��ʱֵ��ѯ����
	eSERVERACK_ERR_MACFLAGREQ,	           //0x0030, �㲥��ѯZIG_MAC��ַӦ�𿪹ز�ѯʧ��
	eSERVERACK_ERR_MACFLAGSET,	           //0x0031, �㲥��ѯZIG_MAC��ַӦ�𿪹�����ʧ��
	eSERVERACK_ERR_HDADDRREQ,			   //0x0032, Ӳ����ַ��ѯʧ��
	eSERVERACK_ERR_HDADDRSET,			   //0x0033, Ӳ����ַ����ʧ��
	eSERVERACK_ERR_SYSMAX,
	
	eSERVERACK_ERR_ALLADDRREQ = 0x1000,	   //0x1000, ·�����е�ַ��ѯʧ��
	eSERVERACK_ERR_ADDDEV,			       //0x1001, ����·���豸ʧ��
	eSERVERACK_ERR_DELDEV,				   //0x1002, ɾ��·���豸ʧ��
	eSERVERACK_ERR_BULKREQ,				   //0x1003, �ƿ���״̬��ѯʧ��
	eSERVERACK_ERR_BULKSET,				   //0x1004, �ƿ���״̬����ʧ��
	eSERVERACK_ERR_POWERREQ,		       //0x1005. �ƹ��ʲ�ѯʧ��
	eSERVERACK_ERR_POWERSET,			   //0x1006. �ƹ�������ʧ��
	eSERVERACK_ERR_PARAMREQ,	           //0x1007. ��һǶ��ʽģ�������ѯ����
	eSERVERACK_ERR_PARAMSET,	           //0x1008. ��һǶ��ʽģ��������ô���
	eSERVERACK_ERR_BULKSTATEREQ,           //0x1009. ��״̬��ѯ����
	eSERVERACK_ERR_ADVALUEREQ,		       //0x100A. ADֵ��ѯ����
	eSERVERACK_ERR_FUPINLEDSET,			   //0x100B. ��ƽLED��ʾ��������ʧ��
	eSERVERACK_ERR_STARTUPMODEREQ,         //0x100C. �ƾ�������ʽ��ѯʧ��
	eSERVERACK_ERR_STARTUPMODESET,         //0x100D. �ƾ�������ʽ����ʧ��
	eSERVERACK_ERR_BULKSETREQ,             //0x100E. ���ص����ò�ѯʧ��
	eSERVERACK_ERR_BULKONCURREQ, 	       //0x100F. ���صƵ�����ֵ��ѯʧ��
	eSERVERACK_ERR_BULKONCURSET,	       //0x1010. ���صƵ�����ֵ����ʧ��
	eSERVERACK_ERR_VOLTAGE,	               //0x1011. �ն˵�ѹ������ѯʧ��
	eSERVERACK_ERR_CURRENT, 	           //0x1012. �ն˵���������ѯʧ��
	eSERVERACK_ERR_TEMPERATURE,            //0x1013. �ն��¶Ȳ�����ѯʧ��
	eSERVERACK_ERR_VOLCUR, 		           //0x1014. �ն˵�ѹ����������ѯʧ��
	
	eSERVERACK_ERR_PURIFIERREQ = 0x1100,   //0x1100. ��������Ŀ��ȡ����ʧ��
	eSERVERACK_ERR_PURIFIERSET,            //0x1101. ��������Ŀ���ò���ʧ��
	eSERVERACK_ERR_ELECTRICSET, 		   //0x1102. ����ģ�����ò���ʧ��
	
	eSERVERACK_ERR_BMM_SYSPARAM_REQ = 0x1200,   //0x1200. BMM������ѯӦ��ʧ��
	eSERVERACK_ERR_BMM_SYSPARAM_SET, 		    //0x1201. BMM��������Ӧ��ʧ��
	eSERVERACK_ERR_BMM_STATE1_REQ,			    //0x1202. BMM״̬1��ѯӦ��ʧ��
	eSERVERACK_ERR_BMM_STATE2_REQ,			    //0x1203. BMM״̬2��ѯӦ��ʧ��
	
	eSERVERACK_ERR_DMX_WRITE_SDHC = 0x1300,     //0x1300. дSD������Ӧ��ʧ��
	eSERVERACK_ERR_DMX_DETEC_DEV,               //0x1301. ����豸����Ӧ��ʧ��


	eSERVERACK_ERR_WSMCARRIERREQ = 0x2000, //0x2000, ��������Ӫ�̲�ѯʧ��
	eSERVERACK_ERR_WSMCARRIERSET, 	       //0x2001. ��������Ӫ������ʧ��
	eSERVERACK_ERR_WSMVOLREQ,	           //0x2002, ��������ѹ������ѯʧ��
	eSERVERACK_ERR_WSMCURREQ,		       //0x2003. ����������������ѯʧ��
	eSERVERACK_ERR_WSMSWITCHINREQ,		   //0x2004. �����������������ѯʧ��
	eSERVERACK_ERR_WSMSWITCHOUTREQ,		   //0x2005. �����������������ѯʧ��
	eSERVERACK_ERR_WSMSWITCHOUTSET, 	   //0x2006. �������������������ʧ��
	eSERVERACK_ERR_WSMCOMPILETIMEREQ, 	   //0x2007. ����������ʱ���ѯʧ��
	eSERVERACK_ERR_WSMSDDATAREQ,	       //0x2008. ������SD�����ݲ�ѯʧ��
	eSERVERACK_ERR_WSMHEARTATTRREQ,		   //0x2009. ���������������Բ�ѯʧ��
	eSERVERACK_ERR_WSMHEARTATTRSET,		   //0x200A. ��������������������ʧ��
	eSERVERACK_ERR_SYSTIMESET,		       //0x200B. ��������ʱʧ��
	eSERVERACK_ERR_TIMECONTROLREQ,		   //0x200C. ������ʱ�ο��Ʋ�ѯʧ��
	eSERVERACK_ERR_TIMECONTROLSET,		   //0x200D. ������ʱ�ο�������ʧ��
	eSERVERACK_ERR_WSMPOLLATTRREQ, 	       //0x200E. ��������ѯ���Բ�ѯʧ��
	eSERVERACK_ERR_WSMPOLLATTRSET, 	       //0x200F. ��������ѯ��������ʧ��
	eSERVERCMD_ERR_REQ_WSMDEVGROUP, 	   //0x2010. �������ն˷����ѯʧ��
	eSERVERCMD_ERR_SET_WSMDEVGROUP, 	   //0x2011. �������ն˷�������ʧ��
	eSERVERACK_ERR_WSMVOLNUMREQ,	       //0x2012, ��������������ѹ��Ų�ѯʧ��
	eSERVERACK_ERR_WSMVOLNUMSET,		   //0x2013, ��������������ѹ�������ʧ��
	eSERVERACK_ERR_WSMCURNUMREQ,		   //0x2014. ������������������Ų�ѯʧ��
	eSERVERACK_ERR_WSMCURNUMSET,		   //0x2015. �����������������������ʧ��
	eSERVERACK_ERR_WSMSWITCHINNUMREQ,	   //0x2016. ������������������������Ų�ѯʧ��
	eSERVERACK_ERR_WSMSWITCHINNUMSET,	   //0x2017. �����������������������������ʧ��
	eSERVERACK_ERR_WSMSWITCHOUTNUMREQ,	   //0x2018. �����������������������Ų�ѯʧ��
	eSERVERACK_ERR_WSMSWITCHOUTNUMSET,	   //0x2019. ����������������������������ʧ��
	eSERVERACK_ERR_WSMPOLLTYPEREQ,	       //0x201A. ��������ѯ�ն��������Ͳ�ѯʧ��
	eSERVERACK_ERR_WSMPOLLTYPESET,	       //0x201B. ��������ѯ�ն�������������ʧ��
	eSERVERACK_ERR_REQ_METERPOW,		   //0x201C. ��������ȡ������ʧ��
	
    eSERVERACK_ERR_APPMAX                
};

//�������������ͨ��֡��ʽ
typedef struct 
{
    uint_8*    phead;       //֡ͷ   
    uint_32*   pret;        //����Ϊ�ϲ㷵��
    uint_16*   pnetid;	    //netid
    uint_8*    proute;      //·�ɷ�ʽ 
    uint_8*    pgroupid;    //���
	uint_16*   pnodeid;     //nodeid
    uint_16*   pcmd;        //������
    uint_8*    pcmdbuf;     //��������
    uint_8*    pchk;        //У����
    uint_8*    pend;        //֡β
}TServerCOMFrame;
#define SERVERCOMFRAME_MINLEN    15


//============================================================================
//������: ComInit
//��  ��: ͨ�Ų������ģ���ʼ��
//��  ��: const TComInfo* const ptcominfo��ͨ�Ų���Ϣ
//��  ��: ��
//============================================================================
void ComServerInit(TComInfo* ptcominfo);

//============================================================================
//������: ComServerProc
//��  ��: ͨ�������
//��  ��: uint_8 comtype������server����terminal��ͨ��֡
//               const uint_8* const pframe��ͨ��֡�׵�ַ
//               uint_16 framelen��pframe��Ч����
//��  ��: ������
//============================================================================
uint_8 ComServerProc(const uint_8* const pframe, uint_16 framelen);

//============================================================================
//������: ComServerAck
//��  ��: �ն˿�������������
//��  ��: uint_8 cmd, ������
//               uint_8* cmdbuf������������
//               uint_16 cmdlen��cmdbuf��Ч����
//��  ��: uint8��������
//============================================================================
uint_8 ComServerAck(uint_16 netid, uint_8 route, uint_8 groupid, uint_16 nodeid, uint_16 acktype, uint_8* packbuf, uint_16 ackbuflen, uint_32 serverret);

//============================================================================
//������: ComServerHeartProc
//��  ��: ͨ�Ų���������
//��  ��: ��
//��  ��: ��
//============================================================================
void ComServerHeartProc(void);

//============================================================================
//������: NodeAddrCheck
//��  ��: �ն˽ڵ��ַ����
//��  ��: ��
//��  ��: ��
//============================================================================
TNodeInfo* NodeAddrCheck(uint_16 nodeid);



#endif // __COM_SERVER_LIB_H__

