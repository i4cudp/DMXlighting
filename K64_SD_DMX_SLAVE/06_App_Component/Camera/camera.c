//===========================================================================
//�ļ����ƣ�camera.c
//���ܸ�Ҫ��PTC06����ͷ����Դ�ļ�
//��Ȩ���У����ݴ�ѧ��˼����Ƕ��ʽ����(sumcu.suda.edu.cn)
//���¼�¼��2016-05-02   V1.0
//��ע��
//		����ǰ[��ѡָ��]
//		����ͼƬ�ߴ�(Ĭ��ֵ��320X240���޸ĳߴ��踴λ)
//		����ͼƬѹ����(Ĭ��ֵ��36)
//
//		����Ƭ��ָ��˳��
//		1.���ͼƬ����
//		2.����ָ��
//		3.��ͼƬ����ָ��
//		4.��ͼƬ����ָ��
//===========================================================================


#include "camera.h"


#if		USING_CAMERA_FOR_MMS

#if     USING_SAVE_SD
FIL JPGE;
FATFS fs;
#endif


//��λָ���븴λ�ظ�
const uint_8 reset_rsp[] = {0x76,0x00,0x26,0x00};
const uint_8 reset_cmd[] = {0x56,0x00,0x26,0x00};


//���ͼƬ����ָ����ظ�
const uint_8 photoBufCls_cmd [] = {0x56,0x00,0x36,0x01,0x02};
const uint_8 photoBufCls_rsp[] = {0x76,0x00,0x36,0x00,0x00};

//����ָ����ظ�
const uint_8 start_photo_cmd[] = {0x56,0x00,0x36,0x01,0x00};
const uint_8 start_photo_rsp[] = {0x76,0x00,0x36,0x00,0x00};

//��ͼƬ����ָ����ظ�
//ͼƬ����ָ��ظ���ǰ7���ֽ��ǹ̶��ģ����2���ֽڱ�ʾͼƬ�ĳ���
//��0xA0,0x00,10���Ʊ�ʾ��40960,��ͼƬ����(��С)Ϊ40K
const uint_8 read_len_cmd[] = {0x56,0x00,0x34,0x01,0x00};
const uint_8 read_len_rsp[] = {0x76,0x00,0x34,0x00,0x04,0x00,0x00};

//��ͼƬ����ָ����ظ�
//get_photo_cmdǰ6���ֽ��ǹ̶��ģ�
//��9,10�ֽ���ͼƬ����ʼ��ַ
//��13,14�ֽ���ͼƬ��ĩβ��ַ�������ζ�ȡ�ĳ���

//�����һ���Զ�ȡ����9,10�ֽڵ���ʼ��ַ��0x00,0x00;
//��13,14�ֽ���ͼƬ���ȵĸ��ֽڣ�ͼƬ���ȵĵ��ֽ�(��0xA0,0x00)

//����Ƿִζ�ȡ��ÿ�ζ�N�ֽ�(N������8�ı���)���ȣ�
//����ʼ��ַ���ȴ�0x00,0x00��ȡN����(��N & 0xff00, N & 0x00ff)��
//�󼸴ζ�����ʼ��ַ������һ�ζ�ȡ���ݵ�ĩβ��ַ
const uint_8 get_photo_cmd [] = {0x56,0x00,0x32,0x0C,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF };
const uint_8 get_photo_rsp []  = {0x76,0x00,0x32,0x00,0x00};

//����ѹ����ָ����ظ������1���ֽ�Ϊѹ����ѡ��
//��Χ�ǣ�00 - FF
//Ĭ��ѹ������36
const uint_8 set_compress_cmd [] = {0x56,0x00,0x31,0x05,0x01,0x01,0x12,0x04,0x36};
const uint_8 compress_rate_rsp [] = {0x76,0x00,0x31,0x00,0x00};

//����ͼƬ�ߴ�ָ����ظ�
//set_photo_size_cmd���1���ֽڵ�����
//0x22 - 160X120
//0x11 - 320X240
//0x00 - 640X480
const uint_8 set_photo_size_cmd [] = {0x56,0x00,0x31,0x05,0x04,0x01,0x00,0x19,0x11};
const uint_8 set_photo_size_rsp [] = {0x76,0x00,0x31,0x00,0x00 };

