//============================================================================
//文件名称：flash.c
//功能概要：K64 Flash擦除/写入底层驱动程序源文件
//============================================================================

//包含头文件
#include "flash.h"


//=================内部调用函数声明==========================================
//==========================================================================
//函数名称：flash_sign_off
//函数返回：无
//参数说明：无
//功能概要：配置Flash存储控制器，清除Flash预读取缓冲
//==========================================================================
static void flash_sign_off(void);

//==========================================================================
//函数名称：flash_cmd_launch
//函数返回：0-成功 1-失败
//参数说明：无
//功能概要：启动Flash命令
//==========================================================================
static uint_32 flash_cmd_launch(void);

//===========================================================================

//=================外部接口函数==============================================
//==========================================================================
//函数名称：flash_init
//函数返回：无
//参数说明：无
//功能概要：初始化flash模块
//==========================================================================
void flash_init(void)
{
	//清除FMC缓冲区
    flash_sign_off();
    
    // 禁止看门狗
    WDOG_UNLOCK = 0xC520;
    WDOG_UNLOCK = 0xD928;
    WDOG_STCTRLH = 0;    // 禁止看门狗
    
    // 等待命令完成
    while(!(FTFE_FSTAT & CCIF));
    
    // 清除访问出错标志位
    FTFE_FSTAT = ACCERR | FPVIOL;
}

//==========================================================================
//函数名称：flash_erase_sector
//函数返回：函数执行执行状态：0=正常；非0=异常。
//参数说明：sectorNo：扇区号（MK64FX512实际使用0~127）
//功能概要：擦除指定flash扇区
//==========================================================================
uint_8 flash_erase_sector(uint_16 sectorNo)
{
    union
    {
        uint_32  word;
        uint_8   byte[4];
    } dest;
    
    dest.word    = (uint_32)(sectorNo*(1<<12));

    // 设置擦除命令
    FTFE_FCCOB0 = ERSSCR; // 擦除扇区命令
    
    // 设置目标地址
    FTFE_FCCOB1 = dest.byte[2];
    FTFE_FCCOB2 = dest.byte[1];
    FTFE_FCCOB3 = dest.byte[0];
    
    // 执行命令序列
    if(1 == flash_cmd_launch())    //若执行命令出现错误
        return 1;     //擦除命令错误
   
    // 若擦除sector0时，则解锁设备
    if(dest.word <= 0x800)
    {
        // 写入8字节
        FTFE_FCCOB0 = PGM8;
        // 设置目标地址
        FTFE_FCCOB1 = 0x00;
        FTFE_FCCOB2 = 0x04;
        FTFE_FCCOB3 = 0x0C;
        // 数据
        FTFE_FCCOB4 = 0xFF;
        FTFE_FCCOB5 = 0xFF;
        FTFE_FCCOB6 = 0xFF;
        FTFE_FCCOB7 = 0xFF;

        // 数据
        FTFE_FCCOB8 = 0xFF;
        FTFE_FCCOB9 = 0xFF;
        FTFE_FCCOBA = 0xFF;
        FTFE_FCCOBB = 0xFE;
        // 执行命令序列
        if(1 == flash_cmd_launch())  //若执行命令出现错误
            return 2;   //解锁命令错误
    }  
    
    return 0;  //成功返回
}

