//============================================================================
//�ļ����ƣ�flash.c
//���ܸ�Ҫ��K64 Flash����/д��ײ���������Դ�ļ�
//============================================================================

//����ͷ�ļ�
#include "flash.h"


//=================�ڲ����ú�������==========================================
//==========================================================================
//�������ƣ�flash_sign_off
//�������أ���
//����˵������
//���ܸ�Ҫ������Flash�洢�����������FlashԤ��ȡ����
//==========================================================================
static void flash_sign_off(void);

//==========================================================================
//�������ƣ�flash_cmd_launch
//�������أ�0-�ɹ� 1-ʧ��
//����˵������
//���ܸ�Ҫ������Flash����
//==========================================================================
static uint_32 flash_cmd_launch(void);

//===========================================================================

//=================�ⲿ�ӿں���==============================================
//==========================================================================
//�������ƣ�flash_init
//�������أ���
//����˵������
//���ܸ�Ҫ����ʼ��flashģ��
//==========================================================================
void flash_init(void)
{
	//���FMC������
    flash_sign_off();
    
    // ��ֹ���Ź�
    WDOG_UNLOCK = 0xC520;
    WDOG_UNLOCK = 0xD928;
    WDOG_STCTRLH = 0;    // ��ֹ���Ź�
    
    // �ȴ��������
    while(!(FTFE_FSTAT & CCIF));
    
    // ������ʳ�����־λ
    FTFE_FSTAT = ACCERR | FPVIOL;
}

//==========================================================================
//�������ƣ�flash_erase_sector
//�������أ�����ִ��ִ��״̬��0=��������0=�쳣��
//����˵����sectorNo�������ţ�MK64FX512ʵ��ʹ��0~127��
//���ܸ�Ҫ������ָ��flash����
//==========================================================================
uint_8 flash_erase_sector(uint_16 sectorNo)
{
    union
    {
        uint_32  word;
        uint_8   byte[4];
    } dest;
    
    dest.word    = (uint_32)(sectorNo*(1<<12));

    // ���ò�������
    FTFE_FCCOB0 = ERSSCR; // ������������
    
    // ����Ŀ���ַ
    FTFE_FCCOB1 = dest.byte[2];
    FTFE_FCCOB2 = dest.byte[1];
    FTFE_FCCOB3 = dest.byte[0];
    
    // ִ����������
    if(1 == flash_cmd_launch())    //��ִ��������ִ���
        return 1;     //�����������
   
    // ������sector0ʱ��������豸
    if(dest.word <= 0x800)
    {
        // д��8�ֽ�
        FTFE_FCCOB0 = PGM8;
        // ����Ŀ���ַ
        FTFE_FCCOB1 = 0x00;
        FTFE_FCCOB2 = 0x04;
        FTFE_FCCOB3 = 0x0C;
        // ����
        FTFE_FCCOB4 = 0xFF;
        FTFE_FCCOB5 = 0xFF;
        FTFE_FCCOB6 = 0xFF;
        FTFE_FCCOB7 = 0xFF;

        // ����
        FTFE_FCCOB8 = 0xFF;
        FTFE_FCCOB9 = 0xFF;
        FTFE_FCCOBA = 0xFF;
        FTFE_FCCOBB = 0xFE;
        // ִ����������
        if(1 == flash_cmd_launch())  //��ִ��������ִ���
            return 2;   //�����������
    }  
    
    return 0;  //�ɹ�����
}

