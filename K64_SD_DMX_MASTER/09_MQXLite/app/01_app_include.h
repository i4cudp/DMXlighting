#ifndef __APP_INC_H_
#define __APP_INC_H_
//---------------------------------------------------------------------------

//1.包含头文件
//#include "bsp.h"
#include "mqxlite.h"
#include "mqx_inc.h"
#include "light.h"
#include "spi.h"
#include "lwevent.h"
#include "printf.h"
#include "DMX512.h"
#include "diskio.h"
#include "znfat.h"
#include "buflist.h"
#include "camera.h"
#include "flash.h"
#include "gprs.h"
#include "common.h"
#include "light.h"
#include "uart.h"
#include "comlib_terminal.h"
#include "comlib_server.h"
#include "comlib.h"
#include "bsp.h"

#include "md5.h"
#include "socket.h"
#include "SPI2.h"
#include "w5500.h"
#include "dhcp.h"
#include "enet.h"

//2.声明全局变量
//防止全局变量重复声明的前缀处理方法
#ifdef GLOBLE_VAR             //GLOBLE_VAR在task_main.c文件中宏定义
#define G_VAR_PREFIX          //task_main.c文件中使用全局变量不加“extern”前缀
#else
#define G_VAR_PREFIX  extern  //其他文件中使用全局变量自动加“extern”前缀
#endif

///////////////////////////////////////////////////////////////////////
//宏开关
#define ZIGBEE_CTRL           1        //zigbee控制开关
#define DEBUG_OUT             1

//版本信息
#define VER_PROT_MAJOR        2        // 协议版本号，MAJOR
#define VER_PROT_MINOR        3        // 协议版本号，MINOR
#define VER_SOFT_MAJOR        3        // 软件版本号，MAJOR
#define VER_SOFT_MINOR        0        // 软件版本号，MINOR
#define VER_HARD_MAJOR        2        // 硬件版本号，MAJOR
#define VER_HARD_MINOR        0        // 硬件版本号，MINOR

#define  FeedTheWatchDog()

//zigbee复位管脚
//#define PORT_ZIG_RST              PORT_E
#define PIN_ZIG_RST               7
#define ZIG_RST_ON                0
#define ZIG_RST_OFF               1

#define CHNUM_DMX512              4

//UART
#define UART_ID_DEBUG             UART_0
#define UART_BAUD_DEBUG           115200
#define UART_ID_ZIGBEE            UART_3
#define UART_BAUD_ZIGBEE          115200

#define ZIGBEE_DISABLE_INT()    uart_disable_re_int(UART_ID_ZIGBEE)
#define ZIGBEE_ENABLE_INT()     uart_enable_re_int(UART_ID_ZIGBEE)




#define  STATE_TCP_START        0
#define  STATE_TCP_LINK         1
#define  STATE_TCP_FINISH       2



#ifdef DEBUG_OUT
#define DEBUGOUT_STR(level, ptr) \
if(DEBUGOUT_LEVEL >= level){ \
    printf(ptr);\
}

#define DEBUGOUT_CHAR(level, p) \
if(DEBUGOUT_LEVEL >= level){ \
    printf("%c", p);\
}

#define DEBUGOUT_DEC(level, n) \
if(DEBUGOUT_LEVEL >= level){ \
    printf("%d", n);\
}

#define DEBUGOUT_HEX(level, n) \
if(DEBUGOUT_LEVEL >= level){ \
    printf("0x%X", n);\
}
#else
#define DEBUGOUT_STR(level, ptr)
#define DEBUGOUT_CHAR(level, p)
#define DEBUGOUT_DEC(level, n)
#define DEBUGOUT_HEX(level, n)
#endif
inline void DEBUGOUT_TIME(uint_8 level);

#define LAN_DEFAULT_NETID       11
#define LAN_DEFAULT_NODEID      10

//ENET有关Buf的宏定义
//#define BUF_NODE_NUM_ENET_RECV             3
//#define BUF_NODE_SIZE_ENET_RECV            2500    //经过转义

//GPRS小包发送
#define BUF_NODE_NUM_COMSERVER_SEND        2
#define BUF_NODE_SIZE_COMSERVER_SEND       200
#define BUF_NODE_NUM_COMTERMINAL_SEND      1
#define BUF_NODE_SIZE_COMTERMINAL_SEND     10
#define BUF_NODE_NUM_COMTERMINAL_RECV      1
#define BUF_NODE_SIZE_COMTERMINAL_RECV     10
#define NODE_NUM_MAX                       100       //主控器存储最大终端设备数

//信息记录
//#define GPRS_DEFAULT_DSTIP1     121
//#define GPRS_DEFAULT_DSTIP2     40
//#define GPRS_DEFAULT_DSTIP3     158
//#define GPRS_DEFAULT_DSTIP4     140
//#define GPRS_DEFAULT_DSTPORT    61000

//IP地址和端口
#define GPRS_DEFAULT_DSTIP1     218
#define GPRS_DEFAULT_DSTIP2     4
#define GPRS_DEFAULT_DSTIP3     189
#define GPRS_DEFAULT_DSTIP4     26
#define GPRS_DEFAULT_DSTPORT    12024

