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
#ifndef __COM_SERVER_LIB_H__
#define __COM_SERVER_LIB_H__

// 头文件

#include "01_app_include.h"
/*****************主控器与服务器通信相关************************/
#define SERVERCOM_START_FLAG           0x53       //帧头标示
#define SERVERCOM_END_FLAG             0x54       //帧尾标示
#define SERVERCOM_ESC_FLAG             0x55       //转义字符

//主控器与服务器通信帧命令类型
enum eSERVERCMDType
{
    eSERVERCMD_REQ_ADDR = 0,               //0x0000. 地址查询
	eSERVERCMD_SET_ADDR,		           //0x0001. 地址设置
	eSERVERCMD_REQ_RFCH,		           //0x0002. 通道查询
	eSERVERCMD_SET_RFCH, 		           //0x0003. 通道设置
	eSERVERCMD_REQ_ROUTE, 		           //0x0004. 路由查询
	eSERVERCMD_SET_ROUTE,		           //0x0005. 路由设置
	eSERVERCMD_REQ_TRANUM,		           //0x0006. 最大转发跳数查询
	eSERVERCMD_SET_TRANUM,		           //0x0007. 最大转发跳数设置
	eSERVERCMD_REQ_CSMACA,		           //0x0008. CSMA/CA策略查询
	eSERVERCMD_SET_CSMACA,		           //0x0009. CSMA/CA策略设置
	eSERVERCMD_REQ_HEARTTIME,	           //0x000A. 心跳包上传时间查询
	eSERVERCMD_SET_HEARTTIME,	           //0x000B. 心跳包上传时间设置
	eSERVERCMD_REQ_POLLTIME,		       //0x000C. 主控器轮询时间查询
	eSERVERCMD_SET_POLLTIME,		       //0x000D. 主控器轮询时间设置
	eSERVERCMD_REQ_IPPORT,		           //0x000E. 服务器IP Port查询
	eSERVERCMD_SET_IPPORT,		           //0x000F. 服务器IP Port设置
	eSERVERCMD_REQ_VERSION,			       //0x0010. 版本信息查询
	eSERVERCMD_REQ_SIMNUM, 		           //0x0011. 查询主控器SIM卡号
	eSERVERCMD_REQ_MOBILEALARM, 		   //0x0012. 手机告警功能开关查询
	eSERVERCMD_SET_MOBILEALARM, 		   //0x0013. 手机告警功能开关设置
	eSERVERCMD_REQ_MOBILEALARMNUM, 		   //0x0014. 手机告警号码查询
	eSERVERCMD_SET_MOBILEALARMNUM, 		   //0x0015. 手机告警号码设置
	eSERVERCMD_RESET,				       //0x0016. 设备复位命令
	eSERVERCMD_REQ_INDIFLAG,	           //0x0017.主动上传标志查询		
	eSERVERCMD_SET_INDIFLAG,	           //0x0018.主动上传标志设置	
	eSERVERCMD_SET_TIMECON,			       //0x0019.时段控制设置	
	eSERVERCMD_PHONEALARM_DATA, 		   //0x001A.发送手机告警内容	
	eSERVERCMD_REQ_BROADSETTIME, 		   //0x001B.查询主控器广播设置命令执行次数	
	eSERVERCMD_SET_BROADSETTIME,		   //0x001C.设置主控器广播设置命令执行次数	
	eSERVERCMD_REQ_MAC,		               //0x001D. 通过ZIG_IP地址查询ZIG_MACIP地址
	eSERVERCMD_REQ_MACIP,		           //0x001E. 通过ZIG_MAC地址查询ZIG_MACIP地址
	eSERVERCMD_SET_MACIP,		           //0x001F. 通过ZIG_MAC地址设置ZIG_IP地址
	eSERVERCMD_REQ_POLLTIMEOUT,			   //0x0020. 查询主控器每次轮询超时时间
	eSERVERCMD_SET_POLLTIMEOUT, 		   //0x0021. 设置主控器每次轮询超时时间
	eSERVERCMD_REQ_POLLCOUNT1, 		       //0x0022. 查询主控器每个节点最大轮询次数
	eSERVERCMD_SET_POLLCOUNT1, 		       //0x0023. 设置主控器最大轮询次数
	eSERVERCMD_REQ_POLLCOUNT2,			   //0x0024. 查询主控器在轮询不到的情况下，轮询次数
	eSERVERCMD_SET_POLLCOUNT2,			   //0x0025. 设置主控器在轮询不到的情况下，轮询次数，注意应小于等于最大轮询次数
	eSERVERCMD_REQ_RESETIME,			   //0x0026. 查询主控器或终端控制器复位时间
	eSERVERCMD_REQ_WDITIME,                //0x0027. Zigbee模块看门狗复位时间查询
	eSERVERCMD_SET_WDITIME,	               //0x0028. Zigbee模块看门狗复位时间设置
	eSERVERCMD_REQ_MACFLAG,	               //0x0029. 广播查询ZIG_MAC地址应答标志查询
	eSERVERCMD_SET_MACFLAG,                //0x002A. 广播查询ZIG_MAC地址应答标志设置
	eSERVERCMD_MACBROADREQ,                //0x002B. 广播查询ZIG_MAC地址
	eSERVERCMD_REQ_HDADDR,	               //0x002C. 硬件地址查询
	eSERVERCMD_SET_HDADDR,                 //0x002D. 硬件地址设置
	eSERVERCMD_SYSMAX, 			   

