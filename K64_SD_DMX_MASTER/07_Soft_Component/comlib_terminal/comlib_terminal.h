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
#ifndef __COM_TERMINAL_LIB_H__
#define __COM_TERMINAL_LIB_H__

// 头文件
#include "comlib.h"
#include "common.h"

//命令类型
enum eTERMINALCOMType
{
    eTERMINALCOM_DATA = 0,      //0x00. 数据帧
	eTERMINALCOM_REQ_ADDR,		//0x01. 地址查询
	eTERMINALCOM_SET_ADDR,		//0x02. 地址设置
	eTERMINALCOM_REQ_RFCH,		//0x03. 通道查询
	eTERMINALCOM_SET_RFCH, 		//0x04. 通道设置
	eTERMINALCOM_REQ_ROUTE, 	//0x05. 路由查询
	eTERMINALCOM_SET_ROUTE,		//0x06. 路由设置
	eTERMINALCOM_REQ_TRANUM,	//0x07. 最大转发跳数查询
	eTERMINALCOM_SET_TRANUM,	//0x08. 最大转发跳数设置
	eTERMINALCOM_SET_BAUD,		//0x09. 波特率设置
	eTERMINALCOM_REQ_CSMACA,	//0x0A. CSMA/CA策略查询
	eTERMINALCOM_SET_CSMACA,    //0x0B. CSMA/CA策略设置
	eTERMINALCOM_REQ_MAC,		//0x0C. 通过ZIG_IP地址查询ZIG_MAC地址
	eTERMINALCOM_REQ_MACIP,		//0x0D. 通过ZIG_MAC地址查询ZIG_MACIP地址
	eTERMINALCOM_SET_MACIP,		//0x0E. 通过ZIG_MAC地址设置ZIG_IP地址
	eTERMINALCOM_SET_MAC,		//0x0F. 设置ZIG_MAC地址
	eTERMINALCOM_REQ_MACFLAG,	//0x10. 广播查询ZIG_MAC地址应答标志查询
	eTERMINALCOM_SET_MACFLAG,   //0x11. 广播查询ZIG_MAC地址应答标志设置
	eTERMINALCOM_REQ_MACBROAD,  //0x12. 广播查询ZIG_MAC地址
	eTERMINALCOM_CMDMAX,				

	eTERMINALCOM_ACK = 0xA0,    //0xA0. 应答帧
	eTERMINALCOM_ACK_ADDR,		//0xA1. 地址应答
	eTERMINALCOM_ACK_RFCH, 		//0xA2. 通道应答
	eTERMINALCOM_ACK_ROUTE,		//0xA3. 路由应答
	eTERMINALCOM_ACK_TRANUM,	//0xA4. 最大转发跳数应答
	eTERMINALCOM_ACK_CSMACA,	//0xA5. CSMA/CA策略应答
	eTERMINALCOM_ACK_MACIP,		//0xA6. MACIP地址应答
	eTERMINALCOM_ACK_MACFLAG,	//0xA7. 广播查询ZIG_MAC地址应答标志应答
    eTERMINALCOM_ACKMAX                
};

//ACK类型
enum eTERMINALACKType
{
    eTERMINALACK_ERR_NONE = 0,      //0x00. 正确返回
    eTERMINALACK_ERR_CHK,           //0x01. 校验和错误
	eTERMINALACK_ERR_LEN,			//0x02. 长度错误
	eTERMINALACK_ERR_CMD,			//0x03, 命令字错误
	eTERMINALACK_ERR_ADDREQ,	    //0x04, 查询地址失败
	eTERMINALACK_ERR_ADDSET,		//0x05, 设置地址失败
	eTERMINALACK_ERR_CHREQ,		    //0x06, 查询通道失败
	eTERMINALACK_ERR_CHSET,		    //0x07, 设置通道失败
	eTERMINALACK_ERR_ROUTREQ, 		//0x08, 查询路由功能失败
	eTERMINALACK_ERR_ROUTSET, 		//0x09, 设置路由功能失败
	eTERMINALACK_ERR_BRTSET,		//0x0A, 设置波特率失败
	eTERMINALACK_ERR_CSMAREQ,		//0x0B, 查询CSMA/CA功能失败
	eTERMINALACK_ERR_CSMASET,		//0x0C, 设置CSMA/CA功能失败
	eTERMINALACK_ERR_MACREQ,		//0x0D, 通过ZIG_IP地址查询ZIG_MAC地址失败
	eTERMINALACK_ERR_MACIPREQ,		//0x0E, 通过ZIG_MAC地址查询ZIG_IP地址失败
	eTERMINALACK_ERR_MACIPSET,		//0x0F, 通过ZIG_MAC地址设置ZIG_IP地址失败
	eTERMINALACK_ERR_MACSET,		//0x10, ZIG_MAC地址设置失败
	eTERMINALACK_ERR_MACFLAGREQ,	//0x11, 广播查询ZIG_MAC地址应答开关查询失败
	eTERMINALACK_ERR_MACFLAGSET,	//0x12, 广播查询ZIG_MAC地址应答开关设置失败
	
