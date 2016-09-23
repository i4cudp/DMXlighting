//============================================================================
//�ļ����ƣ�SPI.c
//���ܸ�Ҫ��SPI�ײ���������Դ�ļ�
//��Ȩ���У����ݴ�ѧ��˼����Ƕ��ʽ����(sumcu.suda.edu.cn)
//���¼�¼��2015-12-29
//============================================================================

#include "spi.h"
static const uint32_t s_baudratePrescaler[] = {2, 3, 5, 7};
static const uint32_t s_baudrateScaler[] = {2,   4,   6,    8,    16,   32,   64,    128,
                                            256, 512, 1024, 2048, 4096, 8192, 16384, 32768};

SPI_MemMapPtr spi_get_base_address(uint8_t spiNo);


IRQn_Type table_irq_spi[3] ={ SPI0_IRQn, SPI1_IRQn, SPI2_IRQn };


void spi_init(uint8_t spiNo,uint8_t spiMode,uint8_t CPOL,uint8_t CPHA)
{
    SPI_MemMapPtr BaseAdd = spi_get_base_address(spiNo);
    if(0==spiNo)
    {
        SIM_SCGC6 |= SIM_SCGC6_SPI0_MASK;//ʹ��ģ��ʱ��
        PORTD_PCR0 = 0 | PORT_PCR_MUX(0x2) | PORT_PCR_DSE_MASK;//PCS0
        PORTD_PCR1 = 0 | PORT_PCR_MUX(0x2) | PORT_PCR_DSE_MASK;//SCK
        PORTA_PCR16 = 0 | PORT_PCR_MUX(0x2) | PORT_PCR_DSE_MASK;//SOUT
        PORTA_PCR17 = 0 | PORT_PCR_MUX(0x2) | PORT_PCR_DSE_MASK;//SIN
    }
    else if(1==spiNo)
    {
        SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK;//ʹ��ģ��ʱ��
        PORTB_PCR10 = 0 | PORT_PCR_MUX(0x2) | PORT_PCR_DSE_MASK;//PCS0
        PORTB_PCR11 = 0 | PORT_PCR_MUX(0x2) | PORT_PCR_DSE_MASK;//SCK
        PORTB_PCR16 = 0 | PORT_PCR_MUX(0x2) | PORT_PCR_DSE_MASK;//SOUT
        PORTB_PCR17 = 0 | PORT_PCR_MUX(0x2) | PORT_PCR_DSE_MASK;//SIN
    }
    else
    {
        SIM_SCGC3 |= SIM_SCGC3_SPI2_MASK;//ʹ��ģ��ʱ��
//        PORTB_PCR20 = 0 | PORT_PCR_MUX(0x2) ;//PCS0
        PORTB_PCR21 = 0 | PORT_PCR_MUX(0x2) ;//SCK
        PORTB_PCR22 = 0 | PORT_PCR_MUX(0x2) ;//SOUT
        PORTB_PCR23 = 0 | PORT_PCR_MUX(0x2) ;//SIN
    }
    SPI_MCR_REG(BaseAdd) &= ~SPI_MCR_MDIS_MASK;
    SPI_MCR_REG(BaseAdd) = 0
                 | SPI_MCR_CLR_TXF_MASK     //Clear the Tx FIFO counter.
                 | SPI_MCR_CLR_RXF_MASK     //Clear the Rx FIFO counter.
                 | SPI_MCR_PCSIS_MASK
                 | SPI_MCR_HALT_MASK;
    //�������ӻ�ģʽ���ù���ģʽ
    if(MASTER==spiMode)
    {
        SPI_MCR_REG(BaseAdd) |= SPI_MCR_MSTR_MASK;//����Ϊ����ģʽ
    }
    else
    {
    	SPI_MCR_REG(BaseAdd) &= ~SPI_MCR_MSTR_MASK;//����Ϊ�ӻ�ģʽ
    	SPI_CTAR_SLAVE_REG(BaseAdd, 0) = SPI_CTAR_SLAVE_FMSZ(3);

    }
    SPI_CTAR_REG(BaseAdd, 0) = (SPI_CTAR_BR(0)       //������Scaler��BR=6��SCALER��ֵΪ64
			 | SPI_CTAR_PBR(0)     // PBR=2,�����ʷ�Ƶ����ֵΪ5�����Բ�����
			 | SPI_CTAR_FMSZ(0x07)   //�������ݴ���λ��Ϊ7+1= 8λ
			 | SPI_CTAR_PCSSCK(1)    // PCS to SCK Prescaler value is 5,t(csc)=
			 | SPI_CTAR_CSSCK(1)
			 | SPI_CTAR_PASC(1)          //t(asc)=(1/50M)*5*2=0.2ms
			 | SPI_CTAR_ASC(1)
			 | SPI_CTAR_PDT(1)             //t(dt)=(1/50M)*5*2=0.2ms
			 | SPI_CTAR_DT(1)
			 | SPI_CTAR_DBR_MASK
			 | SPI_CTAR_CPOL(CPOL)     //SCK����Ծ״̬Ϊ�ߵ�ƽ
			 | SPI_CTAR_CPHA(CPHA));   //������SCKʱ�ӵ�ǰ���ط�������,��SCKʱ��
    SPI_CTAR_REG(BaseAdd, 0) &= ~SPI_CTAR_LSBFE_MASK; //���������Ǹ�λ��ǰ��MSB first
    SPI_SR_REG(BaseAdd) |= ( SPI_SR_EOQF_MASK      //SPI����֡������־λ��0
				 | SPI_SR_TFUF_MASK        //TX FIFO�����־��0
				 | SPI_SR_TFFF_MASK        //TX FIFO����־��0
				 | SPI_SR_TCF_MASK
				 | SPI_SR_RFOF_MASK       //RX FIFO�����־��0
				 | SPI_SR_RFDF_MASK);     //RX FIFO���ձ�־��0����ʾRX FIFOΪ��
    SPI_MCR_REG(BaseAdd) &= ~SPI_MCR_HALT_MASK;//��ʼ����
}

