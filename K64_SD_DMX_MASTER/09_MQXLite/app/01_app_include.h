#ifndef __APP_INC_H_
#define __APP_INC_H_
//---------------------------------------------------------------------------

//1.����ͷ�ļ�
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

//2.����ȫ�ֱ���
//��ֹȫ�ֱ����ظ�������ǰ׺������
#ifdef GLOBLE_VAR             //GLOBLE_VAR��task_main.c�ļ��к궨��
#define G_VAR_PREFIX          //task_main.c�ļ���ʹ��ȫ�ֱ������ӡ�extern��ǰ׺
#else
#define G_VAR_PREFIX  extern  //�����ļ���ʹ��ȫ�ֱ����Զ��ӡ�extern��ǰ׺
#endif

///////////////////////////////////////////////////////////////////////
//�꿪��
#define ZIGBEE_CTRL           1        //zigbee���ƿ���
#define DEBUG_OUT             1

//�汾��Ϣ
#define VER_PROT_MAJOR        2        // Э��汾�ţ�MAJOR
#define VER_PROT_MINOR        3        // Э��汾�ţ�MINOR
#define VER_SOFT_MAJOR        3        // ����汾�ţ�MAJOR
#define VER_SOFT_MINOR        0        // ����汾�ţ�MINOR
#define VER_HARD_MAJOR        2        // Ӳ���汾�ţ�MAJOR
#define VER_HARD_MINOR        0        // Ӳ���汾�ţ�MINOR

#define  FeedTheWatchDog()

//zigbee��λ�ܽ�
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

//ENET�й�Buf�ĺ궨��
//#define BUF_NODE_NUM_ENET_RECV             3
//#define BUF_NODE_SIZE_ENET_RECV            2500    //����ת��

//GPRSС������
#define BUF_NODE_NUM_COMSERVER_SEND        2
#define BUF_NODE_SIZE_COMSERVER_SEND       200
#define BUF_NODE_NUM_COMTERMINAL_SEND      1
#define BUF_NODE_SIZE_COMTERMINAL_SEND     10
#define BUF_NODE_NUM_COMTERMINAL_RECV      1
#define BUF_NODE_SIZE_COMTERMINAL_RECV     10
#define NODE_NUM_MAX                       100       //�������洢����ն��豸��

//��Ϣ��¼
//#define GPRS_DEFAULT_DSTIP1     121
//#define GPRS_DEFAULT_DSTIP2     40
//#define GPRS_DEFAULT_DSTIP3     158
//#define GPRS_DEFAULT_DSTIP4     140
//#define GPRS_DEFAULT_DSTPORT    61000

//IP��ַ�Ͷ˿�
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
#define GPRS_DEFAULT_POLL_COUNT1           10   //������ÿ���ڵ������ѯ����
#define GPRS_DEFAULT_POLL_COUNT2           3    //����������ѯ����������£���ѯ������ע��pollcount2<=pollcount1
#define GPRS_DEFAULT_BROAD_SET_TIME        3    //�㲥���ô���
#define GPRS_DEFAULT_CPU_RESET_TIME        0    //cpuĬ�ϸ�λʱ��

////GPRS����̫��
//#define GPRS_NET

#define SIZE_FLASHPAGE                     4096
#define APPINFOFLASHPAGE                   127         //��������Ϣ�洢��ַҳ��  ���һҳ
#define TERMINALINFOFLASHPAGE              124         //���3ҳ�� �ն˿�������Ϣ�洢��ַҳ��(ǰ2���ֽڴ洢�ն˿���������)
#define TERMIANL_CHANGE_TIME               30000       //�ն˿������仯��������ʱ�䣬��ֹƵ������
#define GPRS_SEND_BREAK_COUNT              3           //gprs����3��������ʱ�䷢��ʧ�ܣ���������
#define GPRS_LINK_FAIL_COUNT               3           //gprs����5������ʧ�ܣ�����cpu
#define ZIG_BREAK_TIMEOUT                  300000      //zigbee����5���������ݽ��գ�����zigbee

//GSM�ֻ��澯
#define GSM_ALRAM_ON            TRUE
#define GSM_PHONE_NUM           1
#define GSM_PHONE_DEF           "13962105621"

