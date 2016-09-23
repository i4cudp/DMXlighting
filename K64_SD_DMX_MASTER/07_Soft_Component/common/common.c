#include "common.h"

//=========================================================================
//�������ƣ�delay_us
//����˵����us:��λ��us
//�������أ���
//���ܸ�Ҫ����ʱ���������ݲ�ͬоƬ������ѭ������
//=========================================================================
void  delay_us(uint_32 us)
{
	uint_32 i,j;
	for(i=0;i<us;i++)
	{
		for(j=0;j<9;j++)    //������Ƶ����
		{
			__asm("NOP");
		}
	}
}

//============================================================================
//������: is_timeout
//��  ��: ��ʱ�ж� 
//��  ��: uint_32 LastTime���ϴμ�¼ʱ��
//	    uint_32 NowTime����ǰ�ж�ʱ��
//		uint_32 timeoutms����ʱʱ��(ms)
//��  ��: TRUE:��ʱ���أ�FALSE:δ��ʱ����
//============================================================================
bool is_timeout(uint_32 lasttick, uint_32 timeoutms)
{
	bool bRet = FALSE;
	uint_32 ticknum = timeoutms / 5;
	MQX_TICK_STRUCT mqx_tick;

	_time_get_ticks(&mqx_tick);

	//TICKS�ѻ�ͷ
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
		
	//�ȴ���ʱ�˳�
	while(is_timeout(mqx_tick.TICKS[0], ms) == FALSE);
}

//============================================================================
//������: hex_to_string
//��  ��:  �ֽ����ݰ�16����ת��Ϊ�ɴ�ӡ�ַ���
//                �磺{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������
//               pDst - Ŀ������ָ��
//��  ��: Ŀ�����ݳ���
//˵  ��:
//============================================================================
uint_32 hex_to_string(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	const uint_8 tab[]="0123456789ABCDEF";	// 0x0-0xf���ַ����ұ�
	uint_32 i;

	for (i = 0; i < nSrcLen; i++)
	{
		*pDst++ = tab[*pSrc >> 4];		// �����4λ
		*pDst++ = tab[*pSrc & 0x0F];	// �����4λ
		pSrc++;
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ���ַ�������
	return (nSrcLen << 1);
}

//-------------------------------------------------------------------------*
//������: string_to_hex
//��  ��:  �ɴ�ӡ�ַ�����16����ת��Ϊ�ֽ�����
//                �磺"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������
//               pDst - Ŀ������ָ��
//��  ��: Ŀ�����ݳ���
//˵  ��:
//-------------------------------------------------------------------------*
uint_32 string_to_hex(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 i = 0;

	//������������λ��0
	if((nSrcLen % 2) != 0)
	{
		// �����4λ
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
		// �����4λ
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

		// �����4λ
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

	// ����Ŀ�����ݳ���
	return ((nSrcLen + 1) >> 1);
}

