//----------------------------------------------------------------------------
//common.h
//2015��4��3��
//Author: SUMCU
//----------------------------------------------------------------------------

#ifndef COMMON_H_
#define COMMON_H_

// 1��оƬ�Ĵ���ӳ���ļ����������ں������ļ�
#include "core_cmFunc.h"
#include "core_cmInstr.h"
#include "core_cm4.h"
#include "MK64F12.h"           // ����оƬͷ�ļ�
#include "system_MK64F12.h"    // ����оƬϵͳ��ʼ���ļ�
#include "psptypes.h"
#include "mqxlite.h"
//#include "01_app_include.h"


// 2�����忪�����ж�
#define ENABLE_INTERRUPTS   __enable_irq   // �����ж�
#define DISABLE_INTERRUPTS  __disable_irq  // �����ж�


// 3��λ�����꺯������λ����λ����üĴ���һλ��״̬��
#define BSET(bit,Register)  ((Register)|= (1<<(bit)))    // �üĴ�����һλ
#define BCLR(bit,Register)  ((Register) &= ~(1<<(bit)))  // ��Ĵ�����һλ
#define BGET(bit,Register)  (((Register) >> (bit)) & 1)  // ��üĴ���һλ��״̬


//4���ض�������������ͣ����ͱ����궨�壩
typedef unsigned char        uint_8;   // �޷���8λ�����ֽ�
typedef unsigned short int   uint_16;  // �޷���16λ������
typedef unsigned long int    uint_32;  // �޷���32λ��������
typedef unsigned long long   uint_64;  // �޷���64λ����������

typedef signed char          int_8;    // �з��ŵ�8λ�����ֽ�
typedef signed short int     int_16;   // �з��ŵ�16λ������
typedef signed long int      int_32;   // �з��ŵ�32λ��������
typedef signed long long     int_64;   // �з��ŵ�64λ����������

//typedef unsigned char        bool;   // �޷���8λ�����ֽ�
#ifndef null
#define null  0L
#endif

//5. ����ϵͳʹ�õ�ʱ��Ƶ��
#define  SYSTEM_CLK_KHZ   SystemCoreClock/1000     // оƬϵͳʱ��Ƶ��(KHz)
#define  BUS_CLK_KHZ      SYSTEM_CLK_KHZ/2         // оƬ����ʱ��Ƶ��(KHz)

//��С��ת��
#define REVERSE32(n)   ((((uint_32)(n)&0x000000ff)<<24)|(((uint_32)(n)&0x0000ff00)<<8)|\
               (((uint_32)(n)&0x00ff0000)>>8)|(((uint_32)(n)&0xff000000)>>24))
#define REVERSE16(n) ((((uint_16)(n)&0x00ff) << 8) | (((uint_16)(n)&0xff00) >> 8))


//=========================================================================
//�������ƣ�delay_us
//����˵����us:��λ��us
//�������أ���
//���ܸ�Ҫ����ʱ���������ݲ�ͬоƬ������ѭ������
//=========================================================================
void  delay_us(uint_32 us);

//=========================================================================
//�������ƣ�delay_us
//����˵����us:��λ��us
//�������أ���
//���ܸ�Ҫ����ʱ���������ݲ�ͬоƬ������ѭ������
//=========================================================================
void delay_ms(uint_32 ms);

//============================================================================
//������: is_timeout
//��  ��: ��ʱ�ж� 
//��  ��: uint_32 LastTime���ϴμ�¼ʱ��
//	    uint_32 NowTime����ǰ�ж�ʱ��
//		uint_32 timeoutms����ʱʱ��(ms)
//��  ��: TRUE:��ʱ���أ�FALSE:δ��ʱ����
//============================================================================
bool is_timeout(uint_32 lasttick, uint_32 timeoutms);

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
uint_32 hex_to_string(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst);

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
uint_32 string_to_hex(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst);


#endif /* COMMON_H_ */
