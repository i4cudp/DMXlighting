#include "01_app_include.h"    //Ӧ�����񹫹�ͷ�ļ�


//===========================================================================
//�������ƣ�task_detect
//���ܸ�Ҫ����������ѹ
//����˵����δʹ�� ��ַ��2B  + �����֣�1B  +  ���ݣ� 4B   + ����ͣ� 1B
//===========================================================================
void task_detect(uint32_t initial_data)
{
	TBufNode* pnode = NULL;
	uint_8 detect_sendcmd[23];
	uint_8 cmd[8] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

	detect_sendcmd[0] = SERVERCOM_START_FLAG;                            //֡ͷ   1B
	detect_sendcmd[1] = detect_sendcmd[2] = detect_sendcmd[3] = 0x00;    //Ӧ�÷���   4B
	detect_sendcmd[4] = 0x01;
	detect_sendcmd[5] = 0x00;                                            //�����ַ  2B
	detect_sendcmd[6] = 0x0B;
	detect_sendcmd[7] = detect_sendcmd[8] = detect_sendcmd[9] = 0x00;    //���ڵ�ַ 4B
	detect_sendcmd[10] = 0x0A;
	detect_sendcmd[11] = 0x13;                                           //������
	detect_sendcmd[12] = 0x01;
    memcpy(&detect_sendcmd[13], cmd, 8);                                 //��������
    detect_sendcmd[21] = 0x00;                                           //У���
    detect_sendcmd[22] = SERVERCOM_END_FLAG;                             //֡β


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

	    _time_delay_ticks(360000);		//���СʱҪһ������
    }
}


