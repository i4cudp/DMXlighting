/*
** ###################################################################
**     Processors:          MK64FN1M0VDC12
**                          MK64FN1M0VLL12
**                          MK64FN1M0VLQ12
**                          MK64FN1M0VMD12
**
**     Compilers:           Keil ARM C/C++ Compiler
**                          Freescale C/C++ for Embedded ARM
**                          GNU C Compiler
**                          GNU C Compiler - CodeSourcery Sourcery G++
**                          IAR ANSI C/C++ Compiler for ARM
**
**     Reference manual:    K64P144M120SF5RM, Rev.2, January 2014
**     Version:             rev. 2.8, 2015-02-19
**     Build:               b150223
**
**     Abstract:
**         Provides a system configuration function and a global variable that
**         contains the system frequency. It configures the device and initializes
**         the oscillator (PLL) that is part of the microcontroller device.
**
**     Copyright (c) 2015 Freescale Semiconductor, Inc.
**     All rights reserved.
**
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
**
**     o Redistributions of source code must retain the above copyright notice, this list
**       of conditions and the following disclaimer.
**
**     o Redistributions in binary form must reproduce the above copyright notice, this
**       list of conditions and the following disclaimer in the documentation and/or
**       other materials provided with the distribution.
**
**     o Neither the name of Freescale Semiconductor, Inc. nor the names of its
**       contributors may be used to endorse or promote products derived from this
**       software without specific prior written permission.
**
**     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
**     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**     http:                 www.freescale.com
**     mail:                 support@freescale.com
**
**     Revisions:
**     - rev. 1.0 (2013-08-12)
**         Initial version.
**     - rev. 2.0 (2013-10-29)
**         Register accessor macros added to the memory map.
**         Symbols for Processor Expert memory map compatibility added to the memory map.
**         Startup file for gcc has been updated according to CMSIS 3.2.
**         System initialization updated.
**         MCG - registers updated.
**         PORTA, PORTB, PORTC, PORTE - registers for digital filter removed.
**     - rev. 2.1 (2013-10-30)
**         Definition of BITBAND macros updated to support peripherals with 32-bit acces disabled.
**     - rev. 2.2 (2013-12-09)
**         DMA - EARS register removed.
**         AIPS0, AIPS1 - MPRA register updated.
**     - rev. 2.3 (2014-01-24)
**         Update according to reference manual rev. 2
**         ENET, MCG, MCM, SIM, USB - registers updated
**     - rev. 2.4 (2014-02-10)
**         The declaration of clock configurations has been moved to separate header file system_MK64F12.h
**         Update of SystemInit() and SystemCoreClockUpdate() functions.
**     - rev. 2.5 (2014-02-10)
**         The declaration of clock configurations has been moved to separate header file system_MK64F12.h
**         Update of SystemInit() and SystemCoreClockUpdate() functions.
**         Module access macro module_BASES replaced by module_BASE_PTRS.
**     - rev. 2.6 (2014-08-28)
**         Update of system files - default clock configuration changed.
**         Update of startup files - possibility to override DefaultISR added.
**     - rev. 2.7 (2014-10-14)
**         Interrupt INT_LPTimer renamed to INT_LPTMR0, interrupt INT_Watchdog renamed to INT_WDOG_EWM.
**     - rev. 2.8 (2015-02-19)
**         Renamed interrupt vector LLW to LLWU.
**
** ###################################################################
*/

/*!
 * @file MK64F12
 * @version 2.8
 * @date 2015-02-19
 * @brief Device specific configuration file for MK64F12 (implementation file)
 *
 * Provides a system configuration function and a global variable that contains
 * the system frequency. It configures the device and initializes the oscillator
 * (PLL) that is part of the microcontroller device.
 */

#include <stdint.h>
#include "MK64F12.h"



/* ----------------------------------------------------------------------------
   -- Core clock
   ---------------------------------------------------------------------------- */

uint32_t SystemCoreClock = 120000000;  //内核时钟的值
//uint32_t SystemCoreClock = DEFAULT_SYSTEM_CLOCK;  //内核时钟的值

/* ----------------------------------------------------------------------------
   -- SystemInit()
   ---------------------------------------------------------------------------- */

void SystemInit (void) {
#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
  SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));    /* set CP10, CP11 Full Access */
