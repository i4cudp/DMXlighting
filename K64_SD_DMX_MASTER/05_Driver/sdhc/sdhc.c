/*
 * sdhc.c
 *
 *  Created on: 2015��12��10��
 *      Author: Jane
 */

#include "sdhc.h"
SDCARD_STRUCT 		SDHC_Card;

//SDHC�������ÿ������ִ��ʱ��XFERTYP�Ĵ��������������
static const unsigned long ESDHC_COMMAND_XFERTYP[] =
{
	// CMD0
    SDHC_XFERTYP_CMDINX(ESDHC_CMD0) | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_NO),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD1) | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_NO),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD2) | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_136),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD3) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD4) | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_NO),
    // CMD5
    SDHC_XFERTYP_CMDINX(ESDHC_CMD5) | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD6) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD7) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD8) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD9) | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_136),
    // CMD10
    SDHC_XFERTYP_CMDINX(ESDHC_CMD10) | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_136),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD11) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD12) | SDHC_XFERTYP_CMDTYP(ESDHC_XFERTYP_CMDTYP_ABORT) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD13) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    0,
    // CMD15
    SDHC_XFERTYP_CMDINX(ESDHC_CMD15) | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_NO),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD16) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD17) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD18) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    0,
    // CMD20
    SDHC_XFERTYP_CMDINX(ESDHC_CMD20) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    0,
    SDHC_XFERTYP_CMDINX(ESDHC_ACMD22) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_ACMD23) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD24) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    // CMD25
    SDHC_XFERTYP_CMDINX(ESDHC_CMD25) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD26) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD27) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD28) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD29) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY),
    // CMD30
    SDHC_XFERTYP_CMDINX(ESDHC_CMD30) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    0,
    SDHC_XFERTYP_CMDINX(ESDHC_CMD32) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD33) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD34) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    // CMD35
    SDHC_XFERTYP_CMDINX(ESDHC_CMD35) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD36) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD37) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD38) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD39) | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    // CMD40
    SDHC_XFERTYP_CMDINX(ESDHC_CMD40) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_ACMD41) | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD42) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY),
    0,
    0,
    // CMD45
    0,
    0,
    0,
    0,
    0,
    // CMD50
    0,
    SDHC_XFERTYP_CMDINX(ESDHC_ACMD51) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD52) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD53) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    0,
    // CMD55
    SDHC_XFERTYP_CMDINX(ESDHC_CMD55) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD56) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY),
    0,
    0,
    0,
    // CMD60
    SDHC_XFERTYP_CMDINX(ESDHC_CMD60) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY),
    SDHC_XFERTYP_CMDINX(ESDHC_CMD61) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY),
    0,
    0
};
static void SDHC_set_baudrate(uint32_t clock, uint32_t baudrate);
static uint8_t SDHC_is_running(void);
static uint32_t SDHC_status_wait(uint32_t mask);
static uint32_t SDHC_send_command (ESDHC_COMMAND_STRUCT_PTR command);

