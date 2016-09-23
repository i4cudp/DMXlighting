#include "01_app_include.h"    //应用任务公共头文件


//===========================================================================
//任务名称：task_detect
//功能概要：检测电流电压
//参数说明：未使用 地址：2B  + 命令字：1B  +  数据： 4B   + 检验和： 1B
//===========================================================================
void task_detect(uint32_t initial_data)
{
	TBufNode* pnode = NULL;
	uint_8 detect_sendcmd[23];
	uint_8 cmd[8] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

	detect_sendcmd[0] = SERVERCOM_START_FLAG;                            //帧头   1B
	detect_sendcmd[1] = detect_sendcmd[2] = detect_sendcmd[3] = 0x00;    //应用返回   4B
	detect_sendcmd[4] = 0x01;
	detect_sendcmd[5] = 0x00;                                            //网络地址  2B
	detect_sendcmd[6] = 0x0B;
	detect_sendcmd[7] = detect_sendcmd[8] = detect_sendcmd[9] = 0x00;    //网内地址 4B
	detect_sendcmd[10] = 0x0A;
	detect_sendcmd[11] = 0x13;                                           //命令字
	detect_sendcmd[12] = 0x01;
    memcpy(&detect_sendcmd[13], cmd, 8);                                 //命令内容
    detect_sendcmd[21] = 0x00;                                           //校验和
    detect_sendcmd[22] = SERVERCOM_END_FLAG;                             //帧尾


    while (TRUE)
    {
    	/*listnode_get(&enetrecvfreelist, &pnode);
    	if(pnode != null)
    	{
    		pnode->len = 23;
    		memcpy(pnode->pbuf, detect_sendcmd, 23);
    		listnode_put(&enetrecvdatalist, pnode);
//    		printf("[task_detect]:start to detect device!\r\n");
    	}
    	else
    	{
    		listnode_put(&enetrecvfreelist, pnode);
    	}*/

	    _time_delay_ticks(360000);		//半个小时要一次数据
    }
}


