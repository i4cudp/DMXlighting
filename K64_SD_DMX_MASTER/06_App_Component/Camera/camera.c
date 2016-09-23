//===========================================================================
//文件名称：camera.c
//功能概要：PTC06摄像头构件源文件
//版权所有：苏州大学飞思卡尔嵌入式中心(sumcu.suda.edu.cn)
//更新记录：2016-05-02   V1.0
//备注：
//		拍照前[可选指令]
//		设置图片尺寸(默认值：320X240，修改尺寸需复位)
//		设置图片压缩率(默认值：36)
//
//		拍照片的指令顺序：
//		1.清空图片缓存
//		2.拍照指令
//		3.读图片长度指令
//		4.读图片数据指令
//===========================================================================


#include "camera.h"


#if		USING_CAMERA_FOR_MMS

#if     USING_SAVE_SD
FIL JPGE;
FATFS fs;
#endif


//复位指令与复位回复
const uint_8 reset_rsp[] = {0x76,0x00,0x26,0x00};
const uint_8 reset_cmd[] = {0x56,0x00,0x26,0x00};


//清除图片缓存指令与回复
const uint_8 photoBufCls_cmd [] = {0x56,0x00,0x36,0x01,0x02};
const uint_8 photoBufCls_rsp[] = {0x76,0x00,0x36,0x00,0x00};

//拍照指令与回复
const uint_8 start_photo_cmd[] = {0x56,0x00,0x36,0x01,0x00};
const uint_8 start_photo_rsp[] = {0x76,0x00,0x36,0x00,0x00};

//读图片长度指令与回复
//图片长度指令回复的前7个字节是固定的，最后2个字节表示图片的长度
//如0xA0,0x00,10进制表示是40960,即图片长度(大小)为40K
const uint_8 read_len_cmd[] = {0x56,0x00,0x34,0x01,0x00};
const uint_8 read_len_rsp[] = {0x76,0x00,0x34,0x00,0x04,0x00,0x00};

//读图片数据指令与回复
//get_photo_cmd前6个字节是固定的，
//第9,10字节是图片的起始地址
//第13,14字节是图片的末尾地址，即本次读取的长度

//如果是一次性读取，第9,10字节的起始地址是0x00,0x00;
//第13,14字节是图片长度的高字节，图片长度的低字节(如0xA0,0x00)

//如果是分次读取，每次读N字节(N必须是8的倍数)长度，
//则起始地址最先从0x00,0x00读取N长度(即N & 0xff00, N & 0x00ff)，
//后几次读的起始地址就是上一次读取数据的末尾地址
const uint_8 get_photo_cmd [] = {0x56,0x00,0x32,0x0C,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF };
const uint_8 get_photo_rsp []  = {0x76,0x00,0x32,0x00,0x00};

//设置压缩率指令与回复，最后1个字节为压缩率选项
//范围是：00 - FF
//默认压缩率是36
const uint_8 set_compress_cmd [] = {0x56,0x00,0x31,0x05,0x01,0x01,0x12,0x04,0x36};
const uint_8 compress_rate_rsp [] = {0x76,0x00,0x31,0x00,0x00};

//设置图片尺寸指令与回复
//set_photo_size_cmd最后1个字节的意义
//0x22 - 160X120
//0x11 - 320X240
//0x00 - 640X480
const uint_8 set_photo_size_cmd [] = {0x56,0x00,0x31,0x05,0x04,0x01,0x00,0x19,0x11};
const uint_8 set_photo_size_rsp [] = {0x76,0x00,0x31,0x00,0x00 };

//读取图片尺寸指令与回复
//read_photo_size_rsp最后1个字节的意义
//0x22 - 160X120
//0x11 - 320X240
//0x00 - 640X480
const uint_8 read_photo_size_cmd [] = {0x56,0x00,0x30,0x04,0x04,0x01,0x00,0x19};
const uint_8 read_photo_size_rsp [] = {0x76,0x00,0x30,0x00,0x01,0x00};



