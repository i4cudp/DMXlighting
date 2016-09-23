/*********************************************************************
ģ����  �� GPRS
�ļ���  �� gprs.h
����ļ��� gprs.c
�ļ�ʵ�ֹ��ܣ�ʵ���˻�ΪGPRSģ�� MG323ģ��ײ�GPRSģ������
�汾    ��1.0.0.0.0
----------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2013/08/02    1.0              ����                  ����
*********************************************************************/

#ifndef _GPRS_H           
#define _GPRS_H

#ifdef __cplusplus 
extern "C" { 
#endif // __cplusplus

//1.����ͷ�ļ�
//#include "string.h"		//�ַ���������ͷ�ļ�
//#include "common.h"		//ͨ�ú���ͷ�ļ�
//#include "buflist.h"	//������� ����ͷ�ļ�
//#include "gpio.h"
//#include "uart.h"
#include "01_app_include.h"

//GPRS����̫��
//#define GPRS_NET

/*****************************Ӳ����������****************************************/
//GPRSģ��Ӳ������
//#define GPRS_POWER_PORT           PORT_E  //[OUT]����GPRS��Դ
//#define GPRS_POWER_PIN            (26)    //
#define GPRS_POWER                (PTC_NUM|2)
#define GPRS_POWER_ON             (1)     //GPRS�ϵ�
#define GPRS_POWER_OFF            (0)     //GPRS����

//#define GPRS_TERMON_PORT          PORT_E  //[OUT]����GPRSģ��Ŀ�����ر�
//#define GPRS_TERMON_PIN           (27)
#define GPRS_TERMON               (PTC_NUM|0)
#define GPRS_TTERMON_ON           (0)     //0=��GPRSģ��
#define GPRS_TTERMON_OFF          (1)     //1=�ر�GPRSģ��

//#define GPRS_RESET_PORT           PORT_E  //[OUT]����GPRSģ��ĸ�λ
//#define GPRS_RESET_PIN            (28)
#define GPRS_RESET                (PTB_NUM|19)
#define GPRS_RESET_ON             (0)     //0=��λ
#define GPRS_RESET_OFF            (1)     //1=ƽʱ״̬

//#define GPRS_IRQ_PORT             PORT_A  //[IN] �����ж�����
//#define GPRS_IRQ_PIN              (0)
#define  GPRS_IRQ                (PTC_NUM|1)

//#define GPRS_SIMDET_PORT          PORT_A  //[IN]���SIM���Ƿ���λ
//#define GPRS_SIMDET_PIN           (1)     //1=��λ��0=����λ
#define  GPRS_SIMDET              (PTB_NUM|18)

#define GPRS_UART_BAUD            115200  //GPRSģ�����õ��Ĵ���
#define GPRS_UART                 UART_2  //GPRSģ�����õ��Ĵ���

//�������
#define _GPRS_DEBUG_STR(p)        DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, p)
#define _GPRS_DEBUG_CHAR(p)       DEBUGOUT_CHAR(DEBUGOUT_LEVEL_ALARM, p)
#define _GPRS_DEBUG_DEC(p)        DEBUGOUT_DEC(DEBUGOUT_LEVEL_ALARM, p)
#define _GPRS_DEBUG_HEX(p)        DEBUGOUT_HEX(DEBUGOUT_LEVEL_ALARM, p)       
/*****************************Ӳ����������****************************************/

#define _GPRS_SEND_STR(p)         uart_send_string(GPRS_UART, (uint_8*)p)
#define _GPRS_SEND_N(n, p)        uart_sendN(GPRS_UART, n, (uint_8*)p)
#define _GPRS_RECV_1(p)           uart_re1(GPRS_UART, (uint_8*)p)
#define _GPRS_DISABLE_RE_INT()    uart_disable_re_int(GPRS_UART)
#define _GPRS_ENABLE_RE_INT()     uart_enable_re_int(GPRS_UART)

// ����Ϣ���뷽ʽ
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8
#define GSM_ORIG		9      //ԭʼ���ݣ����ñ���


//��������
enum GPRSLinkType
{
	GPRSLINK_CMNET = 0,     //�ƶ�
	GPRSLINK_UNINET,        //��ͨ
	GPRSLINK_MAX
};

//GPRS������ݵ�����
enum GPRSFrameType
{
	GPRSFRAME_DATA = 0,	    //GPRS->MCU������֡
	GPRSFRAME_CMD,	        //GPRS->MCU�������
	GPRSFRAME_MAX
};

//GPRS����
enum eGPRSRETVAL
{
    eGPRSRET_NONE = 0,      //0x00. ��ȷ����
	eGPRSRET_PARAM,		    //0x01. ��������
	eGPRSRET_START, 		//0x02. ģ����������
	eGPRSRET_CLOSEECHO,     //0x03. �رջ��Գ���
	eGPRSRET_DETSIM, 	    //0x04. ���SIM������
	eGPRSRET_GPRS0,		    //0x05. ע��GPRS0����
	eGPRSRET_APN, 		    //0x06. ע��APN����
	eGPRSRET_SOCKET,	    //0x07. ע��socket����
	eGPRSRET_CONID,		    //0x08. ע��CONID����
	eGPRSRET_LINKIP, 		//0x09. ����IPPORT����
	eGPRSRET_SISO,		    //0x0A. ����SISO����
	eGPRSRET_SICI,			//0x0B. ����SICI����
	eGPRSRET_GETCSQ,		//0x0C, ��ѯCSQʧ��
	eGPRSRET_BROKEN,		//0x0D,GPRS����
	eGPRSRET_TRANS,		    //0x0E,͸��ģʽ���ó���
	eGPRSRET_SNDDATA, 		//0x0F,���ݷ��ͳ���
	eGPRSRET_SETCPSM,		//0x10. ����SM����
	eGPRSRET_SETCMGF, 		//0x11. ����GF����
	eGPRSRET_GETCSCA,		//0x12. ��ȡ���ĺ������
	eGPRSRET_CMGS,		    //0x13. ���Ͷ��ų��ȳ���
	eGPRSRET_SNDGSM,		//0x14,���ŷ��ͳ���
	eGPRSRET_RECVGSM,		//0x15, ���ն���ʧ��
	eGPRSRET_DELGSM,		//0x16, ɾ������ʧ��
    eGPRSRET_MAX                
};

//����ɾ������
enum GSMDelType
{
	GSMDEL_INDEX = 0,       //ɾ��ָ������λ�õĶ���
	GSMDEL_READ,            //ȫ��ɾ���洢���е��Ѷ�����
	GSMDEL_READ_SND,	    //ȫ��ɾ���洢���е��Ѷ����ѷ��Ͷ���
	GSMDEL_READ_SND_NOSND,  //ȫ��ɾ���洢���е��Ѷ����ѷ��ͺ�δ���Ͷ���
	GSMDEL_ALL,	            //ȫ��ɾ���洢���е��Ѷ���δ�����ѷ��ͺ�δ���Ͷ���
	GSMDEL_MAX
};

//GPRS���������жϻص�
enum GPRSRECVDATARET
{
	GPRS_RECVDATA_OK = 0,       //����֡��ȷ���ս���
	GPRS_RECVDATA_ERR,          //����֡���մ���
	GPRS_RECVDATA_ING,	        //����֡��ȷ��δ����
	GPRS_RECVDATA_MAX
};

//����ֵ:0,����֡��ȷ���ս�����1,����֡���մ���2,����֡��ȷ��δ����
typedef uint_8 (*GPRSRECVDATACallBack)(const TBufNode* const pnode);

//�ṹ������
//1.GPRS���ͺͽ�������֡
typedef struct
{
	struct list_head* pgprsdatalist;   //GPRS���ݻ�������
	struct list_head* pgprsfreelist;   //GPRS���ݻ���� ����
	GPRSRECVDATACallBack pgprsdatacb;
}TGPRSInfo;

// ����Ϣ�����ṹ������/���빲��
// ���У��ַ�����'\0'��β
typedef struct {
	uint_8 sca[14];			// ����Ϣ�������ĺ���(SMSC��ַ)
	uint_8 tpa[14];			// Ŀ������ظ�����(TP-DA��TP-RA)
	uint_8 tp_pid;			// �û���ϢЭ���ʶ(TP-PID)
	uint_8 tp_dcs;			// �û���Ϣ���뷽ʽ(TP-DCS)
	uint_8 tp_scts[16];		// ����ʱ����ַ���(TP_SCTS), ����ʱ�õ�
	uint_8 tp_ud[500];		// ԭʼ�û���Ϣ(����ǰ�������TP-UD)
	uint_8 index;			// ����Ϣ��ţ��ڶ�ȡʱ�õ�
}TGSM_PARAM;

//=========================================================================
//�������ƣ�gprs_start
//����˵����pDataList��������������ͷָ���ַ��
//                           pFreeList:��������ͷָ���ַ��
//�������أ�0=��ʼ���ɹ�;
//                         ��0�����󷵻�
//���ܸ�Ҫ����ʼ���õ���GPRSģ�����ţ���������������GPRSģ�飬����Ŀ�������
//       GPRS����������������տ�������ָ�봫��
//˵����
//=========================================================================
uint_8 gprs_start(struct list_head* pDataList,struct list_head* pFreeList, GPRSRECVDATACallBack pdatacallback);

//=========================================================================
//�������ƣ�gprs_init 
//����˵����linktype:�������ͣ��ƶ�����ͨ
//                           pIpPort��Ŀ���������IP��ַ�Ͷ˿ںţ�
//�������أ�0=��ʼ���ɹ�;
//                         ��0�����󷵻�
//���ܸ�Ҫ����ʼ���õ���GPRSģ�����ţ���������������GPRSģ�飬����Ŀ�������
//       GPRS����������������տ�������ָ�봫��
//˵��������IP_PORT(Ŀ���������IP��ַ�Ͷ˿ں�)��ʽ����202.195.128.106:8100��
//       SET_IP_PORTΪ�����չ����
//=========================================================================
uint_8 gprs_init(uint_8 linktype, const uint_8* ip, uint_16 port);

//=========================================================================
//�������ƣ�gprs_set_iptrans
//����˵����bTrans��=0  ����͸��ģʽ       bTrans=0  �˳�͸��ģʽ  
//���ܸ�Ҫ��������˳�͸������ģʽ
//�������أ�0=���óɹ�          1=����ʧ��
//=========================================================================
uint_8 gprs_set_iptrans(uint_8 bTrans);

//=========================================================================
//������: gprs_get_csq                                                        
//��  ��: �õ�Ŀǰ���ź�����                                                    
//��  ��: pcsq:  ��ʾ�ź������Ľ��������ڲ����� 
//˵  ��:��                                                       
//=========================================================================
uint_8 gprs_get_csq(uint_8* pcsq);

//=========================================================================
//�������ƣ�gprs_send
//����˵����pdata:���������׵�ַ  ��len:�������ݳ���
//�������أ���
//���ܸ�Ҫ���������ݸ�Ŀ���������������Ŀ������������ӳɹ�       
//=========================================================================
uint_8 gprs_send(uint_8* pdata,uint_16 len);

//=========================================================================
//�������ƣ�gprs_recv_intproc 
//����˵������
//�������أ���
//���ܸ�Ҫ���жϴ����������
//=========================================================================
void gprs_recv_intproc();

//=========================================================================
//�������ƣ�gprs_isbroken
//����˵�����ж�gprs�Ƿ����
//�������أ���
//���ܸ�Ҫ���������ݸ�Ŀ���������������Ŀ������������ӳɹ�       
//=========================================================================
bool gprs_isbroken();


//=========================================================================
//������: gsm_init                                                        
//��  ��:��ʼ��GPRSģ��Ķ��Ź���                            
//��  ��: void,���贫���κβ���
//��  ��:0--��ʾGPRSģ��Ķ��Ź��ܳ�ʼ���ɹ�                                           
//		    ��0--��ʾGPRSģ��Ķ��Ź��ܳ�ʼ��ʧ��
//˵  ��: 
//=========================================================================
uint_8 gsm_init();

//=========================================================================
//������: gsm_send                                                        
//��  ��:���ŷ���(�ֻ�����,���͵�����,���볤�Ⱥ����ݳ���)                         
//��  ��: 
//��  ��:0--���ͳɹ�                                           
//		    ��0--����ʧ��
//˵  ��: 
//=========================================================================
uint_8 gsm_send(const uint_8* const phone, uint_8* psrc,uint_16 srclen);

//=========================================================================
//������: gsm_recv                                                        
//��  ��:����δ��ȡ�Ķ���Ϣ
//��  ��: ��
//��  ��: 0--����Ϣ���ճɹ�                                           
//		     ��0--����Ϣ����ʧ��                                        
//˵  ��: ÿ��ֻ����һ��
//=========================================================================
uint_8 gsm_recv(TGSM_PARAM* ptGsmParam);

//=========================================================================
//������: gsm_del                                                        
//��  ��:���ݴӺ��������д���Ĳ���(��Ҫ���ص��ֻ�����,��Ҫ���صķ��͵����ݺ͸���ļ���ѡ��ֵ)                         
//��  ��: uint_8 delIndex:��ʾ��Ҫɾ���������ݵ�������
//              uint_8 delType:
//                         0--ɾ��ָ������λ�õĶ���
//                         1--ȫ��ɾ���洢���е��Ѷ�����
//                         2--ȫ��ɾ���洢���е��Ѷ����ѷ��Ͷ���
//                         3--ȫ��ɾ���洢���е��Ѷ����ѷ��ͺ�δ���Ͷ���
//                         4--ȫ��ɾ���洢���е��Ѷ���δ�����ѷ��ͺ�δ���Ͷ���
//��  ��:  0--����Ϣɾ���ɹ�                                           
//		     ��0--����Ϣɾ��ʧ��                                            
//˵  ��: 
//=========================================================================
uint_8 gsm_del(uint_8 delIndex, uint_8 delType);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_GPRS_H
