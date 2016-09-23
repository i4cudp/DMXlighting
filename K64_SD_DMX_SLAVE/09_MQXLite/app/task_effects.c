#include "01_app_include.h"    //Ӧ�����񹫹�ͷ�ļ�

extern uint_16 lightnum;
extern uint16_t effects;
extern uint8_t effectflag;
//uint_8 colorBuf[4096];
uint_8 __attribute__((section (".m_data_2"))) colorBuf[2048];
//===========================================================================
//�������ƣ�task_effects
//���ܸ�Ҫ����ʾ��Ч
//����˵����
//===========================================================================
void task_effects(uint32_t initial_data)
{
	uint_16 i = 0;
	while(1)
	{
		_lwevent_wait_for(&lwevent_group, EVENT_EFFECTS, FALSE, NULL);

		switch(effects)
		{
			case 1:   //ȫ�죺���Կ��Ե�������յ�ƣ�682յ��42������
				_lwevent_clear(&lwevent_group, EVENT_WATERLAMP);
				_lwevent_clear(&lwevent_group, EVENT_HORSERACE);
				lightnum = 682;
				for(i=0;i<lightnum;i++)
				{
					colorBuf[i*3]=0xFF;
					colorBuf[i*3+1]=0x00;
					colorBuf[i*3+2]=0x00;
				}
				effectflag = 1;
				break;
			case 2:       //�����
				_lwevent_clear(&lwevent_group, EVENT_WATERLAMP);
				_lwevent_set(&lwevent_group,EVENT_HORSERACE);

//				for(i=0;i<lightnum;i++)
//				{
//					colorBuf[i*3]=0x00;
//					colorBuf[i*3+1]=0xFF;
//					colorBuf[i*3+2]=0x00;
//				}
				effectflag = 1;
				break;
			case 3:   //��ˮ��
				_lwevent_clear(&lwevent_group, EVENT_HORSERACE);
				_lwevent_set(&lwevent_group,EVENT_WATERLAMP);

//				for(i=0;i<lightnum;i++)
//				{
//					colorBuf[i*3]=0x00;
//					colorBuf[i*3+1]=0x00;
//					colorBuf[i*3+2]=0xFF;
//				}
				effectflag = 1;

				break;
			case 4:
				_lwevent_clear(&lwevent_group, EVENT_WATERLAMP);
				_lwevent_clear(&lwevent_group, EVENT_HORSERACE);
				effectflag = 0;
				break;
		}
		_lwevent_clear(&lwevent_group, EVENT_EFFECTS);
	}
}