	eSERVERCMD_REQ_ALLADDR = 0x1000,       //0x1000. 路段所有地址查询
	eSERVERCMD_ADD_DEV,		               //0x1001. 增加路段设备
	eSERVERCMD_DEL_DEV, 		           //0x1002. 删除路段设备
	eSERVERCMD_REQ_BULK,		           //0x1003. 灯开关状态查询
	eSERVERCMD_SET_BULK,		           //0x1004. 灯开关状态设置
	eSERVERCMD_REQ_POWER,		           //0x1005. 灯功率查询
	eSERVERCMD_SET_POWER,			       //0x1006. 灯功率设置
	eSERVERCMD_REQ_PARAM,		           //0x1007. 三一嵌入式模块参数查询
	eSERVERCMD_SET_PARAM,		           //0x1008. 三一嵌入式模块参数设置
	eSERVERCMD_REQ_BULKSTATE,			   //0x1009. 灯状态查询
	eSERVERCMD_REQ_ADVALUE,			       //0x100A. AD值查询
	eSERVERCMD_SET_FUPINLED, 			   //0x100B. 富平项目LED显示设置
	eSERVERCMD_MACSETBULK, 		           //0x100C. 通过MAC地址进行开关灯
	eSERVERCMD_REQ_STARTUPMODE,            //0x100D. 灯具启动方式查询
	eSERVERCMD_SET_STARTUPMODE,            //0x100E. 灯具启动方式设置
	eSERVERCMD_REQ_BULKSET,		           //0x100F. 开关灯设置查询
	eSERVERCMD_REQ_BULKONCUR, 	           //0x1010. 开关灯电流阈值查询
	eSERVERCMD_SET_BULKONCUR,	           //0x1011. 开关灯电流阈值设置
	eSERVERCMD_REQ_VOLTAGE,		           //0x1012. 终端电压采样查询
	eSERVERCMD_REQ_CURRENT, 		       //0x1013. 终端电流采样查询
	eSERVERCMD_REQ_TEMPERATURE, 	       //0x1014. 终端温度采样查询
	eSERVERCMD_REQ_VOLCUR, 		           //0x1015. 终端电压、电流采样查询

	//增加净化器项目命令
	eSERVERCMD_REQ_PURIFIER = 0x1100,      //0x1100. 净化器项目获取参数
	eSERVERCMD_SET_PURIFIER,		       //0x1101. 净化器项目设置参数
	eSERVERCMD_SET_ELECTRIC,			   //0x1102. 净化器静电模块设置参数

	
	//增加BMM项目命令
	eSERVERCMD_BMM_SYSPARAM_REQ = 0x1200,  //0x1200. BMM参数查询命令
	eSERVERCMD_BMM_SYSPARAM_SET,		   //0x1201. BMM参数设置命令
	eSERVERCMD_BMM_STATE1_REQ,		       //0x1202. BMM状态1查询命令
	eSERVERCMD_BMM_STATE2_REQ,		       //0x1203. BMM状态2查询命令
	