#endif /* ((__FPU_PRESENT == 1) && (__FPU_USED == 1)) */
#if (DISABLE_WDOG)
  /* WDOG->UNLOCK: WDOGUNLOCK=0xC520 */
  WDOG->UNLOCK = WDOG_UNLOCK_WDOGUNLOCK(0xC520); /* Key 1 */
  /* WDOG->UNLOCK: WDOGUNLOCK=0xD928 */
  WDOG->UNLOCK = WDOG_UNLOCK_WDOGUNLOCK(0xD928); /* Key 2 */
  /* WDOG->STCTRLH: ?=0,DISTESTWDOG=0,BYTESEL=0,TESTSEL=0,TESTWDOG=0,?=0,?=1,WAITEN=1,STOPEN=1,DBGEN=0,ALLOWUPDATE=1,WINEN=0,IRQRSTEN=0,CLKSRC=1,WDOGEN=0 */
  WDOG->STCTRLH = WDOG_STCTRLH_BYTESEL(0x00) |
                 WDOG_STCTRLH_WAITEN_MASK |
                 WDOG_STCTRLH_STOPEN_MASK |
                 WDOG_STCTRLH_ALLOWUPDATE_MASK |
                 WDOG_STCTRLH_CLKSRC_MASK |
                 0x0100U;
#endif /* (DISABLE_WDOG) */

  //时钟初始化
  //使用外部50M晶振
  SystemClockSetup(ClockSource_EX50M,CoreClock_120M);
//  SystemClockSetup(ClockSource_EX50M,CoreClock_200M);

  //使用内部32.768K时钟源
//  SystemClockSetup(ClockSource_IRC,CoreClock_96M);


  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK |
		       SIM_SCGC5_PORTB_MASK |
			   SIM_SCGC5_PORTC_MASK |
			   SIM_SCGC5_PORTD_MASK |
			   SIM_SCGC5_PORTE_MASK;
}

/**
 * @brief  设置/更改系统主频。
 * @param  ClockOption  时钟源选择,选择如下：
 *                         ClockSource_IRC        : 内部32K时钟源
 *                         ClockSource_EX4M       : 外部4M晶振
 *                         ClockSource_EX50M      : 外部50M晶振
 * @param  CoreClock    希望得到的内核频率
 *                         CoreClock_48M  : CoreClock = 48M
 *                         CoreClock_64M  : CoreClock = 64M
 *                         CoreClock_72M  : CoreClock = 72M
 *                         CoreClock_96M  : CoreClock = 96M
 *                         CoreClock_100M : CoreClock = 100M
 *                         CoreClock_120M : CoreClock = 120M
 * @retval None
 *
 * 使用该函数来选择时钟源经过倍频得到希望的内核时钟。
 */
