#include "common.h"

//=========================================================================
//函数名称：delay_us
//参数说明：us:单位是us
//函数返回：无
//功能概要：延时函数，根据不同芯片更改内循环个数
//=========================================================================
void  delay_us(uint_32 us)
{
	uint_32 i,j;
	for(i=0;i<us;i++)
	{
		for(j=0;j<9;j++)    //根据主频调整
		{
			__asm("NOP");
		}
	}
}

//============================================================================
//函数名: is_timeout
//功  能: 超时判断 
//参  数: uint_32 LastTime，上次记录时间
//	    uint_32 NowTime，当前判断时间
//		uint_32 timeoutms，超时时间(ms)
//返  回: TRUE:超时返回；FALSE:未超时返回
//============================================================================
bool is_timeout(uint_32 lasttick, uint_32 timeoutms)
{
	bool bRet = FALSE;
	uint_32 ticknum = timeoutms / 5;
	MQX_TICK_STRUCT mqx_tick;

	_time_get_ticks(&mqx_tick);

	//TICKS已回头
	if(lasttick > mqx_tick.TICKS[0])
	{
		lasttick = 0xFFFFFFFF - lasttick;
		if(lasttick + mqx_tick.TICKS[0] >= ticknum)
			bRet = TRUE;
	}
	else if(lasttick + ticknum <= mqx_tick.TICKS[0])
	{
		bRet = TRUE;
	}

	return bRet;	
}

void delay_ms(uint_32 ms)
{
	MQX_TICK_STRUCT mqx_tick;

	_time_get_ticks(&mqx_tick);
		
	//等待超时退出
	while(is_timeout(mqx_tick.TICKS[0], ms) == FALSE);
}

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
uint_32 hex_to_string(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	const uint_8 tab[]="0123456789ABCDEF";	// 0x0-0xf的字符查找表
	uint_32 i;

	for (i = 0; i < nSrcLen; i++)
	{
		*pDst++ = tab[*pSrc >> 4];		// 输出高4位
		*pDst++ = tab[*pSrc & 0x0F];	// 输出低4位
		pSrc++;
	}

	// 输出字符串加个结束符
	*pDst = '\0';

	// 返回目标字符串长度
	return (nSrcLen << 1);
}

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
uint_32 string_to_hex(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 i = 0;

	//对于奇数，高位补0
	if((nSrcLen % 2) != 0)
	{
		// 输出低4位
		if((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst = *pSrc - '0';
		}
		else if((*pSrc >= 'A') && (*pSrc <= 'F'))
		{
			*pDst = *pSrc - 'A' + 10;
		}
		else if((*pSrc >= 'a') && (*pSrc <= 'f'))
		{
			*pDst = *pSrc - 'a' + 10;
		}

		i = 1;
		pSrc++;
		pDst++;
	}

	for(; i < nSrcLen; i += 2)
	{
		// 输出高4位
		if((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else if((*pSrc >= 'A') && (*pSrc <= 'F'))
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}
		else if((*pSrc >= 'a') && (*pSrc <= 'f'))
		{
			*pDst = (*pSrc - 'a' + 10) << 4;
		}

		pSrc++;

		// 输出低4位
		if((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst |= *pSrc - '0';
		}
		else if((*pSrc >= 'A') && (*pSrc <= 'F'))
		{
			*pDst |= *pSrc - 'A' + 10;
		}
		else if((*pSrc >= 'a') && (*pSrc <= 'f'))
		{
			*pDst |= *pSrc - 'a' + 10;
		}

		pSrc++;
		pDst++;
	}

	// 返回目标数据长度
	return ((nSrcLen + 1) >> 1);
}