	//增加DMX灯光项目命令
	eSERVERCMD_DMX_WRITE_SDHC = 0x1300,    //0x1300. 写SD卡任务
	eSERVERCMD_DMX_DETEC_DEV,              //0x1301. 检测模块任务

	eSERVERCMD_REQ_WSMCARRIER = 0x2000,	   //0x2000. 主控器运营商查询
	eSERVERCMD_SET_WSMCARRIER, 	           //0x2001. 主控器运营商设置
	eSERVERCMD_REQ_WSMVOL,				   //0x2002. 主控器电压采样查询
	eSERVERCMD_REQ_WSMCUR,		           //0x2003. 主控器电流采样查询
	eSERVERCMD_REQ_WSMSWITCHIN,			   //0x2004. 主控器开关量输入查询
	eSERVERCMD_REQ_WSMSWITCHOUT,		   //0x2005. 主控器开关量输出查询
	eSERVERCMD_SET_WSMSWITCHOUT, 		   //0x2006. 主控器开关量输出设置
	eSERVERCMD_REQ_WSMCOMPILETIME,		   //0x2007. 主控器编译时间查询
	eSERVERCMD_REQ_WSMSDDATA,		       //0x2008. 主控器SD内容查询
	eSERVERCMD_REQ_WSMHEARTATTR,		   //0x2009. 主控器心跳包属性查询
	eSERVERCMD_SET_WSMHEARTATTR,		   //0x200A. 主控器心跳包属性设置
	eSERVERCMD_SET_SYSTIME, 		       //0x200B. 主控器对时
	eSERVERCMD_REQ_TIMECONTROL, 		   //0x200C. 主控器时段控制查询
	eSERVERCMD_SET_TIMECONTROL, 		   //0x200D. 主控器时段控制设置
	eSERVERCMD_REQ_WSMPOLLATTR,		       //0x200E. 主控器轮询属性查询
	eSERVERCMD_SET_WSMPOLLATTR,		       //0x200F. 主控器轮询属性设置
	eSERVERCMD_REQ_WSMDEVGROUP, 		   //0x2010. 主控器终端分组查询
	eSERVERCMD_SET_WSMDEVGROUP, 		   //0x2011. 主控器终端分组设置
	eSERVERCMD_REQ_WSMVOLNUM, 		       //0x2012. 主控器心跳包电压组号查询
	eSERVERCMD_SET_WSMVOLNUM, 		       //0x2013. 主控器心跳包电压组号设置
	eSERVERCMD_REQ_WSMCURNUM,			   //0x2014. 主控器心跳包电流组号查询
	eSERVERCMD_SET_WSMCURNUM,			   //0x2015. 主控器心跳包电流组号设置
	eSERVERCMD_REQ_WSMSWITCHINNUM,		   //0x2016. 主控器心跳包开关量输入组号查询
	eSERVERCMD_SET_WSMSWITCHINNUM,		   //0x2017. 主控器心跳包开关量输入组号设置
	eSERVERCMD_REQ_WSMSWITCHOUTNUM,		   //0x2018. 主控器心跳包开关量输出组号查询
	eSERVERCMD_SET_WSMSWITCHOUTNUM,		   //0x2019. 主控器心跳包开关量输出组号设置
	eSERVERCMD_REQ_METERPOW, 	           //0x201A. 主控器读取电表电能
	eSERVERCMD_APPMAX,

