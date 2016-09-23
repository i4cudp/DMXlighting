//===========================================================================
//文件名称：task_sdhc.c
//功能概要：从SD卡中读取数据放入 链表
//版权所有：苏州大学嵌入式中心(sumcu.suda.edu.cn)
//版本更新：根据需要更新
//===========================================================================
#include "01_app_include.h"    //应用任务公共头文件

static uint_8  FileParam[10];    //文件参数数组
static uint_8  fileNum;          //文件编号
static uint_16 MCUNum;           //控制器编号
static uint_8  chNum;            //总通道数
static uint_32 blockNum;         //块号
static uint_32 Flen;             //每帧画面所需字节，包括块信息，只赋值一次，用来做同步的

//terminal函数中运用
static uint_32 s_sumLen = 0, Block_Index = 0;


struct FileInfo fileinfo1;
extern bool b_sd;       //文件未更新
extern bool g_bSDWaitEvent;


//task_DMX512任务中需要使用
uint_16 timeInterval;   //时间间隔
uint_32 FrameNum;       //视频总帧数

uint_16 lightnum;

uint_8 cmd_array[250];

void CalculateFlen();
void TerminalDeal();
//===========================================================================
//任务名称：task_sdhc
//功能概要：读取SD卡中的数据
//参数说明：未使用
//===========================================================================
void task_sdhc(uint32_t initial)
{	
	//1. 声明任务使用的变量
	uint_8  sd_state;
	uint_8 j;
	uint_32 len, FileLength;
	uint_16 blocklen[8];  //记录块长，用来统计每帧画面的字节数
	uint_8  chNodeNum = SMALL_BUF_NODE_NUM / CHNUM_DMX512;

//	//SDHC初始化及znFAT文件系统初始化
//	sd_state = znFAT_Device_Init();   //SD卡系统初始化
//	//串口发送初始化提示
//	if (RES_OK == sd_state)
//	{
//		printf("SD初始化完成！\n");
//	}
//	else
//	{
//		printf("SD初始化失败！\n");
//	}
//	znFAT_Select_Device(0, &sdInitArgs);
//	sd_state = znFAT_Init();   //znFAT文件系统初始化
//	//串口发送初始化提示
//	if (RES_OK == sd_state)
//	{
//		printf("文件系统初始化完成！\n");
//	}
//	else
//	{
//		printf("文件系统初始化失败！\n");
//	}

	//读出文件信息，文件格式见文档
	znFAT_Open_File(&fileinfo1,"/f0_2CH.txt",0,1);
//	znFAT_Delete_File("/f0_2CH.txt");

	//读文件信息及B0的块儿信息（前3个字节）
	len=znFAT_ReadData(&fileinfo1,0,13,FileParam);
	if(len!=0)
	{
		fileNum=FileParam[0];                        //文件号 1个字节
		MCUNum=(FileParam[1]<<8)+FileParam[2];        //MCU号 2个字节
		timeInterval=(FileParam[3]<<8)+FileParam[4];  //时间间隔 2个字节
		chNum=FileParam[5];                          //通道个数 1个字节
		blockNum=(FileParam[6]<<24)+(FileParam[7]<<16)+(FileParam[8]<<8)+FileParam[9];
		printf("\r\nThe file's basic info:\r\nfileNum:%d\r\nMCUNum:%d\r\ntimeInterval:%d\r\nchNum:%d\r\nblockNum:%d\r\n",
				fileNum,MCUNum,timeInterval,chNum,blockNum);
	}

	CalculateFlen();                //得到帧长包括块儿信息
	FrameNum = blockNum / chNum;    //视频帧数

	//预取4个node进data_list
	for(j = 0; j < chNodeNum; j++)
	{
        TerminalDeal();
	}

	//置时间位，令高优先级的task_DMX512任务得到执行
	_lwevent_set(&lwevent_group,EVENT_DMX512);
    //开始执行读SD任务
	_lwevent_set(&lwevent_group,EVENT_SDHC);
	//3. 进入任务循环体
	while(TRUE) 
	{
		g_bSDWaitEvent = TRUE;
		_lwevent_wait_for(&lwevent_group, EVENT_SDHC, FALSE, NULL);
		g_bSDWaitEvent = FALSE;

		//sd卡内容已更新
		if( true == b_sd )
		{
			//terminal使用的静态变量
			s_sumLen = 0;
			Block_Index = 0;

			//读出文件信息，文件格式见文档
			znFAT_Open_File(&fileinfo1,"/f0_2CH.txt",0,1);
			//读出新文件的基本信息
			len=znFAT_ReadData(&fileinfo1,0,13,FileParam);
			if(len!=0)
			{
				fileNum=FileParam[0];                         //文件号 1个字节
				MCUNum=(FileParam[1]<<8)+FileParam[2];        //MCU号 2个字节
				timeInterval=(FileParam[3]<<8)+FileParam[4];  //时间间隔 2个字节
				chNum=FileParam[5];                           //通道个数 1个字节
				blockNum=(FileParam[6]<<24)+(FileParam[7]<<16)+(FileParam[8]<<8)+FileParam[9];
				printf("\r\nThe file's basic info:\r\nfileNum:%d\r\nMCUNum:%d\r\ntimeInterval:%d\r\nchNum:%d\r\nblockNum:%d\r\n",
						fileNum,MCUNum,timeInterval,chNum,blockNum);
				printf("File size = %d", fileinfo1.File_Size);
			}

			CalculateFlen();                //得到帧长包括块儿信息
			FrameNum = blockNum / chNum;    //视频帧数

			//预取4个node进data_list
			for(j=0;j<4;j++)
			{
				TerminalDeal();
			}

			//置时间位，令高优先级的task_DMX512任务得到执行
			_lwevent_set(&lwevent_group,EVENT_DMX512);

			//变成旧文件了
			b_sd = false;
		}
		
		//以下加入用户程序-----------------------------------------------------
        TerminalDeal();
	    _time_delay_ticks(10);
	}//任务循环体end_while

}

