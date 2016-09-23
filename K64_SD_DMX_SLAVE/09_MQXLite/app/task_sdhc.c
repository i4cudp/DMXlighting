//===========================================================================
//�ļ����ƣ�task_sdhc.c
//���ܸ�Ҫ����SD���ж�ȡ���ݷ��� ����
//��Ȩ���У����ݴ�ѧǶ��ʽ����(sumcu.suda.edu.cn)
//�汾���£�������Ҫ����
//===========================================================================
#include "01_app_include.h"    //Ӧ�����񹫹�ͷ�ļ�

static uint_8  FileParam[10];    //�ļ���������
static uint_8  fileNum;          //�ļ����
static uint_16 MCUNum;           //���������
static uint_8  chNum;            //��ͨ����
static uint_32 blockNum;         //���
static uint_32 Flen;             //ÿ֡���������ֽڣ���������Ϣ��ֻ��ֵһ�Σ�������ͬ����

//terminal����������
static uint_32 s_sumLen = 0, Block_Index = 0;


struct FileInfo fileinfo1;
extern bool b_sd;       //�ļ�δ����
extern bool g_bSDWaitEvent;


//task_DMX512��������Ҫʹ��
uint_16 timeInterval;   //ʱ����
uint_32 FrameNum;       //��Ƶ��֡��

uint_16 lightnum;

uint_8 cmd_array[250];

void CalculateFlen();
void TerminalDeal();
//===========================================================================
//�������ƣ�task_sdhc
//���ܸ�Ҫ����ȡSD���е�����
//����˵����δʹ��
//===========================================================================
void task_sdhc(uint32_t initial)
{	
	//1. ��������ʹ�õı���
	uint_8  sd_state;
	uint_8 j;
	uint_32 len, FileLength;
	uint_16 blocklen[8];  //��¼�鳤������ͳ��ÿ֡������ֽ���
	uint_8  chNodeNum = SMALL_BUF_NODE_NUM / CHNUM_DMX512;

//	//SDHC��ʼ����znFAT�ļ�ϵͳ��ʼ��
//	sd_state = znFAT_Device_Init();   //SD��ϵͳ��ʼ��
//	//���ڷ��ͳ�ʼ����ʾ
//	if (RES_OK == sd_state)
//	{
//		printf("SD��ʼ����ɣ�\n");
//	}
//	else
//	{
//		printf("SD��ʼ��ʧ�ܣ�\n");
//	}
//	znFAT_Select_Device(0, &sdInitArgs);
//	sd_state = znFAT_Init();   //znFAT�ļ�ϵͳ��ʼ��
//	//���ڷ��ͳ�ʼ����ʾ
//	if (RES_OK == sd_state)
//	{
//		printf("�ļ�ϵͳ��ʼ����ɣ�\n");
//	}
//	else
//	{
//		printf("�ļ�ϵͳ��ʼ��ʧ�ܣ�\n");
//	}

	//�����ļ���Ϣ���ļ���ʽ���ĵ�
	znFAT_Open_File(&fileinfo1,"/f0_2CH.txt",0,1);
//	znFAT_Delete_File("/f0_2CH.txt");

	//���ļ���Ϣ��B0�Ŀ����Ϣ��ǰ3���ֽڣ�
	len=znFAT_ReadData(&fileinfo1,0,13,FileParam);
	if(len!=0)
	{
		fileNum=FileParam[0];                        //�ļ��� 1���ֽ�
		MCUNum=(FileParam[1]<<8)+FileParam[2];        //MCU�� 2���ֽ�
		timeInterval=(FileParam[3]<<8)+FileParam[4];  //ʱ���� 2���ֽ�
		chNum=FileParam[5];                          //ͨ������ 1���ֽ�
		blockNum=(FileParam[6]<<24)+(FileParam[7]<<16)+(FileParam[8]<<8)+FileParam[9];
		printf("\r\nThe file's basic info:\r\nfileNum:%d\r\nMCUNum:%d\r\ntimeInterval:%d\r\nchNum:%d\r\nblockNum:%d\r\n",
				fileNum,MCUNum,timeInterval,chNum,blockNum);
	}

	CalculateFlen();                //�õ�֡�����������Ϣ
	FrameNum = blockNum / chNum;    //��Ƶ֡��

	//Ԥȡ4��node��data_list
	for(j = 0; j < chNodeNum; j++)
	{
        TerminalDeal();
	}

	//��ʱ��λ��������ȼ���task_DMX512����õ�ִ��
	_lwevent_set(&lwevent_group,EVENT_DMX512);
    //��ʼִ�ж�SD����
	_lwevent_set(&lwevent_group,EVENT_SDHC);
	//3. ��������ѭ����
	while(TRUE) 
	{
		g_bSDWaitEvent = TRUE;
		_lwevent_wait_for(&lwevent_group, EVENT_SDHC, FALSE, NULL);
		g_bSDWaitEvent = FALSE;

		//sd�������Ѹ���
		if( true == b_sd )
		{
			//terminalʹ�õľ�̬����
			s_sumLen = 0;
			Block_Index = 0;

			//�����ļ���Ϣ���ļ���ʽ���ĵ�
			znFAT_Open_File(&fileinfo1,"/f0_2CH.txt",0,1);
			//�������ļ��Ļ�����Ϣ
			len=znFAT_ReadData(&fileinfo1,0,13,FileParam);
			if(len!=0)
			{
				fileNum=FileParam[0];                         //�ļ��� 1���ֽ�
				MCUNum=(FileParam[1]<<8)+FileParam[2];        //MCU�� 2���ֽ�
				timeInterval=(FileParam[3]<<8)+FileParam[4];  //ʱ���� 2���ֽ�
				chNum=FileParam[5];                           //ͨ������ 1���ֽ�
				blockNum=(FileParam[6]<<24)+(FileParam[7]<<16)+(FileParam[8]<<8)+FileParam[9];
				printf("\r\nThe file's basic info:\r\nfileNum:%d\r\nMCUNum:%d\r\ntimeInterval:%d\r\nchNum:%d\r\nblockNum:%d\r\n",
						fileNum,MCUNum,timeInterval,chNum,blockNum);
				printf("File size = %d", fileinfo1.File_Size);
			}

			CalculateFlen();                //�õ�֡�����������Ϣ
			FrameNum = blockNum / chNum;    //��Ƶ֡��

			//Ԥȡ4��node��data_list
			for(j=0;j<4;j++)
			{
				TerminalDeal();
			}

			//��ʱ��λ��������ȼ���task_DMX512����õ�ִ��
			_lwevent_set(&lwevent_group,EVENT_DMX512);

			//��ɾ��ļ���
			b_sd = false;
		}
		
		//���¼����û�����-----------------------------------------------------
        TerminalDeal();
	    _time_delay_ticks(10);
	}//����ѭ����end_while

}