	eSERVERCMD_ACK = 0x8000,               //0x8000. 应答帧
	eSERVERCMD_ACK_ADDR,		           //0x8001. 地址应答
	eSERVERCMD_ACK_RFCH, 		           //0x8002. 通道应答
	eSERVERCMD_ACK_ROUTE,		           //0x8003. 路由应答
	eSERVERCMD_ACK_TRANUM,		           //0x8004. 最大转发跳数应答
	eSERVERCMD_ACK_CSMACA,		           //0x8005. CSMA/CA策略应答
	eSERVERCMD_ACK_HEARTTIME,		       //0x8006. 心跳包上传时间应答
	eSERVERCMD_ACK_POLLTIME, 		       //0x8007. 主控器轮询时间应答
	eSERVERCMD_ACK_IPPORT,		           //0x8008. 服务器IP Port应答
	eSERVERCMD_ACK_VERSION,	               //0x8009. 版本信息应答
	eSERVERCMD_ACK_SIMNUM,			       //0x800A. 主控器SIM卡号应答
	eSERVERCMD_ACK_MOBILEALARM,		       //0x800B. 主控器手机告警开关功能应答
	eSERVERCMD_ACK_MOBILEALARMNUM, 	       //0x800C. 主控器手机告警号码应答
	eSERVERCMD_ACK_INDIFLAG,	           //0x800D. 终端控制器主动上传标志应答
	eSERVERCMD_ACK_BROADSETTIME,		   //0x800E. 主控器广播设置命令执行次数应答
	eSERVERCMD_ACK_MACIP,		           //0x800F. MACIP地址应答
	eSERVERCMD_ACK_POOLTIMEOUT,			   //0x8010. 轮询超时时间应答
	eSERVERCMD_ACK_POOLCOUNT1, 	    	   //0x8011. 主控器最大轮询次数应答帧
	eSERVERCMD_ACK_POOLCOUNT2,			   //0x8012. 主控器在轮询不到的情况下，轮询次数应答帧
	eSERVERCMD_ACK_RESETIME,			   //0x8013. 主控器或终端控制器复位时间应答帧
	eSERVERCMD_ACK_WIDTIME,                //0x8014.Zigbee模块看门狗复位时间应当
	eSERVERCMD_ACK_MACFLAG,	               //0x8015. 广播查询ZIG_MAC地址应答标志应答
	eSERVERCMD_ACK_HDADDR, 			       //0x8016. 硬件地址应答帧
	eSERVERCMD_ACK_SYSMAX,
	
	eSERVERCMD_HEART_V0 = 0xA000,          //0xA000. V0类型心跳包
	eSERVERCMD_ACK_ALLADDR,		           //0xA001. 路段所有设备地址应答
	eSERVERCMD_ACK_BULK, 		           //0xA002. 灯开关状态应答
	eSERVERCMD_ACK_POWER,		           //0xA003. 灯功率应答
	eSERVERCMD_ACK_PARAM,		           //0xA004. 三一嵌入式模块参数应答
	eSERVERCMD_ACK_BULKSTATE,			   //0xA005. 灯状态应答
	eSERVERCMD_ACK_ADVALUE,			       //0xA006. AD值应答	
	eSERVERCMD_ACK_STARTUPMODE,            //0XA007.灯具启动方式应答帧
	eSERVERCMD_ACK_BULKSETACK,		       //0xA008. 开关设置应答
	eSERVERCMD_ACK_BULKONCURACK, 	       //0xA009. 开关灯电流阈值应答
	eSERVERCMD_ACK_VOLTAGE,		           //0xA00A. 终端电压采样应答
	eSERVERCMD_ACK_CURRENT,		           //0xA00B. 终端电流采样应答
	eSERVERCMD_ACK_TEMPERATURE,	           //0xA00C. 终端温度采样应答
	eSERVERCMD_ACK_VOLCUR, 		           //0xA00D. 终端电压、电流采样应答

	eSERVERCMD_ACK_PURIFIERREQ = 0xA100,   //0xA100. 净化器项目获取参数应答
	
	eSERVERCMD_ACK_BMM_SYSPARAM_REQ = 0xA200,   //0xA200. BMM参数应答帧
	eSERVERCMD_ACK_BMM_STATE1_REQ,              //0xA201. BMM状态1应答帧
	eSERVERCMD_ACK_BMM_STATE2_REQ,		        //0xA202. BMM状态2应答帧
	
	eSERVERCMD_ACK_DMX_WRITE_SDHC = 0xA300,
	eSERVERCMD_ACK_DMX_DETEC_DEV,