//===========================================================================
//函数名称：TerminalDeal
//功能概要：将初始化后的node加入队列
//参数说明：无
//===========================================================================
void TerminalDeal()
{
	uint_8 BlockParam[3],CH,k,chk = 0;
    uint_32 i,len = 0,Blen;
	TBufNode* pnode = NULL;

    const uint_16 fsyn=100;  //每隔多少帧画面同步一次

    //读块信息
	for(i = 0; i < chNum; i++)
	{
		if(Block_Index == 0)   //重新开始播放视频，从B0开始
		{
			s_sumLen = 10;
		}
		len=znFAT_ReadData(&fileinfo1, s_sumLen, 3, BlockParam);
		if(0 != len)
		{
			CH = BlockParam[0];  //通道从1计数，0通道保留
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
					s_sumLen = s_sumLen + 3 + len;  //记录下一次开始读取的位置
					Block_Index++;                  //读下一块儿数据
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
	if(Block_Index > blockNum - 1)  //到达最后一块的数据
	{
		Block_Index = 0;
		s_sumLen = 10;
	}
//	znFAT_Close_File(&fileinfo1);
//	znFAT_Flush_FS();
}

//===========================================================================
//函数名称：CalculateFlen
//功能概要：计算每帧画面的字节数，包括块信息及像素信息
//参数说明：无
//===========================================================================
void CalculateFlen()
{
	uint_8 i,BlockParam[3];
    uint_32 len = 0,Blen=0;
    static uint_32 sumLen=10;

    //读块信息
	for(i=0;i<chNum;i++)
	{
		len=znFAT_ReadData(&fileinfo1,sumLen,3,BlockParam);
		if(0!=len)
		{
		    Blen = (BlockParam[1]<<8)|BlockParam[2];
		    lightnum = Blen / 3;
			sumLen=sumLen+3+Blen;  //记录下一次开始读取的位置
		}
	}

	Flen = sumLen - chNum * 3 - 10;    //每帧画面的字节数=像素数据+块信息，每个块信息占3个字节
}



