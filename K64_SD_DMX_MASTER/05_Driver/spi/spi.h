//============================================================================
//�ļ����ƣ�SPI.h
//���ܸ�Ҫ��SPIͷ�ļ�
//��Ȩ���У����ݴ�ѧ��˼����Ƕ��ʽ����(sumcu.suda.edu.cn)
//���¼�¼��2015-12-27      V1.0
//============================================================================

#ifndef _SPI_H_
#define _SPI_H_


//1 ͷ�ļ�
#include "common.h"
#include "printf.h"
//#include <MK64F12.h>
//2 �궨��
//����SPI�ں�
#define SPI_0 0
#define SPI_1 1
#define SPI_2 2
#define FSL_SIM_SCGC_BIT(SCGCx, n) (((SCGCx-1U)<<5U) + n)
//�������ӻ�ģʽ
enum spimode
{
	MASTER,    //����ģʽ
	SLAVE     //����ģʽ
};

#define TRANSFER_BAUDRATE (500000U) /*! Transfer baudrate - 500k */
//SPI���ڵ�IRQ�жϺ�
#define SPI0IRQ 0
#define SPI1IRQ 1
#define SPI2IRQ 2


#define CPOL_0 0
#define CPOL_1 1
#define CPHA_0 0
#define CPHA_1 1
//===========================================================================
//�������ƣ�SPI_init��
//����˵����SPI��ʼ��
//����������spiNo��ģ��ţ�ȡֵΪ0��1
//       spiMode spiMode���ӻ�ģʽ����
//�������أ���
//===========================================================================
void spi_init(uint8_t spiNo,uint8_t spiMode,uint8_t CPOL,uint8_t CPHA);

//=========================================================================
//�������ƣ�spi_send1
//����������spiNo��SPIͨ���š�
//       data[]����Ҫ���͵����ݡ�
//�������أ���
//���ܸ�Ҫ��SPI�������ݡ�
//=========================================================================
void SPI_send1(uint_8 spiNo,uint_16 data);

//==============================================================================
//�������ƣ�SPI_send1_slave
//����˵����SPI����һ�ֽ����ݡ�
//����������SPI_No��ģ��š� ��ȡֵΪ0��1
//       data��     ��Ҫ���͵�һ�ֽ����ݡ�
//�������أ�0������ʧ�ܣ�1�����ͳɹ���
//==============================================================================
void SPI_send1_slave(uint8_t spiNo,uint16_t data);
//=========================================================================
//�������ƣ�spi_re
//����������spiNo��SPIͨ���š�
//       *data���յ����ݴ�Ŵ�
//�������أ�0���ɹ� 1��ʧ��
//���ܸ�Ҫ��SPI�������ݡ�
//=========================================================================
uint8_t spi_recieve1(uint8_t spiNo,uint16_t* data);

//=========================================================================
//�������ƣ�hw_spi_enable_re_int
//����˵����spiNo: SPI��
//�������أ� ��
//���ܸ�Ҫ����SPI�����ж�
//=========================================================================
void spi_enable_re_int(uint8_t spiNo);

//=========================================================================
//�������ƣ�spi_disable_re_int
//����˵����spiNo: SPI��
//�������أ� ��
//���ܸ�Ҫ����SPI�����ж�
//=========================================================================
void spi_disable_re_int(uint8_t spiNo);


//=========================================================================
//�������ƣ�spi_get_base_address
//����������spiNo��SPIͨ����
//�������أ���
//���ܸ�Ҫ����SPIͨ����ת���ɶ�Ӧ��ָ�롣
//=========================================================================
SPI_MemMapPtr spi_get_base_address(uint8_t spiNo);

#endif /* 05_DRIVER_KEL_SPI_SPI_H_ */