	eSERVERCMD_ACK_WSMCARRIER = 0xB000,	   //0xB000. 主控器运营商应答帧
	eSERVERCMD_ACK_WSMVOL, 	               //0xB001. 主控器电压采样查询应答帧
	eSERVERCMD_ACK_WSMCUR,		           //0xB002. 主控器电流采样查询应答帧
	eSERVERCMD_ACK_WSMSWITCHIN,			   //0xB003. 主控器开关量输入查询应答帧
	eSERVERCMD_ACK_WSMSWITCHOUT, 		   //0xB004. 主控器开关量输出查询应答帧
	eSERVERCMD_ACK_WSMCOMPILETIME,		   //0xB005. 主控器编译时间查询应答帧
	eSERVERCMD_ACK_WSMSDDATA, 	           //0xB006. 主控器SD卡内容查询应答帧
	eSERVERCMD_ACK_WSMHEARTATTR,		   //0xB007. 主控器心跳包属性查询应答帧
	eSERVERCMD_ACK_TIMECONTROL, 		   //0xB008. 主控器时段控制查询应答帧
	eSERVERCMD_ACK_WSMPOLLATTR,		       //0xB009. 主控器轮询属性查询应答帧
	eSERVERCMD_ACK_WSMDEVGROUP, 		   //0xB00A. 主控器终端分组查询应答帧
	eSERVERCMD_ACK_WSMVOLNUM, 	           //0xB00B. 主控器心跳包电压组号查询应答帧
	eSERVERCMD_ACK_WSMCURNUM,			   //0xB00C. 主控器心跳包电流组号查询应答帧
	eSERVERCMD_ACK_WSMSWITCHINNUM,		   //0xB00D. 主控器心跳包开关量输入组号查询应答帧
	eSERVERCMD_ACK_WSMSWITCHOUTNUM,		   //0xB00E. 主控器心跳包开关量输出组号查询应答帧
	eSERVERCMD_ACK_METERPOW, 	           //0xB00F. 主控器读取电表电能应答帧
	eSERVERCMD_ACK_APPMAX,

	eSERVERCMD_HEART_V1 = 0xC000,          //0xC000. V1类型心跳包
	eSERVERCMD_HEART_V2,		           //0xC001. V2类型心跳包
	
	eSERVERCMD_HEART_AIRCLEANER = 0xC100,  //0xC100. 空气净化器心跳包
	eSERVERCMD_HEART_PURIFIER_V1,          //0xC101. 空气净化设备Purifier V1心跳包
	
	eSERVERCMD_HEART_BMM_V1 = 0xC200,      //0xC200. BMM设备心跳包V1
	
	eSERVERCMD_HEART_DMX_V1 = 0xC300,      //0xC300. DMX设备心跳包V1

	eSERVERCMD_APP_MAX
};