    eTERMINALACK_ERR_MAX                
};

//主控器与终端控制器通信帧格式
typedef struct 
{
    uint_8*    plen;       //总长度
    uint_32*   pret;       //上层返回
    uint_16*   pnetid;     //网络号
    uint_8*    proute;     //路由方式
    uint_8*    pgroupid;   //组号
    uint_16*   pnodeid;    //节点号
    uint_8*    pcmd;	   //命令类型
	uint_8*    pbuf;       //命令内容
    uint_8*    pchk;       //累加和校验
}TTerminalCOMFrame;
#define TERMINALCOMFRAME_MINLEN    13

//命令类型
enum eTerminalAppCmd
{
    eTERMINALAPPCMD_REQ_BULK = 0,      //0x00. 灯开关状态查询
	eTERMINALAPPCMD_SET_BULK,		   //0x01. 开关灯设置
	eTERMINALAPPCMD_REQ_POWER,		   //0x02. 功率因数查询
	eTERMINALAPPCMD_SET_POWER,		   //0x03. 灯功率调节
	eTERMINALAPPCMD_REQ_PARAM,		   //0x04. 三一嵌入式模块参数查询
	eTERMINALAPPCMD_SET_PARAM,		   //0x05. 三一嵌入式模块参数设置
	eTERMINALAPPCMD_REQ_VERSION, 	   //0x06.版本信息查询		
	eTERMINALAPPCMD_REQ_BULKSTATE, 	   //0x07.灯总状态查询		
	eTERMINALAPPCMD_REQ_INDIFLAG,	   //0x08.主动上传标志查询		
	eTERMINALAPPCMD_SET_INDIFLAG,	   //0x09.主动上传标志设置	
	eTERMINALAPPCMD_RESET,	           //0x0A.终端控制器复位命令
	eTERMINALAPPCMD_ADVALUE,		   //0x0B.终端控制器读取AD值
	eTERMINALAPPCMD_REQ_RESETIME,      //0x0C.终端控制器复位时间查询
	eTERMINALAPPCMD_WDIFEED,		   //0x0D. Zigbee模块看门狗喂狗命令
	eTERMINALAPPCMD_REQ_WDITIME,       //0x0E. Zigbee模块看门狗复位时间查询
	eTERMINALAPPCMD_SET_WDITIME,	   //0x0F. Zigbee模块看门狗复位时间设置
	eTERMINALAPPCMD_MACSETBULK, 	   //0x10. 通过MAC地址进行开关灯
	eTERMINALAPPCMD_REQ_STARTUPMODE,   //0x11. 灯具启动方式查询
	eTERMINALAPPCMD_SET_STARTUPMODE,   //0x12. 灯具启动方式设置
	eTERMINALAPPCMD_REQ_BULKSET,	   //0x13. 开关灯设置查询
	eTERMINALAPPCMD_REQ_BULKONCUR, 	   //0x14. 开关灯电流阈值查询
	eTERMINALAPPCMD_SET_BULKONCUR,	   //0x15. 开关灯电流阈值设置
	eTERMINALAPPCMD_VOLTAGEREQ,		   //0x16. 电压采样查询
	eTERMINALAPPCMD_CURRENTREQ, 	   //0x17. 电流采样查询
	eTERMINALAPPCMD_TEMPERATUREREQ,    //0x18. 温度采样查询
	eTERMINALAPPCMD_V2POLL, 	       //0x19. V2轮询命令查询
	eTERMINALAPPCMD_HDADDRREQ,         //0x1A. 硬件地址查询
	eTERMINALAPPCMD_HDADDRESET, 	   //0x1B. 硬件地址设置

	
	//增加净化器项目命令
	eTERMINALCOM_PURIFIERREQ = 0x50,   //0x50. 净化器项目获取参数
	eTERMINALCOM_PURIFIERSET,		   //0x51. 净化器项目设置参数

	eTERMINALCOM_BMM_SYSPARAM_REQ = 0x58,     //0x58. BMM参数查询命令
	eTERMINALCOM_BMM_SYSPARAM_SET,            //0x59. BMM参数设置命令
	eTERMINALCOM_BMM_STATE1_REQ,		      //0x5A. BMM状态1查询命令
	eTERMINALCOM_BMM_STATE2_REQ,		      //0x5B. BMM状态2查询命令
	