//移动侦测指令
//motion_enable_cmd 打开移动侦测
//motion_disable_cmd 关闭移动侦测
const uint_8 motion_enable_cmd [] = {0x56,0x00,0x37,0x01,0x01};
const uint_8 motion_disable_cmd [] = {0x56,0x00,0x37,0x01,0x00};
const uint_8 motion_rsp [] = {0x76,0x00,0x37,0x00,0x00};

//当系统检测到有移动时，自动从串口输出motio_detecte
const uint_8 motion_detecte [] = {0x76,0x00,0x39,0x00,0x00};

//移动侦测灵敏度设置
const uint_8 motion_sensitivity_cmd [] = {0x56,0x00,0x31,0x05,0x01,0x01,0x1A,0x6E,0x03};
const uint_8 motion_sensitivity_rsp [] = {0x76,0x00,0x31,0x00,0x00};


volatile uint_8 cameraReady = 0;
uint_32 picLen = 0;   //数据长度


uint_8 gprs_buf[CAMERA_SEND_BUFF] = "";

//CommandPacket和ResponsePacket用于拷贝只读区的指令及应答到内存
uint_8 CommandPacket[16];
uint_8 ResponsePacket[7];

const nSerialNum SerialNum_Byte;//序列号枚举变量
uint_8 g_SerialNumber = 0;//串口上报移动侦测时保存的当前序列号



uint_8 Camera_Rx_Buff[RX_BUFF_LEN] = "";  //摄像头串口接受缓冲区
uint_16 Len = 0;   //缓冲区长度
uint_8 Hundle_Flag = 0;  //缓冲区处理标志

/****************************************************************
函数名：SetSerailNumber
函数描述: 修改协议中的序号
输入参数：目标指令缓存首地址，源指令首地址，源指令长度，
          目标应答缓存首地址，源应答首地址，源应答长度，需要修改的
          序号值
返回:无
******************************************************************/		
void SetSerailNumber(uint_8 *DstCmd, const uint_8 *SrcCmd, uint_8 SrcCmdLength,
                     uint_8 *DstRsp, const uint_8 *SrcRsp, uint_8 SrcRspLength,uint_8 nID)
{
    memset(&CommandPacket,0,sizeof(CommandPacket));
    memset(&ResponsePacket,0,sizeof(ResponsePacket));
    
    memcpy(DstCmd,SrcCmd,SrcCmdLength);
    memcpy(DstRsp,SrcRsp,SrcRspLength);
    
    DstCmd[ID_SERIAL_NUM] = nID & 0xFF;
    DstRsp[ID_SERIAL_NUM] = nID & 0xFF;

}

/****************************************************************
函数名：cam_write
函数描述: 接口函数，写入控制摄像头的串口
输入参数：数据的首地址，长度
返回:无
******************************************************************/		
void cam_write(const uint_8 *buf,uint_8 len)
{ 
    
    //需要调试时开启，观察打印的数据,这里使用串口CAMERA_DEBUG_UART调试
    #if ECHO_CMD_DEBUG_INFO
//	uart_send_string(CAMERA_DEBUG_UART, "\r\n[cam_write]Write Data:\r\n");
//	uart_sendN(CAMERA_DEBUG_UART, len, (uint_8 *)buf);
//	uart_send_string(CAMERA_DEBUG_UART, "\r\n");
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[cam_write]:Write Data:\r\n");
	uint_8 i;
	for(i = 0; i < len; i++)
	{
		DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR, buf[i]);
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "  ");
	}
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
//    DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR,g_psdloghead->sdindex);
    #endif

	//写命令之前清接受缓冲区
	Hundle_Flag = 1;
    //写串口驱动函数
	uart_sendN(CAMERA_UART, len, (uint_8 *)buf);

//    printf("%x",buf);
}