//==============================================================================
//�������ƣ�SPI_send1.
//����˵����SPI����һ�ֽ����ݡ�
//����������SPI_No��ģ��š� ��ȡֵΪ0��1
//       data��     ��Ҫ���͵�һ�ֽ����ݡ�
//�������أ�0������ʧ�ܣ�1�����ͳɹ���
//==============================================================================
void SPI_send1(uint8_t spiNo,uint16_t data)
{
    uint32_t i;
    SPI_MemMapPtr BaseAdd = spi_get_base_address(spiNo);
    SPI_SR_REG(BaseAdd) = (SPI_SR_EOQF_MASK
				   | SPI_SR_TFUF_MASK
				   | SPI_SR_TFFF_MASK
				   | SPI_SR_RFOF_MASK
				   | SPI_SR_RFDF_MASK
				   | SPI_SR_TCF_MASK); //�ڷ�������ǰ�����״̬�Ĵ����еı�־λ

    //�巢��FIFO
    SPI_MCR_REG(BaseAdd) |= (SPI_MCR_CLR_TXF_MASK
				   | SPI_MCR_CLR_RXF_MASK); //�ڷ�������֮ǰ�����TX FIFO��RX FIFO

    SPI_PUSHR_REG(BaseAdd) |= (SPI_PUSHR_CTAS(0) //ѡ��CTAR0�Ĵ������崫������
				   | SPI_PUSHR_EOQ_MASK //��֡SPI���������һ֡����
				   | SPI_PUSHR_PCS(1) //Ƭѡ�ź�PCS0
				   | SPI_PUSHR_TXDATA(data)); //д������
//    SPI_PUSHR_SLAVE_REG(BaseAdd) = data;

	while(!(SPI_SR_REG(BaseAdd)&SPI_SR_TCF_MASK));
	SPI_SR_REG(BaseAdd) |= SPI_SR_TCF_MASK;
}