	eTERMINALAPPCMD_MAX,				

	eTERMINALAPPCMD_ACK = 0xA0,        //0xA0. 应答帧
	eTERMINALAPPCMD_ACK_BULK,		   //0xA1. 地址应答
	eTERMINALAPPCMD_ACK_POWER, 		   //0xA2. 通道应答
	eTERMINALAPPCMD_ACK_PARAM,		   //0xA3. 三一嵌入式模块参数应答
	eTERMINALAPPCMD_ACK_VERSION,	   //0xA4. 版本信息应答
	eTERMINALAPPCMD_ACK_BULKSTATE,	   //0xA5. 灯状态应答
	eTERMINALAPPCMD_ACK_INDIFLAG,	   //0xA6. 主动上传标志应答
	eTERMINALAPPCMD_INDICATION,	       //0xA7. 灯状态主动上传包
	eTERMINALAPPCMD_ACK_ADVALUE, 	   //0xA8. AD值应答
	eTERMINALAPPCMD_ACK_RESETIME,      //0xA9. 终端控制器复位时间应答
	eTERMINALAPPCMD_ACK_WIDTIME,       //0XAA.看门狗复位时间应当
	eTERMINALAPPCMD_ACK_STARTUPMODE,   //0XAB.灯具启动方式应答帧
	eTERMINALAPPCMD_ACK_BULKSET,	   //0xAC. 开关设置应答
	eTERMINALAPPCMD_ACK_BULKONCUR,     //0xAD. 开关灯电流阈值应答
	eTERMINALAPPCMD_ACK_VOLTAGE,	   //0xAE. 电压采样应答
	eTERMINALAPPCMD_ACK_CURRENT,	   //0xAF. 电流采样应答
	eTERMINALAPPCMD_ACK_TEMPERATURE,   //0xB0. 温度采样应答
	eTERMINALAPPCMD_ACK_V2POLL,	       //0xB1. V2轮询命令应答
	eTERMINALAPPCMD_ACK_HDADDR,		   //0XB2.硬件地址应答

	eTERMINALAPPCMD_ACK_PURIFIERREQ = 0xD0,	 //0xD0. 净化器项目获取参数应答

	eTERMINALAPPCMD_ACK_BMM_SYSPARAM_REQ = 0xD8,    //0xD8. BMM参数应答帧
	eTERMINALAPPCMD_ACK_BMM_STATE1_REQ,             //0xD9. BMM状态1应答帧
	eTERMINALAPPCMD_ACK_BMM_STATE2_REQ,		        //0xDA. BMM状态2应答帧
	
    eTERMINALAPPCMD_ACKMAX                
};

//ACK类型
enum eTerminalAppACKType
{
    eTERMINALAppACK_NONE = 0,          //0x00. 正确返回
    eTERMINALAppACK_ERR_BULKREQ,       //0x01. 开关灯查询错误
	eTERMINALAppACK_ERR_BULKSET,	   //0x02. 开关灯设置错误
	eTERMINALAppACK_ERR_POWERREQ,      //0x03. 功率设置错误
	eTERMINALAppACK_ERR_POWERSET,	   //0x04. 功率设置错误
	eTERMINALAppACK_ERR_PARAMREQ,	   //0x05. 三一嵌入式模块参数查询错误
	eTERMINALAppACK_ERR_PARAMSET,	   //0x06. 三一嵌入式模块参数设置错误
	eTERMINALAppACK_ERR_VERREQ,	       //0x07. 版本信息查询错误
	eTERMINALAppACK_ERR_BULKSTATEREQ,  //0x08. 灯状态查询错误
	eTERMINALAppACK_ERR_INDIFLAGREQ,   //0x09. 主动上传标志查询错误
	eTERMINALAppACK_ERR_INDIFLAGSET,   //0x0A. 主动上传标志设置错误
	eTERMINALAppACK_ERR_RESET,         //0x0B. 终端控制器复位命令错误
	eTERMINALAppACK_ERR_ADVALUEREQ,	   //0x0C. 终端控制器读取AD值命令错误
	eTERMINALAppACK_ERR_RESETIMEREQ,   //0x0D. 终端控制器复位时间查询错误
	eTERMINALAppACK_ERR_WDITIMEREQ,    //0x0E. WDI超时值查询错误
	eTERMINALAppACK_ERR_WDITIMESET,	   //0x0F. WDI超时值查询错误
	eTERMINALAppACK_ERR_STARTUPMODEREQ,  //0x10. 灯具启动方式查询失败
	eTERMINALAppACK_ERR_STARTUPMODESET,  //0x11. 灯具启动方式设置失败
	eTERMINALAppACK_ERR_BULKSETREQ,      //0x12. 开关灯设置查询失败
	eTERMINALAppACK_ERR_BULKONCURREQ, 	 //0x13. 开关灯电流阈值查询失败
	eTERMINALAppACK_ERR_BULKONCURSET,	 //0x14. 开关灯电流阈值设置失败
	eTERMINALAppACK_ERR_VOLTAGEREQ,	     //0x15. 电压采样查询失败
	eTERMINALAppACK_ERR_CURRENTREQ, 	 //0x16. 电流采样查询失败
	eTERMINALAppACK_ERR_TEMPERATUREREQ,  //0x17. 温度采样查询失败
	eTERMINALAppACK_ERR_V2POLL,          //0x18. V2轮询命令失败