//==========================================================================
//�������ƣ�flash_write
//�������أ�����ִ��״̬��0=��������0=�쳣��
//����˵����sectNo��Ŀ�������� ��MK64FX512ʵ��ʹ��0~127��
//         offset:д�������ڲ�ƫ�Ƶ�ַ��0~4087��
//         cnt��д���ֽ���Ŀ��0~4087��
//         buf��Դ���ݻ������׵�ַ
//���ܸ�Ҫ��flashд����� 
//==========================================================================
uint_8 flash_write(uint_16 sectNo,uint_16 offset,uint_16 cnt,uint_8 buf[])
{
    uint_32 size;
    uint_32 destaddr;
    
    union
    {
        uint_32   word;
        uint_8  byte[4];
    } dest;
    
    if(offset%8 != 0)
        return 1;   //�����趨����ƫ����δ���루8�ֽڶ��룩
    
    // ����д������
    FTFE_FCCOB0 = PGM8;
    destaddr = (uint_32)(sectNo*(1<<12) + offset);//�����ַ
    dest.word = destaddr;
    for(size=0; size<cnt; size+=8, dest.word+=8, buf+=8)
    {
        // ����Ŀ���ַ
        FTFE_FCCOB1 = dest.byte[2];
        FTFE_FCCOB2 = dest.byte[1];
        FTFE_FCCOB3 = dest.byte[0];
 
        // ��������
        FTFE_FCCOB4 = buf[3];
        FTFE_FCCOB5 = buf[2];
        FTFE_FCCOB6 = buf[1];
        FTFE_FCCOB7 = buf[0];

        FTFE_FCCOB8 = buf[7];
        FTFE_FCCOB9 = buf[6];
        FTFE_FCCOBA = buf[5];
        FTFE_FCCOBB = buf[4];
        
        if(1 == flash_cmd_launch())
            return 2;  //д���������
    }
    
    return 0;  //�ɹ�ִ��
}

//==========================================================================
//�������ƣ�flash_protect
//�������أ���
//����˵����sectNo�����������ţ���0x0000��ַ��ʼ��
//���ܸ�Ҫ��flash��������
//˵        ����������������ʱ�����������ķ�Χֻ�����ӣ����ܼ��٣�
//          128��������0~127��ƽ���ֳ�32�����򣬱�������ţ�0~31����
//          ÿ���������4�����������磬�����0��Ӧ0~3�����������1��Ӧ4~7������
//          �ĸ���λ�Ĵ�����32λ��32������һһ��Ӧ��ĳλΪ1����������Ϊ0������
//==========================================================================
void flash_protect(uint_8 sectNo)
{
	uint_8 regionNO;
	regionNO=sectNo/4;
	uint_8 offset;
	offset=regionNO%8;//���ƫ�ƣ�������λ��
	uint_8 regionCounter;
	regionCounter=3-regionNO/8;//���Ӧ��λ�ļĴ�����
	switch(regionCounter)
	{
	case 3:     //��������[7��0]��ĳ���򣨶�Ӧ[28:0]��ʼ��4��������4�������룩
		FTFE_FPROT3 &= ~(1<<offset);
		break;
	case 2:     //��������[15:8]��ĳ���򣨶�Ӧ[60:32]��ʼ��4��������4�������룩
		FTFE_FPROT2 &= ~(1<<offset);
		break;
	case 1:     //��������[23:16]��ĳ���򣨶�Ӧ[92:64]��ʼ��4��������4�������룩
		FTFE_FPROT1 &= ~(1<<offset);
		break;
	case 0:     //��������[31:24]��ĳ���򣨶�Ӧ[124:96]��ʼ��4��������4�������룩
		FTFE_FPROT0 &= ~(1<<offset);
		break;
	default:
		break;
	}
}


//=================�ڲ�����ʵ��=============================================
//==========================================================================
//�������ƣ�flash_sign_off
//�������أ���
//����˵������
//���ܸ�Ҫ������Flash�洢�����������FlashԤ��ȡ����
//==========================================================================
void flash_sign_off(void)
{  
    // �������
    FMC_PFB0CR |= FMC_PFB0CR_S_B_INV_MASK;
    FMC_PFB1CR |= FMC_PFB0CR_S_B_INV_MASK;
}

//==========================================================================
//�������ƣ�flash_cmd_launch
//�������أ�0-�ɹ� 1-ʧ��
//����˵������
//���ܸ�Ҫ������Flash����
//==========================================================================
static uint_32 flash_cmd_launch(void)
{
    // ������ʴ����־λ�ͷǷ����ʱ�־λ
    FTFE_FSTAT = ACCERR | FPVIOL;
    // ��������
    FTFE_FSTAT = CCIF;
    // �ȴ��������
    while(!(FTFE_FSTAT & CCIF));
//    for(;;)
//    {
//    	if((FTFE_FSTAT & CCIF))
//    		break;
//    	delay_us(5000);
//    }

    // �������־
    if(FTFE_FSTAT & (ACCERR | FPVIOL | MGSTAT0))
        return 1 ; //ִ���������
  
    return 0; //ִ������ɹ�
}
//==========================================================================