//#define ENET_DEFAULT_DSTIP1     10
//#define ENET_DEFAULT_DSTIP2     10
//#define ENET_DEFAULT_DSTIP3     65
//#define ENET_DEFAULT_DSTIP4     100
//#define ENET_DEFAULT_DSTPORT    8084

#define ENET_DEFAULT_DSTIP1     122
#define ENET_DEFAULT_DSTIP2     112
#define ENET_DEFAULT_DSTIP3     217
#define ENET_DEFAULT_DSTIP4     197
#define ENET_DEFAULT_DSTPORT    8084

#define SOCKNUM                 SOCK1

#define GPRS_DEFAULT_HEART                 60000    //60s
#define GPRS_DEFAULT_POLL_TIME             20000    //20s 
#define GPRS_DEFAULT_POLL_TIMEOUT          3000     //3s 
#define GPRS_DEFAULT_POLL_COUNT1           10   //主控器每个节点最大轮询次数
#define GPRS_DEFAULT_POLL_COUNT2           3    //主控器在轮询不到的情况下，轮询次数，注意pollcount2<=pollcount1
#define GPRS_DEFAULT_BROAD_SET_TIME        3    //广播设置次数
#define GPRS_DEFAULT_CPU_RESET_TIME        0    //cpu默认复位时间

////GPRS或以太网
//#define GPRS_NET

#define SIZE_FLASHPAGE                     4096
#define APPINFOFLASHPAGE                   127         //主控器信息存储地址页号  最后一页
#define TERMINALINFOFLASHPAGE              124         //最大3页， 终端控制器信息存储地址页号(前2个字节存储终端控制器数量)
#define TERMIANL_CHANGE_TIME               30000       //终端控制器变化后间隔保存时间，防止频繁保存
#define GPRS_SEND_BREAK_COUNT              3           //gprs连续3个心跳包时间发送失败，重新链接
#define GPRS_LINK_FAIL_COUNT               3           //gprs连续5次链接失败，重启cpu
#define ZIG_BREAK_TIMEOUT                  300000      //zigbee连续5分钟无数据接收，重启zigbee

//GSM手机告警
#define GSM_ALRAM_ON            TRUE
#define GSM_PHONE_NUM           1
#define GSM_PHONE_DEF           "13962105621"

//APPINFOFLASHPAGE存储格式TMHCAppRecord + 时段控制数N(1B)+TMHCTIMECON*N
#pragma pack(1)
typedef struct {  
	TComENETInfo tenetinfo;
    TComGprsInfo tgprsinfo;
	TComLanInfo tlaninfo;
	TComGSMInfo tgsminfo;
}TMHCAppRecord;
#pragma pack() 

#define TMHCTIMECON_NUM_MAX     50   //每天最大时段控制数


//打印等级
#define DEBUGOUT_LEVEL_ERR      0    //在关闭情况下不打印
#define DEBUGOUT_LEVEL_ALARM    1    
#define DEBUGOUT_LEVEL_INFO     2    
#define DEBUGOUT_LEVEL_INDI     3    
#define DEBUGOUT_LEVEL_ZIG      7    
#define DEBUGOUT_LEVEL_GPRS     8   

///////////////////////////////////////////////////////////////////////


//声明全局变量（全局变量类型前一律前缀G_VAR_PREFIX）
G_VAR_PREFIX uint_8 uart_recvBuf[256];

//4个通道所使用的队列
G_VAR_PREFIX struct list_head g_tfdatalist[CHNUM_DMX512];
G_VAR_PREFIX struct list_head g_tffreelist[CHNUM_DMX512];

G_VAR_PREFIX struct list_head g_enetrecvdatalist;          //enet接收数据链表
G_VAR_PREFIX struct list_head g_smallrecvfreelist;          //enet接收空闲链表
G_VAR_PREFIX struct list_head g_largerecvfreelist;          //enet接收空闲链表

//G_VAR_PREFIX struct list_head enetrecvdatalist;          //enet接收数据链表
//G_VAR_PREFIX struct list_head enetrecvfreelist;          //enet接收空闲链表

//G_VAR_PREFIX struct FileInfo fileinfo1;

G_VAR_PREFIX struct DateTime dt1;
G_VAR_PREFIX struct znFAT_Init_Args sdInitArgs;

//extern struct list_head s_gprsrecvdatalist;          //gprs接收数据链表
//extern struct list_head s_gprsrecvfreelist;          //gprs接收空闲链表
extern struct list_head s_comserversenddatalist;     //通信层向server发送数据链表
extern struct list_head s_comterminalsenddatalist;   //通信层向terminal发送数据链表
extern struct list_head s_comserversendfreelist;     //通信层向server发送空闲链表

TMHCAppRecord s_tappinfo;   //应用记录信息（GPRS、LAN、GSM）

//缓冲大小
#define SMALL_BUF_NODE_NUM      24
#define SMALL_BUF_NODE_SIZE     1100
#define LARGE_BUF_NODE_NUM      5
#define LARGE_BUF_NODE_SIZE     1200

//命令码
#define SYN    1   //同步命令

