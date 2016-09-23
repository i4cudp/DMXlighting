//=========================================================================
// �ļ����ƣ�diskio.h
// ���ܸ�Ҫ��diskio����Դ�ļ�
// ��Ȩ����: ���ݴ�ѧ��˼����Ƕ��ʽ����(sumcu.suda.edu.cn)
// �汾����:  2011-12-14     V1.0        diskio������ʼ�汾
//           2011-12-20     V1.1        diskio�����Ż��޸�
//=========================================================================

#include  "diskio.h"

//SD��״̬���Է�����
static volatile unsigned char Stat   = STA_NOINIT; //����SD��״̬
uint8_t GetCardStat(void) { return Stat; }
void SetCardStat(uint8_t state) { Stat = state; }

static volatile unsigned int  Timer = 0; //��дʱ��

//=========================================================================
//�������ƣ�disk_initialize
//���ܸ�Ҫ����ʼ�������̣�ֻ֧��������0�� 
//����˵����drv:�豸��
//�������أ�RES_OK���ɹ���������ʧ��
//=========================================================================
DRESULT disk_initialize (unsigned char drv)
{
    uint32_t  param, c_size, c_size_mult, read_bl_len;
    ESDHC_COMMAND_STRUCT  command;
    SDCARD_STRUCT_PTR     sdcard_ptr = (SDCARD_STRUCT_PTR)&SDHC_Card;

    //������
    if (drv || (NULL == sdcard_ptr)) return RES_PARERR; //����֧���豸0
    //��鿨����״̬
    if (Stat & STA_NODISK) return RES_NOTRDY; //SD��δ����

    //��ʼ��SD���豸�Ǽ���Ϣ
    sdcard_ptr->BITS = ESDHC_BUS_WIDTH_4BIT;//����λ��Ϊ4λ
    sdcard_ptr->SD_TIMEOUT = 10;
    sdcard_ptr->NUM_BLOCKS = 0;
    sdcard_ptr->ADDRESS    = 0;
    sdcard_ptr->ISSDHC     = FALSE;
    sdcard_ptr->VERSION2   = FALSE;

    //��Ⲣ��ʼ��SD��
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_INIT, NULL))
        return RES_ERROR;

    //��ȡSD������
    param = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_GET_CARD, &param))
        return RES_ERROR;
    if (    (ESDHC_CARD_SD == param) 
         || (ESDHC_CARD_SDHC == param) 
         || (ESDHC_CARD_SDCOMBO == param) 
         || (ESDHC_CARD_SDHCCOMBO == param))
    {
        if ((ESDHC_CARD_SDHC == param) || (ESDHC_CARD_SDHCCOMBO == param))
        {
            sdcard_ptr->ISSDHC = TRUE;
        }
    }
    else
        return RES_PARERR;

    //SD����ʶ
    command.COMMAND  = ESDHC_CMD2;
    command.TYPE     = ESDHC_TYPE_NORMAL;
    command.ARGUMENT = 0;
    command.READ     = FALSE;
    command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;

    //��ȡSD����ַ
    command.COMMAND  = ESDHC_CMD3;
    command.TYPE     = ESDHC_TYPE_NORMAL;
    command.ARGUMENT = 0;
    command.READ     = FALSE;
    command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;
    sdcard_ptr->ADDRESS = command.RESPONSE[0] & 0xFFFF0000;
    
    //��ȡSD����д���Բ���
    command.COMMAND  = ESDHC_CMD9;
    command.TYPE     = ESDHC_TYPE_NORMAL;
    command.ARGUMENT = sdcard_ptr->ADDRESS;
    command.READ     = FALSE;
    command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;
    if (0 == (command.RESPONSE[3] & 0x00C00000))
    {
        read_bl_len = (command.RESPONSE[2] >> 8) & 0x0F;
        c_size = command.RESPONSE[2] & 0x03;
        c_size = (c_size << 10) | (command.RESPONSE[1] >> 22);
        c_size_mult = (command.RESPONSE[1] >> 7) & 0x07;
        sdcard_ptr->NUM_BLOCKS = (c_size+1)*(1<<(c_size_mult+2))*(1<<(read_bl_len-9));
    }
    else
    {
        sdcard_ptr->VERSION2 = TRUE;
        c_size = (command.RESPONSE[1] >> 8) & 0x003FFFFF;
        sdcard_ptr->NUM_BLOCKS = (c_size + 1) << 10;
    }

    //ѡ��SD��
    command.COMMAND  = ESDHC_CMD7;
    command.TYPE     = ESDHC_TYPE_NORMAL;
    command.ARGUMENT = sdcard_ptr->ADDRESS;
    command.READ     = FALSE;
    command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;

    //���ÿ��С
    command.COMMAND  = ESDHC_CMD16;
    command.TYPE     = ESDHC_TYPE_NORMAL;
    command.ARGUMENT = IO_SDCARD_BLOCK_SIZE;
    command.READ     = FALSE;
    command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;

    if (ESDHC_BUS_WIDTH_4BIT == sdcard_ptr->BITS)
    {
        //�������������CMD55
        command.COMMAND  = ESDHC_CMD55;
        command.TYPE     = ESDHC_TYPE_NORMAL;
        command.ARGUMENT = sdcard_ptr->ADDRESS;
        command.READ     = FALSE;
        command.BLOCKS  = 0;
        if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
            return RES_ERROR;

        //��������λ��Ϊ4λ
        command.COMMAND = ESDHC_ACMD6;
        command.TYPE = ESDHC_TYPE_NORMAL;
        command.ARGUMENT = 2;
        command.READ = FALSE;
        command.BLOCKS = 0;
        if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
            return RES_ERROR;

        param = ESDHC_BUS_WIDTH_4BIT;
        if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SET_BUS_WIDTH, &param))
            return RES_ERROR;
    }

    Stat &= ~STA_NOINIT;//���������״̬
    
    return RES_OK;
}

