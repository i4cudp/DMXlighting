/*********************************************************************
模块名  ： GPRS
文件名  ： gprs.h
相关文件： gprs.c
文件实现功能：实现了华为GPRS模块 MG323模块底层GPRS模块驱动
版本    ：1.0.0.0.0
----------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2013/08/02    1.0              刘辉                  创建
*********************************************************************/

#ifndef _GPRS_H           
#define _GPRS_H

#ifdef __cplusplus 
extern "C" { 
#endif // __cplusplus

//1.包含头文件
//#include "string.h"		//字符串处理函数头文件
//#include "common.h"		//通用函数头文件
//#include "buflist.h"	//链表相关 操作头文件
//#include "gpio.h"
//#include "uart.h"
#include "01_app_include.h"

//GPRS或以太网
//#define GPRS_NET

/*****************************硬件参数配置****************************************/
//GPRS模块硬件接线
//#define GPRS_POWER_PORT           PORT_E  //[OUT]控制GPRS电源
//#define GPRS_POWER_PIN            (26)    //
#define GPRS_POWER                (PTC_NUM|2)
#define GPRS_POWER_ON             (1)     //GPRS上电
#define GPRS_POWER_OFF            (0)     //GPRS掉电

//#define GPRS_TERMON_PORT          PORT_E  //[OUT]控制GPRS模块的开启与关闭
//#define GPRS_TERMON_PIN           (27)
#define GPRS_TERMON               (PTC_NUM|0)
#define GPRS_TTERMON_ON           (0)     //0=打开GPRS模块
#define GPRS_TTERMON_OFF          (1)     //1=关闭GPRS模块

//#define GPRS_RESET_PORT           PORT_E  //[OUT]控制GPRS模块的复位
//#define GPRS_RESET_PIN            (28)
#define GPRS_RESET                (PTB_NUM|19)
#define GPRS_RESET_ON             (0)     //0=复位
#define GPRS_RESET_OFF            (1)     //1=平时状态

//#define GPRS_IRQ_PORT             PORT_A  //[IN] 振铃中断引脚
//#define GPRS_IRQ_PIN              (0)
#define  GPRS_IRQ                (PTC_NUM|1)

//#define GPRS_SIMDET_PORT          PORT_A  //[IN]检测SIM卡是否在位
//#define GPRS_SIMDET_PIN           (1)     //1=在位，0=不在位
#define  GPRS_SIMDET              (PTB_NUM|18)

#define GPRS_UART_BAUD            115200  //GPRS模块所用到的串口
#define GPRS_UART                 UART_2  //GPRS模块所用到的串口

//调试输出
#define _GPRS_DEBUG_STR(p)        DEBUGOUT_STR(DEBUGOUT_LEVEL_ALARM, p)
#define _GPRS_DEBUG_CHAR(p)       DEBUGOUT_CHAR(DEBUGOUT_LEVEL_ALARM, p)
#define _GPRS_DEBUG_DEC(p)        DEBUGOUT_DEC(DEBUGOUT_LEVEL_ALARM, p)
#define _GPRS_DEBUG_HEX(p)        DEBUGOUT_HEX(DEBUGOUT_LEVEL_ALARM, p)       
/*****************************硬件参数配置****************************************/

#define _GPRS_SEND_STR(p)         uart_send_string(GPRS_UART, (uint_8*)p)
#define _GPRS_SEND_N(n, p)        uart_sendN(GPRS_UART, n, (uint_8*)p)
#define _GPRS_RECV_1(p)           uart_re1(GPRS_UART, (uint_8*)p)
#define _GPRS_DISABLE_RE_INT()    uart_disable_re_int(GPRS_UART)
#define _GPRS_ENABLE_RE_INT()     uart_enable_re_int(GPRS_UART)

// 短消息编码方式
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8
#define GSM_ORIG		9      //原始数据，不用编码


//链接类型
enum GPRSLinkType
{
	GPRSLINK_CMNET = 0,     //移动
	GPRSLINK_UNINET,        //联通
	GPRSLINK_MAX
};

//GPRS相关数据的类型
enum GPRSFrameType
{
	GPRSFRAME_DATA = 0,	    //GPRS->MCU的数据帧
	GPRSFRAME_CMD,	        //GPRS->MCU的命令返回
	GPRSFRAME_MAX
};

//GPRS返回
enum eGPRSRETVAL
{
    eGPRSRET_NONE = 0,      //0x00. 正确返回
	eGPRSRET_PARAM,		    //0x01. 参数出错
	eGPRSRET_START, 		//0x02. 模块启动出错
	eGPRSRET_CLOSEECHO,     //0x03. 关闭回显出错
	eGPRSRET_DETSIM, 	    //0x04. 检查SIM卡出错
	eGPRSRET_GPRS0,		    //0x05. 注册GPRS0出错
	eGPRSRET_APN, 		    //0x06. 注册APN出错
	eGPRSRET_SOCKET,	    //0x07. 注册socket出错
	eGPRSRET_CONID,		    //0x08. 注册CONID出错
	eGPRSRET_LINKIP, 		//0x09. 链接IPPORT出错
	eGPRSRET_SISO,		    //0x0A. 链接SISO出错
	eGPRSRET_SICI,			//0x0B. 链接SICI出错
	eGPRSRET_GETCSQ,		//0x0C, 查询CSQ失败
	eGPRSRET_BROKEN,		//0x0D,GPRS断链
	eGPRSRET_TRANS,		    //0x0E,透传模式设置出错
	eGPRSRET_SNDDATA, 		//0x0F,数据发送出错
	eGPRSRET_SETCPSM,		//0x10. 设置SM出错
	eGPRSRET_SETCMGF, 		//0x11. 设置GF出错
	eGPRSRET_GETCSCA,		//0x12. 获取中心号码出错
	eGPRSRET_CMGS,		    //0x13. 发送短信长度出错
	eGPRSRET_SNDGSM,		//0x14,短信发送出错
	eGPRSRET_RECVGSM,		//0x15, 接收短信失败
	eGPRSRET_DELGSM,		//0x16, 删除短信失败
    eGPRSRET_MAX                
};

//短信删除类型
enum GSMDelType
{
	GSMDEL_INDEX = 0,       //删除指定索引位置的短信
	GSMDEL_READ,            //全部删除存储器中的已读短信
	GSMDEL_READ_SND,	    //全部删除存储器中的已读和已发送短信
	GSMDEL_READ_SND_NOSND,  //全部删除存储器中的已读、已发送和未发送短信
	GSMDEL_ALL,	            //全部删除存储器中的已读、未读、已发送和未发送短信
	GSMDEL_MAX
};

//GPRS接收数据判断回调
enum GPRSRECVDATARET
{
	GPRS_RECVDATA_OK = 0,       //数据帧正确接收结束
	GPRS_RECVDATA_ERR,          //数据帧接收错误
	GPRS_RECVDATA_ING,	        //数据帧正确但未结束
	GPRS_RECVDATA_MAX
};

//返回值:0,数据帧正确接收结束；1,数据帧接收错误；2,数据帧正确但未结束
typedef uint_8 (*GPRSRECVDATACallBack)(const TBufNode* const pnode);

//结构体声明
//1.GPRS发送和接收数据帧
typedef struct
{
	struct list_head* pgprsdatalist;   //GPRS数据缓冲链表
	struct list_head* pgprsfreelist;   //GPRS数据缓冲空 链表
	GPRSRECVDATACallBack pgprsdatacb;
}TGPRSInfo;

// 短消息参数结构，编码/解码共用
// 其中，字符串以'\0'结尾
typedef struct {
	uint_8 sca[14];			// 短消息服务中心号码(SMSC地址)
	uint_8 tpa[14];			// 目标号码或回复号码(TP-DA或TP-RA)
	uint_8 tp_pid;			// 用户信息协议标识(TP-PID)
	uint_8 tp_dcs;			// 用户信息编码方式(TP-DCS)
	uint_8 tp_scts[16];		// 服务时间戳字符串(TP_SCTS), 接收时用到
	uint_8 tp_ud[500];		// 原始用户信息(编码前或解码后的TP-UD)
	uint_8 index;			// 短消息序号，在读取时用到
}TGSM_PARAM;

//=========================================================================
//函数名称：gprs_start
//参数说明：pDataList：接收数据链表头指针地址；
//                           pFreeList:空闲链表头指针地址；
//函数返回：0=初始化成功;
//                         非0，错误返回
//功能概要：初始化用到的GPRS模块引脚，启动并设置设置GPRS模块，连接目标服务器
//       GPRS接收数据链表与接收空闲链表指针传递
//说明：
//=========================================================================
uint_8 gprs_start(struct list_head* pDataList,struct list_head* pFreeList, GPRSRECVDATACallBack pdatacallback);

//=========================================================================
//函数名称：gprs_init 
//参数说明：linktype:链接类型，移动或联通
//                           pIpPort：目标服务器的IP地址和端口号；
//函数返回：0=初始化成功;
//                         非0，错误返回
//功能概要：初始化用到的GPRS模块引脚，启动并设置设置GPRS模块，连接目标服务器
//       GPRS接收数据链表与接收空闲链表指针传递
//说明：参数IP_PORT(目标服务器的IP地址和端口号)格式例：202.195.128.106:8100，
//       SET_IP_PORT为定义的展开宏
//=========================================================================
uint_8 gprs_init(uint_8 linktype, const uint_8* ip, uint_16 port);

//=========================================================================
//函数名称：gprs_set_iptrans
//参数说明：bTrans！=0  进入透传模式       bTrans=0  退出透传模式  
//功能概要：进入和退出透明传输模式
//函数返回：0=设置成功          1=设置失败
//=========================================================================
uint_8 gprs_set_iptrans(uint_8 bTrans);

//=========================================================================
//函数名: gprs_get_csq                                                        
//功  能: 得到目前的信号质量                                                    
//参  数: pcsq:  表示信号质量的结果放在入口参数中 
//说  明:无                                                       
//=========================================================================
uint_8 gprs_get_csq(uint_8* pcsq);

//=========================================================================
//函数名称：gprs_send
//参数说明：pdata:发送数据首地址  ，len:发送数据长度
//函数返回：无
//功能概要：发送数据给目标服务器。条件是目标服务器已链接成功       
//=========================================================================
uint_8 gprs_send(uint_8* pdata,uint_16 len);

//=========================================================================
//函数名称：gprs_recv_intproc 
//参数说明：无
//函数返回：无
//功能概要：中断处理接收数据
//=========================================================================
void gprs_recv_intproc();

//=========================================================================
//函数名称：gprs_isbroken
//参数说明：判断gprs是否断链
//函数返回：无
//功能概要：发送数据给目标服务器。条件是目标服务器已链接成功       
//=========================================================================
bool gprs_isbroken();


//=========================================================================
//函数名: gsm_init                                                        
//功  能:初始化GPRS模块的短信功能                            
//参  数: void,无需传输任何参数
//返  回:0--表示GPRS模块的短信功能初始化成功                                           
//		    非0--表示GPRS模块的短信功能初始化失败
//说  明: 
//=========================================================================
uint_8 gsm_init();

//=========================================================================
//函数名: gsm_send                                                        
//功  能:短信发送(手机号码,发送的内容,号码长度和内容长度)                         
//参  数: 
//返  回:0--发送成功                                           
//		    非0--发送失败
//说  明: 
//=========================================================================
uint_8 gsm_send(const uint_8* const phone, uint_8* psrc,uint_16 srclen);

//=========================================================================
//函数名: gsm_recv                                                        
//功  能:接收未读取的短消息
//参  数: 无
//返  回: 0--短消息接收成功                                           
//		     非0--短消息接收失败                                        
//说  明: 每次只接收一条
//=========================================================================
uint_8 gsm_recv(TGSM_PARAM* ptGsmParam);

//=========================================================================
//函数名: gsm_del                                                        
//功  能:根据从函数参数中传入的参数(需要返回的手机号码,需要返回的发送的内容和给点的检索选项值)                         
//参  数: uint_8 delIndex:表示需要删除短信内容的索引号
//              uint_8 delType:
//                         0--删除指定索引位置的短信
//                         1--全部删除存储器中的已读短信
//                         2--全部删除存储器中的已读和已发送短信
//                         3--全部删除存储器中的已读、已发送和未发送短信
//                         4--全部删除存储器中的已读、未读、已发送和未发送短信
//返  回:  0--短消息删除成功                                           
//		     非0--短消息删除失败                                            
//说  明: 
//=========================================================================
uint_8 gsm_del(uint_8 delIndex, uint_8 delType);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_GPRS_H