//==========================================================================
//函数名称：flash_write
//函数返回：函数执行状态：0=正常；非0=异常。
//参数说明：sectNo：目标扇区号 （MK64FX512实际使用0~127）
//         offset:写入扇区内部偏移地址（0~4087）
//         cnt：写入字节数目（0~4087）
//         buf：源数据缓冲区首地址
//功能概要：flash写入操作 
//==========================================================================
uint_8 flash_write(uint_16 sectNo,uint_16 offset,uint_16 cnt,uint_8 buf[])
{
    uint_32 size;
    uint_32 destaddr;
    
    union
    {
        uint_32   word;
        uint_8  byte[4];
    } dest;
    
    if(offset%8 != 0)
        return 1;   //参数设定错误，偏移量未对齐（8字节对齐）
    
    // 设置写入命令
    FTFE_FCCOB0 = PGM8;
    destaddr = (uint_32)(sectNo*(1<<12) + offset);//计算地址
    dest.word = destaddr;
    for(size=0; size<cnt; size+=8, dest.word+=8, buf+=8)
    {
        // 设置目标地址
        FTFE_FCCOB1 = dest.byte[2];
        FTFE_FCCOB2 = dest.byte[1];
        FTFE_FCCOB3 = dest.byte[0];
 
        // 拷贝数据
        FTFE_FCCOB4 = buf[3];
        FTFE_FCCOB5 = buf[2];
        FTFE_FCCOB6 = buf[1];
        FTFE_FCCOB7 = buf[0];

        FTFE_FCCOB8 = buf[7];
        FTFE_FCCOB9 = buf[6];
        FTFE_FCCOBA = buf[5];
        FTFE_FCCOBB = buf[4];
        
        if(1 == flash_cmd_launch())
            return 2;  //写入命令错误
    }
    
    return 0;  //成功执行
}

//==========================================================================
//函数名称：flash_protect
//函数返回：无
//参数说明：sectNo：保护扇区号（从0x0000地址开始）
//功能概要：flash保护操作
//说        明：进行扇区保护时，扇区保护的范围只能增加，不能减少；
//          128个扇区（0~127）平均分成32个区域，保护区域号（0~31），
//          每个区域包含4个扇区。例如，区域号0对应0~3扇区，区域号1对应4~7扇区。
//          四个八位寄存器共32位与32个区域一一对应，某位为1：不保护，为0：保护
//==========================================================================
void flash_protect(uint_8 sectNo)
{
	uint_8 regionNO;
	regionNO=sectNo/4;
	uint_8 offset;
	offset=regionNO%8;//获得偏移，即保护位号
	uint_8 regionCounter;
	regionCounter=3-regionNO/8;//获得应置位的寄存器号
	switch(regionCounter)
	{
	case 3:     //保护区域[7：0]中某区域（对应[28:0]开始的4个扇区，4扇区对齐）
		FTFE_FPROT3 &= ~(1<<offset);
		break;
	case 2:     //保护区域[15:8]中某区域（对应[60:32]开始的4个扇区，4扇区对齐）
		FTFE_FPROT2 &= ~(1<<offset);
		break;
	case 1:     //保护区域[23:16]中某区域（对应[92:64]开始的4个扇区，4扇区对齐）
		FTFE_FPROT1 &= ~(1<<offset);
		break;
	case 0:     //保护区域[31:24]中某区域（对应[124:96]开始的4个扇区，4扇区对齐）
		FTFE_FPROT0 &= ~(1<<offset);
		break;
	default:
		break;
	}
}


//=================内部函数实现=============================================
//==========================================================================
//函数名称：flash_sign_off
//函数返回：无
//参数说明：无
//功能概要：配置Flash存储控制器，清除Flash预读取缓冲
//==========================================================================
void flash_sign_off(void)
{  
    // 清除缓冲
    FMC_PFB0CR |= FMC_PFB0CR_S_B_INV_MASK;
    FMC_PFB1CR |= FMC_PFB0CR_S_B_INV_MASK;
}

//==========================================================================
//函数名称：flash_cmd_launch
//函数返回：0-成功 1-失败
//参数说明：无
//功能概要：启动Flash命令
//==========================================================================
static uint_32 flash_cmd_launch(void)
{
    // 清除访问错误标志位和非法访问标志位
    FTFE_FSTAT = ACCERR | FPVIOL;
    // 启动命令
    FTFE_FSTAT = CCIF;
    // 等待命令结束
    while(!(FTFE_FSTAT & CCIF));
//    for(;;)
//    {
//    	if((FTFE_FSTAT & CCIF))
//    		break;
//    	delay_us(5000);
//    }

    // 检查错误标志
    if(FTFE_FSTAT & (ACCERR | FPVIOL | MGSTAT0))
        return 1 ; //执行命令出错
  
    return 0; //执行命令成功
}
//==========================================================================