	eTERMINALAppACK_ERR_HDADDRREQ = 0x22,//0x22. 硬件地址查询错误
	eTERMINALAppACK_ERR_HDADDRSET, 	     //0x23. 硬件地址查询错误

	eTERMINALAppACK_ERR_PURIFIERREQ = 0x50, //0x50. 净化器项目获取参数应答失败
	eTERMINALAppACK_ERR_PURIFIERSET,    //0x51. 净化器项目设置参数应答失败

	eTERMINALAppACK_ERR_BMM_SYSPARAM_REQ = 0x58,  //0x58. BMM参数查询应答失败
	eTERMINALAppACK_ERR_BMM_SYSPARAM_SET,         //0x59. BMM参数设置应答失败
	eTERMINALAppACK_ERR_BMM_STATE1_REQ,           //0x5A. BMM状态1查询应答失败
	eTERMINALAppACK_ERR_BMM_STATE2_REQ,	          //0x5B. BMM状态2查询应答失败
	
    eTERMINALAppACK_MAX                
};

//结构声明 
typedef struct 
{
    uint_8*      plen;
	uint_8*      pcmd;
	uint_8*      pbuf;
}TTerminalAppHead;
#define TERMINALAPPFRAME_MINLEN    (TERMINALCOMFRAME_MINLEN + 2)


//广播地址
//0.0为所有网络广播地址
//X.0为网络号X内全地址广播
//X.1为网络号X内奇地址广播
//X.2为网络号X内偶地址广播
#define BROADCAST_NETID        0
#define BROADCAST_NODEID       0
#define BROADCAST_ODD_NODEID   1
#define BROADCAST_EVEN_NODEID  2
#define BROADCAST_GROUPID      0
#define BROADCAST_ODD_GROUPID  1
#define BROADCAST_EVEN_GROUPID 2

enum eLanFrameType
{                                                          //netid      nodeid      groupid
	eLANFRAMETYPE_ALL = 0,	   //0x00. 全网广播帧                                     0             0         
    eLANFRAMETYPE_ME,         //0x01. 本节点帧                                          X              Y  
    eLANFRAMETYPE_NET,        //0x02.网内广播帧                                     X              0            0
	eLANFRAMETYPE_ODDGROUP,   //0x03.网内奇数组广播帧	               	   X		   0		1
	eLANFRAMETYPE_EVENGROUP,  //0x04.网内偶数组广播帧		           X		   0		2
	eLANFRAMETYPE_GROUP,	   //0x05.网内组内广播帧		           X		   0		Z
	eLANFRAMETYPE_ODD_NET,	   //0x06.网内奇地址广播帧			   X		   1		0
	eLANFRAMETYPE_ODD_ODD,	   //0x07.网内奇数组奇地址广播帧	   X		   1		1
	eLANFRAMETYPE_ODD_EVEN,   //0x08.网内偶数组奇地址广播帧	   X		   1		2
	eLANFRAMETYPE_ODD_GROUP,  //0x09.网内组内奇地址广播帧	   X		   1		Z
	eLANFRAMETYPE_EVEN_NET,   //0x0A.网内偶地址广播帧		           X 		   2        	0
	eLANFRAMETYPE_EVEN_ODD,   //0x0B.网内奇数组偶地址广播帧    X 		   2 		1
	eLANFRAMETYPE_EVEN_EVEN,  //0x0C.网内偶数组偶地址广播帧    X 		   2 		2
	eLANFRAMETYPE_EVEN_GROUP, //0x0D.网内组内偶地址广播帧	   X 		   2		Z
	eLANFRAMETYPE_OTHER_GROUP,//0x0E. 网络内部其他组广播帧        X             0/1/2    Z1(!=Z)
	eLANFRAMETYPE_NODE_NET,   //0x0F. 网络内部其他节点帧		  X	       Y1(!=Y)
	eLANFRAMETYPE_OTHEER_NET, //0x10.其他网络帧                                     X1(!=X)