//==============================================================================
//�������ƣ�SPI_send1_slave
//����˵����SPI����һ�ֽ����ݡ�
//����������SPI_No��ģ��š� ��ȡֵΪ0��1
//       data��     ��Ҫ���͵�һ�ֽ����ݡ�
//�������أ�0������ʧ�ܣ�1�����ͳɹ���
//==============================================================================
void SPI_send1_slave(uint8_t spiNo,uint16_t data)
{
    uint32_t i;
    SPI_MemMapPtr BaseAdd = spi_get_base_address(spiNo);
    SPI_SR_REG(BaseAdd) = (SPI_SR_EOQF_MASK
				   | SPI_SR_TFUF_MASK
				   | SPI_SR_TFFF_MASK
				   | SPI_SR_RFOF_MASK
				   | SPI_SR_RFDF_MASK
				   | SPI_SR_TCF_MASK); //�ڷ�������ǰ�����״̬�Ĵ����еı�־λ

    //�巢��FIFO
    SPI_MCR_REG(BaseAdd) |= (SPI_MCR_CLR_TXF_MASK
				   | SPI_MCR_CLR_RXF_MASK); //�ڷ�������֮ǰ�����TX FIFO��RX FIFO

//    SPI_PUSHR_REG(BaseAdd) |= (SPI_PUSHR_CTAS(0) //ѡ��CTAR0�Ĵ������崫������
//				   | SPI_PUSHR_EOQ_MASK //��֡SPI���������һ֡����
//				   | SPI_PUSHR_PCS(1) //Ƭѡ�ź�PCS0
//				   | SPI_PUSHR_TXDATA(data)); //д������
    SPI_PUSHR_SLAVE_REG(BaseAdd) = data;

//	while(!(SPI_SR_REG(BaseAdd)&SPI_SR_TCF_MASK));
//	SPI_SR_REG(BaseAdd) |= SPI_SR_TCF_MASK;
}


//=========================================================================
//�������ƣ�spi_re
//����������spiNo��SPIͨ���š�
//          data[]����Ҫ���յ����ݡ�
//�������أ�0��ʧ�� 1���ɹ�
//���ܸ�Ҫ��SPI�������ݡ�
//=========================================================================
uint8_t spi_recieve1(uint8_t spiNo,uint16_t* data)
{
      SPI_MemMapPtr BaseAdd = spi_get_base_address(spiNo);
      uint32_t temp;
      if((SPI_SR_REG(BaseAdd) & SPI_SR_RFDF_MASK))
      {
    	  temp = SPI_POPR_REG(BaseAdd);
    	  *data = (uint16_t)temp;
		  SPI_SR_REG(BaseAdd) |= (SPI_SR_EOQF_MASK| SPI_SR_TFUF_MASK| SPI_SR_TFFF_MASK| SPI_SR_RFOF_MASK| SPI_SR_RFDF_MASK);
		  SPI_MCR_REG(BaseAdd) |= SPI_MCR_CLR_TXF_MASK     //Clear the Tx FIFO counter.
					| SPI_MCR_CLR_RXF_MASK;     //Clear the Rx FIFO counter.
		  return 1;
      }
      else
      {
		  SPI_SR_REG(BaseAdd) |= (SPI_SR_EOQF_MASK| SPI_SR_TFUF_MASK| SPI_SR_TFFF_MASK| SPI_SR_RFOF_MASK| SPI_SR_RFDF_MASK);

		  SPI_MCR_REG(BaseAdd) |= SPI_MCR_CLR_TXF_MASK     //Clear the Tx FIFO counter.
					| SPI_MCR_CLR_RXF_MASK;     //Clear the Rx FIFO counter.
		  return 0;
      }

}

//=========================================================================
//�������ƣ�spi_enable_re_int
//����˵����spiNo: SPI��
//�������أ� ��
//���ܸ�Ҫ����SPI�����ж�
//=========================================================================
void spi_enable_re_int(uint8_t spiNo)
{
    SPI_MemMapPtr BaseAdd = spi_get_base_address(spiNo);
    if(SPI_RSER_REG(BaseAdd)&SPI_RSER_RFDF_RE_MASK)
    	SPI_RSER_REG(BaseAdd)&=~SPI_RSER_RFDF_RE_MASK;
    if(SPI_RSER_REG(BaseAdd)&SPI_RSER_TFFF_RE_MASK)
    	SPI_RSER_REG(BaseAdd)&=~SPI_RSER_TFFF_RE_MASK;
    if(SPI_RSER_REG(BaseAdd)&SPI_RSER_EOQF_RE_MASK)
    	SPI_RSER_REG(BaseAdd)&=~SPI_RSER_EOQF_RE_MASK;
    if(SPI_RSER_REG(BaseAdd)&SPI_RSER_TCF_RE_MASK)
    	SPI_RSER_REG(BaseAdd)&=~SPI_RSER_TCF_RE_MASK;
    SPI_RSER_REG(BaseAdd)|=SPI_RSER_RFDF_RE_MASK;
//    		           |SPI_RSER_TFFF_RE_MASK
//					   |SPI_RSER_EOQF_RE_MASK
//					   |SPI_RSER_TCF_RE_MASK;
    switch(spiNo)
    {
    case 0:
        NVIC_EnableIRQ(SPI0_IRQn);
    	break;
    case 1:
        NVIC_EnableIRQ(SPI1_IRQn);
    	break;
    case 2:
        NVIC_EnableIRQ(SPI2_IRQn);
    	break;

    }

}