//=========================================================================
//�������ƣ�hw_sdhc_init
//���ܸ�Ҫ����ʼ��SDHCģ�顣
//����˵����coreClk���ں�ʱ��
//         baud��SDHCͨ��Ƶ��
//�������أ��ɹ�ʱ���أ�ESDHC_OK;��������ֵΪ����
//=========================================================================
uint32_t sdhc_init(uint32_t coreClk, uint32_t baud)
{
	SDHC_Card.CARD_TYPE = ESDHC_CARD_NONE;

    //ʹ��SDHCģ���ʱ����
    SIM_SCGC3 |= SIM_SCGC3_SDHC_MASK;

	//��λSDHC
    SDHC_SYSCTL = SDHC_SYSCTL_RSTA_MASK | SDHC_SYSCTL_SDCLKFS(0x80);//��λ���У�ѡ��ʱ�ӷ�Ƶ
    while (SDHC_SYSCTL & SDHC_SYSCTL_RSTA_MASK){};//�ȴ���λ���

    //��ʼ��SDHC��ؼĴ���
    SDHC_VENDOR = 0;//������ؼĴ���
    SDHC_BLKATTR = SDHC_BLKATTR_BLKCNT(1) | SDHC_BLKATTR_BLKSIZE(512);//��������512b��С
    SDHC_PROCTL = SDHC_PROCTL_EMODE(ESDHC_PROCTL_EMODE_INVARIANT)//С��ģʽ
                | SDHC_PROCTL_D3CD_MASK;//DAT3Line��Ϊ�������������ߡ�
    SDHC_WML = SDHC_WML_RDWML(1) | SDHC_WML_WRWML(1); //���ö���дˮλ��Ϊ17��

    //����SDHCģ���ͨ������
    SDHC_set_baudrate (coreClk,baud);//96MHz�ں�ʱ�ӣ�24MHz������

    //���CMDͨ����DATͨ���Ƿ�׼������
    while (SDHC_PRSSTAT & (  SDHC_PRSSTAT_CIHB_MASK
                           | SDHC_PRSSTAT_CDIHB_MASK)){};

    //���ø������Ź���ΪSDHC
    PORTE_PCR(0) = 0xFFFF & (   PORT_PCR_MUX(4)
                              | PORT_PCR_PS_MASK
                              | PORT_PCR_PE_MASK
                              | PORT_PCR_DSE_MASK); // ESDHC.D1
    PORTE_PCR(1) = 0xFFFF & (   PORT_PCR_MUX(4)
                              | PORT_PCR_PS_MASK
                              | PORT_PCR_PE_MASK
                              | PORT_PCR_DSE_MASK); // ESDHC.D0
    PORTE_PCR(2) = 0xFFFF & (   PORT_PCR_MUX(4)
                              | PORT_PCR_DSE_MASK); // ESDHC.CLK
    PORTE_PCR(3) = 0xFFFF & (   PORT_PCR_MUX(4)
                              | PORT_PCR_PS_MASK
                              | PORT_PCR_PE_MASK
                              | PORT_PCR_DSE_MASK); // ESDHC.CMD
    PORTE_PCR(4) = 0xFFFF & (   PORT_PCR_MUX(4)
                              | PORT_PCR_PS_MASK
                              | PORT_PCR_PE_MASK
                              | PORT_PCR_DSE_MASK); // ESDHC.D3
    PORTE_PCR(5) = 0xFFFF & (   PORT_PCR_MUX(4)
                              | PORT_PCR_PS_MASK
                              | PORT_PCR_PE_MASK
                              | PORT_PCR_DSE_MASK);  // ESDHC.D2

    //���SDHCģ����жϱ�־
    SDHC_IRQSTAT = 0xFFFF;
    //ʹ���ж�λ
    SDHC_IRQSTATEN = SDHC_IRQSTATEN_DEBESEN_MASK//���ݽ���λ����
                   | SDHC_IRQSTATEN_DCESEN_MASK//����ѭ������У�������
                   | SDHC_IRQSTATEN_DTOESEN_MASK//���ݳ�ʱ����
                   | SDHC_IRQSTATEN_CIESEN_MASK//������������
                   | SDHC_IRQSTATEN_CEBESEN_MASK//�������λ����
                   | SDHC_IRQSTATEN_CCESEN_MASK//����ѭ������У�������
                   | SDHC_IRQSTATEN_CTOESEN_MASK//���ʱ����
                   | SDHC_IRQSTATEN_BRRSEN_MASK//������������
                   | SDHC_IRQSTATEN_BWRSEN_MASK//д����������
                   | SDHC_IRQSTATEN_CRMSEN_MASK//SD�Ƴ�
                   | SDHC_IRQSTATEN_TCSEN_MASK//ת�����
                   | SDHC_IRQSTATEN_CCSEN_MASK;//ָ�����

    //80��ʱ�����ڵĳ�ʼ��
    SDHC_SYSCTL |= SDHC_SYSCTL_INITA_MASK;
    while (SDHC_SYSCTL & SDHC_SYSCTL_INITA_MASK){};//�ȴ�80��ʱ�����ڷ������


    SDHC_IRQSTAT |= SDHC_IRQSTAT_CRM_MASK;//��ʼ״̬����Ϊ��SD��״̬

    return ESDHC_OK;
}