//��ȡͼƬ�ߴ�ָ����ظ�
//read_photo_size_rsp���1���ֽڵ�����
//0x22 - 160X120
//0x11 - 320X240
//0x00 - 640X480
const uint_8 read_photo_size_cmd [] = {0x56,0x00,0x30,0x04,0x04,0x01,0x00,0x19};
const uint_8 read_photo_size_rsp [] = {0x76,0x00,0x30,0x00,0x01,0x00};



//�ƶ����ָ��
//motion_enable_cmd ���ƶ����
//motion_disable_cmd �ر��ƶ����
const uint_8 motion_enable_cmd [] = {0x56,0x00,0x37,0x01,0x01};
const uint_8 motion_disable_cmd [] = {0x56,0x00,0x37,0x01,0x00};
const uint_8 motion_rsp [] = {0x76,0x00,0x37,0x00,0x00};

//��ϵͳ��⵽���ƶ�ʱ���Զ��Ӵ������motio_detecte
const uint_8 motion_detecte [] = {0x76,0x00,0x39,0x00,0x00};

//�ƶ��������������
const uint_8 motion_sensitivity_cmd [] = {0x56,0x00,0x31,0x05,0x01,0x01,0x1A,0x6E,0x03};
const uint_8 motion_sensitivity_rsp [] = {0x76,0x00,0x31,0x00,0x00};


volatile uint_8 cameraReady = 0;
uint_32 picLen = 0;   //���ݳ���


uint_8 gprs_buf[CAMERA_SEND_BUFF] = "";

//CommandPacket��ResponsePacket���ڿ���ֻ������ָ�Ӧ���ڴ�
uint_8 CommandPacket[16];
uint_8 ResponsePacket[7];

const nSerialNum SerialNum_Byte;//���к�ö�ٱ���
uint_8 g_SerialNumber = 0;//�����ϱ��ƶ����ʱ����ĵ�ǰ���к�



uint_8 Camera_Rx_Buff[RX_BUFF_LEN] = "";  //����ͷ���ڽ��ܻ�����
uint_16 Len = 0;   //����������
uint_8 Hundle_Flag = 0;  //�����������־

/****************************************************************
��������SetSerailNumber
��������: �޸�Э���е����
���������Ŀ��ָ����׵�ַ��Դָ���׵�ַ��Դָ��ȣ�
          Ŀ��Ӧ�𻺴��׵�ַ��ԴӦ���׵�ַ��ԴӦ�𳤶ȣ���Ҫ�޸ĵ�
          ���ֵ
����:��
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
��������cam_write
��������: �ӿں�����д���������ͷ�Ĵ���
������������ݵ��׵�ַ������
����:��
******************************************************************/		
void cam_write(const uint_8 *buf,uint_8 len)
{ 
    
    //��Ҫ����ʱ�������۲��ӡ������,����ʹ�ô���CAMERA_DEBUG_UART����
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

	//д����֮ǰ����ܻ�����
	Hundle_Flag = 1;
    //д������������
	uart_sendN(CAMERA_UART, len, (uint_8 *)buf);

//    printf("%x",buf);
}