    eLANFRAMETYPE_MAX                
};	

//eZigFrameRouteType
enum eZigFrameRouteType
{
    eZIGFRAMEROUTE_V1 = 0,         //0. 1.X版路由方式
	eZIGFRAMEROUTE_NONE = 10,	   //10. 不路由
	eZIGFRAMEROUTE_FLOOD,	       //11. 泛洪路由
	eZIGFRAMEROUTE_UP,             //12. 递增路由
	eZIGFRAMEROUTE_DOWN,           //13. 递减路由
	
    eZIGFRAMEROUTE_MAX                
};

//物理量类型
enum ePHYType
{
    ePHYType_AD = 0,             //0. AD值
    ePHYType_PHY,                //1. 实际物理值
	
    ePHYType_MAX               
};


//============================================================================
//函数名: ComTerminalInit
//功  能: 通信层命令处理模块初始化
//参  数: const TComInfo* const ptcominfo，通信层信息
//返  回: 无
//============================================================================
void ComTerminalInit(TComInfo* ptcominfo);

//============================================================================
//函数名: ComTerminalSendCmd
//功  能: 终端控制器发送命令
//参  数: uint_8 cmd, 命令字
//               uint_8* cmdbuf，命令字内容
//               uint_8 cmdlen，cmdbuf有效长度
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalSendCmd(uint_16 netid, uint_8 route, uint_8 groupid,  
	uint_16 nodeid, uint_8 cmdtype, uint_8* pcmd, uint_8 cmdlen, uint_32 serverret);

//============================================================================
//函数名: ComTerminalGetLanAddr
//功  能: 获取主控器局域网地址
//参  数: uint_16* pnetid, uint_32* pnodeid，返回局域网地址
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetLanAddr(uint_16* pnetid, uint_8* proute, uint_8* pgroupid, uint_16* pnodeid, uint_32 timeout);

//============================================================================
//函数名: ComTerminalGetbRouter
//功  能: 获取主控器局域网路由功能
//参  数: bool* brouter，返回路由功能
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetbRouter(bool* brouter, uint_32 timeout);

//============================================================================
//函数名: ComTerminalGetTranNum
//功  能: 获取主控器局域网转发跳数
//参  数: uint_8* trannum，返回转发跳数
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetTranNum(uint_8* trannum, uint_32 timeout);

//============================================================================
//函数名: ComTerminalGetTranNum
//功  能: 获取主控器局域网csma功能
//参  数: uint_8* trannum，返回转发跳数
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetbCsmaca(bool* bcsmaca, uint_32 timeout);

//============================================================================
//函数名: ComTerminalGetRfCh
//功  能: 获取主控器局域网转发跳数
//参  数: uint_8* trannum，返回转发跳数
//               uint_32 timeout，超时时间
//返  回: uint8，错误码
//============================================================================
uint_8 ComTerminalGetRfCh(uint_8* rfch, uint_32 timeout);

//============================================================================
//函数名: ComTerminalProc
//功  能: 通信命令处理
//参  数: uint_8 comtype，来自server还是terminal的通信帧
//               const uint_8* const pframe，通信帧首地址
//               uint_16 framelen，pframe有效长度
//返  回: 错误码
//============================================================================
uint_8 ComTerminalProc( const uint_8* const pframe, uint_16 framelen);

//-------------------------------------------------------------------------
//函数名: LanFrameType                                                        
//功  能: 判断局域网帧类型           
//参  数: uint_16 netid，netid
//               uint_16 nodeid，nodeid
//返  回: 无                                              
//说  明: TRUE，是自己的帧；FALSE，不是自己的帧
//-------------------------------------------------------------------------
uint_8 LanFrameType(uint_16 netid, uint_16 nodeid, uint_8 groupid);

//-------------------------------------------------------------------------
//函数名: MakeCheckInfo                                                        
//功  能: 计算帧校验信息                                
//参  数: uint_8* pbuf，校验数据首地址
//		     uint_16 buflen，校验数据长度
//返  回: 校验码                                                
//说  明: (1)采用累加和校验
//-------------------------------------------------------------------------
uint_8 MakeCheckInfo(uint_8* pbuf, uint_16 buflen);

//============================================================================
//函数名: TerminalStructAddr
//功  能: 结构地址分配
//参  数: 无
//返  回: 无
//============================================================================
void TerminalAppStructAddr(TTerminalAppHead* ptapp, uint_8* pbuf);

#endif // __COM_TERMINAL_LIB_H__