//=========================================================================
//�������ƣ�hw_sdhc_receive_block
//���ܸ�Ҫ������n���ֽ�
//����˵����buff: ���ջ�����
//		   btr:���ճ���
//�������أ� 1:�ɹ�;0:ʧ��
//=========================================================================
uint32_t sdhc_receive_block (uint8_t *buff,uint32_t btr)
{
    uint32_t	bytes, i, j;
    uint32_t	*ptr = (uint32_t*)buff;

    //����ʶ��Ŀ�����󣬽�������ǰ�����DATͨ���Ƿ�æ������ʹ���У�
    //ʶ���������͵�SD��
    if (    (SDHC_Card.CARD_TYPE == ESDHC_CARD_SD)
         || (SDHC_Card.CARD_TYPE == ESDHC_CARD_SDHC)
         || (SDHC_Card.CARD_TYPE == ESDHC_CARD_MMC)
         || (SDHC_Card.CARD_TYPE == ESDHC_CARD_CEATA))
    {
        while (SDHC_PRSSTAT & SDHC_PRSSTAT_DLA_MASK){};//�ȴ������߿���
    }

    //��ȡ����ʱ��ÿ�ζ�ȡ4���ֽ�
    bytes = btr;
    while (bytes)
    {
        i = bytes > 512 ? 512 : bytes;
        for (j = (i + 3) >> 2; j != 0; j--)
        {
        	//����ǰ�жϼĴ����д��ڴ���������Ĵ���������0
            if (SDHC_IRQSTAT & (    SDHC_IRQSTAT_DEBE_MASK
                                  | SDHC_IRQSTAT_DCE_MASK
                                  | SDHC_IRQSTAT_DTOE_MASK))
            {
                SDHC_IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK
                              | SDHC_IRQSTAT_DCE_MASK
                              | SDHC_IRQSTAT_DTOE_MASK
                              | SDHC_IRQSTAT_BRR_MASK;
                return 0;
            }

            while (0 == (SDHC_PRSSTAT & SDHC_PRSSTAT_BREN_MASK)){};

            *ptr++ = SDHC_DATPORT;//��������
        }
        bytes -= i;
    }

	return 1;						// Return with success
}

//=========================================================================
//�������ƣ�hw_sdhc_send_block
//���ܸ�Ҫ������n���ֽ�
//����˵����buff: ���ͻ�����
//		   btr:���ͳ���
//�������أ� 1:�ɹ�;0:ʧ��
//=========================================================================
uint32_t sdhc_send_block (const uint8_t *buff,uint32_t btr)
{
    uint32_t  bytes, i;
    uint32_t	*ptr = (uint32_t*)buff;

    //��ȡ����ʱ��ÿ�ζ�ȡ4���ֽ�
    bytes = btr;
    while (bytes)
    {
        i = bytes > 512 ? 512 : bytes;
        bytes -= i;
        for (i = (i + 3) >> 2; i != 0; i--)
        {
            if (SDHC_IRQSTAT & (    SDHC_IRQSTAT_DEBE_MASK
                                  | SDHC_IRQSTAT_DCE_MASK
                                  | SDHC_IRQSTAT_DTOE_MASK))
            {
            	SDHC_IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK
            	              | SDHC_IRQSTAT_DCE_MASK
            	              | SDHC_IRQSTAT_DTOE_MASK
            	              | SDHC_IRQSTAT_BWR_MASK;
                return 0;
            }
            while (0 == (SDHC_PRSSTAT & SDHC_PRSSTAT_BWEN_MASK)){};

            SDHC_DATPORT = *ptr++;//д������

        }
    }
	return 1;
}