/****************************************************************
函数名：isr_camera_re
函数描述：摄像头的串口接受中断
输出参数：
返回:
******************************************************************/
void isr_camera_re(void)
{
    uint_8 ch;
    uint_8 flag = 1;
    static int i = 0;
//    DISABLE_INTERRUPTS;

    ch = uart_re1(CAMERA_UART, &flag);  //实验室K60和KL25串口程序不一样！！！！！！！！！！！
//    printf("%d",ch);
//    uart_send1(UART_4,ch);
    if (0 == flag)
    {
        if(Hundle_Flag != 0)//如果缓冲区已经处理，缓冲区从0开始
        {
        	Hundle_Flag = 0;
        	i = 0;
        }
    	Camera_Rx_Buff[i] = ch;
        i++;
        Len = i;
    }

//    ENABLE_INTERRUPTS;
}

/****************************************************************
函数名：camera_delay_ms
函数描述：延时函数
输出参数：
返回:
******************************************************************/
void camera_delay_ms(uint_16 ms)
{
   uint_16 ii,jj;
   if (ms<1) ms=1;
//   ms = ms*12 ;
   for(ii=0;ii<ms;ii++)
//     for(jj=0;jj<1335;jj++);  //16MHz--1ms
     for(jj=0;jj<4006;jj++);  //48MHz--1ms
     //for(jj=0;jj<5341;jj++);    //64MHz--1ms
}

/****************************************************************
函数名：cam_receiver
函数描述：接口函数，读取控制摄像头的串口
输出参数：接收数据的地址，长度
返回:接收到数据个数
******************************************************************/		
uint_16 cam_receiver(uint_8 *buf,uint_16 send_len)
{ 
    static uint_16 i = 0;
    uint_16 mylen;
    
    uint_32 time0=0;
	//等待接收数据 接收到的数据，接收到的个数等待接收的时间
	while(1)
	{
		if(Len >= send_len && Hundle_Flag == 0) //如果串口接收数据大于1则开始复制接收数据
		{
			time0 = 0;
			buf[i] = Camera_Rx_Buff[i];  //复制接收数据

			#if ECHO_CMD_DEBUG_INFO
//			uart_send_string(CAMERA_DEBUG_UART, "\r\n[cam_receiver]COPY\r\n");
//			uart_send1(CAMERA_DEBUG_UART, i);
	    	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[cam_receiver]:COPY\r\n");
	    	DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR,i);
			#endif

			i++;
			if(i == send_len && (send_len != 0))  //完成数据接收，退出
			{
				mylen = i;
				i = 0;
				Hundle_Flag = 1;
				#if ECHO_CMD_DEBUG_INFO
//				uart_send_string(CAMERA_DEBUG_UART, "\r\n[cam_receiver]break\r\n");
//				uart_send1(CAMERA_DEBUG_UART, Len);
		    	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[cam_receiver]:break\r\n");
		    	DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR, Len);
				#endif
				break;
			}
		}
		else  //等待一定时间继续接收，若长时间无数据则退出
		{
			if( time0 > 9000000ul )
			{
				mylen = i;
				i = 0;
				Hundle_Flag = 1;
				#if ECHO_CMD_DEBUG_INFO
//				uart_send_string(CAMERA_DEBUG_UART, "\r\n[cam_receiver]break time\r\n");
		    	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[cam_receiver]:break time\r\n");
		//    	DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR,g_psdloghead->sdindex);
				#endif
				break;
			}
			else
				time0++;
		}
	}
    //需要调试时开启，观察打印的数据,这里使用串口CAMERA_DEBUG_UART调试
    #if ECHO_CMD_DEBUG_INFO
//	uart_send_string(CAMERA_DEBUG_UART, "\r\n[cam_receiver]Receive Data:\r\n");
//	uart_sendN(CAMERA_DEBUG_UART, send_len, (uint_8 *)buf);
//	uart_send_string(CAMERA_DEBUG_UART, "\r\n");
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[cam_receiver]:Receive Data:\r\n");
	uint_8 j;
	for(j = 0; j < send_len; j++)
	{
		DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR, buf[j]);
		DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "  ");
	}
	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n");
//    	DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR,g_psdloghead->sdindex);
    #endif

    return mylen;
}