//=================�ڲ�����ʵ��=============================================
//=========================================================================
//�������ƣ�spi_get_base_address
//����������spiNo��SPIͨ����
//�������أ���
//���ܸ�Ҫ����SPIͨ����ת���ɶ�Ӧ��ָ�롣
//=========================================================================
SPI_MemMapPtr spi_get_base_address(uint8_t spiNo)
{
    switch(spiNo)
    {
    case 0:
      return SPI0_BASE_PTR;
    case 1:
      return SPI1_BASE_PTR;
    case 2:
      return SPI2_BASE_PTR;
    default:
      return 0;
    }
}
uint32_t spi_MasterSetBaudRate(SPI_MemMapPtr base,
                                uint8_t whichCtar,
                                uint32_t baudRate_Bps,
                                uint32_t srcClock_Hz)
{
    /* for master mode configuration, if slave mode detected, return 0*/
//    if (!DSPI_IsMaster(base))
//    {
//        return 0;
//    }
    uint32_t temp;
    uint32_t prescaler, bestPrescaler;
    uint32_t scaler, bestScaler;
    uint32_t dbr, bestDbr;
    uint32_t realBaudrate, bestBaudrate;
    uint32_t diff, min_diff;
    uint32_t baudrate = baudRate_Bps;

    /* find combination of prescaler and scaler resulting in baudrate closest to the requested value */
    min_diff = 0xFFFFFFFFU;
    bestPrescaler = 0;
    bestScaler = 0;
    bestDbr = 1;
    bestBaudrate = 0; /* required to avoid compilation warning */

    /* In all for loops, if min_diff = 0, the exit for loop*/
    for (prescaler = 0; (prescaler < 4) && min_diff; prescaler++)
    {
        for (scaler = 0; (scaler < 16) && min_diff; scaler++)
        {
            for (dbr = 1; (dbr < 3) && min_diff; dbr++)
            {
                realBaudrate = ((srcClock_Hz * dbr) / (s_baudratePrescaler[prescaler] * (s_baudrateScaler[scaler])));

                /* calculate the baud rate difference based on the conditional statement that states that the calculated
                * baud rate must not exceed the desired baud rate.
                */
                if (baudrate >= realBaudrate)
                {
                    diff = baudrate - realBaudrate;
                    if (min_diff > diff)
                    {
                        /* a better match found */
                        min_diff = diff;
                        bestPrescaler = prescaler;
                        bestScaler = scaler;
                        bestBaudrate = realBaudrate;
                        bestDbr = dbr;
                    }
                }
            }
        }
    }

    /* write the best dbr, prescalar, and baud rate scalar to the CTAR */
     SPI_CTAR_REG(base,whichCtar)=0 & ~(SPI_CTAR_DBR_MASK | SPI_CTAR_PBR_MASK | SPI_CTAR_BR_MASK);

//    temp = base->CTAR[whichCtar] & ~(SPI_CTAR_DBR_MASK | SPI_CTAR_PBR_MASK | SPI_CTAR_BR_MASK);
//
//    base->CTAR[whichCtar] = temp | ((bestDbr - 1) << SPI_CTAR_DBR_SHIFT) | (bestPrescaler << SPI_CTAR_PBR_SHIFT) |
//                            (bestScaler << SPI_CTAR_BR_SHIFT);

    /* return the actual calculated baud rate */
    return bestBaudrate;
}


