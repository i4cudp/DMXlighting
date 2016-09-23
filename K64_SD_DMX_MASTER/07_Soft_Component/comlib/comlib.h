/*********************************************************************
模块名  ： COMLIB
文件名  ： comlib.h
相关文件： comlib.c
文件实现功能：实现通信层功能
作者    ：刘辉
版本    ：1.0.0.0.0
----------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2012/07/25   1.0         刘辉          创建
*********************************************************************/
#ifndef __COM_LIB_H__
#define __COM_LIB_H__

// 头文件
//#include "common.h"
//#include "list.h"
#include "buflist.h"

#define GROUPID_MAX     3            //最大控制组数
#define PHONENUM_MAX    5            //最大手机号数

//ComProc返回
enum eRETVAL
{
    eRET_NONE = 0,      //0x00. 正确返回
	eRET_PARAM,			//0x01. 入口参数错误
	eRET_CHK,           //0x02. 校验和错误
	eRET_LEN,			//0x03. 长度错误
	eRET_ADDR,			//0x04. 局域网地址错误
	eRET_CMD,			//0x05, 命令字错误
	eRET_LANADDR,		//0x06, 获取局域网地址失败
	eRET_LANROUTER,		//0x07, 获取局域网路由功能失败
	eRET_LANTRANNUM, 	//0x08, 获取局域网转发跳数失败
	eRET_LANBCSMA, 	    //0x09, 获取局域网转发跳数失败
	eRET_LANRFCH,		//0x09, 获取局域网转发跳数失败
    eRET_MAX                
};

//ComType
enum eComType
{
    eCOMTYPE_SERVER = 0,          //0x00. 来自server的数据
    eCOMTYPE_TERMINAL,            //0x01. 来自terminal的数据
    eCOMTYPE_MAX                
};

//eHEARTType
enum eHEARTType
{
    eHEARTTYPE_V0 = 0,            //0. V0类型心跳包
	eHEARTTYPE_V1,			      //1. V1类型心跳包
	eHEARTTYPE_V2,				  //2. V2类型心跳包
    eHEARTTYPE_MAX                
};

//eHEARTType
enum eDEVINFOType
{
    eDEVINFOTYPE_NO_SEND = 0,       //0. 终端信息不上传
	eDEVINFOTYPE_ALL_SEND,	        //1. 所有终端信息及时上传
	eDEVINFOTYPE_CHANGE_SEND = 10,	//10~255. 终端信息变化及时上传，每隔N分钟（10~255）整体上传所有终端信息
    eDEVINFOTYPE_MAX                
};


#pragma pack(1)
typedef struct {  
	uint_8      gprsdstip[4];
	uint_16     gprsdstport;
	uint_8      gprslinktype;
	uint_8      hearttype;         //心跳包类型
	uint_32     hearttime;         //心跳包时间，单位ms
	uint_16     heartmax;          //心跳包最大长度
	uint_8      devinfonum;        //终端信息上传路数，按位与
	uint_8      devinfotype;       //终端信息上传类型
	uint_32     polltime;          //主控器轮询时间，单位ms
	uint_8      broadsettime;      //广播设置次数
	uint_32     cpuresettime;      //cpu复位时间
	uint_8      heartvolnum;       //心跳包电压组号
	uint_8      heartcurnum;       //心跳包电流组号
	uint_8      heartswitchinnum;  //心跳包开关量输入组号
	uint_8      heartswitchoutnum; //心跳包开关量输出组号
}TComGprsInfo;

#pragma pack(1)
typedef struct {  
	uint_8      enetdstip[4];
	uint_16     enetdstport;
	uint_8      enetlinktype;
	uint_8      enetsock;
	uint_8      hearttype;         //心跳包类型
	uint_32     hearttime;         //心跳包时间，单位ms
	uint_16     heartmax;          //心跳包最大长度
	uint_8      devinfonum;        //终端信息上传路数，按位与
	uint_8      devinfotype;       //终端信息上传类型
	uint_32     polltime;          //主控器轮询时间，单位ms
	uint_8      broadsettime;      //广播设置次数
	uint_32     cpuresettime;      //cpu复位时间
	uint_8      heartvolnum;       //心跳包电压组号
	uint_8      heartcurnum;       //心跳包电流组号
	uint_8      heartswitchinnum;  //心跳包开关量输入组号
	uint_8      heartswitchoutnum; //心跳包开关量输出组号
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
	uint_8        bgsmalarm;         //短信功能开关
    uint_8      phonenum;          //手机号数
	uint_8      phone[PHONENUM_MAX][11];      //手机号码，最多5个
}TComGSMInfo;
#pragma pack()

//TComInfo信息更改回调函数声明
typedef void ( *ComInfoChCallBack)(const void* const pret, uint_8 retlen, uint_8 type); //通信信息改变回调
typedef void ( *ComTermianSndCallBack)(const TBufNode* const pnode);   //通信层终端数据发送回调
typedef void ( *ComTermianDevChCallBack)(void);                        //终端控制器增加删除回调