//主控器与服务器通信帧ACK类型
enum eSERVERACKType
{
    eSERVERACK_ERR_NONE = 0,               //0x0000. 正确返回
    eSERVERACK_ERR_CHK,                    //0x0001. 校验和错误
	eSERVERACK_ERR_HEAD, 			       //0x0002. 帧头错误
	eSERVERACK_ERR_END,				       //0x0003. 帧尾错误
	eSERVERACK_ERR_LEN,			           //0x0004. 长度错误
	eSERVERACK_ERR_CMD,			           //0x0005, 命令字错误
	eSERVERACK_ERR_ADDR, 			       //0x0006, 地址错误
	eSERVERACK_ERR_ADDREQ,	               //0x0007, 查询地址失败
	eSERVERACK_ERR_ADDSET,		           //0x0008, 设置地址失败
	eSERVERACK_ERR_CHREQ,		           //0x0009, 查询通道失败
	eSERVERACK_ERR_CHSET,		           //0x000A, 设置通道失败
	eSERVERACK_ERR_ROUTREQ, 		       //0x000B, 查询路由功能失败
	eSERVERACK_ERR_ROUTSET, 		       //0x000C, 设置路由功能失败
	eSERVERACK_ERR_TRANUMREQ,		       //0x000D. 最大转发跳数查询失败
	eSERVERACK_ERR_TRANUMSET,		       //0x000E. 最大转发跳数设置失败
	eSERVERACK_ERR_CSMAREQ,		           //0x000F, 查询CSMA/CA功能失败
	eSERVERACK_ERR_CSMASET,		           //0x0010, 设置CSMA/CA功能失败
	eSERVERACK_ERR_HEARTTIMEREQ,	       //0x0011, 查询心跳包上传时间失败
	eSERVERACK_ERR_HEARTTIMESET,	       //0x0012, 设置心跳包上传时间失败
	eSERVERACK_ERR_POLLTIMEREQ,		       //0x0013, 查询心跳包上传时间失败
	eSERVERACK_ERR_POLLTIMESET,		       //0x0014, 设置心跳包上传时间失败
	eSERVERACK_ERR_IPPORTREQ,		       //0x0015, 查询服务器IPPort失败
	eSERVERACK_ERR_IPPORTSET,		       //0x0016, 设置服务器IPPort失败
	eSERVERACK_ERR_SIMNUMREQ, 		       //0x0017. 查询主控器SIM卡号失败
	eSERVERACK_ERR_MOBILEALARMREQ, 	       //0x0018. 手机告警功能开关查询失败
	eSERVERACK_ERR_MOBILEALARMSET,	       //0x0019. 手机告警功能开关设置失败
	eSERVERACK_ERR_MOBILEALARMNUMREQ,	   //0x001A. 手机告警功能开关查询失败
	eSERVERACK_ERR_MOBILEALARMNUMSET,	   //0x001B. 手机告警功能开关设置失败
	eSERVERACK_ERR_VERREQ,			       //0x001C, 版本信息查询失败
	eSERVERACK_ERR_RESET,			       //0x001D, 复位设备失败
	eSERVERACK_ERR_INDIFLAGREQ,            //0x001E. 主动上传标志查询错误
	eSERVERACK_ERR_INDIFLAGSET,            //0x001F. 主动上传标志设置错误
	eSERVERACK_ERR_TIMECONSET, 		       //0x0020. 时段控制设置错误
	eSERVERACK_ERR_PHONEALARM,			   //0x0021. 手机告警发送错误
	eSERVERACK_ERR_BROADSETTIMEREQ,		   //0x0022. 主控器广播设置命令执行次数查询错误
	eSERVERACK_ERR_BROADSETTIMESET, 	   //0x0023. 主控器广播设置命令执行次数设置错误
	eSERVERACK_ERR_MACREQ,		           //0x0024, 通过ZIG_IP地址查询ZIG_MAC地址失败
	eSERVERACK_ERR_MACIPREQ,		       //0x0025, 通过ZIG_MAC地址查询ZIG_IP地址失败
	eSERVERACK_ERR_MACIPSET,		       //0x0026, 通过ZIG_MAC地址设置ZIG_IP地址失败
	eSERVERACK_ERR_POLLTIMEOUTREQ,		   //0x0027, 查询主控器每次轮询超时时间失败
	eSERVERACK_ERR_POLLTIMEOUTSET,		   //0x0028, 设置主控器每次轮询超时时间失败
	eSERVERACK_ERR_POLLCOUNT1REQ,		   //0x0029, 查询主控器最大轮询次数失败
	eSERVERACK_ERR_POLLCOUNT1SET,		   //0x002A, 设置主控器最大轮询次数失败
	eSERVERACK_ERR_POLLCOUNT2REQ,		   //0x002B, 查询主控器在轮询不到的情况下，轮询次数失败
	eSERVERACK_ERR_POLLCOUNT2SET,		   //0x002C, 设置主控器在轮询不到的情况下，轮询次数失败
	eSERVERACK_ERR_RESETIMEREQ, 		   //0x002D, 查询主控器复位时间失败
	eSERVERACK_ERR_WDITIMEREQ,             //0x002E. Zigbee模块WDI超时值查询错误
	eSERVERACK_ERR_WDITIMESET,	           //0x002F. Zigbee模块WDI超时值查询错误
	eSERVERACK_ERR_MACFLAGREQ,	           //0x0030, 广播查询ZIG_MAC地址应答开关查询失败
	eSERVERACK_ERR_MACFLAGSET,	           //0x0031, 广播查询ZIG_MAC地址应答开关设置失败
	eSERVERACK_ERR_HDADDRREQ,			   //0x0032, 硬件地址查询失败
	eSERVERACK_ERR_HDADDRSET,			   //0x0033, 硬件地址设置失败
	eSERVERACK_ERR_SYSMAX,
	
