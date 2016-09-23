//----------------------------------------------------------------------------
//common.h
//2015年4月3日
//Author: SUMCU
//----------------------------------------------------------------------------

#ifndef COMMON_H_
#define COMMON_H_

// 1．芯片寄存器映射文件及处理器内核属性文件
#include "core_cmFunc.h"
#include "core_cmInstr.h"
#include "core_cm4.h"
#include "MK64F12.h"           // 包含芯片头文件
#include "system_MK64F12.h"    // 包含芯片系统初始化文件
#include "psptypes.h"
#include "mqxlite.h"
//#include "01_app_include.h"


// 2．定义开关总中断
#define ENABLE_INTERRUPTS   __enable_irq   // 开总中断
#define DISABLE_INTERRUPTS  __disable_irq  // 关总中断


// 3．位操作宏函数（置位、清位、获得寄存器一位的状态）
#define BSET(bit,Register)  ((Register)|= (1<<(bit)))    // 置寄存器的一位
#define BCLR(bit,Register)  ((Register) &= ~(1<<(bit)))  // 清寄存器的一位
#define BGET(bit,Register)  (((Register) >> (bit)) & 1)  // 获得寄存器一位的状态


//4．重定义基本数据类型（类型别名宏定义）
typedef unsigned char        uint_8;   // 无符号8位数，字节
typedef unsigned short int   uint_16;  // 无符号16位数，字
typedef unsigned long int    uint_32;  // 无符号32位数，长字
typedef unsigned long long   uint_64;  // 无符号64位数，长长字

typedef signed char          int_8;    // 有符号的8位数，字节
typedef signed short int     int_16;   // 有符号的16位数，字
typedef signed long int      int_32;   // 有符号的32位数，长字
typedef signed long long     int_64;   // 有符号的64位数，长长字

//typedef unsigned char        bool;   // 无符号8位数，字节
#ifndef null
#define null  0L
#endif

//5. 定义系统使用的时钟频率
#define  SYSTEM_CLK_KHZ   SystemCoreClock/1000     // 芯片系统时钟频率(KHz)
#define  BUS_CLK_KHZ      SYSTEM_CLK_KHZ/2         // 芯片总线时钟频率(KHz)

//大小端转换
#define REVERSE32(n)   ((((uint_32)(n)&0x000000ff)<<24)|(((uint_32)(n)&0x0000ff00)<<8)|\
               (((uint_32)(n)&0x00ff0000)>>8)|(((uint_32)(n)&0xff000000)>>24))
#define REVERSE16(n) ((((uint_16)(n)&0x00ff) << 8) | (((uint_16)(n)&0xff00) >> 8))


//=========================================================================
//函数名称：delay_us
//参数说明：us:单位是us
//函数返回：无
//功能概要：延时函数，根据不同芯片更改内循环个数
//=========================================================================
void  delay_us(uint_32 us);

//=========================================================================
//函数名称：delay_us
//参数说明：us:单位是us
//函数返回：无
//功能概要：延时函数，根据不同芯片更改内循环个数
//=========================================================================
void delay_ms(uint_32 ms);

//============================================================================
//函数名: is_timeout
//功  能: 超时判断 
//参  数: uint_32 LastTime，上次记录时间
//	    uint_32 NowTime，当前判断时间
//		uint_32 timeoutms，超时时间(ms)
//返  回: TRUE:超时返回；FALSE:未超时返回
//============================================================================
bool is_timeout(uint_32 lasttick, uint_32 timeoutms);

//============================================================================
//函数名: hex_to_string
//功  能:  字节数据按16进制转换为可打印字符串
//                如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度
//               pDst - 目标数据指针
//返  回: 目标数据长度
//说  明:
//============================================================================
uint_32 hex_to_string(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst);

//-------------------------------------------------------------------------*
//函数名: string_to_hex
//功  能:  可打印字符串按16进制转换为字节数据
//                如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度
//               pDst - 目标数据指针
//返  回: 目标数据长度
//说  明:
//-------------------------------------------------------------------------*
uint_32 string_to_hex(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst);


#endif /* COMMON_H_ */