//APPINFOFLASHPAGE�洢��ʽTMHCAppRecord + ʱ�ο�����N(1B)+TMHCTIMECON*N
#pragma pack(1)
typedef struct {  
	TComENETInfo tenetinfo;
    TComGprsInfo tgprsinfo;
	TComLanInfo tlaninfo;
	TComGSMInfo tgsminfo;
}TMHCAppRecord;
#pragma pack() 

#define TMHCTIMECON_NUM_MAX     50   //ÿ�����ʱ�ο�����


//��ӡ�ȼ�
#define DEBUGOUT_LEVEL_ERR      0    //�ڹر�����²���ӡ
#define DEBUGOUT_LEVEL_ALARM    1    
#define DEBUGOUT_LEVEL_INFO     2    
#define DEBUGOUT_LEVEL_INDI     3    
#define DEBUGOUT_LEVEL_ZIG      7    
#define DEBUGOUT_LEVEL_GPRS     8   

///////////////////////////////////////////////////////////////////////


//����ȫ�ֱ�����ȫ�ֱ�������ǰһ��ǰ׺G_VAR_PREFIX��
G_VAR_PREFIX uint_8 uart_recvBuf[256];

//4��ͨ����ʹ�õĶ���
G_VAR_PREFIX struct list_head g_tfdatalist[CHNUM_DMX512];
G_VAR_PREFIX struct list_head g_tffreelist[CHNUM_DMX512];

G_VAR_PREFIX struct list_head g_enetrecvdatalist;          //enet������������
G_VAR_PREFIX struct list_head g_smallrecvfreelist;          //enet���տ�������
G_VAR_PREFIX struct list_head g_largerecvfreelist;          //enet���տ�������

//G_VAR_PREFIX struct list_head enetrecvdatalist;          //enet������������
//G_VAR_PREFIX struct list_head enetrecvfreelist;          //enet���տ�������

//G_VAR_PREFIX struct FileInfo fileinfo1;

G_VAR_PREFIX struct DateTime dt1;
G_VAR_PREFIX struct znFAT_Init_Args sdInitArgs;

//extern struct list_head s_gprsrecvdatalist;          //gprs������������
//extern struct list_head s_gprsrecvfreelist;          //gprs���տ�������
extern struct list_head s_comserversenddatalist;     //ͨ�Ų���server������������
extern struct list_head s_comterminalsenddatalist;   //ͨ�Ų���terminal������������
extern struct list_head s_comserversendfreelist;     //ͨ�Ų���server���Ϳ�������

TMHCAppRecord s_tappinfo;   //Ӧ�ü�¼��Ϣ��GPRS��LAN��GSM��

//�����С
#define SMALL_BUF_NODE_NUM      24
#define SMALL_BUF_NODE_SIZE     1100
#define LARGE_BUF_NODE_NUM      5
#define LARGE_BUF_NODE_SIZE     1200

//������
#define SYN    1   //ͬ������

//1.4 �����������¼���
G_VAR_PREFIX LWEVENT_STRUCT  lwevent_group;  //�������¼���

//1.5 �궨���¼�λ
#define  EVENT_DMX512           ((1uL)<<(6))   //�¼�λ
//#define  EVENT_CAMERA           ((1uL)<<(1))   //�¼�λ
//#define  EVENT_WRITE_SDHC       ((1uL)<<(2))   //�¼�λ
//#define  EVENT_ENET             ((1uL)<<(3))   //�¼�λ
#define  EVENT_SDHC             ((1uL)<<(4))   //�¼�λ
#define  EVENT_EFFECTS          ((1uL)<<(5))   //�¼�λ
#define  EVENT_WATERLAMP        ((1uL)<<(3))   //�¼�λ
#define  EVENT_HORSERACE        ((1uL)<<(2))   //�¼�λ
#define  EVENT_UARTSEND         ((1uL)<<(7))   //�¼�λ
#define  EVENT_UARTSENDISR      ((1uL)<<(8))   //�¼�λ

//3.�Ǽ�����ģ����
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
//4.Ϊ���񴴽�����ջ
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

//5.����������
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

//6.�����жϴ�����
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