/****************************************************************
函数名：camera_init
函数描述：摄像头初始化
输入参数：序列号，需要设置的图片尺寸
返回:初始化成功返回1，初始化失败返回0
******************************************************************/		
uint_8 camera_init(uint_8 Serialnumber,uint_8 nSetImageSize)
{    
    uint_8 CurrentImageSize = 0xFF;
    uint_8 CurrentCompressRate = COMPRESS_RATE_36;
    
    //读取当前的图片尺寸到currentImageSize
    if ( current_photo_size(Serialnumber,&CurrentImageSize) != 1)
    {
//    	uart_send_string(CAMERA_DEBUG_UART, "\r\n[camera_init]:read_photo_size error\r\n");
//    	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[camera_init]:read_photo_size error\r\n");
//    	DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR,g_psdloghead->sdindex);
        return 0;
    }
    
    //判断是否需要修改图片尺寸
    if(nSetImageSize != CurrentImageSize)
    {
        //设置图片尺寸，设置后复位生效，该项设置后会永久保存
        if ( !send_photo_size(Serialnumber,nSetImageSize))
        {
//        	uart_send_string(CAMERA_DEBUG_UART, "\r\nset_photo_size error\r\n");
//        	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[camera_init]:set_photo_size error\r\n");
            return 0;
        }
        else
        {
            //复位生效
            if ( !send_reset(Serialnumber))
            {
//            	uart_send_string(CAMERA_DEBUG_UART, "\r\nreset error\r\n");
//            	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[camera_init]:reset error\r\n");
                return 0;
            }
            camera_delay_ms(1000);
            CurrentImageSize = nSetImageSize;
        }

    }
    
    //给不同图片尺寸设置适当的图片压缩率
    if(nSetImageSize == CurrentImageSize)
    {
        switch(CurrentImageSize)
        {
            case IMAGE_SIZE_160X120:
            case IMAGE_SIZE_320X240:
                 CurrentCompressRate = COMPRESS_RATE_36;
                 break;
            case IMAGE_SIZE_640X480:
                 CurrentCompressRate = COMPRESS_RATE_60;
                 break;
            default:
                break;
        }
    }
    //设置图片压缩率，该项不保存，每次上电后需重新设置
    if ( !send_compress_rate(Serialnumber,CurrentCompressRate))
    {
//    	uart_send_string(CAMERA_DEBUG_UART, "\r\nsend_compress_rate error\r\n");
//    	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[camera_init]:send_compress_rate error\r\n");
        return 0;
    }

    //这里要注意,设置压缩率后要延时
    camera_delay_ms(100);

    return 1;
    
}

/****************************************************************
 函数名：send_cmd
 函数描述：发送指令并识别指令返回
 输入参数：指令的首地址，指令的长度，匹配指令的首地址，需验证的个数
 返回：成功返回1,失败返回0
******************************************************************/	
uint_8 send_cmd( const uint_8 *cmd,uint_8 n0,const uint_8 *rev,uint_8 n1)
{
    uint_8  i;
    uint_8  tmp[5] = {0x00,0x00,0x00,0x00,0x00};

  
    cam_write(cmd, n0);

    if ( !cam_receiver(tmp,5) ) 
    {
        return 0;
    }
   
    //检验数据
    for (i = 0; i < n1; i++)
    {  
        if (tmp[i] != rev[i]) 
        {
            return 0;
        }
    }
    
    return 1;

}