	eSERVERACK_ERR_ALLADDRREQ = 0x1000,	   //0x1000, 路段所有地址查询失败
	eSERVERACK_ERR_ADDDEV,			       //0x1001, 增加路段设备失败
	eSERVERACK_ERR_DELDEV,				   //0x1002, 删除路段设备失败
	eSERVERACK_ERR_BULKREQ,				   //0x1003, 灯开关状态查询失败
	eSERVERACK_ERR_BULKSET,				   //0x1004, 灯开关状态设置失败
	eSERVERACK_ERR_POWERREQ,		       //0x1005. 灯功率查询失败
	eSERVERACK_ERR_POWERSET,			   //0x1006. 灯功率设置失败
	eSERVERACK_ERR_PARAMREQ,	           //0x1007. 三一嵌入式模块参数查询错误
	eSERVERACK_ERR_PARAMSET,	           //0x1008. 三一嵌入式模块参数设置错误
	eSERVERACK_ERR_BULKSTATEREQ,           //0x1009. 灯状态查询错误
	eSERVERACK_ERR_ADVALUEREQ,		       //0x100A. AD值查询错误
	eSERVERACK_ERR_FUPINLEDSET,			   //0x100B. 富平LED显示数字设置失败
	eSERVERACK_ERR_STARTUPMODEREQ,         //0x100C. 灯具启动方式查询失败
	eSERVERACK_ERR_STARTUPMODESET,         //0x100D. 灯具启动方式设置失败
	eSERVERACK_ERR_BULKSETREQ,             //0x100E. 开关灯设置查询失败
	eSERVERACK_ERR_BULKONCURREQ, 	       //0x100F. 开关灯电流阈值查询失败
	eSERVERACK_ERR_BULKONCURSET,	       //0x1010. 开关灯电流阈值设置失败
	eSERVERACK_ERR_VOLTAGE,	               //0x1011. 终端电压采样查询失败
	eSERVERACK_ERR_CURRENT, 	           //0x1012. 终端电流采样查询失败
	eSERVERACK_ERR_TEMPERATURE,            //0x1013. 终端温度采样查询失败
	eSERVERACK_ERR_VOLCUR, 		           //0x1014. 终端电压电流采样查询失败
	
	eSERVERACK_ERR_PURIFIERREQ = 0x1100,   //0x1100. 净化器项目获取参数失败
	eSERVERACK_ERR_PURIFIERSET,            //0x1101. 净化器项目设置参数失败
	eSERVERACK_ERR_ELECTRICSET, 		   //0x1102. 静电模块设置参数失败
	
	eSERVERACK_ERR_BMM_SYSPARAM_REQ = 0x1200,   //0x1200. BMM参数查询应答失败
	eSERVERACK_ERR_BMM_SYSPARAM_SET, 		    //0x1201. BMM参数设置应答失败
	eSERVERACK_ERR_BMM_STATE1_REQ,			    //0x1202. BMM状态1查询应答失败
	eSERVERACK_ERR_BMM_STATE2_REQ,			    //0x1203. BMM状态2查询应答失败
	
	eSERVERACK_ERR_DMX_WRITE_SDHC = 0x1300,     //0x1300. 写SD卡任务应答失败
	eSERVERACK_ERR_DMX_DETEC_DEV,               //0x1301. 检测设备任务应答失败


