
#ifndef  _ENET_H_
#define  _ENET_H_

#include "w5500.h"
#include "socket.h"
#include "01_app_include.h"
//ENET接收数据判断
enum ENETRECVDATARET
{
	ENET_RECVDATA_OK = 0,       //数据帧正确接收结束
	ENET_RECVDATA_ERR,          //数据帧接收错误
	ENET_RECVDATA_ING,	        //数据帧正确但未结束
	ENET_RECVDATA_MAX
};

//ENET相关数据的类型
enum ENETFrameType
{
	ENETFRAME_DATA = 0,	    //ENET->MCU的数据帧
	ENETFRAME_CMD,	        //ENET->MCU的命令返回
	ENETFRAME_MAX
};

//1.ENET发送和接收数据帧
typedef struct
{
	struct list_head* penetdatalist;   //ENET数据缓冲链表
	struct list_head* penetfreelist;   //ENET数据缓冲空 链表
}TENETInfo;

#define ENET_POWER     (PTB_NUM|2)    //ENET模块电源控制引脚
#define ENET_INT       (PTB_NUM|19)   //ENET模块中断控制引脚
#define ENET_RESET     (PTB_NUM|18)   //ENET模块复位控制引脚
#define SPI_PSC        (PTB_NUM|20)   //ENET模块片选控制引脚

#define E_NET

#define GPS_POWER     (PTB_NUM|9)    //GPS模块电源控制引脚

void enet_rec(SOCKET s);

#endif
