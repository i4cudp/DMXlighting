//=========================================================================
// 文件名称：diskio.h
// 功能概要：diskio构件源文件
// 版权所有: 苏州大学飞思卡尔嵌入式中心(sumcu.suda.edu.cn)
// 版本更新:  2011-12-14     V1.0        diskio构件初始版本
//           2011-12-20     V1.1        diskio构件优化修改
//=========================================================================

#include  "diskio.h"

//SD卡状态属性访问器
static volatile unsigned char Stat   = STA_NOINIT; //保存SD卡状态
uint8_t GetCardStat(void) { return Stat; }
void SetCardStat(uint8_t state) { Stat = state; }

static volatile unsigned int  Timer = 0; //读写时间

//=========================================================================
//函数名称：disk_initialize
//功能概要：初始化数据盘（只支持数据盘0） 
//参数说明：drv:设备号
//函数返回：RES_OK：成功；其它：失败
//=========================================================================
DRESULT disk_initialize (unsigned char drv)
{
    uint32_t  param, c_size, c_size_mult, read_bl_len;
    ESDHC_COMMAND_STRUCT  command;
    SDCARD_STRUCT_PTR     sdcard_ptr = (SDCARD_STRUCT_PTR)&SDHC_Card;

    //检查参数
    if (drv || (NULL == sdcard_ptr)) return RES_PARERR; //仅仅支持设备0
    //检查卡插入状态
    if (Stat & STA_NODISK) return RES_NOTRDY; //SD卡未插入

    //初始化SD卡设备登记信息
    sdcard_ptr->BITS = ESDHC_BUS_WIDTH_4BIT;//设置位宽为4位
    sdcard_ptr->SD_TIMEOUT = 10;
    sdcard_ptr->NUM_BLOCKS = 0;
    sdcard_ptr->ADDRESS    = 0;
    sdcard_ptr->ISSDHC     = FALSE;
    sdcard_ptr->VERSION2   = FALSE;

    //检测并初始化SD卡
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_INIT, NULL))
        return RES_ERROR;

    //获取SD卡类型
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

    //SD卡标识
    command.COMMAND  = ESDHC_CMD2;
    command.TYPE     = ESDHC_TYPE_NORMAL;
    command.ARGUMENT = 0;
    command.READ     = FALSE;
    command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;

    //获取SD卡地址
    command.COMMAND  = ESDHC_CMD3;
    command.TYPE     = ESDHC_TYPE_NORMAL;
    command.ARGUMENT = 0;
    command.READ     = FALSE;
    command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;
    sdcard_ptr->ADDRESS = command.RESPONSE[0] & 0xFFFF0000;
    
    //获取SD卡读写属性参数
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

    //选择SD卡
    command.COMMAND  = ESDHC_CMD7;
    command.TYPE     = ESDHC_TYPE_NORMAL;
    command.ARGUMENT = sdcard_ptr->ADDRESS;
    command.READ     = FALSE;
    command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;

    //设置块大小
    command.COMMAND  = ESDHC_CMD16;
    command.TYPE     = ESDHC_TYPE_NORMAL;
    command.ARGUMENT = IO_SDCARD_BLOCK_SIZE;
    command.READ     = FALSE;
    command.BLOCKS   = 0;
    if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        return RES_ERROR;

    if (ESDHC_BUS_WIDTH_4BIT == sdcard_ptr->BITS)
    {
        //发送特殊的命令CMD55
        command.COMMAND  = ESDHC_CMD55;
        command.TYPE     = ESDHC_TYPE_NORMAL;
        command.ARGUMENT = sdcard_ptr->ADDRESS;
        command.READ     = FALSE;
        command.BLOCKS  = 0;
        if (ESDHC_OK != sdhc_ioctl (IO_IOCTL_ESDHC_SEND_COMMAND, &command))
            return RES_ERROR;

        //设置数据位宽为4位
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

    Stat &= ~STA_NOINIT;//清除数据盘状态
    
    return RES_OK;
}

//=========================================================================
//函数名称：disk_read                                                        
//功能概要：读数据盘扇区                                                
//参数说明：drv:设备号
//         buff:用于存储读取的数据的缓存区
//         sector:起始扇区号
//         count:扇区数
//函数返回：RES_OK：成功，其它：失败                                                              
//=========================================================================
DRESULT disk_read(uint8_t drv,uint8_t *buff,uint32_t sector, uint8_t count)
{
    ESDHC_COMMAND_STRUCT command;
    SDCARD_STRUCT_PTR    sdcard_ptr = (SDCARD_STRUCT_PTR)&SDHC_Card;
	
    //检查参数
    if (drv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    if ((NULL == buff)) return RES_ERROR;
	
    if (!sdcard_ptr->ISSDHC)
        //sector *= 512;//扇区号转化为起始地址
        sector = sector << IO_SDCARD_BLOCK_SIZE_POWER;

    if (count == 1)//读单个扇区
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
    else //读多个扇区
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
//函数名称：disk_write                                                        
//功能概要：写数据盘扇区                                                
//参数说明：drv:设备号
//         buff:待写入SD卡的数据的缓存区首地址
//         sector:起始扇区号
//         count:扇区数
//函数返回：RES_OK：成功，其它：失败                                                              
//=========================================================================
DRESULT disk_write(uint8_t drv,const uint8_t *buff,uint32_t sector,uint8_t  count)
{
    ESDHC_COMMAND_STRUCT command;
    SDCARD_STRUCT_PTR    sdcard_ptr = (SDCARD_STRUCT_PTR)&SDHC_Card;

    //检查参数
    if (drv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    if (Stat & STA_PROTECT) return RES_WRPRT;

    //检测缓存区是否为空
    if ((NULL == buff)) return RES_ERROR;


    if (!sdcard_ptr->ISSDHC)
        //sector *= 512;	//扇区号转化为起始地址
        sector = sector << IO_SDCARD_BLOCK_SIZE_POWER; 

    if (count == 1)	//写单个扇区
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
    else //写多个扇区
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

    //等待完成
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
//函数名称：disk_ioctl                                                        
//功能概要：数据盘控制                                                
//参数说明：drv:设备号
//         ctrl:命令
//         buff:数据的缓存区首地址
//函数返回：RES_OK：成功，其它：失败                                                              
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
//函数名称：disk_status                                                        
//功能概要：返回数据盘状态                                              
//参数说明：drv：数据盘号
//函数返回：状态                                                        
//=========================================================================
uint8_t disk_status (uint8_t drv)
{
    if (drv) return STA_NOINIT;
    return Stat;
}

//=========================================================================
//函数名称：disk_timerproc                                                        
//功能概要：检测SD卡状态                                              
//参数说明：无
//函数返回：无                                                        
//=========================================================================
void disk_timerproc (void)
{
    uint8_t s;
    
    Timer++;
    s = Stat;
    
    if (SDCARD_GPIO_PROTECT == 0)				      
    {
        s &= ~STA_PROTECT;	           //写使能
    }
    else					          
    {
        s |= STA_PROTECT;			   //写保护
    }
    if (SDCARD_GPIO_DETECT == 0)	   //卡插入
        s &= ~STA_NODISK;
    else					           //卡不存在
        s |= (STA_NODISK | STA_NOINIT);
    
    Stat = s;				           //更新卡状态
}
