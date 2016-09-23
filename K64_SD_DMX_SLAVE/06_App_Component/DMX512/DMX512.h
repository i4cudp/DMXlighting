//===========================================================================
//�ļ����ƣ�DMX512.h
//���ܸ�Ҫ��DMX512�ź��������
//��Ȩ���У����ݴ�ѧǶ��ʽ����(sumcu.suda.edu.cn)
//�汾���£�2016-04-24
//===========================================================================

#ifndef _DMX512_H     //��ֹ�ظ����壨_DMX512_H  ��ͷ)
#define _DMX512_H

//ͷ�ļ�����
#include "common.h"    //��������Ҫ��ͷ�ļ�
#include "gpio.h"      //�õ�gpio����
#include "uart.h"      //�õ�uart����

#include "01_app_include.h"

////UART_0��1=PTA2~1�ţ�2=PTA14~15�ţ�3=PTB17~16�ţ�4=PTD7~6�ţ�TX��RX��
//#define DMXCOM_0_GROUP    4
////
////UART_1: 2=PTE0~1�ţ�1=PTC4~3�ţ�TX��RX��
//#define DMXCOM_1_GROUP    1
////
////UART_2:1=PTD3~2�ţ�TX��RX��
//#define DMXCOM_2_GROUP    1
////
////UART_3��1=PTB11~10�ţ�2=PTC17~16�ţ�3=PTE4~5�ţ�TX��RX��
//#define DMXCOM_3_GROUP    1
//
////UART_4��1=PTC15~14�ţ�2=PTE24~25�ţ�TX��RX��
//#define DMXCOM_4_GROUP    1
//
////UART_5��1=PTD9~8�ţ�2=PTE8~9��
//#define DMXCOM_5_GROUP    2

//����Ӳ�������޸�
//����DMX512���ݵ�
#define CH1_TX       (PTB_NUM|11)
#define CH1_UART     UART_3
#define CH1_DIR      (PTC_NUM|13)

#define CH2_TX       (PTB_NUM|17)
#define CH2_UART     UART_0
#define CH2_DIR      (PTC_NUM|18)
//#define CH2_TX       (PTC_NUM|18)  //CH2�����
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

//д��ַ��������
//#define ADDR_UART    UART_0
//#define ADDR_TX      (PTD_NUM|7)


#define DMX_BOUND 750000
//===========================================================================
//�������ƣ�Chip_DMX512APN_WriteAddr
//�������أ�0:дַ�ɹ�
//       1:ͨ��������
//       2:����Ŀǰ����дַ����
//����˵����channelNum��ͨ���� 0-8,8ͨ������������;
//       startAddr����ʼ��ַ��12λ����ַ��4096
//       stepChannel��ͨ��������������3����4
//	     chipNum:оƬ��Ŀ��ָ������оƬ��Ŀ
//���ܸ�Ҫ����DMX512APNдַ�����ô��ڷ��ͷ�ʽ��������Ϊ250Kbps,�ȷ��͵͵�ַ�ֽڣ��ٷ��͸��ֽڵ�ַ�������У���롣
//       ����DMX512Э�飬һ��д���оƬ�ĵ�ַ��дַʱ��ο��ĵ�
//===========================================================================
uint8_t Chip_DMX512APN_WriteAddr(uint8_t channelNum, uint16_t startAddr,
		uint8_t stepChannel,uint16_t chipNum);


//===========================================================================
//�������ƣ�Chip_SM16512_WriteAddr
//�������أ�0:дַ�ɹ�
//       1:ͨ��������
//       2:����Ŀǰ����дַ����
//����˵����channelNum��ͨ���� 0-8,8ͨ������������;
//       startAddr����ʼ��ַ��12λ����ַ��4096
//       stepChannel��ͨ��������������3����4
//	     chipNum:оƬ��Ŀ��ָ������оƬ��Ŀ
//���ܸ�Ҫ����SM16512дַ�����ô��ڷ��ͷ�ʽ��������Ϊ250Kbps,�ȷ��͵͵�ַ�ֽڣ��ٷ��͸��ֽڵ�ַ��
//       �����У���롣����DMX512Э�飬һ��д���оƬ�ĵ�ַ��дַʱ��ο��ĵ�
//===========================================================================
uint8_t Chip_SM16512_WriteAddr(uint8_t channelNum, uint16_t startAddr,
		uint8_t stepChannel,uint16_t chipNum);

//===========================================================================
//�������ƣ�DMX512_WriteData
//�������أ�0:дַ�ɹ�
//       1:ͨ��������
//       2:����Ŀǰ����дַ����
//       3:ָ��Ϊ�գ�д����ʧ��
//����˵����channelNum��ͨ���� 0-8,8ͨ������������;
//       startAddr����ʼ��ַ��12λ����ַ��4096
//       stepChannel��ͨ��������������3����4
//	     chipNum:оƬ��Ŀ��ָ������оƬ��Ŀ
//       Red,Green,Blue,White:RGB(W)����(����)��ɫ��һ�����ص㣬���ȵȼ���0-255
//       ��stepChannelΪ4��ʱ�򣬷��Ͱ�ɫ����
//���ܸ�Ҫ�����ô��ڷ��ͷ�ʽ��������Ϊ250Kbps,ÿ��������RGB�������ص����(����ɫ)
//===========================================================================
uint8_t DMX512_WriteData(uint8_t channelNum, int16_t startAddr,
		uint8_t stepChannel, uint16_t chipNum, uint8_t *pColor);

#endif    //��ֹ�ظ����壨_DMX512_H  ��β)


//===========================================================================
//������
//��1�����ǿ�����Դ���룬�ڱ������ṩ��Ӳ��ϵͳ����ͨ������Ϸ��׸���ᣬ����֮����
//     ��ӭָ����
//��2������ʹ�÷Ǳ�����Ӳ��ϵͳ���û�����ֲ����ʱ������ϸ�����Լ���Ӳ��ƥ�䡣
//
//���ݴ�ѧ��˼����Ƕ��ʽ���ģ����ݻ�����Ϣ�Ƽ����޹�˾��
//������ѯ��0512-65214835  http://sumcu.suda.edu.cn
//ҵ����ѯ��0512-87661670,18915522016  http://www.hxtek.com.cn