#pragma pack(1)
typedef struct 
{
	struct list_head* pserversenddatalist;     //主控器对server发送数据链表
	struct list_head* pserversendfreelist;     //主控器对server发送空闲链表
	struct list_head* pterminalsenddatalist;   //主控器对terminal发送数据链表
	struct list_head* pterminalsendfreelist;   //主控器对terminal发送空闲链表
	struct list_head* pterminalrecvdatalist;   //主控器对terminal接收数据链表
	struct list_head* pterminalrecvfreelist;   //主控器对terminal接收空闲链表
	struct list_head* pterminalnodedatalist;   //终端设备存储数据链表
	struct list_head* pterminalnodefreelist;   //终端设备存储空闲链表
	TComENETInfo      tcomenetinfo;
	TComGprsInfo      tcomgprsinfo;
	TComLanInfo       tcomlaninfo;
	TComGSMInfo       tcomgsminfo;
	ComInfoChCallBack pcominfochcallback;
	ComTermianSndCallBack pcomterminalsndcallbak;
	ComTermianDevChCallBack pcomterminaldevchcallbak;
}TComInfo;

//节点地址状态
typedef struct {
	uint_16 onoff;	  //V1轮询记录开关灯状态，V2轮询记录电压值
	uint_16 power;    //V1轮询记录功率百分比，V2轮询记录电流值
} TNodeState;
typedef struct 
{
    uint_8      route;             //路由方式
    uint_8      groupid;           //组号
    uint_16     nodeid;            //网内地址
	uint_16     polltimeout;       //主控器每次轮询超时时间，单位ms
	uint_8      pollcount1;        //主控器每个节点最大轮询次数
	uint_8      pollcount2;        //主控器在轮询不到的情况下，轮询次数，注意pollcount2<=pollcount1
}TNodeInfoSave;
/*typedef struct 
{
    TNodeInfoSave tnodeinfosave;
    uint_8      failcount;         //查询失败次数
    uint_8      phytype;           //物理量类型，用于V2终端轮询
	TNodeState state1last;         //node第1路前一状态
	TNodeState state1now;          //node第1路当前状态
	TNodeState state2last;         //node第2路前一状态
	TNodeState state2now;          //node第2路当前状态
    struct  list_head list;
}TNodeInfo;*/
#define APP_DATALEN    30
typedef struct 
{
    TNodeInfoSave tnodeinfosave;
	uint_8      failcount;         //查询失败次数
    uint_8      phytype;           //物理量类型，用于V2终端轮询
    uint_8      data[APP_DATALEN];      
    struct  list_head list;
}TNodeInfo;
#pragma pack()


//============================================================================
//函数名: ComInit
//功  能: 通信层命令处理模块初始化
//参  数: const TComInfo* const ptcominfo，通信层信息
//返  回: 无
//============================================================================
void ComInit(const TComInfo* const ptcominfo);

//============================================================================
//函数名: ComGetLanInfo
//功  能: 获取局域网信息
//参  数: TComLanInfo* ptcomlaninfo，局域网信息
//返  回: 无
//============================================================================
uint_8 ComGetLanInfo(TComLanInfo* ptcomlaninfo, uint_32 timeout);

//============================================================================
//函数名: ComProc
//功  能: 通信命令处理
//参  数: uint_8 comtype，来自server还是terminal的通信帧
//               const uint_8* const pframe，通信帧首地址
//               uint_16 framelen，pframe有效长度
//返  回: 错误码
//============================================================================
uint_8 ComProc(uint_8 comtype, const uint_8* const pframe, uint_16 framelen);

//============================================================================
//函数名: ComHeartProc
//功  能: 通信层心跳处理
//参  数: 无
//返  回: 无
//============================================================================
void ComHeartProc(void);

//============================================================================
//函数名: ServerFrameAddEsc
//功  能: 服务器通信帧增加转义符
//参  数: const uint_8* const psrc，通信帧
//               uint_16 srclen，psrc有效长度
//               uint_8* pdst，增加转义后存放地址
//               uint_16 dstsize，pdst缓冲大小
//返  回: pdst有效长度
//============================================================================
uint_16 ServerFrameAddEsc(const uint_8* const psrc, uint_16 srclen, uint_8* pdst, uint_16 dstsize);

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
uint_16 ServerFrameDelEsc(const uint_8* const psrc, uint_16 srclen, uint_16* psrcretlen, uint_8* pdst, uint_16 dstsize);

//============================================================================
//函数名: ZigbeeWDIFeed
//功  能: zigbee模块看门狗喂狗
//参  数: 无
//返  回: 无
//============================================================================
void ZigbeeWDIFeed(void);

//============================================================================
//函数名: BroadSetBulk
//功  能: 广播开关灯设置
//参  数: 无
//返  回: 无
//============================================================================
void BroadSetBulk(uint_8 groupid, bool bFlag);


#endif // __COM_LIB_H__