/****************************************************************
函数名：current_photo_size
函数描述:读取当前设置的图片尺寸
输入参数：Serialnumber序列号，nImageSize传递图片尺寸的引用变量
返回:成功返回1,失败返回0
******************************************************************/	
uint_8 current_photo_size(uint_8 Serialnumber,uint_8 * nImageSize)
{  
    uint_8  i;
    uint_8  tmp[6] = {0x00,0x00,0x00,0x00,0x00,0x00};

    SetSerailNumber( CommandPacket,
                     read_photo_size_cmd,
                     sizeof(read_photo_size_cmd),
                     ResponsePacket,
                     read_photo_size_rsp,
                     sizeof(read_photo_size_rsp),
                     Serialnumber );
      
    cam_write(CommandPacket, sizeof(read_photo_size_cmd));

    if ( !cam_receiver(tmp,6) ) 
    {
        return 0;
    }
   
    //检验数据,对比前5个字节
    for (i = 0; i < 5; i++)
    {  
        if (tmp[i] != ResponsePacket[i]) 
        {
            return 0;
        }
    }
    
    //最后一个字节表示当前的图片大小
    *nImageSize = tmp[5];
    return 1;
}


/****************************************************************
函数名：send_photo_size
函数描述：设置拍照的图片尺寸（可选择：160X120,320X240,640X480）
输入参数：序列号，需要设置的图片尺寸
返回:成功返回1,失败返回0
******************************************************************/	
uint_8 send_photo_size(uint_8 Serialnumber,uint_8 nImageSize)
{  
    uint_8  i;
    
    SetSerailNumber( CommandPacket,
                     set_photo_size_cmd,
                     sizeof(set_photo_size_cmd),
                     ResponsePacket,
                     set_photo_size_rsp,
                     sizeof(set_photo_size_rsp),
                     Serialnumber );
    
    CommandPacket [sizeof(set_photo_size_cmd) - 1] = nImageSize;
    
    i = send_cmd( CommandPacket,
                  sizeof(set_photo_size_cmd),
                  ResponsePacket,
                  sizeof(set_photo_size_rsp) );
    return i;
}


/****************************************************************
函数名：send_reset
函数描述：发送复位指令复位后要延时1-2秒
输入参数：序列号
返回:成功返回1 失败返回0
******************************************************************/		
uint_8 send_reset(uint_8 Serialnumber)
{  
    uint_8 i;
    //复制命令与应答，修改序号
    SetSerailNumber( CommandPacket,
                     reset_cmd,
                     sizeof(reset_cmd),
                     ResponsePacket,
                     reset_rsp,
                     sizeof(reset_rsp),
                     Serialnumber );
    
    i = send_cmd( CommandPacket,
                  sizeof(reset_cmd),
                  ResponsePacket,
                  sizeof(reset_rsp) );
    
    return i;

}

/****************************************************************
函数名：send_stop_photo
函数描述：清空图片缓存
输入参数：序列号
返回:成功返回1,失败返回0
******************************************************************/		 
uint_8 send_photoBuf_cls(uint_8 Serialnumber)
{ 
    uint_8 i;
    
    SetSerailNumber( CommandPacket,
                     photoBufCls_cmd,
                     sizeof(photoBufCls_cmd),
                     ResponsePacket,
                     photoBufCls_rsp,
                     sizeof(photoBufCls_rsp),
                     Serialnumber );
    
    i = send_cmd( CommandPacket,
                  sizeof(photoBufCls_cmd),
                  ResponsePacket,
                  sizeof(photoBufCls_rsp) );
    return i;
}  


/****************************************************************
函数名：send_compress_rate
函数描述：发送设置图片压缩率
输入参数：序列号
返回:成功返回1,失败返回0
******************************************************************/		 
uint_8 send_compress_rate(uint_8 Serialnumber,uint_8 nCompressRate)
{
    uint_8 i;
    
    SetSerailNumber( CommandPacket,
                     set_compress_cmd,
                     sizeof(set_compress_cmd),
                     ResponsePacket,
                     compress_rate_rsp,
                     sizeof(compress_rate_rsp),
                     Serialnumber );
    
    if(nCompressRate > 0x36)
    {
        //最后一个字节表示压缩率
        CommandPacket [sizeof(set_compress_cmd) - 1] = nCompressRate;
    }
    
    i = send_cmd( CommandPacket,
                  sizeof(set_compress_cmd),
                  ResponsePacket,
                  sizeof(compress_rate_rsp) );
    return i;
}