//=========================================================================
//�������ƣ�disk_read                                                        
//���ܸ�Ҫ��������������                                                
//����˵����drv:�豸��
//         buff:���ڴ洢��ȡ�����ݵĻ�����
//         sector:��ʼ������
//         count:������
//�������أ�RES_OK���ɹ���������ʧ��                                                              
//=========================================================================
DRESULT disk_read(uint8_t drv,uint8_t *buff,uint32_t sector, uint8_t count)
{
    ESDHC_COMMAND_STRUCT command;
    SDCARD_STRUCT_PTR    sdcard_ptr = (SDCARD_STRUCT_PTR)&SDHC_Card;
	
    //������
    if (drv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    if ((NULL == buff)) return RES_ERROR;
	
    if (!sdcard_ptr->ISSDHC)
        //sector *= 512;//������ת��Ϊ��ʼ��ַ
        sector = sector << IO_SDCARD_BLOCK_SIZE_POWER;

    if (count == 1)//����������
    {
        command.COMMAND  = ESDHC_CMD17;
        command.TYPE     = ESDHC_TYPE_NORMAL;
        command.ARGUMENT = sector;
        command.READ     = TRUE;
        command.BLOCKS   = count;	

        if (ESDHC_OK == sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        {
            //if (sdhc_receive_block(buff, 512))
            if (sdhc_receive_block(buff, IO_SDCARD_BLOCK_SIZE))
                count = 0;
        }
    }
    else //���������
    {     
        command.COMMAND  = ESDHC_CMD18;
        command.TYPE     = ESDHC_TYPE_NORMAL;
        command.ARGUMENT = sector;
        command.READ     = TRUE;
        command.BLOCKS   = count;		

        if (ESDHC_OK == sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        {
            if (sdhc_receive_block(buff, IO_SDCARD_BLOCK_SIZE*count))
                count = 0;
        }
    }

    return count ? RES_ERROR : RES_OK;
}

//=========================================================================
//�������ƣ�disk_write                                                        
//���ܸ�Ҫ��д����������                                                
//����˵����drv:�豸��
//         buff:��д��SD�������ݵĻ������׵�ַ
//         sector:��ʼ������
//         count:������
//�������أ�RES_OK���ɹ���������ʧ��                                                              
//=========================================================================
DRESULT disk_write(uint8_t drv,const uint8_t *buff,uint32_t sector,uint8_t  count)
{
    ESDHC_COMMAND_STRUCT command;
    SDCARD_STRUCT_PTR    sdcard_ptr = (SDCARD_STRUCT_PTR)&SDHC_Card;

    //������
    if (drv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    if (Stat & STA_PROTECT) return RES_WRPRT;

    //��⻺�����Ƿ�Ϊ��
    if ((NULL == buff)) return RES_ERROR;


    if (!sdcard_ptr->ISSDHC)
        //sector *= 512;	//������ת��Ϊ��ʼ��ַ
        sector = sector << IO_SDCARD_BLOCK_SIZE_POWER; 

    if (count == 1)	//д��������
    {
        command.COMMAND  = ESDHC_CMD24;
        command.TYPE     = ESDHC_TYPE_NORMAL;
        command.ARGUMENT = sector;
        command.READ = FALSE;
        command.BLOCKS = count;

        if (ESDHC_OK == sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        {
            //if (sdhc_send_block(buff,512))
            if (sdhc_send_block(buff, IO_SDCARD_BLOCK_SIZE))
                count = 0;
        }
    }
    else //д�������
    {
        command.COMMAND  = ESDHC_CMD25;
        command.TYPE     = ESDHC_TYPE_NORMAL;
        command.ARGUMENT = sector;
        command.READ     = FALSE;
        command.BLOCKS   = count;

        if (ESDHC_OK == sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        {
            if (sdhc_send_block(buff,IO_SDCARD_BLOCK_SIZE*count))
                count = 0;
        }
    }

    //�ȴ����
    do
    {
        command.COMMAND  = ESDHC_CMD13;
        command.TYPE     = ESDHC_TYPE_NORMAL;
        command.ARGUMENT = sdcard_ptr->ADDRESS;
        command.READ     = FALSE;
        command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;
    
    if (command.RESPONSE[0] & 0xFFD98008)
        return RES_ERROR;

    } while (0x000000900 != (command.RESPONSE[0] & 0x00001F00));		

    return count ? RES_ERROR : RES_OK;
}


//=========================================================================
//�������ƣ�disk_ioctl                                                        
//���ܸ�Ҫ�������̿���                                                
//����˵����drv:�豸��
//         ctrl:����
//         buff:���ݵĻ������׵�ַ
//�������أ�RES_OK���ɹ���������ʧ��                                                              
//=========================================================================
DRESULT disk_ioctl(uint8_t drv,uint8_t ctrl,void  *buff)
{
    DRESULT 			 res;
    ESDHC_COMMAND_STRUCT command;
    SDCARD_STRUCT_PTR	 sdcard_ptr = (SDCARD_STRUCT_PTR)&SDHC_Card;	
    
    if (drv) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    
    res = RES_ERROR;
    switch (ctrl) 
    {
    case CTRL_SYNC :
        res = RES_OK;
        break;
    case GET_SECTOR_COUNT :
        *(unsigned long*)buff = sdcard_ptr->NUM_BLOCKS;
        res = RES_OK;
        break; 
    case GET_SECTOR_SIZE :
        *(unsigned short*)buff = IO_SDCARD_BLOCK_SIZE;
        res = RES_OK;
        break;
    case GET_BLOCK_SIZE :
        command.COMMAND  = ESDHC_CMD9;
        command.TYPE     = ESDHC_TYPE_NORMAL;
        command.ARGUMENT = sdcard_ptr->ADDRESS;
        command.READ     = FALSE;
        command.BLOCKS   = 0;
        if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
            return RES_ERROR;

        if (0 == (command.RESPONSE[3] & 0x00C00000)) //SD V1
            *(unsigned long*)buff = ((((command.RESPONSE[2] >> 18) & 0x7F) + 1) << (((command.RESPONSE[3] >> 8) & 0x03) - 1));
        else //SD V2
            *(unsigned long*)buff = (((command.RESPONSE[2] >> 18) & 0x7F) << (((command.RESPONSE[3] >> 8) & 0x03) - 1));				
        res = RES_OK;
        break;
    case CTRL_DISK_TYPE:
        res = sdcard_ptr->CARD_TYPE;
    default:
        res = RES_PARERR;
    }
    
    return res;
}

//=========================================================================
//�������ƣ�disk_status                                                        
//���ܸ�Ҫ������������״̬                                              
//����˵����drv�������̺�
//�������أ�״̬                                                        
//=========================================================================
uint8_t disk_status (uint8_t drv)
{
    if (drv) return STA_NOINIT;
    return Stat;
}

//=========================================================================
//�������ƣ�disk_timerproc                                                        
//���ܸ�Ҫ�����SD��״̬                                              
//����˵������
//�������أ���                                                        
//=========================================================================
void disk_timerproc (void)
{
    uint8_t s;
    
    Timer++;
    s = Stat;
    
    if (SDCARD_GPIO_PROTECT == 0)				      
    {
        s &= ~STA_PROTECT;	           //дʹ��
    }
    else					          
    {
        s |= STA_PROTECT;			   //д����
    }
    if (SDCARD_GPIO_DETECT == 0)	   //������
        s &= ~STA_NODISK;
    else					           //��������
        s |= (STA_NODISK | STA_NOINIT);
    
    Stat = s;				           //���¿�״̬
}
