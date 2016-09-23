//===========================================================================
//文件名称：DMX512.h
//功能概要：DMX512信号输出驱动
//版权所有：苏州大学嵌入式中心(sumcu.suda.edu.cn)
//版本更新：2016-04-24
//===========================================================================

#ifndef _DMX512_H     //防止重复定义（_DMX512_H  开头)
#define _DMX512_H

//头文件包含
#include "common.h"    //包含公共要素头文件
#include "gpio.h"      //用到gpio构件
#include "uart.h"      //用到uart构件

#include "01_app_include.h"

////UART_0：1=PTA2~1脚，2=PTA14~15脚，3=PTB17~16脚，4=PTD7~6脚（TX、RX）
//#define DMXCOM_0_GROUP    4
////
////UART_1: 2=PTE0~1脚，1=PTC4~3脚（TX、RX）
//#define DMXCOM_1_GROUP    1
////
////UART_2:1=PTD3~2脚（TX、RX）
//#define DMXCOM_2_GROUP    1
////
////UART_3：1=PTB11~10脚，2=PTC17~16脚，3=PTE4~5脚（TX、RX）
//#define DMXCOM_3_GROUP    1
//
////UART_4：1=PTC15~14脚，2=PTE24~25脚（TX、RX）
//#define DMXCOM_4_GROUP    1
//
////UART_5：1=PTD9~8脚，2=PTE8~9脚
//#define DMXCOM_5_GROUP    2

//根据硬件连接修改
//输送DMX512数据的
#define CH1_TX       (PTB_NUM|11)
#define CH1_UART     UART_3
#define CH1_DIR      (PTC_NUM|13)

#define CH2_TX       (PTB_NUM|17)
#define CH2_UART     UART_0
#define CH2_DIR      (PTC_NUM|18)
//#define CH2_TX       (PTC_NUM|18)  //CH2不输出
//#define CH2_UART     UART_2
//#define CH2_DIR      (PTC_NUM|18)

#define CH3_TX      (PTD_NUM|7)
#define CH3_UART    UART_0
#define CH3_DIR     (PTE_NUM|6)

#define CH4_TX      (PTA_NUM|14)
#define CH4_UART    UART_0
#define CH4_DIR     (PTA_NUM|13)

#define K64_TX      (PTC_NUM|4)
#define K64_UART    UART_1
#define K64_DIR     (PTC_NUM|5)


#define CH5_TX
#define CH5_UART

//写地址的数据线
//#define ADDR_UART    UART_0
//#define ADDR_TX      (PTD_NUM|7)


#define DMX_BOUND 750000
//===========================================================================
//函数名称：Chip_DMX512APN_WriteAddr
//函数返回：0:写址成功
//       1:通道数有误
//       2:超过目前最大的写址个数
//参数说明：channelNum：通道号 0-8,8通道用作其他用途
//       startAddr：起始地址，12位最大地址是4096
//       stepChannel：通道数，这里设置3或者4
//	     chipNum:芯片数目，指并联的芯片数目
//功能概要：对DMX512APN写址，设置串口发送方式，波特率为250Kbps,先发送低地址字节，再发送高字节地址，最后发送校验码。
//       符合DMX512协议，一次写多个芯片的地址，写址时序参考文档
//===========================================================================
uint8_t Chip_DMX512APN_WriteAddr(uint8_t channelNum, uint16_t startAddr,
		uint8_t stepChannel,uint16_t chipNum);


//===========================================================================
//函数名称：Chip_SM16512_WriteAddr
//函数返回：0:写址成功
//       1:通道数有误
//       2:超过目前最大的写址个数
//参数说明：channelNum：通道号 0-8,8通道用作其他用途
//       startAddr：起始地址，12位最大地址是4096
//       stepChannel：通道数，这里设置3或者4
//	     chipNum:芯片数目，指并联的芯片数目
//功能概要：对SM16512写址，设置串口发送方式，波特率为250Kbps,先发送低地址字节，再发送高字节地址，
//       最后发送校验码。符合DMX512协议，一次写多个芯片的地址，写址时序参考文档
//===========================================================================
uint8_t Chip_SM16512_WriteAddr(uint8_t channelNum, uint16_t startAddr,
		uint8_t stepChannel,uint16_t chipNum);

//===========================================================================
//函数名称：DMX512_WriteData
//函数返回：0:写址成功
//       1:通道数有误
//       2:超过目前最大的写址个数
//       3:指针为空，写数据失败
//参数说明：channelNum：通道号 0-8,8通道用作其他用途
//       startAddr：起始地址，12位最大地址是4096
//       stepChannel：通道数，这里设置3或者4
//	     chipNum:芯片数目，指并联的芯片数目
//       Red,Green,Blue,White:RGB(W)三种(四种)颜色是一个像素点，亮度等级是0-255
//       当stepChannel为4的时候，发送白色像素
//功能概要：设置串口发送方式，波特率为250Kbps,每个灯珠由RGB三个像素点组成(或四色)
//===========================================================================
uint8_t DMX512_WriteData(uint8_t channelNum, int16_t startAddr,
		uint8_t stepChannel, uint16_t chipNum, uint8_t *pColor);

#endif    //防止重复定义（_DMX512_H  结尾)


//===========================================================================
//声明：
//（1）我们开发的源代码，在本中心提供的硬件系统测试通过，真诚奉献给社会，不足之处，
//     欢迎指正。
//（2）对于使用非本中心硬件系统的用户，移植代码时，请仔细根据自己的硬件匹配。
//
//苏州大学飞思卡尔嵌入式中心（苏州华祥信息科技有限公司）
//技术咨询：0512-65214835  http://sumcu.suda.edu.cn
//业务咨询：0512-87661670,18915522016  http://www.hxtek.com.cn