//=========================================================================
//�������ƣ�hw_sdhc_ioctl
//���ܸ�Ҫ������SDHCģ��
//����˵����cmd: ��������
//		   param_ptr:����ָ��
//�������أ� ��ʱ���أ�ESDHC_OK;��������ֵΪ����
//=========================================================================
uint32_t sdhc_ioctl(uint32_t cmd,void *param_ptr)
{
    ESDHC_COMMAND_STRUCT    command;
    uint8_t   mem, io, mmc, ceata, mp, hc; //��־��ǰ������
    uint32_t  i, val;
    uint32_t  result = ESDHC_OK;
    uint32_t  *param32_ptr = param_ptr;

    switch (cmd)
    {
        //��ʼ��SD����дϵͳ
        case IO_IOCTL_ESDHC_INIT:
            result = sdhc_init (CORE_CLOCK_HZ, BAUD_RATE_HZ);
            if (ESDHC_OK != result)
            {
                break;
            }

            mem = FALSE;
            io = FALSE;
            mmc = FALSE;
            ceata = FALSE;
            hc = FALSE;
            mp = FALSE;

            //CMD0,ʹ��SD���������ģʽ����λSD��
            command.COMMAND = ESDHC_CMD0;
            command.TYPE = ESDHC_TYPE_NORMAL;
            command.ARGUMENT = 0;
            command.READ = FALSE;
            command.BLOCKS = 0;
            if (SDHC_send_command (&command))
            {
                result = ESDHC_ERROR_INIT_FAILED;
                break;
            }

            for(i = 0;i < 2000000;i++)
            {
            }

            //CMD8
            command.COMMAND = ESDHC_CMD8;
            command.TYPE = ESDHC_TYPE_NORMAL;
            command.ARGUMENT = 0x000001AA;
            command.READ = FALSE;
            command.BLOCKS = 0;
            val = SDHC_send_command (&command);

            if (val == 0)
            {
                // SDHC��
            	if (command.RESPONSE[0] != command.ARGUMENT)
                {
                    result = ESDHC_ERROR_INIT_FAILED;
                    break;
                }
                hc = TRUE;
            }

            mp = TRUE;

            if (mp)
            {
                //CMD55������Ƿ�ΪMMC��
                command.COMMAND = ESDHC_CMD55;
                command.TYPE = ESDHC_TYPE_NORMAL;
                command.ARGUMENT = 0;
                command.READ = FALSE;
                command.BLOCKS = 0;
                val = SDHC_send_command (&command);
                if (val > 0)
                {
                    result = ESDHC_ERROR_INIT_FAILED;
                    break;
                }
                if (val < 0)
                {
                    // MMC �� CE-ATA
                    io = FALSE;
                    mem = FALSE;
                    hc = FALSE;

                    //CMD1
                    command.COMMAND = ESDHC_CMD1;
                    command.TYPE = ESDHC_TYPE_NORMAL;
                    command.ARGUMENT = 0x40300000;
                    command.READ = FALSE;
                    command.BLOCKS = 0;
                    if (SDHC_send_command (&command))
                    {
                        result = ESDHC_ERROR_INIT_FAILED;
                        break;
                    }
                    if (0x20000000 == (command.RESPONSE[0] & 0x60000000))
                    {
                        hc = TRUE;
                    }
                    mmc = TRUE;

                    //CMD39
                    command.COMMAND = ESDHC_CMD39;
                    command.TYPE = ESDHC_TYPE_NORMAL;
                    command.ARGUMENT = 0x0C00;
                    command.READ = FALSE;
                    command.BLOCKS = 0;
                    if (SDHC_send_command (&command))
                    {
                        result = ESDHC_ERROR_INIT_FAILED;
                        break;
                    }
                    if (0xCE == (command.RESPONSE[0] >> 8) & 0xFF)
                    {
                        //CMD39
                        command.COMMAND = ESDHC_CMD39;
                        command.TYPE = ESDHC_TYPE_NORMAL;
                        command.ARGUMENT = 0x0D00;
                        command.READ = FALSE;
                        command.BLOCKS = 0;
                        if (SDHC_send_command (&command))
                        {
                            result = ESDHC_ERROR_INIT_FAILED;
                            break;
                        }
                        if (0xAA == (command.RESPONSE[0] >> 8) & 0xFF)
                        {
                            mmc = FALSE;
                            ceata = TRUE;
                        }
                    }
                }
                else
                {
                    //��ΪSD��ʱ
                    // ACMD41
                    command.COMMAND = ESDHC_ACMD41;
                    command.TYPE = ESDHC_TYPE_NORMAL;
                    command.ARGUMENT = 0;
                    command.READ = FALSE;
                    command.BLOCKS = 0;
                    if (SDHC_send_command (&command))
                    {
                        result = ESDHC_ERROR_INIT_FAILED;
                        break;
                    }
                    if (command.RESPONSE[0] & 0x300000)
                    {
                        val = 0;
                        do
                        {
                            for(i = 0;i < 500000;i++)
                            {
                            }
                            val++;

                            // CMD55 + ACMD41 - Send OCR
                            command.COMMAND = ESDHC_CMD55;
                            command.TYPE = ESDHC_TYPE_NORMAL;
                            command.ARGUMENT = 0;
                            command.READ = FALSE;
                            command.BLOCKS = 0;
                            if (SDHC_send_command (&command))
                            {
                                result = ESDHC_ERROR_INIT_FAILED;
                                break;
                            }

                            command.COMMAND = ESDHC_ACMD41;
                            command.TYPE = ESDHC_TYPE_NORMAL;
                            if (hc)
                            {
                                command.ARGUMENT = 0x40300000;
                            }
                            else
                            {
                                command.ARGUMENT = 0x00300000;
                            }
                            command.READ = FALSE;
                            command.BLOCKS = 0;
                            if (SDHC_send_command (&command))
                            {
                                result = ESDHC_ERROR_INIT_FAILED;
                                break;
                            }
                        } while ((0 == (command.RESPONSE[0] & 0x80000000)) && (val < 10));
                        if (ESDHC_OK != result)
                        {
                            break;
                        }
                        if (val >= 10)
                        {
                            hc = FALSE;
                        }
                        else
                        {
                            mem = TRUE;
                            if (hc)
                            {
                                hc = FALSE;
                                if (command.RESPONSE[0] & 0x40000000)
                                {
                                    hc = TRUE;
                                }
                            }
                        }
                    }
                }
            }


            if (mmc)
            {
            	SDHC_Card.CARD_TYPE = ESDHC_CARD_MMC;
            }
            if (ceata)
            {
            	SDHC_Card.CARD_TYPE = ESDHC_CARD_CEATA;
            }
            if (io)
            {
            	SDHC_Card.CARD_TYPE = ESDHC_CARD_SDIO;
            }
            if (mem)
            {
            	SDHC_Card.CARD_TYPE = ESDHC_CARD_SD;
                if (hc)
                {
                	SDHC_Card.CARD_TYPE = ESDHC_CARD_SDHC;
                }
            }
            if (io && mem)
            {
            	SDHC_Card.CARD_TYPE = ESDHC_CARD_SDCOMBO;
                if (hc)
                {
                	SDHC_Card.CARD_TYPE = ESDHC_CARD_SDHCCOMBO;
                }
            }

            /*
            //������Ÿ��üĴ���
            PORTE_PCR(0) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D1
            PORTE_PCR(1) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D0
            PORTE_PCR(2) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK);                                          // ESDHC.CLK
            PORTE_PCR(3) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.CMD
            PORTE_PCR(4) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D3
            PORTE_PCR(5) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D2

            //����SDHCģ��Ĳ�����
            SDHC_set_baudrate (CORE_CLOCK_HZ, BAUD_RATE_HZ);

            //���ø������Ź���ΪSDHC
            PORTE_PCR(0) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D1
            PORTE_PCR(1) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D0
            PORTE_PCR(2) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK);                                          // ESDHC.CLK
            PORTE_PCR(3) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.CMD
            PORTE_PCR(4) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D3
            PORTE_PCR(5) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D2

            //ʹ��SDHCģ���ʱ��
            SIM_SCGC3 |= SIM_SCGC3_SDHC_MASK;
            */
            break;
        //�򿨷�������
        case IO_IOCTL_ESDHC_SEND_COMMAND:
            val = SDHC_send_command ((ESDHC_COMMAND_STRUCT_PTR)param32_ptr);
            if (val > 0)
            {
                result = ESDHC_ERROR_COMMAND_FAILED;
            }
            if (val < 0)
            {
                result = ESDHC_ERROR_COMMAND_TIMEOUT;
            }
            break;
        //��ȡ��ǰͨ�Ų�����״̬
        case IO_IOCTL_ESDHC_GET_BAUDRATE:
            if (NULL == param32_ptr)
            {
                result = IO_ERROR_INVALID_PARAMETER;
            }
            else
            {
                //��ȡ��ǰ���õĲ�����
                val = ((SDHC_SYSCTL & SDHC_SYSCTL_SDCLKFS_MASK) >> SDHC_SYSCTL_SDCLKFS_SHIFT) << 1;
                val *= ((SDHC_SYSCTL & SDHC_SYSCTL_DVS_MASK) >> SDHC_SYSCTL_DVS_SHIFT) + 1;
                *param32_ptr = (uint32_t)(CORE_CLOCK_HZ / val);
            }
            break;
        //�趨��ǰͨ�Ų�����״̬
        case IO_IOCTL_ESDHC_SET_BAUDRATE:
            if (NULL == param32_ptr)
            {
                result = IO_ERROR_INVALID_PARAMETER;
            }
            else if (0 == (*param32_ptr))
            {
                result = IO_ERROR_INVALID_PARAMETER;
            }
            else
            {
                if (! SDHC_is_running())
                {
                	//������Ÿ��üĴ���
                    PORTE_PCR(0) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D1
                    PORTE_PCR(1) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D0
                    PORTE_PCR(2) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK);                                          // ESDHC.CLK
                    PORTE_PCR(3) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.CMD
                    PORTE_PCR(4) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D3
                    PORTE_PCR(5) = 0 & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D2

                    //����SDHCģ��Ĳ�����
                    SDHC_set_baudrate (CORE_CLOCK_HZ, *param32_ptr);

                    //���ø������Ź���ΪSDHC
                    PORTE_PCR(0) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D1
                    PORTE_PCR(1) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D0
                    PORTE_PCR(2) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK);                                          // ESDHC.CLK
                    PORTE_PCR(3) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.CMD
                    PORTE_PCR(4) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D3
                    PORTE_PCR(5) = 0xFFFF & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    // ESDHC.D2

                    //ʹ��SDHCģ���ʱ��
                    SIM_SCGC3 |= SIM_SCGC3_SDHC_MASK;
                }
                else
                {
                    result = IO_ERROR_DEVICE_BUSY;
                }
            }
            break;
        //��ȡ�鳤��
        case IO_IOCTL_ESDHC_GET_BLOCK_SIZE:
            if (NULL == param32_ptr)
            {
                result = IO_ERROR_INVALID_PARAMETER;
            }
            else
            {
                //��ȡSDHCģ�����õĿ�Ĵ�С
                *param32_ptr = (SDHC_BLKATTR & SDHC_BLKATTR_BLKSIZE_MASK) >> SDHC_BLKATTR_BLKSIZE_SHIFT;
            }
            break;
        //�趨�鳤��
        case IO_IOCTL_ESDHC_SET_BLOCK_SIZE:
            if (NULL == param32_ptr)
            {
                result = IO_ERROR_INVALID_PARAMETER;
            }
            else
            {
                //����SDHCģ�鴦��Ŀ�Ĵ�С
                if (! SDHC_is_running())
                {
                    if (*param32_ptr > 0x0FFF)
                    {
                        result = IO_ERROR_INVALID_PARAMETER;
                    }
                    else
                    {
                        SDHC_BLKATTR &= (~ SDHC_BLKATTR_BLKSIZE_MASK);
                        SDHC_BLKATTR |= SDHC_BLKATTR_BLKSIZE(*param32_ptr);
                    }
                }
                else
                {
                    result = IO_ERROR_DEVICE_BUSY;
                }
            }
            break;
        //��ȡ��ͨ������λ��
        case IO_IOCTL_ESDHC_GET_BUS_WIDTH:
            if (NULL == param32_ptr)
            {
                result = IO_ERROR_INVALID_PARAMETER;
            }
            else
            {
                //��ȡ��ǰ���õ�SDHCģ������߿��
                val = (SDHC_PROCTL & SDHC_PROCTL_DTW_MASK) >> SDHC_PROCTL_DTW_SHIFT;
                if (ESDHC_PROCTL_DTW_1BIT == val)
                {
                    *param32_ptr = ESDHC_BUS_WIDTH_1BIT;
                }
                else if (ESDHC_PROCTL_DTW_4BIT == val)
                {
                    *param32_ptr = ESDHC_BUS_WIDTH_4BIT;
                }
                else if (ESDHC_PROCTL_DTW_8BIT == val)
                {
                    *param32_ptr = ESDHC_BUS_WIDTH_8BIT;
                }
                else
                {
                    result = ESDHC_ERROR_INVALID_BUS_WIDTH;
                }
            }
            break;
        //�趨��ͨ������λ��
        case IO_IOCTL_ESDHC_SET_BUS_WIDTH:
            if (NULL == param32_ptr)
            {
                result = IO_ERROR_INVALID_PARAMETER;
            }
            else
            {
            	//����SDHCģ������߿��
                if (! SDHC_is_running())
                {
                    if (ESDHC_BUS_WIDTH_1BIT == *param32_ptr)
                    {
                        SDHC_PROCTL &= (~ SDHC_PROCTL_DTW_MASK);
                        SDHC_PROCTL |= SDHC_PROCTL_DTW(ESDHC_PROCTL_DTW_1BIT);
                    }
                    else if (ESDHC_BUS_WIDTH_4BIT == *param32_ptr)
                    {
                        SDHC_PROCTL &= (~ SDHC_PROCTL_DTW_MASK);
                        SDHC_PROCTL |= SDHC_PROCTL_DTW(ESDHC_PROCTL_DTW_4BIT);
                    }
                    else if (ESDHC_BUS_WIDTH_8BIT == *param32_ptr)
                    {
                        SDHC_PROCTL &= (~ SDHC_PROCTL_DTW_MASK);
                        SDHC_PROCTL |= SDHC_PROCTL_DTW(ESDHC_PROCTL_DTW_8BIT);
                    }
                    else
                    {
                        result = ESDHC_ERROR_INVALID_BUS_WIDTH;
                    }
                }
                else
                {
                    result = IO_ERROR_DEVICE_BUSY;
                }
            }
            break;
        //��ȡ����ǰ״̬
        case IO_IOCTL_ESDHC_GET_CARD:
            if (NULL == param32_ptr)
            {
                result = IO_ERROR_INVALID_PARAMETER;
            }
            else
            {
                //�ȴ�80��ʱ��
                SDHC_SYSCTL |= SDHC_SYSCTL_INITA_MASK;
                while (SDHC_SYSCTL & SDHC_SYSCTL_INITA_MASK){};

                //��ȡSD�����ص�״̬
                if (SDHC_IRQSTAT & SDHC_IRQSTAT_CRM_MASK)
                {
                    SDHC_IRQSTAT |= SDHC_IRQSTAT_CRM_MASK;
                    SDHC_Card.CARD_TYPE = ESDHC_CARD_NONE;
                }
                if (SDHC_PRSSTAT & SDHC_PRSSTAT_CINS_MASK)
                {
                    if (ESDHC_CARD_NONE == SDHC_Card.CARD_TYPE)
                    {
                    	SDHC_Card.CARD_TYPE = ESDHC_CARD_UNKNOWN;
                    }
                }
                else
                {
                	SDHC_Card.CARD_TYPE = ESDHC_CARD_NONE;
                }
                *param32_ptr = SDHC_Card.CARD_TYPE;
            }
            break;
        default:
            result = IO_ERROR_INVALID_IOCTL_CMD;
            break;
    }
    return result;
}