void SystemClockSetup(uint8_t ClockOption,uint16_t CoreClock)
{
	//内部32K时钟源
	if(ClockOption == ClockSource_IRC)
	{
		// 进入 FEI 模式
		MCG_C1 = (uint8_t)0x06u;  //内部32KHz时钟被选择
		MCG_C2 = (uint8_t)0x20u;
		MCG_C4 &= ~((1<<6)|(1<<7)|(1<<5));
		switch(CoreClock)
		{
			case CoreClock_96M:
				MCG_C4|= (1<<6)|(1<<7)|(1<<5);
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
						|SIM_CLKDIV1_OUTDIV3(1)|SIM_CLKDIV1_OUTDIV4(3));

				MCG->SC = 0x00;   //20150821

				SystemCoreClock    = 96000000;
				break;
			case CoreClock_72M:
				MCG_C4|= (1<<6)|(1<<7)|(0<<5);
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
						|SIM_CLKDIV1_OUTDIV3(1)|SIM_CLKDIV1_OUTDIV4(2));
				SystemCoreClock    = 72000000;
				break;
			case CoreClock_48M:
				MCG_C4|= (0<<6)|(1<<7)|(1<<5);
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(0)
						|SIM_CLKDIV1_OUTDIV3(0)|SIM_CLKDIV1_OUTDIV4(1));
				SystemCoreClock    = 48000000;
				break;
			default: //48M
				MCG_C4|= (0<<6)|(1<<7)|(1<<5);
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(0)
						|SIM_CLKDIV1_OUTDIV3(0)|SIM_CLKDIV1_OUTDIV4(1));
				SystemCoreClock    = 48000000;
				break;
		}
		MCG_C5 = (uint8_t)0x00u;
		MCG_C6 = (uint8_t)0x00u;
		while((MCG_S & MCG_S_IREFST_MASK) == 0u);  //检查 FLL参考时钟是内部参考时钟
		while((MCG_S & MCG_S_CLKST_MASK ) != 0x00u);           //等待FLL被选择

		//20150821
		// Use LPTMR to wait for 1ms dor FLL clock stabilization
	    SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;   /* Alow software control of LPMTR */
	    LPTMR0->CMR = LPTMR_CMR_COMPARE(0);  /* Default 1 LPO tick */
	    LPTMR0->CSR = (LPTMR_CSR_TCF_MASK | LPTMR_CSR_TPS(0x00));
	    LPTMR0->PSR = (LPTMR_PSR_PCS(0x01) | LPTMR_PSR_PBYP_MASK); /* Clock source: LPO, Prescaler bypass enable */
	    LPTMR0->CSR = LPTMR_CSR_TEN_MASK;    /* LPMTR enable */
	    while((LPTMR0_CSR & LPTMR_CSR_TCF_MASK) == 0u) {
	    }
	    LPTMR0_CSR = 0x00;                   /* Disable LPTMR */
	    SIM_SCGC5 &= (uint32_t)~(uint32_t)SIM_SCGC5_LPTMR_MASK;

	}
	if(ClockOption == ClockSource_EX4M || ClockOption == ClockSource_EX50M)
	{
		SIM_SCGC5 |= (uint32_t)0x0200UL; // 使能引脚时钟门控，从外部获得时钟源
		if ( *((uint8_t*) 0x03FFU) != 0xFFU)
		{
			MCG_C3 = *((uint8_t*) 0x03FFU);
			MCG_C4 = (MCG_C4 & 0xE0U) | ((*((uint8_t*) 0x03FEU)) & 0x1FU);
		}
		SIM_CLKDIV1 = (uint32_t)0x01130000UL; // 更新时钟系统预分频
		SIM_CLKDIV2 = (uint32_t)((SIM_CLKDIV2 & (uint32_t)~0x0DUL)
					| (uint32_t)0x02UL);      // 更新USB时钟与分频
		SIM_SOPT2 |= (uint32_t)0x00010000UL;  // 选定PLL为外设时钟源
		SIM_SOPT1 |= (uint32_t)0x00080000UL;  // 配置32kHz时钟源
		PORTA_PCR18 &= (uint32_t)~0x01000700UL;

		//进入 FEI 模式，转到 FBE 模式
		OSC_CR = (uint8_t)0x00u;
		SIM_SOPT2 &= (uint8_t)~(uint8_t)0x01u;
		MCG_C2 = (uint8_t)0x21u;    //选用外部高频参考时钟
		MCG_C1 = (uint8_t)0xA8u;    //选用外部参考时钟，分频因子为1024

		while((MCG_S & MCG_S_IREFST_MASK) != 0u); //检查 FLL参考时钟是外部参考时钟
		while((MCG_S & 0x0Cu) != 0x08u);          //等待外部参考时钟被选定为MCG输出

		//进入FBE，转到PBE模式
		if(ClockOption == ClockSource_EX4M)
		{
			MCG_C5 = (uint8_t)(0x20|MCG_C5_PRDIV0(1));        //4/2 = 2M
		}
		else if (ClockOption == ClockSource_EX50M)
		{
			MCG_C5 = (uint8_t)(0x20|MCG_C5_PRDIV0(24));       //50/25 = 2M
		}
		switch(CoreClock)
		{
			case CoreClock_48M:
				MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(0));
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(0)
						|SIM_CLKDIV1_OUTDIV3(0)|SIM_CLKDIV1_OUTDIV4(1));
				SystemCoreClock    = 48000000;
				break;
			case CoreClock_64M:
				MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(8));
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
						|SIM_CLKDIV1_OUTDIV3(1)|SIM_CLKDIV1_OUTDIV4(2));
				SystemCoreClock    = 64000000;
				break;
			case CoreClock_72M:
				MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(12));
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
						|SIM_CLKDIV1_OUTDIV3(1)|SIM_CLKDIV1_OUTDIV4(2));
				SystemCoreClock    = 72000000;
				break;
			case CoreClock_96M:
				MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(24));
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
						|SIM_CLKDIV1_OUTDIV3(1)|SIM_CLKDIV1_OUTDIV4(3));
				SystemCoreClock    = 96000000;
				break;

			case CoreClock_100M:
				MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(26));
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
						|SIM_CLKDIV1_OUTDIV3(1)|SIM_CLKDIV1_OUTDIV4(3));
				SystemCoreClock    = 100000000;
				break;

			case CoreClock_120M:
				if(ClockOption == ClockSource_EX4M)
				{
					MCG_C5 = (uint8_t)MCG_C5_PRDIV0(0);        //4/1 = 4M
					MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(6));
					SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
							|SIM_CLKDIV1_OUTDIV3(1)|SIM_CLKDIV1_OUTDIV4(7));
				}
				else if (ClockOption == ClockSource_EX50M)
				{
					MCG_C5 = (uint8_t)MCG_C5_PRDIV0(19);     //50/20
					MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(24)); //50/20*48 = 120M
					SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
						   |SIM_CLKDIV1_OUTDIV3(2)|SIM_CLKDIV1_OUTDIV4(4));
				}
				SystemCoreClock    = 120000000;
			   break;

			case CoreClock_200M:
				if(ClockOption == ClockSource_EX4M)
				{
					MCG_C5 = (uint8_t)MCG_C5_PRDIV0(0);        //4/1 = 4M
					MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(26));
					SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
							|SIM_CLKDIV1_OUTDIV3(3)|SIM_CLKDIV1_OUTDIV4(7));
				}
				else if (ClockOption == ClockSource_EX50M)
				{
					MCG_C5 = (uint8_t)MCG_C5_PRDIV0(12);
					MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(28));
					SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(1)
							|SIM_CLKDIV1_OUTDIV3(3)|SIM_CLKDIV1_OUTDIV4(7));
				}
				SystemCoreClock    = 200000000;
				break;
			default:
				MCG_C6 = (uint8_t)(0x40u|MCG_C6_VDIV0(0));
				SIM_CLKDIV1 =(SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV2(0)
						|SIM_CLKDIV1_OUTDIV3(0)|SIM_CLKDIV1_OUTDIV4(1));
				SystemCoreClock    = 48000000;
				break;
		}
		while((MCG_S & 0x0Cu) != 0x08u);           //等待外部参考时钟被选定为MCG输出
		while((MCG_S & MCG_S_LOCK0_MASK) == 0u);    //等待锁定

		//转到PEE
		MCG_C1 = (uint8_t)0x28u;                   //选择PLL“作为时钟源
		while((MCG_S & 0x0Cu) != 0x0Cu);           //等待PLL输出
		while((MCG_S & MCG_S_LOCK0_MASK) == 0u);    //等待PLL锁定
		MCG_C6 |= (uint8_t)0x20U; // 使能时钟监控
	}
}



