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
#ifndef __COM_LIB_H__
#define __COM_LIB_H__

// ͷ�ļ�
//#include "common.h"
//#include "list.h"
#include "buflist.h"

#define GROUPID_MAX     3            //����������
#define PHONENUM_MAX    5            //����ֻ�����

//ComProc����
enum eRETVAL
{
    eRET_NONE = 0,      //0x00. ��ȷ����
	eRET_PARAM,			//0x01. ��ڲ�������
	eRET_CHK,           //0x02. У��ʹ���
	eRET_LEN,			//0x03. ���ȴ���
	eRET_ADDR,			//0x04. ��������ַ����
	eRET_CMD,			//0x05, �����ִ���
	eRET_LANADDR,		//0x06, ��ȡ��������ַʧ��
	eRET_LANROUTER,		//0x07, ��ȡ������·�ɹ���ʧ��
	eRET_LANTRANNUM, 	//0x08, ��ȡ������ת������ʧ��
	eRET_LANBCSMA, 	    //0x09, ��ȡ������ת������ʧ��
	eRET_LANRFCH,		//0x09, ��ȡ������ת������ʧ��
    eRET_MAX                
};

//ComType
enum eComType
{
    eCOMTYPE_SERVER = 0,          //0x00. ����server������
    eCOMTYPE_TERMINAL,            //0x01. ����terminal������
    eCOMTYPE_MAX                
};

//eHEARTType
enum eHEARTType
{
    eHEARTTYPE_V0 = 0,            //0. V0����������
	eHEARTTYPE_V1,			      //1. V1����������
	eHEARTTYPE_V2,				  //2. V2����������
    eHEARTTYPE_MAX                
};

//eHEARTType
enum eDEVINFOType
{
    eDEVINFOTYPE_NO_SEND = 0,       //0. �ն���Ϣ���ϴ�
	eDEVINFOTYPE_ALL_SEND,	        //1. �����ն���Ϣ��ʱ�ϴ�
	eDEVINFOTYPE_CHANGE_SEND = 10,	//10~255. �ն���Ϣ�仯��ʱ�ϴ���ÿ��N���ӣ�10~255�������ϴ������ն���Ϣ
    eDEVINFOTYPE_MAX                
};


#pragma pack(1)
typedef struct {  
	uint_8      gprsdstip[4];
	uint_16     gprsdstport;
	uint_8      gprslinktype;
	uint_8      hearttype;         //����������
	uint_32     hearttime;         //������ʱ�䣬��λms
	uint_16     heartmax;          //��������󳤶�
	uint_8      devinfonum;        //�ն���Ϣ�ϴ�·������λ��
	uint_8      devinfotype;       //�ն���Ϣ�ϴ�����
	uint_32     polltime;          //��������ѯʱ�䣬��λms
	uint_8      broadsettime;      //�㲥���ô���
	uint_32     cpuresettime;      //cpu��λʱ��
	uint_8      heartvolnum;       //��������ѹ���
	uint_8      heartcurnum;       //�������������
	uint_8      heartswitchinnum;  //�������������������
	uint_8      heartswitchoutnum; //������������������
}TComGprsInfo;

#pragma pack(1)
typedef struct {  
	uint_8      enetdstip[4];
	uint_16     enetdstport;
	uint_8      enetlinktype;
	uint_8      enetsock;
	uint_8      hearttype;         //����������
	uint_32     hearttime;         //������ʱ�䣬��λms
	uint_16     heartmax;          //��������󳤶�
	uint_8      devinfonum;        //�ն���Ϣ�ϴ�·������λ��
	uint_8      devinfotype;       //�ն���Ϣ�ϴ�����
	uint_32     polltime;          //��������ѯʱ�䣬��λms
	uint_8      broadsettime;      //�㲥���ô���
	uint_32     cpuresettime;      //cpu��λʱ��
	uint_8      heartvolnum;       //��������ѹ���
	uint_8      heartcurnum;       //�������������
	uint_8      heartswitchinnum;  //�������������������
	uint_8      heartswitchoutnum; //������������������
}TComENETInfo;


typedef struct {
    uint_16     netid;
	uint_8      route;
	uint_8      groupid;
	uint_16     nodeid;
    uint_8      brouter;
	uint_8      transnum;
	uint_8      bcsmaca;
	uint_8      rfch;
}TComLanInfo;

typedef struct {  
	uint_8        bgsmalarm;         //���Ź��ܿ���
    uint_8      phonenum;          //�ֻ�����
	uint_8      phone[PHONENUM_MAX][11];      //�ֻ����룬���5��
}TComGSMInfo;
#pragma pack()

//TComInfo��Ϣ���Ļص���������
typedef void ( *ComInfoChCallBack)(const void* const pret, uint_8 retlen, uint_8 type); //ͨ����Ϣ�ı�ص�
typedef void ( *ComTermianSndCallBack)(const TBufNode* const pnode);   //ͨ�Ų��ն����ݷ��ͻص�
typedef void ( *ComTermianDevChCallBack)(void);                        //�ն˿���������ɾ���ص�

#pragma pack(1)
typedef struct 
{
	struct list_head* pserversenddatalist;     //��������server������������
	struct list_head* pserversendfreelist;     //��������server���Ϳ�������
	struct list_head* pterminalsenddatalist;   //��������terminal������������
	struct list_head* pterminalsendfreelist;   //��������terminal���Ϳ�������
	struct list_head* pterminalrecvdatalist;   //��������terminal������������
	struct list_head* pterminalrecvfreelist;   //��������terminal���տ�������
	struct list_head* pterminalnodedatalist;   //�ն��豸�洢��������
	struct list_head* pterminalnodefreelist;   //�ն��豸�洢��������
	TComENETInfo      tcomenetinfo;
	TComGprsInfo      tcomgprsinfo;
	TComLanInfo       tcomlaninfo;
	TComGSMInfo       tcomgsminfo;
	ComInfoChCallBack pcominfochcallback;
	ComTermianSndCallBack pcomterminalsndcallbak;
	ComTermianDevChCallBack pcomterminaldevchcallbak;
}TComInfo;

//�ڵ��ַ״̬
typedef struct {
	uint_16 onoff;	  //V1��ѯ��¼���ص�״̬��V2��ѯ��¼��ѹֵ
	uint_16 power;    //V1��ѯ��¼���ʰٷֱȣ�V2��ѯ��¼����ֵ
} TNodeState;
typedef struct 
{
    uint_8      route;             //·�ɷ�ʽ
    uint_8      groupid;           //���
    uint_16     nodeid;            //���ڵ�ַ
	uint_16     polltimeout;       //������ÿ����ѯ��ʱʱ�䣬��λms
	uint_8      pollcount1;        //������ÿ���ڵ������ѯ����
	uint_8      pollcount2;        //����������ѯ����������£���ѯ������ע��pollcount2<=pollcount1
}TNodeInfoSave;
/*typedef struct 
{
    TNodeInfoSave tnodeinfosave;
    uint_8      failcount;         //��ѯʧ�ܴ���
    uint_8      phytype;           //���������ͣ�����V2�ն���ѯ
	TNodeState state1last;         //node��1·ǰһ״̬
	TNodeState state1now;          //node��1·��ǰ״̬
	TNodeState state2last;         //node��2·ǰһ״̬
	TNodeState state2now;          //node��2·��ǰ״̬
    struct  list_head list;
}TNodeInfo;*/
#define APP_DATALEN    30
typedef struct 
{
    TNodeInfoSave tnodeinfosave;
	uint_8      failcount;         //��ѯʧ�ܴ���
    uint_8      phytype;           //���������ͣ�����V2�ն���ѯ
    uint_8      data[APP_DATALEN];      
    struct  list_head list;
}TNodeInfo;
#pragma pack()


//============================================================================
//������: ComInit
//��  ��: ͨ�Ų������ģ���ʼ��
//��  ��: const TComInfo* const ptcominfo��ͨ�Ų���Ϣ
//��  ��: ��
//============================================================================
void ComInit(const TComInfo* const ptcominfo);

//============================================================================
//������: ComGetLanInfo
//��  ��: ��ȡ��������Ϣ
//��  ��: TComLanInfo* ptcomlaninfo����������Ϣ
//��  ��: ��
//============================================================================
uint_8 ComGetLanInfo(TComLanInfo* ptcomlaninfo, uint_32 timeout);

//============================================================================
//������: ComProc
//��  ��: ͨ�������
//��  ��: uint_8 comtype������server����terminal��ͨ��֡
//               const uint_8* const pframe��ͨ��֡�׵�ַ
//               uint_16 framelen��pframe��Ч����
//��  ��: ������
//============================================================================
uint_8 ComProc(uint_8 comtype, const uint_8* const pframe, uint_16 framelen);

//============================================================================
//������: ComHeartProc
//��  ��: ͨ�Ų���������
//��  ��: ��
//��  ��: ��
//============================================================================
void ComHeartProc(void);

//============================================================================
//������: ServerFrameAddEsc
//��  ��: ������ͨ��֡����ת���
//��  ��: const uint_8* const psrc��ͨ��֡
//               uint_16 srclen��psrc��Ч����
//               uint_8* pdst������ת����ŵ�ַ
//               uint_16 dstsize��pdst�����С
//��  ��: pdst��Ч����
//============================================================================
uint_16 ServerFrameAddEsc(const uint_8* const psrc, uint_16 srclen, uint_8* pdst, uint_16 dstsize);

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
uint_16 ServerFrameDelEsc(const uint_8* const psrc, uint_16 srclen, uint_16* psrcretlen, uint_8* pdst, uint_16 dstsize);

//============================================================================
//������: ZigbeeWDIFeed
//��  ��: zigbeeģ�鿴�Ź�ι��
//��  ��: ��
//��  ��: ��
//============================================================================
void ZigbeeWDIFeed(void);

//============================================================================
//������: BroadSetBulk
//��  ��: �㲥���ص�����
//��  ��: ��
//��  ��: ��
//============================================================================
void BroadSetBulk(uint_8 groupid, bool bFlag);


#endif // __COM_LIB_H__