//=========================================================================
//�������ƣ�SDHC_set_baudrate
//���ܸ�Ҫ������SDHCģ���ʱ�ӡ�
//����˵����clock:ϵͳʱ��
//         baudrate��������
//�������أ���
//=========================================================================
static void SDHC_set_baudrate(uint32_t clock,uint32_t baudrate)
{
	uint32_t i, pres, div, min, minpres = 0x80, mindiv = 0x0F;
	int32_t  val;

    //�ҵ�����ķ�Ƶ����
    min = (uint32_t)-1;
    for (pres = 2; pres <= 256; pres <<= 1)
    {
        for (div = 1; div <= 16; div++)
        {
            val = pres * div * baudrate - clock;
            if (val >= 0)
            {
                if (min > val)
                {
                    min = val;
                    minpres = pres;
                    mindiv = div;
                }
            }
        }
    }

    //��ֹSDHCģ��ʱ��
    SDHC_SYSCTL &= (~ SDHC_SYSCTL_SDCLKEN_MASK);

    //�޸ķ�Ƶ����
    div = SDHC_SYSCTL & (~ (SDHC_SYSCTL_DTOCV_MASK | SDHC_SYSCTL_SDCLKFS_MASK | SDHC_SYSCTL_DVS_MASK));
    SDHC_SYSCTL = div | (SDHC_SYSCTL_DTOCV(0x0E) | SDHC_SYSCTL_SDCLKFS(minpres >> 1) | SDHC_SYSCTL_DVS(mindiv - 1));

    //����ʱ���ȶ�
    while (0 == (SDHC_PRSSTAT & SDHC_PRSSTAT_SDSTB_MASK))
    {
		for(i = 0;i < 200000;i++)
		{
		}
    };

    //ʹ��SDHCģ��ʱ��
    SDHC_SYSCTL |= SDHC_SYSCTL_SDCLKEN_MASK;
    SDHC_IRQSTAT |= SDHC_IRQSTAT_DTOE_MASK;
}