/****************************************************************
��������isr_camera_re
��������������ͷ�Ĵ��ڽ����ж�
���������
����:
******************************************************************/
void isr_camera_re(void)
{
    uint_8 ch;
    uint_8 flag = 1;
    static int i = 0;
//    DISABLE_INTERRUPTS;

    ch = uart_re1(CAMERA_UART, &flag);  //ʵ����K60��KL25���ڳ���һ������������������������
//    printf("%d",ch);
//    uart_send1(UART_4,ch);
    if (0 == flag)
    {
        if(Hundle_Flag != 0)//����������Ѿ�������������0��ʼ
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
��������camera_delay_ms
������������ʱ����
���������
����:
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
��������cam_receiver
�����������ӿں�������ȡ��������ͷ�Ĵ���
����������������ݵĵ�ַ������
����:���յ����ݸ���
******************************************************************/		
uint_16 cam_receiver(uint_8 *buf,uint_16 send_len)
{ 
    static uint_16 i = 0;
    uint_16 mylen;
    
    uint_32 time0=0;
	//�ȴ��������� ���յ������ݣ����յ��ĸ����ȴ����յ�ʱ��
	while(1)
	{
		if(Len >= send_len && Hundle_Flag == 0) //������ڽ������ݴ���1��ʼ���ƽ�������
		{
			time0 = 0;
			buf[i] = Camera_Rx_Buff[i];  //���ƽ�������

			#if ECHO_CMD_DEBUG_INFO
//			uart_send_string(CAMERA_DEBUG_UART, "\r\n[cam_receiver]COPY\r\n");
//			uart_send1(CAMERA_DEBUG_UART, i);
	    	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[cam_receiver]:COPY\r\n");
	    	DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR,i);
			#endif

			i++;
			if(i == send_len && (send_len != 0))  //������ݽ��գ��˳�
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
		else  //�ȴ�һ��ʱ��������գ�����ʱ�����������˳�
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
    //��Ҫ����ʱ�������۲��ӡ������,����ʹ�ô���CAMERA_DEBUG_UART����
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
��������camera_init
��������������ͷ��ʼ��
������������кţ���Ҫ���õ�ͼƬ�ߴ�
����:��ʼ���ɹ�����1����ʼ��ʧ�ܷ���0
******************************************************************/		
uint_8 camera_init(uint_8 Serialnumber,uint_8 nSetImageSize)
{    
    uint_8 CurrentImageSize = 0xFF;
    uint_8 CurrentCompressRate = COMPRESS_RATE_36;
    
    //��ȡ��ǰ��ͼƬ�ߴ絽currentImageSize
    if ( current_photo_size(Serialnumber,&CurrentImageSize) != 1)
    {
//    	uart_send_string(CAMERA_DEBUG_UART, "\r\n[camera_init]:read_photo_size error\r\n");
//    	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[camera_init]:read_photo_size error\r\n");
//    	DEBUGOUT_HEX(DEBUGOUT_LEVEL_ERR,g_psdloghead->sdindex);
        return 0;
    }
    
    //�ж��Ƿ���Ҫ�޸�ͼƬ�ߴ�
    if(nSetImageSize != CurrentImageSize)
    {
        //����ͼƬ�ߴ磬���ú�λ��Ч���������ú�����ñ���
        if ( !send_photo_size(Serialnumber,nSetImageSize))
        {
//        	uart_send_string(CAMERA_DEBUG_UART, "\r\nset_photo_size error\r\n");
//        	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[camera_init]:set_photo_size error\r\n");
            return 0;
        }
        else
        {
            //��λ��Ч
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
    
    //����ͬͼƬ�ߴ������ʵ���ͼƬѹ����
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
    //����ͼƬѹ���ʣ�������棬ÿ���ϵ������������
    if ( !send_compress_rate(Serialnumber,CurrentCompressRate))
    {
//    	uart_send_string(CAMERA_DEBUG_UART, "\r\nsend_compress_rate error\r\n");
//    	DEBUGOUT_STR(DEBUGOUT_LEVEL_ERR, "\r\n[camera_init]:send_compress_rate error\r\n");
        return 0;
    }

    //����Ҫע��,����ѹ���ʺ�Ҫ��ʱ
    camera_delay_ms(100);

    return 1;
    
}

/****************************************************************
 ��������send_cmd
 ��������������ָ�ʶ��ָ���
 ���������ָ����׵�ַ��ָ��ĳ��ȣ�ƥ��ָ����׵�ַ������֤�ĸ���
 ���أ��ɹ�����1,ʧ�ܷ���0
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
   
    //��������
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
��������current_photo_size
��������:��ȡ��ǰ���õ�ͼƬ�ߴ�
���������Serialnumber���кţ�nImageSize����ͼƬ�ߴ�����ñ���
����:�ɹ�����1,ʧ�ܷ���0
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
   
    //��������,�Ա�ǰ5���ֽ�
    for (i = 0; i < 5; i++)
    {  
        if (tmp[i] != ResponsePacket[i]) 
        {
            return 0;
        }
    }
    
    //���һ���ֽڱ�ʾ��ǰ��ͼƬ��С
    *nImageSize = tmp[5];
    return 1;
}


/****************************************************************
��������send_photo_size
�����������������յ�ͼƬ�ߴ磨��ѡ��160X120,320X240,640X480��
������������кţ���Ҫ���õ�ͼƬ�ߴ�
����:�ɹ�����1,ʧ�ܷ���0
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
��������send_reset
�������������͸�λָ�λ��Ҫ��ʱ1-2��
������������к�
����:�ɹ�����1 ʧ�ܷ���0
******************************************************************/		
uint_8 send_reset(uint_8 Serialnumber)
{  
    uint_8 i;
    //����������Ӧ���޸����
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
��������send_stop_photo
�������������ͼƬ����
������������к�
����:�ɹ�����1,ʧ�ܷ���0
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
��������send_compress_rate
������������������ͼƬѹ����
������������к�
����:�ɹ�����1,ʧ�ܷ���0
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
        //���һ���ֽڱ�ʾѹ����
        CommandPacket [sizeof(set_compress_cmd) - 1] = nCompressRate;
    }
    
    i = send_cmd( CommandPacket,
                  sizeof(set_compress_cmd),
                  ResponsePacket,
                  sizeof(compress_rate_rsp) );
    return i;
}


/****************************************************************
��������send_start_photo
�������������Ϳ�ʼ���յ�ָ��
������������к�
����:ʶ��ɹ�����1 ʧ�ܷ���0
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
��������send_read_len
������������ȡ���պ��ͼƬ���ȣ���ͼƬռ�ÿռ��С
������������к�
����:ͼƬ�ĳ���
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
    
    //���Ͷ�ͼƬ����ָ��
    cam_write(CommandPacket, 5);

    if ( !cam_receiver(tmp,9)) 
    {
        return 0;
    }

    //��������
    for (i = 0; i < 7; i++)
    {
        if ( tmp[i] != ResponsePacket[i]) 
        {
            return 0;
        }
    }
    
    len = (uint_32)tmp[7] << 8;//���ֽ�
    len |= tmp[8];//���ֽ�
    
    return len;
}


/****************************************************************
��������send_get_photo
������������ȡͼƬ����
�����������ͼƬ��ʼ��ַStaAdd, 
          ��ȡ�ĳ���readLen ��
          �������ݵĻ�����buf
          ���к�
����:�ɹ�����1��ʧ�ܷ���0
FF D8 ... FF D9 ��JPG��ͼƬ��ʽ

1.һ���Զ�ȡ�Ļظ���ʽ��76 00 32 00 00 FF D8 ... FF D9 76 00 32 00 00

2.�ִζ�ȡ��ÿ�ζ�N�ֽ�,ѭ��ʹ�ö�ȡͼƬ����ָ���ȡM�λ���(M + 1)�ζ�ȡ��ϣ�
���һ��ִ�к�ظ���ʽ
76 00 32 00 <FF D8 ... N> 76 00 32 00
�´�ִ�ж�ȡָ��ʱ����ʼ��ַ��Ҫƫ��N�ֽڣ�����һ�ε�ĩβ��ַ���ظ���ʽ
76 00 32 00 <... N> 76 00 32 00
......
76 00 32 00 <... FF D9> 76 00 32 00 //lastBytes <= N

Length = N * M �� Length = N * M + lastBytes

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
    
    //װ����ʼ��ַ�ߵ��ֽ�
    CommandPacket[8] = (staAdd >> 8) & 0xff;
    CommandPacket[9] = staAdd & 0xff;
    //װ��ĩβ��ַ�ߵ��ֽ�
    CommandPacket[12] = (readLen >> 8) & 0xff;
    CommandPacket[13] = readLen & 0xff;
    
    //ִ��ָ��
    cam_write(CommandPacket,16);
    
    //�ȴ�ͼƬ���ݴ洢��buf����ʱ�������ݻظ��򷵻�0
    if ( !cam_receiver(buf,readLen + 10))
    {
        return 0;
    }
    
    //����֡ͷ76 00 32 00 00
    for (i = 0; i < 5; i++)
    {
        if ( buf[i] != ResponsePacket[i] )
        {
            return 0;
        }
    }

    //����֡β76 00 32 00 00
    for (i = 0; i < 5; i++)
    {
        if ( buf[i + 5 + readLen] != ResponsePacket[i] )
        {
            return 0;
        }
    }
    
    
    //�꿪��ѡ����/���� ֡ͷ֡β76 00 32 00 00
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
��������send_open_motion
�������������ʹ��ƶ����ָ��
������������к�
����:ʶ��ɹ�����1 ʧ�ܷ���0
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
��������send_open_motion
�������������ʹ��ƶ����ָ��
������������к�
����:ʶ��ɹ�����1 ʧ�ܷ���0
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
��������send_close_motion
�������������͹ر��ƶ����ָ��
������������к�
����:ʶ��ɹ�����1 ʧ�ܷ���0
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
��������Motion_Detecte_Idle
��������: �ȴ��ƶ�����¼�,�ú�������RS485ͬʱ�Ӷ������ͷʱ������
          ��ǰ�ǵڼ������к��ϱ��ƶ����
�������������һ��ָ�����
����:�ɹ�����1 ʧ�ܷ���0
******************************************************************/		
uint_8 Motion_Detecte_Idle(uint_8 *pSerialnumber)
{
    uint_8  tmp[5] = {0x00,0x00,0x00,0x00,0x00};
    
    if ( !cam_receiver(tmp,5) ) 
    {
        return 0;
    }
    
    //��������
    //ȫ����5�����ݣ�ֻУ��4�������������±�Ϊ1�����к�
    if(!(tmp[0] == motion_detecte[0] && 
         tmp[2] == motion_detecte[2] &&
         tmp[3] == motion_detecte[3] &&
         tmp[4] == motion_detecte[4] ))
    {
        return 0;
    }
    
    //ȡ�����к�
    *pSerialnumber = tmp[1];
    return 1;

}
/****************************************************************
��������CameraDemoApp
��������������ͷӦ��ʵ��
������������к�,ͼƬ�ߴ�
����:�ɹ�����1��ʧ�ܷ���0
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
    
    
    //��ʼ������ͷ
    cameraReady = camera_init(Serialnumber,nCameraImageSize);
    if(!cameraReady)
    {
        return 0;
    }
    //���ͼƬ����
    if( !send_photoBuf_cls(Serialnumber) )
    {
        return 0;
    }
    //��ʼ����
    if( !send_start_photo(Serialnumber) )
    {
        return 0;
    }
    else
    {
        //��ȡ���պ��ͼƬ����
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
            res = f_mkdir("0:/PHOTO");		//����PHOTO�ļ���
            if(res != FR_EXIST && res != FR_OK) 	//�����˴���
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
    //��M�Σ�ÿ�ζ�N_BYTE�ֽ�
    if( cntM )
    {
        for( i = 0; i < cntM; i++)
        {
            memset(gprs_buf, 0, sizeof(gprs_buf));

            //��ͼƬ���ȶ�ȡ����
            if( !send_get_photo( i * N_BYTE,
                               N_BYTE,
                               gprs_buf,
                               Serialnumber) )
            {
                return 0;
            }
            else
            {
                //�˷�֧�ɽ�ͼƬ���������ָ���Ĵ���
                //��ӿں�������ͼƬ����д�뵽����2
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
    //ʣ��ͼƬ����
    if(lastBytes)
    {

        memset(gprs_buf, 0, sizeof(gprs_buf));	
        
        //��ȡʣ�೤��
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
��������camera_new_pathname
�������������ͼƬ����
�����������
��������ֵ����
********************************************************/
uint_8 camera_new_pathname(uint_8 *pname)//���ͼƬ����
{
    FRESULT res;
    uint_16 index = 0;
    while(index < 0XFFFF)
    {
        sprintf((char*)pname,"0:PHOTO/PTC%04d.jpg",index);
        res = f_open(&JPGE,(const TCHAR*)pname,FA_READ);//���Դ�����ļ�
        if(res==FR_NO_FILE)
        {
            break;		//���ļ���������,����������Ҫ��.
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