/****************************************************************
函数名：send_start_photo
函数描述：发送开始拍照的指令
输入参数：序列号
返回:识别成功返回1 失败返回0
******************************************************************/		
uint_8 send_start_photo(uint_8 Serialnumber)
{
    uint_8 i;
    
    SetSerailNumber( CommandPacket,
                     start_photo_cmd,
                     sizeof(start_photo_cmd),
                     ResponsePacket,
                     start_photo_rsp,
                     sizeof(start_photo_rsp),
                     Serialnumber );
    
    i = send_cmd( CommandPacket,
                  sizeof(start_photo_cmd),
                  ResponsePacket,
                  sizeof(start_photo_rsp) );
    return i;
}	  


/****************************************************************
函数名：send_read_len
函数描述：读取拍照后的图片长度，即图片占用空间大小
输入参数：序列号
返回:图片的长度
******************************************************************/	
uint_32 send_read_len(uint_8 Serialnumber)
{
    uint_8 i;
    uint_32 len;
    uint_8 tmp[9];
    
    SetSerailNumber( CommandPacket,
                     read_len_cmd,
                     sizeof(read_len_cmd),
                     ResponsePacket,
                     read_len_rsp,
                     sizeof(read_len_rsp),
                     Serialnumber );
    
    //发送读图片长度指令
    cam_write(CommandPacket, 5);

    if ( !cam_receiver(tmp,9)) 
    {
        return 0;
    }

    //检验数据
    for (i = 0; i < 7; i++)
    {
        if ( tmp[i] != ResponsePacket[i]) 
        {
            return 0;
        }
    }
    
    len = (uint_32)tmp[7] << 8;//高字节
    len |= tmp[8];//低字节
    
    return len;
}


/****************************************************************
函数名：send_get_photo
函数描述：读取图片数据
输入参数：读图片起始地址StaAdd, 
          读取的长度readLen ，
          接收数据的缓冲区buf
          序列号
返回:成功返回1，失败返回0
FF D8 ... FF D9 是JPG的图片格式

1.一次性读取的回复格式：76 00 32 00 00 FF D8 ... FF D9 76 00 32 00 00

2.分次读取，每次读N字节,循环使用读取图片数据指令读取M次或者(M + 1)次读取完毕：
如第一次执行后回复格式
76 00 32 00 <FF D8 ... N> 76 00 32 00
下次执行读取指令时，起始地址需要偏移N字节，即上一次的末尾地址，回复格式
76 00 32 00 <... N> 76 00 32 00
......
76 00 32 00 <... FF D9> 76 00 32 00 //lastBytes <= N

Length = N * M 或 Length = N * M + lastBytes

******************************************************************/	
uint_8 send_get_photo(uint_16 staAdd,uint_16 readLen,uint_8 *buf,uint_8 Serialnumber)
{
    uint_8 i = 0;
    uint_8 *ptr = NULL;
    
    
    SetSerailNumber( CommandPacket,
                     get_photo_cmd,
                     sizeof(get_photo_cmd),
                     ResponsePacket,
                     get_photo_rsp,
                     sizeof(get_photo_rsp),
                     Serialnumber );
    
    //装入起始地址高低字节
    CommandPacket[8] = (staAdd >> 8) & 0xff;
    CommandPacket[9] = staAdd & 0xff;
    //装入末尾地址高低字节
    CommandPacket[12] = (readLen >> 8) & 0xff;
    CommandPacket[13] = readLen & 0xff;
    
    //执行指令
    cam_write(CommandPacket,16);
    
    //等待图片数据存储到buf，超时或无数据回复则返回0
    if ( !cam_receiver(buf,readLen + 10))
    {
        return 0;
    }
    
    //检验帧头76 00 32 00 00
    for (i = 0; i < 5; i++)
    {
        if ( buf[i] != ResponsePacket[i] )
        {
            return 0;
        }
    }

    //检验帧尾76 00 32 00 00
    for (i = 0; i < 5; i++)
    {
        if ( buf[i + 5 + readLen] != ResponsePacket[i] )
        {
            return 0;
        }
    }
    
    
    //宏开关选择丢弃/保留 帧头帧尾76 00 32 00 00
    #if CLEAR_FRAME
//    memcpy(buf,buf + 5,read_len);
    ptr = buf;
    
    for (; readLen > 0; ++ptr)
    {
        *(ptr) = *(ptr + 5);
        readLen--;
    }
    #endif
    
    return 1;
}