//=========================================================================
//�������ƣ�SDHC_is_running
//���ܸ�Ҫ�����SDHCģ���Ƿ�æ
//����˵������
//�������أ�1����æ;0��������
//=========================================================================
static uint8_t SDHC_is_running(void)
{
    return (0 != (SDHC_PRSSTAT & (SDHC_PRSSTAT_RTA_MASK | SDHC_PRSSTAT_WTA_MASK | SDHC_PRSSTAT_DLA_MASK | SDHC_PRSSTAT_CDIHB_MASK | SDHC_PRSSTAT_CIHB_MASK)));
}

//=========================================================================
//�������ƣ�SDHC_status_wait
//���ܸ�Ҫ���ȴ��жϱ�־λ��λ
//����˵����mask�������ϵı�־λ����
//�������أ����ش������
//=========================================================================
static uint32_t SDHC_status_wait(uint32_t mask)
{
    uint32_t	result;
    do
    {
        result = SDHC_IRQSTAT & mask;
    }
    while (0 == result);
    return result;
}

//=========================================================================
//�������ƣ�SDHC_send_command
//���ܸ�Ҫ����������
//����˵����command������ṹ��ָ��
//�������أ�0���ɹ���1������-1����ʱ��
//=========================================================================
static uint32_t SDHC_send_command (ESDHC_COMMAND_STRUCT_PTR command)
{
    uint32_t	xfertyp;//Transfer Type Register (SDHC_XFERTYP)

    //������������
    xfertyp = ESDHC_COMMAND_XFERTYP[command->COMMAND & 0x3F];
    if ((0 == xfertyp) && (0 != command->COMMAND))
    {
        return 1;
    }

    //���Ƴ����
    SDHC_IRQSTAT |= SDHC_IRQSTAT_CRM_MASK;

    //�ȴ�CMDͨ������
    while (SDHC_PRSSTAT & SDHC_PRSSTAT_CIHB_MASK){};

    //���ô������ͼĴ�������
    //д��������Ĵ���(Command Argument Register��SDHC_CMDARG)
    SDHC_CMDARG = command->ARGUMENT;
    //���XFERTYP�Ĵ���������������
    xfertyp &= (~ SDHC_XFERTYP_CMDTYP_MASK);
    //����XFERTYP�Ĵ���������������
    xfertyp |= SDHC_XFERTYP_CMDTYP(command->TYPE);
    //���Ϊ�ָ�����
    if (ESDHC_TYPE_RESUME == command->TYPE)
    {
    	//�����������Ϊ�ָ�CMD52д����ѡ���������ݴ���ѡ��λ��
        xfertyp |= SDHC_XFERTYP_DPSEL_MASK;   //data present
    }
    //������л�æ����
    if (ESDHC_TYPE_SWITCH_BUSY == command->TYPE)
    {

        if ((xfertyp & SDHC_XFERTYP_RSPTYP_MASK) == SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48))
        {
            xfertyp &= (~ SDHC_XFERTYP_RSPTYP_MASK);
            xfertyp |= SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY);
        }
        else
        {
            xfertyp &= (~ SDHC_XFERTYP_RSPTYP_MASK);
            xfertyp |= SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48);
        }
    }
    //�������
    SDHC_BLKATTR &= (~ SDHC_BLKATTR_BLKCNT_MASK);
    //�����ж�
    if (0 != command->BLOCKS)
    {
    	//�鲻Ϊ0
        if ((xfertyp & SDHC_XFERTYP_RSPTYP_MASK) != SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_48BUSY))
        {
            xfertyp |= SDHC_XFERTYP_DPSEL_MASK;
        }
        if (command->READ)
        {
            xfertyp |= SDHC_XFERTYP_DTDSEL_MASK;
        }
        if (command->BLOCKS > 1)
        {
            xfertyp |= SDHC_XFERTYP_MSBSEL_MASK;
        }
        if ((uint32_t)-1 != command->BLOCKS)
        {
        	SDHC_BLKATTR |= SDHC_BLKATTR_BLKCNT(command->BLOCKS);
            xfertyp |= SDHC_XFERTYP_BCEN_MASK;
        }
    }

    //ִ������
    SDHC_DSADDR = 0;
    SDHC_XFERTYP = xfertyp;

    //�ȴ�״̬�Ĵ�����λ
    if (SDHC_status_wait (SDHC_IRQSTAT_CIE_MASK | SDHC_IRQSTAT_CEBE_MASK | SDHC_IRQSTAT_CCE_MASK |
    		SDHC_IRQSTAT_TC_MASK |SDHC_IRQSTAT_CC_MASK) != SDHC_IRQSTAT_CC_MASK)
    {
    	SDHC_IRQSTAT |= SDHC_IRQSTAT_CTOE_MASK | SDHC_IRQSTAT_CIE_MASK | SDHC_IRQSTAT_CEBE_MASK | SDHC_IRQSTAT_CCE_MASK | SDHC_IRQSTAT_CC_MASK;
        return 1;
    }

    //��⿨�Ƿ��Ƴ�
    if (SDHC_IRQSTAT & SDHC_IRQSTAT_CRM_MASK)
    {
    	SDHC_IRQSTAT |= SDHC_IRQSTAT_CTOE_MASK | SDHC_IRQSTAT_CC_MASK;
        return 1;
    }

    //��������Ƿ�ʱ
    if (SDHC_IRQSTAT & SDHC_IRQSTAT_CTOE_MASK)
    {
    	SDHC_IRQSTAT |= SDHC_IRQSTAT_CTOE_MASK | SDHC_IRQSTAT_CC_MASK;
        return -1;
    }
    if ((xfertyp & SDHC_XFERTYP_RSPTYP_MASK) != SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_NO))
    {
        command->RESPONSE[0] = SDHC_CMDRSP(0);
        if ((xfertyp & SDHC_XFERTYP_RSPTYP_MASK) == SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_136))
        {
            command->RESPONSE[1] = SDHC_CMDRSP(1);
            command->RESPONSE[2] = SDHC_CMDRSP(2);
            command->RESPONSE[3] = SDHC_CMDRSP(3);
        }
    }

    SDHC_IRQSTAT |= SDHC_IRQSTAT_CC_MASK;

    return 0;
}