//1.4 声明轻量级事件组
G_VAR_PREFIX LWEVENT_STRUCT  lwevent_group;  //轻量级事件组

//1.5 宏定义事件位
#define  EVENT_DMX512           ((1uL)<<(6))   //事件位
//#define  EVENT_CAMERA           ((1uL)<<(1))   //事件位
//#define  EVENT_WRITE_SDHC       ((1uL)<<(2))   //事件位
//#define  EVENT_ENET             ((1uL)<<(3))   //事件位
#define  EVENT_SDHC             ((1uL)<<(4))   //事件位
#define  EVENT_EFFECTS          ((1uL)<<(5))   //事件位
#define  EVENT_WATERLAMP        ((1uL)<<(3))   //事件位
#define  EVENT_HORSERACE        ((1uL)<<(2))   //事件位
#define  EVENT_UARTSEND         ((1uL)<<(7))   //事件位
#define  EVENT_UARTSENDISR      ((1uL)<<(8))   //事件位

//3.登记任务模板编号
#define TASK_MAIN                     1
#define TASK_LIGHT                    2
#define TASK_DMX512                   3
#define TASK_DETECT                   4
//#define TASK_CAMERA                   4
//#define TASK_GPRS                     5
//#define TASK_WRITE_SDHC               6
#define TASK_ENET                     7
#define TASK_SDHC                     8
#define TASK_EFFECTS                  9
#define TASK_WATERLAMP                5
#define TASK_HORSERACE                6
#define TASK_UARTSEND                 10
//4.为任务创建任务栈
#define TASK_MAIN_STACK_SIZE        (sizeof(TD_STRUCT) + 1024*55 + PSP_STACK_ALIGNMENT + 1)
#define TASK_LIGHT_STACK_SIZE       (sizeof(TD_STRUCT) + 1000 + PSP_STACK_ALIGNMENT + 1)
#define TASK_DMX512_STACK_SIZE      (sizeof(TD_STRUCT) + 1024 + PSP_STACK_ALIGNMENT + 1)
#define TASK_SDHC_STACK_SIZE        (sizeof(TD_STRUCT) + 1024 + PSP_STACK_ALIGNMENT + 1)
#define TASK_ENET_STACK_SIZE        (sizeof(TD_STRUCT) + 1024*15 + PSP_STACK_ALIGNMENT + 1)
#define TASK_DETECT_STACK_SIZE      (sizeof(TD_STRUCT) + 1024 + PSP_STACK_ALIGNMENT + 1)
#define TASK_EFFECTS_STACK_SIZE     (sizeof(TD_STRUCT) + 512 + PSP_STACK_ALIGNMENT + 1)
#define TASK_WATERLAMP_STACK_SIZE   (sizeof(TD_STRUCT) + 512 + PSP_STACK_ALIGNMENT + 1)
#define TASK_HORSERACE_STACK_SIZE   (sizeof(TD_STRUCT) + 512 + PSP_STACK_ALIGNMENT + 1)
#define TASK_UARTSEND_STACK_SIZE     (sizeof(TD_STRUCT) + 3000 + PSP_STACK_ALIGNMENT + 1)

G_VAR_PREFIX uint_8 task_main_stack[TASK_MAIN_STACK_SIZE];
G_VAR_PREFIX uint_8 task_light_stack[TASK_LIGHT_STACK_SIZE];
//G_VAR_PREFIX uint_8 task_waterlamp_stack[TASK_WATERLAMP_STACK_SIZE];
//G_VAR_PREFIX uint_8 task_DMX512_stack[TASK_DMX512_STACK_SIZE];
//G_VAR_PREFIX uint_8 task_sdhc_stack[TASK_SDHC_STACK_SIZE];
//G_VAR_PREFIX uint_8 task_enet_stack[TASK_ENET_STACK_SIZE];
//G_VAR_PREFIX uint_8 task_detect_stack[TASK_DETECT_STACK_SIZE];

//5.声明任务函数
void task_main(uint32_t initial_data);
void task_light(uint32_t initial_data);
void task_DMX512(uint32_t initial_data);
//void task_camera(uint32_t initial_data);
void task_sdhc(uint32_t initial_data);
//void task_GPRSData(uint32_t initial_data);
//void task_write_sdhc(uint32_t initial_data);
void task_enet(uint32_t initial_data);
void task_detect(uint32_t initial_data);
void task_effects(uint32_t initial_data);
void task_waterlamp(uint32_t initial_data);
void task_horserace(uint32_t initial_data);
void task_uartsend(uint32_t initial_data);

//6.声明中断处理函数
void UART1_RX_ISR();
void UART2_RX_ISR();
void UART3_RX_ISR();
void UART4_RX_ISR();
void GPIO_ISR();
void SPI0_ISR();


//void SPI2_ISR();

extern uint_8 DEBUGOUT_LEVEL;
extern TBufNode g_tSmallBufNode[SMALL_BUF_NODE_NUM];
extern TBufNode g_tLargeBufNode[LARGE_BUF_NODE_NUM];

//---------------------------------------------------------------------------
#endif    //app_inc.h
