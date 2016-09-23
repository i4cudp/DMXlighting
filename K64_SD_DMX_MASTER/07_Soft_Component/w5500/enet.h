
#ifndef  _ENET_H_
#define  _ENET_H_

#include "w5500.h"
#include "socket.h"
#include "01_app_include.h"
//ENET���������ж�
enum ENETRECVDATARET
{
	ENET_RECVDATA_OK = 0,       //����֡��ȷ���ս���
	ENET_RECVDATA_ERR,          //����֡���մ���
	ENET_RECVDATA_ING,	        //����֡��ȷ��δ����
	ENET_RECVDATA_MAX
};

//ENET������ݵ�����
enum ENETFrameType
{
	ENETFRAME_DATA = 0,	    //ENET->MCU������֡
	ENETFRAME_CMD,	        //ENET->MCU�������
	ENETFRAME_MAX
};

//1.ENET���ͺͽ�������֡
typedef struct
{
	struct list_head* penetdatalist;   //ENET���ݻ�������
	struct list_head* penetfreelist;   //ENET���ݻ���� ����
}TENETInfo;

#define ENET_POWER     (PTB_NUM|2)    //ENETģ���Դ��������
#define ENET_INT       (PTB_NUM|19)   //ENETģ���жϿ�������
#define ENET_RESET     (PTB_NUM|18)   //ENETģ�鸴λ��������
#define SPI_PSC        (PTB_NUM|20)   //ENETģ��Ƭѡ��������

#define E_NET

#define GPS_POWER     (PTB_NUM|9)    //GPSģ���Դ��������

void enet_rec(SOCKET s);

#endif