/****************************************************************
函数名：send_open_motion
函数描述：发送打开移动侦测指令
输入参数：序列号
返回:识别成功返回1 失败返回0
******************************************************************/		
uint_8 send_motion_sensitivity(uint_8 Serialnumber)
{
    uint_8 i;
    
    SetSerailNumber( CommandPacket,
                     motion_sensitivity_cmd,
                     sizeof(motion_sensitivity_cmd),
                     ResponsePacket,
                     motion_sensitivity_rsp,
                     sizeof(motion_sensitivity_rsp),
                     Serialnumber );
    
    i = send_cmd( CommandPacket,
                  sizeof(motion_sensitivity_cmd),
                  ResponsePacket,
                  sizeof(motion_sensitivity_rsp) );
    return i;
}

/****************************************************************
函数名：send_open_motion
函数描述：发送打开移动侦测指令
输入参数：序列号
返回:识别成功返回1 失败返回0
******************************************************************/		
uint_8 send_open_motion(uint_8 Serialnumber)
{
    uint_8 i;
    
    SetSerailNumber( CommandPacket,
                     motion_enable_cmd,
                     sizeof(motion_enable_cmd),
                     ResponsePacket,
                     motion_rsp,
                     sizeof(motion_rsp),
                     Serialnumber );
    
    i = send_cmd( CommandPacket,
                  sizeof(motion_enable_cmd),
                  ResponsePacket,
                  sizeof(motion_rsp) );
    return i;
}

/****************************************************************
函数名：send_close_motion
函数描述：发送关闭移动侦测指令
输入参数：序列号
返回:识别成功返回1 失败返回0
******************************************************************/		
uint_8 send_close_motion(uint_8 Serialnumber)
{
    uint_8 i;
    
    SetSerailNumber( CommandPacket,
                     motion_disable_cmd,
                     sizeof(motion_disable_cmd),
                     ResponsePacket,
                     motion_rsp,
                     sizeof(motion_rsp),
                     Serialnumber );
    
    i = send_cmd( CommandPacket,
                  sizeof(motion_disable_cmd),
                  ResponsePacket,
                  sizeof(motion_rsp) );
    return i;
}

