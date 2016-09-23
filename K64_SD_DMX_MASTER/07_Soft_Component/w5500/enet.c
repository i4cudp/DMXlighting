
#include "enet.h"

//uint_8 s_EnetRecvTempBuf[BUF_NODE_SIZE_ENET_RECV];

//============================================================================
//������: ENETRecvDataJuge
//��  ��: ENET���������ж�
//��  ��:��
//��  ��: ��
//============================================================================
uint_8 ENETRecvDataJuge(const TBufNode* const pnode)
{
	uint_8 ret = ENET_RECVDATA_MAX;
	if(pnode != NULL)
	{
		if(pnode->len > 2 && SERVERCOM_END_FLAG == pnode->pbuf[pnode->len - 1]
			&& pnode->pbuf[pnode->len - 2] != SERVERCOM_ESC_FLAG)
			ret = ENET_RECVDATA_OK;
		else
		{
			printf("[ENETRecvDataJuge]:data err, len = %d, end flag = 0x%X, end pre = 0x%X\r\n", 
				pnode->len, pnode->pbuf[pnode->len - 1], pnode->pbuf[pnode->len - 2]);

			ret = ENET_RECVDATA_ERR;
		}
	}

	return ret;
}


void enet_rec(SOCKET s)
{
	uint_16 serverlen;
	TBufNode* pnode = NULL;
	uint_8 nodenum;
	uint_16 i;
	uint_8 data[20];
	bool bAck = FALSE;
	MQX_TICK_STRUCT nowticks;

	serverlen = getSn_RX_RSR(s);
	 if(serverlen > 0)
	 {
//	    _time_get_ticks(&nowticks);
//		printf("[enet_rec]:ticks 1111 = %d\r\n", nowticks.TICKS[0]);

//		printf("[enet_rec]:serverlen = %d, &g_smallrecvfreelist=0x%X, g_smallrecvfreelist.next=0x%X, g_smallrecvfreelist.prev=0x%X\r\n",
//		serverlen, &g_smallrecvfreelist, g_smallrecvfreelist.next, g_smallrecvfreelist.prev);

		//��ȫ������"penetfreelist" ��ȡ���ڵ�
//	    listnodenum_get(&g_smallrecvfreelist, &nodenum);
//		printf("[enet_rec]:free small node num = %d\r\n", nodenum);
//		listnodenum_get(&g_largerecvfreelist, &nodenum);
//		printf("[enet_rec]:free large node num = %d\r\n", nodenum);

		//С֡
		if(serverlen <= SMALL_BUF_NODE_SIZE)
		{
			listnode_get(&g_smallrecvfreelist, &pnode);
		}

		//δȡ������g_largerecvfreelist��ȡ
		if(pnode == NULL)
		{
			listnode_get(&g_largerecvfreelist, &pnode);
		}
		
		if(pnode != NULL && pnode->size >= serverlen)                  //ȡ�����нڵ�
		{
			pnode->len = serverlen;        //���ȳ�ʼ����Ҫ�ж�֡�����ȸ�ֵ
 #ifdef E_NET
			pnode->type = ENETFRAME_DATA;
 #else
			pnode->type = ENETFRAME_CMD;
 #endif
			recv(s, pnode->pbuf, serverlen);
 
			//������ȷ����,���಻����
			if(ENETRecvDataJuge(pnode) == ENET_RECVDATA_OK)
			{
				pnode->len = serverlen;
				listnode_put(&g_enetrecvdatalist, pnode);

				bAck = TRUE;
			}
			else
			{
				if(pnode->size == LARGE_BUF_NODE_SIZE)
				{
					listnode_put(&g_largerecvfreelist, pnode);
				}
				else
				{
					listnode_put(&g_smallrecvfreelist, pnode);
				}
			}

			//Ӧ��
			data[0] = SERVERCOM_START_FLAG;
			data[1] = 0x00;
			data[2] = 0x0A;
			data[3] = 0x00;
			data[4] = 0x00;
			memcpy(&data[5], &pnode->pbuf[5], 6);
			data[11] = 0x00;
			data[12] = bAck ? 0xB1 : 0xB2;
			//data[13] = MakeCheckInfo(data, 13);
			data[13] = 0x00;
			data[14] = SERVERCOM_END_FLAG;
			send(1, data, 15);

//			_time_get_ticks(&nowticks);
//			printf("[enet_rec]:ticks 2222 = %d\r\n", nowticks.TICKS[0]);
			
		}
		else
		{
			printf("[enet_rec]:data not recv, serverlen = %d\r\n", serverlen);	

			if(pnode != NULL)
			{
				if(pnode->size == LARGE_BUF_NODE_SIZE)
				{
					listnode_put(&g_largerecvfreelist, pnode);
				}
				else
				{
					listnode_put(&g_smallrecvfreelist, pnode);
				}
			}

			//if(serverlen <= BUF_NODE_SIZE_ENET_RECV)
			//{
			//	recv(s, s_EnetRecvTempBuf, serverlen);
			//}
		}
	 }
}