//===========================================================================
//�������ƣ�TerminalDeal
//���ܸ�Ҫ������ʼ�����node�������
//����˵������
//===========================================================================
void TerminalDeal()
{
	uint_8 BlockParam[3],CH,k,chk = 0;
    uint_32 i,len = 0,Blen;
	TBufNode* pnode = NULL;

    const uint_16 fsyn=100;  //ÿ������֡����ͬ��һ��

    //������Ϣ
	for(i = 0; i < chNum; i++)
	{
		if(Block_Index == 0)   //���¿�ʼ������Ƶ����B0��ʼ
		{
			s_sumLen = 10;
		}
		len=znFAT_ReadData(&fileinfo1, s_sumLen, 3, BlockParam);
		if(0 != len)
		{
			CH = BlockParam[0];  //ͨ����1������0ͨ������
			if(CH > 4)
			{
				znFAT_Delete_File("/f0_2CH.txt");
				znFAT_Create_File(&fileinfo1,"/f0_2CH.txt",&dt1);
				b_sd = true;
				break;
			}
			listnode_get(&g_tffreelist[CH-1], &pnode);
			if(pnode != NULL)
			{
				Blen = (BlockParam[1]<<8) | BlockParam[2];
				if(Blen <= pnode->size)
				{
					DISABLE_INTERRUPTS;

					len = znFAT_ReadData(&fileinfo1, s_sumLen + 3, Blen, pnode->pbuf);
					pnode->CHNum = CH;
					pnode->len = Blen;
					pnode->CHSeleted = 1;
					s_sumLen = s_sumLen + 3 + len;  //��¼��һ�ο�ʼ��ȡ��λ��
					Block_Index++;                  //����һ�������
					listnode_put(&g_tfdatalist[CH-1], pnode);

					ENABLE_INTERRUPTS;
				}
				else
				{
					listnode_put(&g_tffreelist[CH-1], pnode);
				}
			}
		}
	}
	if(Block_Index > blockNum - 1)  //�������һ�������
	{
		Block_Index = 0;
		s_sumLen = 10;
	}
//	znFAT_Close_File(&fileinfo1);
//	znFAT_Flush_FS();
}

//===========================================================================
//�������ƣ�CalculateFlen
//���ܸ�Ҫ������ÿ֡������ֽ�������������Ϣ��������Ϣ
//����˵������
//===========================================================================
void CalculateFlen()
{
	uint_8 i,BlockParam[3];
    uint_32 len = 0,Blen=0;
    static uint_32 sumLen=10;

    //������Ϣ
	for(i=0;i<chNum;i++)
	{
		len=znFAT_ReadData(&fileinfo1,sumLen,3,BlockParam);
		if(0!=len)
		{
		    Blen = (BlockParam[1]<<8)|BlockParam[2];
		    lightnum = Blen / 3;
			sumLen=sumLen+3+Blen;  //��¼��һ�ο�ʼ��ȡ��λ��
		}
	}

	Flen = sumLen - chNum * 3 - 10;    //ÿ֡������ֽ���=��������+����Ϣ��ÿ������Ϣռ3���ֽ�
}