/****************************************************************
函数名：Motion_Detecte_Idle
函数描述: 等待移动侦测事件,该函数可在RS485同时接多个摄像头时，传递
          当前是第几个序列号上报移动侦测
输入参数：传递一个指针变量
返回:成功返回1 失败返回0
******************************************************************/		
uint_8 Motion_Detecte_Idle(uint_8 *pSerialnumber)
{
    uint_8  tmp[5] = {0x00,0x00,0x00,0x00,0x00};
    
    if ( !cam_receiver(tmp,5) ) 
    {
        return 0;
    }
    
    //检验数据
    //全部有5个数据，只校验4个，其中数组下标为1是序列号
    if(!(tmp[0] == motion_detecte[0] && 
         tmp[2] == motion_detecte[2] &&
         tmp[3] == motion_detecte[3] &&
         tmp[4] == motion_detecte[4] ))
    {
        return 0;
    }
    
    //取出序列号
    *pSerialnumber = tmp[1];
    return 1;

}
/****************************************************************
函数名：CameraDemoApp
函数描述：摄像头应用实例
输入参数：序列号,图片尺寸
返回:成功返回1，失败返回0
******************************************************************/		
uint_8 CameraDemoApp(uint_8 Serialnumber,uint_8 nCameraImageSize)
{
    uint_32 cntM = 0,lastBytes = 0,i = 0;
    
    #if USING_SAVE_SD
    FRESULT res;
    uint_8 pname[20];
    uint_8 Issuccess = 0;
    uint_32 defaultbw = 1;
    #endif
    
    
    //初始化摄像头
    cameraReady = camera_init(Serialnumber,nCameraImageSize);
    if(!cameraReady)
    {
        return 0;
    }
    //清空图片缓存
    if( !send_photoBuf_cls(Serialnumber) )
    {
        return 0;
    }
    //开始拍照
    if( !send_start_photo(Serialnumber) )
    {
        return 0;
    }
    else
    {
        //读取拍照后的图片长度
        picLen = send_read_len(Serialnumber);
    }
    
    if( !picLen )
    {
        return 0;
    }
    else
    {
        cntM = picLen / N_BYTE;
        lastBytes = picLen % N_BYTE;
        
        #if USING_SAVE_SD

        disk_initialize(0);
        if(SD_OK)
        {
            f_mount(0,&fs);
            res = f_mkdir("0:/PHOTO");		//创建PHOTO文件夹
            if(res != FR_EXIST && res != FR_OK) 	//发生了错误
            {
                f_mount(0,0);
                Issuccess = 0;
            }
            else
            {
                if(!camera_new_pathname(pname))
                {
                    res = f_open(&JPGE,
                             (const TCHAR*)pname,
                             FA_CREATE_ALWAYS | FA_WRITE);
                    if(res != FR_OK)
                    {
                        Issuccess = 0;
                    }
                    else
                    {
                        Issuccess = 1;
                    }
                }
            }
        }

        #endif
    }
    //分M次，每次读N_BYTE字节
    if( cntM )
    {
        for( i = 0; i < cntM; i++)
        {
            memset(gprs_buf, 0, sizeof(gprs_buf));

            //按图片长度读取数据
            if( !send_get_photo( i * N_BYTE,
                               N_BYTE,
                               gprs_buf,
                               Serialnumber) )
            {
                return 0;
            }
            else
            {
                //此分支可将图片数据输出到指定的串口
                //如接口函数，将图片数据写入到串口2
            	uart_sendN(CAMERA_DEBUG_UART, N_BYTE, gprs_buf);

                #if USING_SAVE_SD
                if(Issuccess)
                {
                    res = f_write(&JPGE,
                                  gprs_buf,
                                  N_BYTE,
                                  &defaultbw);
                }
                #endif
            }

            camera_delay_ms(100);
        }
    }
    //剩余图片长度
    if(lastBytes)
    {

        memset(gprs_buf, 0, sizeof(gprs_buf));	
        
        //读取剩余长度
        if( !send_get_photo( i * N_BYTE,
                            lastBytes,
                            gprs_buf,
                            Serialnumber) )
        {
        
            return 0;
        }
        else
        {
        	uart_sendN(CAMERA_DEBUG_UART, lastBytes, gprs_buf);
            //UART2Write(gprs_buf,lastBytes);
            
            #if USING_SAVE_SD
            if(Issuccess)
            {
                res = f_write(&JPGE,
                              gprs_buf,
                              N_BYTE,
                              &defaultbw);
                f_close(&JPGE);
            }

            #endif
        }
        camera_delay_ms(100);
    }
    
   
    return 1;
}

#if USING_SAVE_SD
/*******************************************************
函数名：camera_new_pathname
功能描述：存的图片名字
输入参数：无
函数返回值：无
********************************************************/
uint_8 camera_new_pathname(uint_8 *pname)//存的图片名字
{
    FRESULT res;
    uint_16 index = 0;
    while(index < 0XFFFF)
    {
        sprintf((char*)pname,"0:PHOTO/PTC%04d.jpg",index);
        res = f_open(&JPGE,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
        if(res==FR_NO_FILE)
        {
            break;		//该文件名不存在,正是我们需要的.
        }
        else if(res == FR_OK)
        {
            f_close(&JPGE);
        }
        else
        {
            return 1;
        }
        index++;
    }
    return 0;
}

#endif

#endif