	eSERVERACK_ERR_WSMCARRIERREQ = 0x2000, //0x2000, 主控器运营商查询失败
	eSERVERACK_ERR_WSMCARRIERSET, 	       //0x2001. 主控器运营商设置失败
	eSERVERACK_ERR_WSMVOLREQ,	           //0x2002, 主控器电压采样查询失败
	eSERVERACK_ERR_WSMCURREQ,		       //0x2003. 主控器电流采样查询失败
	eSERVERACK_ERR_WSMSWITCHINREQ,		   //0x2004. 主控器开关量输入查询失败
	eSERVERACK_ERR_WSMSWITCHOUTREQ,		   //0x2005. 主控器开关量输出查询失败
	eSERVERACK_ERR_WSMSWITCHOUTSET, 	   //0x2006. 主控器开关量输出设置失败
	eSERVERACK_ERR_WSMCOMPILETIMEREQ, 	   //0x2007. 主控器编译时间查询失败
	eSERVERACK_ERR_WSMSDDATAREQ,	       //0x2008. 主控器SD卡内容查询失败
	eSERVERACK_ERR_WSMHEARTATTRREQ,		   //0x2009. 主控器心跳包属性查询失败
	eSERVERACK_ERR_WSMHEARTATTRSET,		   //0x200A. 主控器心跳包属性设置失败
	eSERVERACK_ERR_SYSTIMESET,		       //0x200B. 主控器对时失败
	eSERVERACK_ERR_TIMECONTROLREQ,		   //0x200C. 主控器时段控制查询失败
	eSERVERACK_ERR_TIMECONTROLSET,		   //0x200D. 主控器时段控制设置失败
	eSERVERACK_ERR_WSMPOLLATTRREQ, 	       //0x200E. 主控器轮询属性查询失败
	eSERVERACK_ERR_WSMPOLLATTRSET, 	       //0x200F. 主控器轮询属性设置失败
	eSERVERCMD_ERR_REQ_WSMDEVGROUP, 	   //0x2010. 主控器终端分组查询失败
	eSERVERCMD_ERR_SET_WSMDEVGROUP, 	   //0x2011. 主控器终端分组设置失败
	eSERVERACK_ERR_WSMVOLNUMREQ,	       //0x2012, 主控器心跳包电压组号查询失败
	eSERVERACK_ERR_WSMVOLNUMSET,		   //0x2013, 主控器心跳包电压组号设置失败
	eSERVERACK_ERR_WSMCURNUMREQ,		   //0x2014. 主控器心跳包电流组号查询失败
	eSERVERACK_ERR_WSMCURNUMSET,		   //0x2015. 主控器心跳包电流组号设置失败
	eSERVERACK_ERR_WSMSWITCHINNUMREQ,	   //0x2016. 主控器心跳包开关量输入组号查询失败
	eSERVERACK_ERR_WSMSWITCHINNUMSET,	   //0x2017. 主控器心跳包开关量输入组号设置失败
	eSERVERACK_ERR_WSMSWITCHOUTNUMREQ,	   //0x2018. 主控器心跳包开关量输出组号查询失败
	eSERVERACK_ERR_WSMSWITCHOUTNUMSET,	   //0x2019. 主控器心跳包开关量输出组号设置失败
	eSERVERACK_ERR_WSMPOLLTYPEREQ,	       //0x201A. 主控器轮询终端命令类型查询失败
	eSERVERACK_ERR_WSMPOLLTYPESET,	       //0x201B. 主控器轮询终端命令类型设置失败
	eSERVERACK_ERR_REQ_METERPOW,		   //0x201C. 主控器读取电表电能失败
	
    eSERVERACK_ERR_APPMAX                
};

//主控器与服务器通信帧格式
typedef struct 
{
    uint_8*    phead;       //帧头   
    uint_32*   pret;        //用于为上层返回
    uint_16*   pnetid;	    //netid
    uint_8*    proute;      //路由方式 
    uint_8*    pgroupid;    //组号
	uint_16*   pnodeid;     //nodeid
    uint_16*   pcmd;        //命令字
    uint_8*    pcmdbuf;     //命令内容
    uint_8*    pchk;        //校验字
    uint_8*    pend;        //帧尾
}TServerCOMFrame;
#define SERVERCOMFRAME_MINLEN    15


//============================================================================
//函数名: ComInit
//功  能: 通信层命令处理模块初始化
//参  数: const TComInfo* const ptcominfo，通信层信息
//返  回: 无
//============================================================================
void ComServerInit(TComInfo* ptcominfo);

//============================================================================
//函数名: ComServerProc
//功  能: 通信命令处理
//参  数: uint_8 comtype，来自server还是terminal的通信帧
//               const uint_8* const pframe，通信帧首地址
//               uint_16 framelen，pframe有效长度
//返  回: 错误码
//============================================================================
uint_8 ComServerProc(const uint_8* const pframe, uint_16 framelen);

//============================================================================
//函数名: ComServerAck
//功  能: 终端控制器发送命令
//参  数: uint_8 cmd, 命令字
//               uint_8* cmdbuf，命令字内容
//               uint_16 cmdlen，cmdbuf有效长度
//返  回: uint8，错误码
//============================================================================
uint_8 ComServerAck(uint_16 netid, uint_8 route, uint_8 groupid, uint_16 nodeid, uint_16 acktype, uint_8* packbuf, uint_16 ackbuflen, uint_32 serverret);

//============================================================================
//函数名: ComServerHeartProc
//功  能: 通信层心跳处理
//参  数: 无
//返  回: 无
//============================================================================
void ComServerHeartProc(void);

//============================================================================
//函数名: NodeAddrCheck
//功  能: 终端节点地址分配
//参  数: 无
//返  回: 无
//============================================================================
TNodeInfo* NodeAddrCheck(uint_16 nodeid);



#endif // __COM_SERVER_LIB_H__