/* ----------------------------------------------------------------------------
   -- SystemCoreClockUpdate()
   ---------------------------------------------------------------------------- */

void SystemCoreClockUpdate (void) {
  uint32_t MCGOUTClock;                /* Variable to store output clock frequency of the MCG module */
  uint16_t Divider;

  if ((MCG->C1 & MCG_C1_CLKS_MASK) == 0x00U) {
    /* Output of FLL or PLL is selected */
    if ((MCG->C6 & MCG_C6_PLLS_MASK) == 0x00U) {
      /* FLL is selected */
      if ((MCG->C1 & MCG_C1_IREFS_MASK) == 0x00U) {
        /* External reference clock is selected */
        switch (MCG->C7 & MCG_C7_OSCSEL_MASK) {
        case 0x00U:
          MCGOUTClock = CPU_XTAL_CLK_HZ; /* System oscillator drives MCG clock */
          break;
        case 0x01U:
          MCGOUTClock = CPU_XTAL32k_CLK_HZ; /* RTC 32 kHz oscillator drives MCG clock */
          break;
        case 0x02U:
        default:
          MCGOUTClock = CPU_INT_IRC_CLK_HZ; /* IRC 48MHz oscillator drives MCG clock */
          break;
        }
        if (((MCG->C2 & MCG_C2_RANGE_MASK) != 0x00U) && ((MCG->C7 & MCG_C7_OSCSEL_MASK) != 0x01U)) {
          switch (MCG->C1 & MCG_C1_FRDIV_MASK) {
          case 0x38U:
            Divider = 1536U;
            break;
          case 0x30U:
            Divider = 1280U;
            break;
          default:
            Divider = (uint16_t)(32LU << ((MCG->C1 & MCG_C1_FRDIV_MASK) >> MCG_C1_FRDIV_SHIFT));
            break;
          }
        } else {/* ((MCG->C2 & MCG_C2_RANGE_MASK) != 0x00U) */
          Divider = (uint16_t)(1LU << ((MCG->C1 & MCG_C1_FRDIV_MASK) >> MCG_C1_FRDIV_SHIFT));
        }
        MCGOUTClock = (MCGOUTClock / Divider); /* Calculate the divided FLL reference clock */
      } else { /* (!((MCG->C1 & MCG_C1_IREFS_MASK) == 0x00U)) */
        MCGOUTClock = CPU_INT_SLOW_CLK_HZ; /* The slow internal reference clock is selected */
      } /* (!((MCG->C1 & MCG_C1_IREFS_MASK) == 0x00U)) */
      /* Select correct multiplier to calculate the MCG output clock  */
      switch (MCG->C4 & (MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS_MASK)) {
        case 0x00U:
          MCGOUTClock *= 640U;
          break;
        case 0x20U:
          MCGOUTClock *= 1280U;
          break;
        case 0x40U:
          MCGOUTClock *= 1920U;
          break;
        case 0x60U:
          MCGOUTClock *= 2560U;
          break;
        case 0x80U:
          MCGOUTClock *= 732U;
          break;
        case 0xA0U:
          MCGOUTClock *= 1464U;
          break;
        case 0xC0U:
          MCGOUTClock *= 2197U;
          break;
        case 0xE0U:
          MCGOUTClock *= 2929U;
          break;
        default:
          break;
      }
    } else { /* (!((MCG->C6 & MCG_C6_PLLS_MASK) == 0x00U)) */
      /* PLL is selected */
      Divider = (((uint16_t)MCG->C5 & MCG_C5_PRDIV0_MASK) + 0x01U);
      MCGOUTClock = (uint32_t)(CPU_XTAL_CLK_HZ / Divider); /* Calculate the PLL reference clock */
      Divider = (((uint16_t)MCG->C6 & MCG_C6_VDIV0_MASK) + 24U);
      MCGOUTClock *= Divider;          /* Calculate the MCG output clock */
    } /* (!((MCG->C6 & MCG_C6_PLLS_MASK) == 0x00U)) */
  } else if ((MCG->C1 & MCG_C1_CLKS_MASK) == 0x40U) {
    /* Internal reference clock is selected */
    if ((MCG->C2 & MCG_C2_IRCS_MASK) == 0x00U) {
      MCGOUTClock = CPU_INT_SLOW_CLK_HZ; /* Slow internal reference clock selected */
    } else { /* (!((MCG->C2 & MCG_C2_IRCS_MASK) == 0x00U)) */
      Divider = (uint16_t)(0x01LU << ((MCG->SC & MCG_SC_FCRDIV_MASK) >> MCG_SC_FCRDIV_SHIFT));
      MCGOUTClock = (uint32_t) (CPU_INT_FAST_CLK_HZ / Divider); /* Fast internal reference clock selected */
    } /* (!((MCG->C2 & MCG_C2_IRCS_MASK) == 0x00U)) */
  } else if ((MCG->C1 & MCG_C1_CLKS_MASK) == 0x80U) {
    /* External reference clock is selected */
    switch (MCG->C7 & MCG_C7_OSCSEL_MASK) {
    case 0x00U:
      MCGOUTClock = CPU_XTAL_CLK_HZ;   /* System oscillator drives MCG clock */
      break;
    case 0x01U:
      MCGOUTClock = CPU_XTAL32k_CLK_HZ; /* RTC 32 kHz oscillator drives MCG clock */
      break;
    case 0x02U:
    default:
      MCGOUTClock = CPU_INT_IRC_CLK_HZ; /* IRC 48MHz oscillator drives MCG clock */
      break;
    }
  } else { /* (!((MCG->C1 & MCG_C1_CLKS_MASK) == 0x80U)) */
    /* Reserved value */
    return;
  } /* (!((MCG->C1 & MCG_C1_CLKS_MASK) == 0x80U)) */
  SystemCoreClock = (MCGOUTClock / (0x01U + ((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> SIM_CLKDIV1_OUTDIV1_SHIFT)));
}
