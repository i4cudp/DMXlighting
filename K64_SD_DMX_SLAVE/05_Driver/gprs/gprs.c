/*====================================================================
ģ����  �� GPRS
�ļ���  �� gprs.c
����ļ��� gprs.h
�ļ�ʵ�ֹ��ܣ�ʵ��GPRSģ��ײ������ӿں���
����    ������
�汾    ��1.0.0.0.0
----------------------------------------------------------------------
�޸ļ�¼    ��
��  ��         �汾        �޸���        �޸�����
2013/08/02       1.0             ����                  ����
====================================================================*/

#include "gprs.h"


//GPRSģ���ڲ���ȫ�ֱ���
static TGPRSInfo s_tGPRSInfo;
static TBufNode* s_pNodePtr = NULL;
//static uint_32 s_GPRSIntTick = 0;
static uint_8 s_GSMCSCA[14];
static bool s_bIPTRANS = FALSE;
static bool s_bLinkBroken = FALSE;
static MQX_TICK_STRUCT mqx_tick_gprsint;



//-------------------------------------------------------------------------------------------
//������: gprs_send_cmd                                                        
//��  ��: ���������GPRS�����ػ�Ӧ�Ƿ�ʱ�ˣ������յ������ݽ���
//        �Ƿ��������Ļ�Ӧ����                                                     
//��  ��: uint_8* pcmd: �����͵������ַ��� 
//               uint_8 *responed1:�����Ļ�Ӧ�ַ���1   
//               uint_8 *responed2:�����Ļ�Ӧ�ַ���2   
//               uint_32 delayms:����͵ȴ���ʱ,ms
//               uint_8 sendtimes:�����ʧ�ܵ�����£�����ʹ���
//               uint_8* pret:���ؽ��յ���Ӧ������
//��  ��: 0--��ȷ���� 
//        	     1--���󷵻�     
//˵  ��: ��
//-------------------------------------------------------------------------------------------
#ifdef GPRS_NET
static uint_8 gprs_send_cmd(uint_8* pcmd, uint_8* presponed1, uint_8* presponed2,
                                 uint_32 delayms, uint_8 sendtimes, uint_8* pret)
{
	uint_8 ret = 1;
	uint_8 i;

	for(i = 0; i < sendtimes; i++)
	{
		//ͨ�����ڷ��ͺ�����GPRSģ��
		_GPRS_SEND_STR(pcmd); 
		
		//_GPRS_DEBUG_STR("[gprs_send_cmd]: send cmd: ");
	    //_GPRS_DEBUG_STR(pcmd);
		//_GPRS_DEBUG_STR("\r\n");
		
		//����GPRSģ�鷵������֡��Ҫһ��ʱ��������Ҫ��ʱ
		if(delayms > 0)
			delay_ms(delayms);
		
		//�ȴ������жϽ�����ʱ
		_time_get_ticks(&mqx_tick_gprsint);
		//s_GPRSIntTick = sys_tick;
		while(is_timeout(mqx_tick_gprsint.TICKS[0], 20) == FALSE);
		
		//��ʾ�Ѿ����յ����ݲ���������� ֡
		if(s_pNodePtr != NULL && s_pNodePtr->type == GPRSFRAME_CMD && s_pNodePtr->len > 1)
		{
			s_pNodePtr->pbuf[s_pNodePtr->len] = '\0';//�ڽڵ���ջ�������ĩβ����'\0'�Ľ�β��ʶ��

			//_GPRS_DEBUG_STR("[gprs_send_cmd]:s_pNodePtr->len=");
			//_GPRS_DEBUG_HEX(s_pNodePtr->len);
			//_GPRS_DEBUG_STR(",s_pNodePtr->pbuf: ");
			//_GPRS_DEBUG_STR(s_pNodePtr->pbuf);
			//_GPRS_DEBUG_STR("\r\n");
			if((presponed1 != NULL && strstr((const int_8*)s_pNodePtr->pbuf, (const int_8*)presponed1) != NULL) ||
				(presponed2 != NULL && strstr((const int_8*)s_pNodePtr->pbuf, (const int_8*)presponed2) != NULL) ||
				(presponed1 == NULL && presponed2 == NULL))
			{
				if(pret != NULL)
					memcpy((int_8*)pret, (int_8*)s_pNodePtr->pbuf, s_pNodePtr->len + 1);
				//_GPRS_DEBUG_STR("[gprs_send_cmd]: cmd success!\r\n");
				ret = 0;
			}
			
			//����ȡ�Ľڵ�Ž�����pgprsfreelist
			s_pNodePtr->len = 0;
			s_pNodePtr->type = GPRSFRAME_MAX;
			listnode_put(s_tGPRSInfo.pgprsfreelist, s_pNodePtr);
			s_pNodePtr = NULL;   //���㾲̬ȫ�ֱ���
		}

		if(0 == ret)
			break;
	}

	return ret;
}
#endif

//=========================================================================
//�������ƣ�gprs_set_iptrans
//����˵����bTrans��=0  ����͸��ģʽ       bTrans=0  �˳�͸��ģʽ  
//���ܸ�Ҫ��������˳�͸������ģʽ
//�������أ�0=���óɹ�          1=����ʧ��
//=========================================================================
uint_8 gprs_set_iptrans(uint_8 bTrans)
{
	uint_8 ret = 0;
	uint_8 atret[100];
	static uint_8 failtime = 0;   //����3�ν���͸��ģʽ������
	
	if(s_bIPTRANS != bTrans)
	{
#ifdef GPRS_NET
		//����͸������ģʽ
		if(bTrans)
		{
			ret = gprs_send_cmd((uint_8*)"AT^IPENTRANS=0\r",(uint_8*)"OK", NULL, 50, 1, atret);
			if(0 == ret && strstr((int_8*)atret, (int_8*)"ERROR") == NULL)
			{
				s_bIPTRANS = true;                    //����͸������ģʽ�ɹ�
				failtime = 0;
				s_bLinkBroken = FALSE;
			}
			else if(++failtime >= 3)
			{
				s_bLinkBroken = TRUE;
			}
		}
		else   //�˳�͸������ģʽ
		{
			delay_ms(1000);                     //�ȴ�1s����ֹ����ΪΪ����
			/*//�����ã�Ϊ�жϽ���ʹ��
			s_bIPTRANS = FALSE;
			ret = gprs_send_cmd((uint_8*)"+++",(uint_8*)"OK", NULL, 1000, 1, NULL);
			if(ret != 0)
			{
				s_bIPTRANS = TRUE;
			}*/
			//20140223���ж�
			s_bIPTRANS = FALSE;
			gprs_send_cmd((uint_8*)"+++",(uint_8*)"OK", NULL, 1000, 1, NULL);
			delay_ms(1000);                     //�ȴ�1s����ֹ����ΪΪ����
		}
#else
        s_bIPTRANS = bTrans;
#endif
	}
	
	return ret;
}

//-------------------------------------------------------------------------------------------
//������:ip_to_str 											
//��  ��: IPPortת����д										
//��  ��: ��
//�ڲ�����:										
//��������:���ӳ���
//-------------------------------------------------------------------------------------------	
#ifdef GPRS_NET
static uint_8 ip_to_str(const uint_8* ip, uint_8* pdst)
{
    uint_8 i, hundred, decade, entries, len, ret;

	//ipaddr
	for(i = 0, len = 0; i < 4; i++)
	{
		hundred = decade = entries = 0;
		ret = ip[i];
		
		//hundred
		if(ret >= 100)
		{
			hundred = ret / 100;
			ret %= 100;
		}

		//decade
		if(ret >= 10)
		{
			decade = ret / 10;
			ret %= 10;
		}

		//entries
		entries = ret;

		//add
		if(hundred > 0)
		{
			pdst[len++] = 0x30 + hundred;
			pdst[len++] = 0x30 + decade;
			pdst[len++] = 0x30 + entries;
		}
		else if(decade > 0)
		{
			pdst[len++] = 0x30 + decade;
			pdst[len++] = 0x30 + entries;
		}
		else
			pdst[len++] = 0x30 + entries;
		
		pdst[len++] = '.';
	}

	//ipaddr ѭ������һ��'.'
	return --len;
}
#endif

//-------------------------------------------------------------------------------------------
//������:port_to_str 											
//��  ��: uint32PortTostrPortת����д										
//��  ��: ��
//�ڲ�����:										
//��������:���ӳ���
//-------------------------------------------------------------------------------------------	
#ifdef GPRS_NET
static uint_8 port_to_str(uint_16 port, uint_8* pdst)
{
    uint_8 myria = 0;
	uint_8 thousand = 0;
	uint_8 hundred = 0;
	uint_8 decade = 0;
	uint_8 entries = 0;
	uint_8 len = 0;

	//myria
	if(port >= 10000)
	{
		myria = port / 10000;
		port %= 10000;
	}
	
	//thousand
	if(port >= 1000)
	{
		thousand = port / 1000;
		port %= 1000;
	}

	//hundred
	if(port >= 100)
	{
		hundred = port / 100;
		port %= 100;
	}

	//decade
	if(port >= 10)
	{
		decade = port / 10;
		port %= 10;
	}

	//entries
	entries = port;

	//add
	if(myria > 0)
	{
		pdst[len++] = 0x30 + myria;
		pdst[len++] = 0x30 + thousand;
		pdst[len++] = 0x30 + hundred;
		pdst[len++] = 0x30 + decade;
		pdst[len++] = 0x30 + entries;
	}
	else if(thousand > 0)
	{
		pdst[len++] = 0x30 + thousand;
		pdst[len++] = 0x30 + hundred;
		pdst[len++] = 0x30 + decade;
		pdst[len++] = 0x30 + entries;
	}
	else if(hundred > 0)
	{
		pdst[len++] = 0x30 + hundred;
		pdst[len++] = 0x30 + decade;
		pdst[len++] = 0x30 + entries;
	}
	else if(decade > 0)
	{
		pdst[len++] = 0x30 + decade;
		pdst[len++] = 0x30 + entries;
	}
	else
		pdst[len++] = 0x30 + entries;

	return len;
}
#endif

//=========================================================================
//�������ƣ�gprs_start
//����˵����pDataList��������������ͷָ���ַ��
//                           pFreeList:��������ͷָ���ַ��
//�������أ�0=��ʼ���ɹ�;
//                         ��0�����󷵻�
//���ܸ�Ҫ����ʼ���õ���GPRSģ�����ţ���������������GPRSģ�飬����Ŀ�������
//       GPRS����������������տ�������ָ�봫��
//˵����
//=========================================================================
uint_8 gprs_start(struct list_head* pDataList,struct list_head* pFreeList, GPRSRECVDATACallBack pdatacallback)
{
	uint_8 ret = eGPRSRET_NONE;
	uint_8 atcmd[50];
	uint_8 i;

	//�������
	if(NULL == pDataList || NULL == pFreeList || NULL == pdatacallback)
	{
		ret = eGPRSRET_PARAM;
		goto gprs_start_exit;
	}	

	//�����ʼ��
	s_tGPRSInfo.pgprsdatalist = pDataList;
	s_tGPRSInfo.pgprsfreelist = pFreeList;
	s_tGPRSInfo.pgprsdatacb = pdatacallback;
	s_bIPTRANS = FALSE;

	//Ӳ���ܽ�����
	_GPRS_DEBUG_STR("[gprs_start]:gprs power off\r\n");
	uart_init(GPRS_UART, GPRS_UART_BAUD);      // ��ʼ������ 
    uart_enable_re_int(GPRS_UART);                              //�����ڽ����ж�

#ifdef GPRS_NET
	gpio_init(GPRS_POWER,1,GPRS_POWER_OFF);
	gpio_init(GPRS_TERMON,1,GPRS_TTERMON_OFF);
	//gpio_init(GPRS_RESET_PORT,GPRS_RESET_PIN,1,GPRS_RESET_OFF);
	delay_ms(2000);    //�ϵ�2s�����ݷŵ�
	_GPRS_DEBUG_STR("[gprs_start]:gprs power on\r\n");
	gpio_set(GPRS_POWER,GPRS_POWER_ON);
	delay_ms(1000);
	gpio_set(GPRS_TERMON,GPRS_TTERMON_ON);
	delay_ms(3000);

	//�ȴ��������
	while(1)
	{
		if(s_pNodePtr != NULL && s_pNodePtr->type == GPRSFRAME_CMD && s_pNodePtr->len > 8)
		{
			s_pNodePtr->pbuf[s_pNodePtr->len] = '\0';//�ڽڵ���ջ�������ĩβ����'\0'�Ľ�β��ʶ��

			if(strstr((const int_8*)s_pNodePtr->pbuf, (const int_8*)"^SYSSTART") != NULL)
			{
				//����ȡ�Ľڵ�Ž�����pgprsfreelist
				s_pNodePtr->len = 0;
				s_pNodePtr->type = GPRSFRAME_MAX;
				listnode_put(s_tGPRSInfo.pgprsfreelist, s_pNodePtr);
				s_pNodePtr = NULL;   //���㾲̬ȫ�ֱ���
				break;
			}
		}
	}	
	
	_GPRS_DEBUG_STR("[gprs_start]:gprs start ok!\r\n");

	//����
	if(gprs_send_cmd((uint_8*)"AT\r", 
			         (uint_8*)"OK", NULL, 100, 50, NULL) != 0) 
	{
		ret = eGPRSRET_START;
		goto gprs_start_exit;
	}
	_GPRS_DEBUG_STR("[gprs_start]:gprs AT ok!\r\n");
	
	//�رջ���
	if(gprs_send_cmd((uint_8*)"ATE0\r", 
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)	
	{
		ret = eGPRSRET_CLOSEECHO;
		goto gprs_start_exit;
	}
	_GPRS_DEBUG_STR("[gprs_start]:gprs close echo ok!\r\n");
	
	//���SIM��
	if(gprs_send_cmd((uint_8*)"AT+CPIN?\r", 
			         (uint_8*)"OK", NULL, 500, 10, NULL) != 0)	
	{
		ret = eGPRSRET_DETSIM;
		_GPRS_DEBUG_STR("[gprs_start]:gprs detect sim failed!\r\n");
		goto gprs_start_exit;
	}
	_GPRS_DEBUG_STR("[gprs_start]:gprs detect sim ok!\r\n");
#endif
		
gprs_start_exit:
	return ret;

}

//=========================================================================
//������: gprs_ready                                                        
//��  ��: �õ���ǰgprs�Ƿ�����                                                    
//��  ��: ��
//˵  ��:��                                                       
//=========================================================================
#ifdef GPRS_NET
static bool gprs_ready()
{
	uint_8* pstart = NULL;
	uint_8 atret[100];
	bool bret = FALSE;
	uint_8 tmp8;
	
	//��ȡCSQ
	memset(atret, 0, 100);
	/*if(gprs_send_cmd((uint_8*)"AT+CSQ\r", 
			         (uint_8*)"OK", NULL, 100, 10, atret) != 0)	
	{
		_GPRS_DEBUG_STR("[gprs_ready]: gprs get csq failed!\r\n");
		goto gprs_ready_exit;
	}
	pstart = (uint_8*)strstr((int_8*)atret, (int_8*)"+CSQ: ");
	if(pstart != NULL)
	{
		pstart += strlen("+CSQ: ");
		//CSQ����Ϊʮ������
		sscanf(pstart, "%d", &tmp8);	// ��ȡCSQ
		if(0 == tmp8)
		{
			_GPRS_DEBUG_STR("[gprs_ready]: gprs csq failed:0\r\n");
			goto gprs_ready_exit;
		}
	}

	//��ѯ����ע��״̬
	if(gprs_send_cmd((uint_8*)"AT+CREG?\r",
			         (uint_8*)"OK", NULL, 100, 10, atret) != 0)
	{
		_GPRS_DEBUG_STR("[gprs_ready]: gprs get creg failed!\r\n");
		goto gprs_ready_exit;
	}
	pstart = (uint_8*)strstr((int_8*)atret, (int_8*)"+CREG: ");
	if(pstart != NULL)
	{
		pstart += strlen("+CREG: ");
		pstart += 2;
		sscanf(pstart, "%d", &tmp8);	
		if(tmp8 != 1)
		{
			_GPRS_DEBUG_STR("[gprs_ready]: gprs creg failed:");
			_GPRS_DEBUG_DEC(tmp8);
			_GPRS_DEBUG_STR("\r\n");
			goto gprs_ready_exit;
		}
	}
	*/
	//��ѯ����ע��״̬���صĲ���
	if(gprs_send_cmd((uint_8*)"AT+CGREG?\r",
			         (uint_8*)"OK", NULL, 100, 10, atret) != 0)
	{
		_GPRS_DEBUG_STR("[gprs_ready]: gprs get cgreg failed!\r\n");
		goto gprs_ready_exit;
	}
	pstart = (uint_8*)strstr((int_8*)atret, (int_8*)"+CGREG: ");
	if(pstart != NULL)
	{
		pstart += strlen("+CGREG: ");
		pstart += 2;
		sscanf(pstart, "%d", &tmp8);	
		if(tmp8 == 1 || tmp8 == 5)
		{
			bret = TRUE;
		}
	}
gprs_ready_exit:
	return bret;
}
#endif

//=========================================================================
//�������ƣ�gprs_init 
//����˵����linktype:�������ͣ��ƶ�����ͨ
//                           pIpPort��Ŀ���������IP��ַ�Ͷ˿ںţ�
//�������أ�0=��ʼ���ɹ�;
//                         ��0�����󷵻�
//���ܸ�Ҫ����ʼ���õ���GPRSģ�����ţ���������������GPRSģ�飬����Ŀ�������
//       GPRS����������������տ�������ָ�봫��
//˵��������IP_PORT(Ŀ���������IP��ַ�Ͷ˿ں�)��ʽ����202.195.128.106:8100��
//       SET_IP_PORTΪ�����չ����
//=========================================================================
uint_8 gprs_init(uint_8 linktype, const uint_8* ip, uint_16 port)
{
	uint_8 ret = eGPRSRET_NONE;
	
#ifdef GPRS_NET
	uint_8 atcmd[50];
	uint_8 i;
	uint_32 ticks;

	//�������
	if(linktype >= GPRSLINK_MAX || NULL == ip)
	{
		ret = eGPRSRET_PARAM;
		goto gprs_init_exit;
	}	

	//ע������
	//��IDΪ0��Profile�� ���ӷ�ʽ ����ΪGPRS0
	if(gprs_send_cmd((uint_8*)"AT^SICS=0,conType,GPRS0\r",
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{
		ret = eGPRSRET_GPRS0;
		_GPRS_DEBUG_STR("[gprs_init]:gprs set gprs0 failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs set gprs0 ok!\r\n");

	//��IDΪ0��Profile�� ��������� ����Ϊcmnet /uninet
	if(GPRSLINK_CMNET == linktype)
	{
		if(gprs_send_cmd((uint_8*)"AT^SICS=0,apn,cmnet\r", 
				         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
		{
			ret = eGPRSRET_APN;
			_GPRS_DEBUG_STR("[gprs_init]:gprs set cmnet failed!\r\n");
			goto gprs_init_exit;
		}
	}
	else if(gprs_send_cmd((uint_8*)"AT^SICS=0,apn,uninet\r", 
				         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{
		ret = eGPRSRET_APN;
		_GPRS_DEBUG_STR("[gprs_init]:gprs set uninet failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs set apn ok!\r\n");
		
	//��IDΪ0��Profile�������� ����ΪSocket
	if(gprs_send_cmd((uint_8*)"AT^SISS=0,srvType,Socket\r", 
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{
		ret = eGPRSRET_SOCKET;
		_GPRS_DEBUG_STR("[gprs_init]:gprs set socket failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs set socket ok!\r\n");
	
	//��IDΪ0��Profile��Internet���Ӻ� ����Ϊ0
	if(gprs_send_cmd((uint_8*)"AT^SISS=0,conId,0\r", 
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{
		ret = eGPRSRET_CONID;
		_GPRS_DEBUG_STR("[gprs_init]:gprs set conid failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs set conid ok!\r\n");
	
	//������
	//����IP��ַ�Ͷ˿ں�
	strcpy((int_8*)atcmd, "AT^SISS=0,address,\"socktcp://");
	i = (uint_8)strlen("AT^SISS=0,address,\"socktcp://");
    //��Ӵ����ӵķ�������IP��ַ
    i += ip_to_str(ip, &atcmd[i]);
	//��Ӵ����ӵķ������˿�
	atcmd[i++] = ':';
    i += port_to_str(port, &atcmd[i]);
	atcmd[i++] = '\"';
	atcmd[i++] = '\r';
	//ͨ�����ڷ���AT ����
	//"OK"��ʾ�򿪳ɹ���"ERROR: 2\r"��ʾ�����Ѿ�����
	if(gprs_send_cmd(atcmd,(uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{   
		ret = eGPRSRET_LINKIP;
		_GPRS_DEBUG_STR("[gprs_init]:gprs link ip port failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs link ip port ok!\r\n");

	//�ȴ�ϵͳ����
	while(1)
	{
		if(gprs_ready())
		{
			break;
		}
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs ready!\r\n");
	
	//��һ��Internet����,0Ϊ�����ʶ��ȡֵ��Χ0-9��
	if(gprs_send_cmd((uint_8*)"AT^SISO=0\r", 
			         (uint_8*)"OK", NULL, 4000, 10, NULL) != 0)
	{   
		ret = eGPRSRET_SISO;
		_GPRS_DEBUG_STR("[gprs_init]:gprs set siso failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs set siso ok!\r\n");

	//��ѯ�����IP��ַ
	if(gprs_send_cmd((uint_8*)"AT^SICI?\r", 
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{   
		ret = eGPRSRET_SICI;
		_GPRS_DEBUG_STR("[gprs_init]:gprs SICI failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs init ok!\r\n");

	s_bLinkBroken = FALSE;
#endif
	
gprs_init_exit:
	return ret;	
}


//=========================================================================
//������: gprs_get_csq                                                        
//��  ��: �õ�Ŀǰ���ź�����                                                    
//��  ��: pcsq:  ��ʾ�ź������Ľ��������ڲ����� 
//˵  ��:��                                                       
//=========================================================================
uint_8 gprs_get_csq(uint_8* pcsq)
{
#ifdef GPRS_NET
	uint_8* pstart = NULL;
	uint_8 ret = eGPRSRET_GETCSQ;
	uint_8 atret[100];
	
	//�˳�͸��ģʽ
	/*if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gprs_get_csq]: gprs quit trans failed!\r\n");
			goto gprs_get_csq_exit;
		}
	}*/

	//��ȡCSQ
	memset(atret, 0, 100);
	if(gprs_send_cmd((uint_8*)"AT+CSQ\r", 
			         (uint_8*)"OK", NULL, 100, 10, atret) != 0)	
	{
		_GPRS_DEBUG_STR("[gprs_get_csq]: gprs get csq failed!\r\n");
		goto gprs_get_csq_exit;
	}

	//ȡ������ֵ
	pstart = (uint_8*)strstr((int_8*)atret, (int_8*)"+CSQ: ");
	if(pstart != NULL)
	{
		pstart += strlen("+CSQ: ");
		//CSQ����Ϊʮ������
		sscanf(pstart, "%d", pcsq);	// ��ȡCSQ
		ret = eGPRSRET_NONE;
	}

gprs_get_csq_exit:
	//�ٴν���͸��ģʽ
	//gprs_set_iptrans(TRUE);

#else
	uint_8 ret = eGPRSRET_NONE;
	*pcsq = 31;
#endif

	return ret;
}

//=========================================================================
//�������ƣ�gprs_send
//����˵����pdata:���������׵�ַ  ��len:�������ݳ���
//�������أ���
//���ܸ�Ҫ���������ݸ�Ŀ���������������Ŀ������������ӳɹ�       
//=========================================================================
uint_8 gprs_send(uint_8* pdata,uint_16 len)
{
	uint_8 ret = eGPRSRET_NONE;
	
#ifdef GPRS_NET
	//�Ѷ���
	if(s_bLinkBroken)
	{
		ret = eGPRSRET_BROKEN;
	    goto gprs_snd_exit;
	}
	
	//͸��ģʽ
	if(!s_bIPTRANS && gprs_set_iptrans(true))
	{
		ret = eGPRSRET_TRANS;
		goto gprs_snd_exit;
	}
#endif

	//���ݷ���
	if(_GPRS_SEND_N(len, pdata))
	{
		ret = eGPRSRET_SNDDATA;
	}
	
gprs_snd_exit:
	return ret;
}

//=========================================================================
//�������ƣ�gprs_isbroken
//����˵�����ж�gprs�Ƿ����
//�������أ���
//���ܸ�Ҫ���������ݸ�Ŀ���������������Ŀ������������ӳɹ�       
//=========================================================================
bool gprs_isbroken()
{
	return s_bLinkBroken;
}


//=========================================================================
//�������ƣ�gprs_recv_intproc 
//����˵������
//�������أ���
//���ܸ�Ҫ���жϴ����������
//=========================================================================
void gprs_recv_intproc()	                    
{
	uint_8 flag;
	uint_8 ch;

	//��ȷ���յ�����
	ch = _GPRS_RECV_1(&flag);
	if(flag == 0)
	{
		//��֡δ���������ݶ���
		if(is_timeout(mqx_tick_gprsint.TICKS[0], 1000) && s_pNodePtr != NULL)
		{
			//�������
			s_pNodePtr->len = 0;
#ifdef GPRS_NET
			s_pNodePtr->type = s_bIPTRANS ? GPRSFRAME_DATA : GPRSFRAME_CMD;
#else
            s_pNodePtr->type = GPRSFRAME_DATA;
#endif
		}

		//��ȫ������"pgprsfreelist" ��ȡ���ڵ�
		if(NULL == s_pNodePtr)
		{	
			//��ȫ������"pgprsfreelist" ��ȡ���ڵ�
			listnode_get(s_tGPRSInfo.pgprsfreelist, &s_pNodePtr);
			if(s_pNodePtr != NULL)                  //ȡ�����нڵ�
			{
				s_pNodePtr->len = 0;                //���ȳ�ʼ��
#ifdef GPRS_NET
							s_pNodePtr->type = s_bIPTRANS ? GPRSFRAME_DATA : GPRSFRAME_CMD;
#else
							s_pNodePtr->type = GPRSFRAME_DATA;
#endif
			}
		}

		//��������
		if(s_pNodePtr != NULL)
		{
			//��������
			if(s_pNodePtr->len < s_pNodePtr->size)
				s_pNodePtr->pbuf[s_pNodePtr->len++] = ch;

			
			//����֡Ӧ�ò��ж��Ƿ����
			if(GPRSFRAME_DATA == s_pNodePtr->type)
			{
#ifdef GPRS_NET
				//����Ƿ����
				if(10 == s_pNodePtr->len)
				{
					ch = s_pNodePtr->pbuf[10];
					s_pNodePtr->pbuf[10] = '\0';
					if(strstr((int_8*)s_pNodePtr->pbuf,(int_8*)"ERROR") != NULL)
					{
						s_bLinkBroken = TRUE;
						s_pNodePtr->len = 0;                //���ȳ�ʼ��
						s_bIPTRANS = FALSE;                 //�Զ��˳�͸��ģʽ
						s_pNodePtr->type = GPRSFRAME_CMD;
					}
					else
						s_pNodePtr->pbuf[10] = ch;
				}
#endif
				
				//������ȷ����,���಻����
				if(GPRS_RECVDATA_OK == s_tGPRSInfo.pgprsdatacb(s_pNodePtr))
				{
					listnode_put(s_tGPRSInfo.pgprsdatalist, s_pNodePtr);
					s_pNodePtr = NULL;
				}

			}
		}

		//s_GPRSIntTick = sys_tick;
		_time_get_ticks(&mqx_tick_gprsint);
	}
}

//=========================================================================
//������: gsm_get_csca                                                        
//��     ��: �õ�Ŀǰ�����ĺ���                                                    
//��     ��: pcsq:  ��ʾ�ź������Ľ��������ڲ����� 
//�������أ�0=���óɹ�          1=����ʧ��
//˵          ��:��                                                       
//=========================================================================
#ifdef GPRS_NET
static uint_8 gsm_get_csca(void)
{
	uint_8* pstart = NULL;
	uint_8 ret = 1;
	uint_8 atret[100];
	
	//�˳�͸��ģʽ
	if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gprs_get_csca]: gprs quit trans failed!\r\n");
			return ret;
		}
	}

	//��ȡCSMA
	if(gprs_send_cmd((uint_8*)"AT+CSCA?\r", 
			         (uint_8*)"+CSCA:", (uint_8*)"OK", 100, 10, atret) != 0)	
	{
		_GPRS_DEBUG_STR("[gprs_get_csca]: gprs get csca failed!\r\n");
		return ret;
	}

	//�ҵ���һ��8��λ��
	pstart = (uint_8*)strchr((int_8*)atret, '8');  
	if(pstart != NULL)
	{
		memcpy((int_8*)s_GSMCSCA, (int_8*)pstart, 13);
		//_GPRS_DEBUG_STR("[gsm_get_csca]:s_GSMCSCA: ");
		//for(ret = 0; ret < 13; ret++)
			//_GPRS_DEBUG_CHAR(s_GSMCSCA[ret]);
		//_GPRS_DEBUG_STR("\r\n");
		ret = 0;
	}
	
	return ret;
}
#endif

//=========================================================================
//������: gsm_encode_7bit                                                   
//��  ��:  ����Ϣ7bit����
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������   
//               pDst - Ŀ������ָ��
//��  ��: Ŀ����봮����                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_encode_7bit(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 nSrc;		// Դ�ַ����ļ���ֵ
	uint_32 nDst;		// Ŀ����봮�ļ���ֵ
	uint_32 nChar;		// ��ǰ���ڴ���������ַ��ֽڵ���ţ���Χ��0-7
	uint_8 nLeft;	    // ��һ�ֽڲ��������

	// ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;

	// ��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
	// ѭ���ô�����̣�ֱ��Դ����������
	// ������鲻��8�ֽڣ�Ҳ����ȷ����
	while (nSrc < nSrcLen)
	{
		// ȡԴ�ַ����ļ���ֵ�����3λ
		nChar = nSrc & 7;

		// ����Դ����ÿ���ֽ�
		if(nChar == 0)
		{
			// ���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
			nLeft = *pSrc;
		}
		else
		{
			// ���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
			*pDst = (*pSrc << (8-nChar)) | nLeft;

			// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
			nLeft = *pSrc >> nChar;

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}

	// ����Ŀ�괮����
	return nDst;
}
#endif

//=========================================================================
//������: gsm_decode_7bit                                                   
//��  ��:  ����Ϣ7bit����
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������   
//               pDst - Ŀ������ָ��
//��  ��: Ŀ����봮����                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_decode_7bit(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 nSrc;		// Դ�ַ����ļ���ֵ
	uint_32 nDst;		// Ŀ����봮�ļ���ֵ
	uint_32 nByte;		// ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6
	uint_8 nLeft;	    // ��һ�ֽڲ��������

	// ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;
	
	// �����ֽ���źͲ������ݳ�ʼ��
	nByte = 0;
	nLeft = 0;

	// ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�
	// ѭ���ô�����̣�ֱ��Դ���ݱ�������
	// ������鲻��7�ֽڣ�Ҳ����ȷ����
	while(nSrc < nSrcLen)
	{
		// ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;

		// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
		nLeft = *pSrc >> (7-nByte);

		// �޸�Ŀ�괮��ָ��ͼ���ֵ
		pDst++;
		nDst++;

		// �޸��ֽڼ���ֵ
		nByte++;

		// ����һ������һ���ֽ�
		if(nByte == 7)
		{
			// ����õ�һ��Ŀ������ֽ�
			*pDst = nLeft;

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;

			// �����ֽ���źͲ������ݳ�ʼ��
			nByte = 0;
			nLeft = 0;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ�괮����
	return nDst;
}
#endif

//=========================================================================
//������: gsm_encode_8bit                                                 
//��  ��:  ����Ϣ8bit����
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������   
//               pDst - Ŀ������ָ��
//��  ��: Ŀ����봮����                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_encode_8bit(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	// �򵥸���
	memcpy(pDst, pSrc, nSrcLen);

	return nSrcLen;
}
#endif

//=========================================================================
//������: gsm_decode_8bit                                                 
//��  ��:  ����Ϣ8bit����
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������   
//               pDst - Ŀ������ָ��
//��  ��: Ŀ����봮����                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_decode_8bit(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	// �򵥸���
	memcpy(pDst, pSrc, nSrcLen);

	// ����ַ����Ӹ�������
	*pDst = '\0';

	return nSrcLen;
}
#endif

//=========================================================================
//������: gsm_encode_ucs2                                                 
//��  ��:  ����ϢUCS2����
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������   
//               pDst - Ŀ������ָ��
//��  ��: Ŀ����봮����                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_encode_ucs2(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	/*int nDstLength;		// UNICODE���ַ���Ŀ
	WCHAR wchar[128];	// UNICODE��������

	// �ַ���-->UNICODE��
	nDstLength = MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);

	// �ߵ��ֽڶԵ������
	for(int i=0; i<nDstLength; i++)
	{
		*pDst++ = wchar[i] >> 8;		// �������λ�ֽ�
		*pDst++ = wchar[i] & 0xff;		// �������λ�ֽ�
	}
	// ����Ŀ����봮����
	return nDstLength * 2;
	*/
	memcpy(pDst, pSrc, nSrcLen);
	return nSrcLen;
}
#endif

//=========================================================================
//������: gsm_decode_ucs2                                                 
//��  ��:   ����ϢUCS2����
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������   
//               pDst - Ŀ������ָ��
//��  ��: Ŀ����봮����                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_decode_ucs2(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	/*int nDstLength;		// UNICODE���ַ���Ŀ
	WCHAR wchar[128];	// UNICODE��������

	// �ߵ��ֽڶԵ���ƴ��UNICODE
	for(int i=0; i<nSrcLength/2; i++)
	{
		wchar[i] = *pSrc++ << 8;	// �ȸ�λ�ֽ�
		wchar[i] |= *pSrc++;		// ���λ�ֽ�
	}

	// UNICODE��-->�ַ���
	nDstLength = WideCharToMultiByte(CP_ACP, 0, wchar, nSrcLength/2, pDst, 160, NULL, NULL);

	// ����ַ����Ӹ�������
	pDst[nDstLength] = '\0';

	// ����Ŀ���ַ�������
	return nDstLength;
	*/
	
	memcpy(pDst, pSrc, nSrcLen);
	pDst[nSrcLen] = '\0';    // ����ַ����Ӹ�������
	return nSrcLen;
}
#endif

//=========================================================================
//������: gsm_invert_num                                                 
//��  ��:  ����˳����ַ���ת��Ϊ�����ߵ����ַ�����
//               ������Ϊ��������'F'�ճ�ż��
//                �磺"8613851872468" --> "683158812764F8"
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������   
//               pDst - Ŀ������ָ��
//��  ��: Ŀ���ַ�������                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_invert_num(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 nDstLength;		// Ŀ���ַ�������
	uint_8 ch;			    // ���ڱ���һ���ַ�
	uint_32 i;

	// ���ƴ�����
	nDstLength = nSrcLen;

	// �����ߵ�
	for(i=0; i<nSrcLen;i+=2)
	{
		ch = *pSrc++;		// �����ȳ��ֵ��ַ�
		*pDst++ = *pSrc++;	// ���ƺ���ֵ��ַ�
		*pDst++ = ch;		// �����ȳ��ֵ��ַ�
	}

	// Դ��������������
	if(nSrcLen & 1)
	{
		*(pDst-2) = 'F';	// ��'F'
		nDstLength++;		// Ŀ�괮���ȼ�1
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ���ַ�������
	return nDstLength;
}
#endif

//=========================================================================
//������: gsm_serialize_num                                                 
//��  ��:  �����ߵ����ַ���ת��Ϊ����˳����ַ���
//              �磺"683158812764F8" --> "8613851872468"
//��  ��: pSrc - Դ�ַ���ָ��
//               nSrcLen - Դ�ַ�������   
//               pDst - Ŀ������ָ��
//��  ��: Ŀ���ַ�������                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_serialize_num(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 nDstLength;		// Ŀ���ַ�������
	uint_8 ch;			// ���ڱ���һ���ַ�
	uint_32 i;

	// ���ƴ�����
	nDstLength = nSrcLen;

	// �����ߵ�
	for(i=0; i<nSrcLen;i+=2)
	{
		ch = *pSrc++;		// �����ȳ��ֵ��ַ�
		*pDst++ = *pSrc++;	// ���ƺ���ֵ��ַ�
		*pDst++ = ch;		// �����ȳ��ֵ��ַ�
	}

	// �����ַ���'F'��
	if(*(pDst-1) == 'F')
	{
		pDst--;
		nDstLength--;		// Ŀ���ַ������ȼ�1
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ���ַ�������
	return nDstLength;
}
#endif

//=========================================================================
//������: gsm_encode_pdu                                                 
//��  ��:  ����ϢPDU���룬���ڱ��ơ����Ͷ���Ϣ
//��  ��: pSrc - ԴPDU����ָ��
//               pDst - Ŀ��PDU��ָ��
//��  ��: Ŀ���ַ�������                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_encode_pdu(const TGSM_PARAM* pSrc, uint_8* pDst)
{
	uint_32 nLength;			// �ڲ��õĴ�����
	uint_32 nDstLength;	    // Ŀ��PDU������
	uint_8 buf[256];	        // �ڲ��õĻ�����

	// SMSC��ַ��Ϣ��
	nLength = strlen((int_8*)pSrc->sca);	// SMSC��ַ�ַ����ĳ���	
	buf[0] = (uint_8)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	// SMSC��ַ��Ϣ����
	buf[1] = 0x91;		           // �̶�: �ù��ʸ�ʽ����
	nDstLength = hex_to_string(buf, 2, pDst);		// ת��2���ֽڵ�Ŀ��PDU��
	nDstLength += gsm_invert_num(pSrc->sca, nLength, &pDst[nDstLength]);	// ת��SMSC���뵽Ŀ��PDU��

	// TPDU�λ���������Ŀ���ַ��
	nLength = strlen((int_8*)pSrc->tpa);	// TP-DA��ַ�ַ����ĳ���
	buf[0] = 0x11;					// �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
	buf[1] = 0;						// TP-MR=0
	buf[2] = (uint_8)nLength;			// Ŀ���ַ���ָ���(TP-DA��ַ�ַ�����ʵ����)
	buf[3] = 0x91;					// �̶�: �ù��ʸ�ʽ����
	nDstLength += hex_to_string(buf, 4, &pDst[nDstLength]);		// ת��4���ֽڵ�Ŀ��PDU��
	nDstLength += gsm_invert_num(pSrc->tpa, nLength, &pDst[nDstLength]);	// ת��TP-DA��Ŀ��PDU��

	// TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
	//Unicode���ϲ�ȷ��
	nLength = strlen((int_8*)pSrc->tp_ud);	// �û���Ϣ�ַ����ĳ���
	buf[0] = pSrc->tp_pid;			// Э���ʶ(TP-PID)
	buf[1] = pSrc->tp_dcs;			// �û���Ϣ���뷽ʽ(TP-DCS)
	buf[2] = 0;						// ��Ч��(TP-VP)Ϊ5����
	// 7-bit���뷽ʽ
	if(pSrc->tp_dcs == GSM_7BIT)	
	{
		buf[3] = nLength;			// ����ǰ����
		nLength = gsm_encode_7bit(pSrc->tp_ud, nLength+1, &buf[4]) + 4;	// ת��TP-DA��Ŀ��PDU��
	}
	// UCS2���뷽ʽ
	else if(pSrc->tp_dcs == GSM_UCS2)
	{
		buf[3] = gsm_encode_ucs2(pSrc->tp_ud, nLength, &buf[4]);	// ת��TP-DA��Ŀ��PDU��		
		nLength = buf[3] + 4;		// nLength���ڸö����ݳ���
	}
	// 8-bit���뷽ʽ
	else if(pSrc->tp_dcs == GSM_8BIT)
	{
		buf[3] = gsm_encode_8bit(pSrc->tp_ud, nLength, &buf[4]);	// ת��TP-DA��Ŀ��PDU��
		nLength = buf[3] + 4;		// nLength���ڸö����ݳ���
	}
	//ԭʼ����
	else if(pSrc->tp_dcs == GSM_ORIG)
	{
		buf[1] = GSM_UCS2;
		buf[3] = nLength >> 1;
		nLength = 4;
	}

	nDstLength += hex_to_string(buf, nLength, &pDst[nDstLength]);		// ת���ö����ݵ�Ŀ��PDU��

	//ԭʼ����
	if(pSrc->tp_dcs == GSM_ORIG)
	{
		strcat((int_8*)&pDst[nDstLength], (int_8*)pSrc->tp_ud);
		nDstLength += buf[3] << 1;
	}
	
	// ����Ŀ���ַ�������
	return nDstLength;
}
#endif

//=========================================================================
//������: gsm_decode_pdu                                                 
//��  ��:  ����ϢPDU���룬���ڽ��ա��Ķ�����Ϣ
//��  ��: pSrc - ԴPDU����ָ��
//               pDst - Ŀ��PDU��ָ��
//��  ��: Ŀ���ַ�������                                                       
//˵  ��:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_decode_pdu(const uint_8* pSrc, TGSM_PARAM* pDst)
{
	uint_32 nDstLength;			// Ŀ��PDU������
	uint_8 tmp;		            // �ڲ��õ���ʱ�ֽڱ���
	uint_8 buf[500];	        // �ڲ��õĻ�����
	uint_32 ntmp;

	// SMSC��ַ��Ϣ��
	string_to_hex(pSrc, 2, &tmp);   // ȡ����
	tmp = (tmp - 1) * 2;	       // SMSC���봮����
	pSrc += 4;			           // ָ����ƣ�������SMSC��ַ��ʽ
	gsm_serialize_num(pSrc, tmp, pDst->sca);	// ת��SMSC���뵽Ŀ��PDU��
	pSrc += tmp;		           // ָ�����

	// TPDU�λ�������
	string_to_hex(pSrc, 2, &tmp);   // ȡ��������
	pSrc += 2;		               // ָ�����

	// ȡ�ظ�����
	string_to_hex(pSrc, 2, &tmp);   // ȡ����
	if(tmp & 1) tmp += 1;	       // ������ż��
	pSrc += 4;			           // ָ����ƣ������˻ظ���ַ(TP-RA)��ʽ
	gsm_serialize_num(pSrc, tmp, pDst->tpa);	// ȡTP-RA����
	pSrc += tmp;		           // ָ�����

	// TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
	string_to_hex(pSrc, 2, (uint_8*)&pDst->tp_pid);	// ȡЭ���ʶ(TP-PID)
	pSrc += 2;		// ָ�����
	string_to_hex(pSrc, 2, (uint_8*)&pDst->tp_dcs);	// ȡ���뷽ʽ(TP-DCS)
	pSrc += 2;		// ָ�����
	gsm_serialize_num(pSrc, 14, pDst->tp_scts);	// ����ʱ����ַ���(TP_SCTS) 
	pSrc += 14;		// ָ�����
	string_to_hex(pSrc, 2, &tmp);	// �û���Ϣ����(TP-UDL)
	pSrc += 2;		// ָ�����
	// 7-bit����
	if(pDst->tp_dcs == GSM_7BIT)	
	{
		//7λ������Ч���ݳ���
		ntmp = (tmp * 7 + 8) >> 3;
		ntmp <<= 1;
		nDstLength = string_to_hex(pSrc, ntmp, buf);	// ��ʽת��
		gsm_decode_7bit(buf, nDstLength, pDst->tp_ud);	// ת����TP-DU
		nDstLength = tmp;
	}
	// UCS2����
	else if(pDst->tp_dcs == GSM_UCS2)
	{
		//nDstLength = String2Bytes(pSrc, tmp << 1, buf);			    // ��ʽת��
		nDstLength = tmp << 1;
		nDstLength = gsm_decode_ucs2(buf, nDstLength, pDst->tp_ud);	// ת����TP-DU
	}
	// 8-bit����
	else if(pDst->tp_dcs == GSM_8BIT)
	{
		nDstLength = string_to_hex(pSrc, tmp << 1, buf);			    // ��ʽת��
		nDstLength = gsm_decode_8bit(buf, nDstLength, pDst->tp_ud);	// ת����TP-DU
	}

	// ����Ŀ���ַ�������
	return nDstLength;
}
#endif

//=========================================================================
//������: gsm_init                                                        
//��  ��:��ʼ��GPRSģ��Ķ��Ź���                            
//��  ��: void,���贫���κβ���
//��  ��:0--��ʾGPRSģ��Ķ��Ź��ܳ�ʼ���ɹ�                                           
//		    ��0--��ʾGPRSģ��Ķ��Ź��ܳ�ʼ��ʧ��
//˵  ��: 
//=========================================================================
uint_8 gsm_init()
{
	uint_8 ret = eGPRSRET_NONE;
	
#ifdef GPRS_NET
	uint_8 i;

	//�˳�͸��ģʽ
	/*if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gsm_init]: gprs quit trans failed!\r\n");
			goto gsm_init_exit;
		}
	}*/

	//����SMΪ�洢������ߴ�ȡЧ��
	if(gprs_send_cmd((uint_8*)"AT+CPMS=\"SM\"\r", 
			         (uint_8*)"OK", NULL, 2000, 10, NULL) != 0)
	{   
		ret = eGPRSRET_SETCPSM;
		_GPRS_DEBUG_STR("[gsm_init]:gsm set sm memorizer failed!\r\n");
		goto gsm_init_exit;
	}
	_GPRS_DEBUG_STR("[gsm_init]:gsm set sm memorizer ok!\r\n");

    
	//����pduģʽ
	if(gprs_send_cmd((uint_8*)"AT+CMGF=0\r", 
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{   
		ret = eGPRSRET_SETCMGF;
		_GPRS_DEBUG_STR("[gsm_init]:gsm set pdu mode failed!\r\n");
		goto gsm_init_exit;
	}
	_GPRS_DEBUG_STR("[gsm_init]:gsm set pdu mode ok!\r\n");

	//��ȡ�������ĺ���
	if(gsm_get_csca() != 0)
	{   
		ret = eGPRSRET_GETCSCA;
		_GPRS_DEBUG_STR("[gsm_init]:gsm get csca failed!\r\n");
		goto gsm_init_exit;
	}
	_GPRS_DEBUG_STR("[gsm_init]:gsm init ok!\r\n");

gsm_init_exit:
	//����͸��ģʽ
	//gprs_set_iptrans(TRUE);
#endif

	return ret;
}

//=========================================================================
//������: gsm_send                                                        
//��  ��:���ŷ���(�ֻ�����,���͵�����,���볤�Ⱥ����ݳ���)                         
//��  ��: 
//��  ��:0--���ͳɹ�                                           
//		    ��0--����ʧ��
//˵  ��: 
//=========================================================================
uint_8 gsm_send(const uint_8* const phone, uint_8* psrc,uint_16 srclen)
{
#ifdef GPRS_NET
	uint_8 i, ret = eGPRSRET_SNDGSM;
	uint_8 atCmd[16];
	TGSM_PARAM tGsmParam;
	uint_8 pdu[512];		// PDU��
	uint_16 nPduLength;		// PDU������
	uint_8 nSmscLength;	    // SMSC������

	if(phone == NULL || psrc == NULL || srclen == 0)
		return eGPRSRET_PARAM;

	//�˳�͸��ģʽ
	/*if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gsm_send]: gprs quit trans failed!\r\n");
			return eGPRSRET_TRANS;
		}
	}*/

	// ������Ϣ�ṹ
	memcpy(tGsmParam.sca, s_GSMCSCA, 13);
	tGsmParam.sca[13] = '\0';
	tGsmParam.tpa[0] = '8';
	tGsmParam.tpa[1] = '6';
	memcpy(tGsmParam.tpa + 2, phone, 11);
	tGsmParam.tpa[13] = '\0';
	memcpy(tGsmParam.tp_ud, (int_8*)psrc, srclen);
	tGsmParam.tp_ud[srclen] = '\0';
	tGsmParam.tp_pid = 0;
	tGsmParam.tp_dcs = GSM_ORIG;

	// ����PDU����������PDU��
	nPduLength = gsm_encode_pdu(&tGsmParam, pdu);	

	//strcat(pdu, "\x01A");		// ��Ctrl-Z����
	pdu[nPduLength] = 0x1A;  // ��Ctrl-Z����
	pdu[nPduLength + 1] = '\r'; 
	
	string_to_hex(pdu, 2, &nSmscLength);	// ȡPDU���е�SMSC��Ϣ����
	nSmscLength++;		// ���ϳ����ֽڱ���

	// �����еĳ��ȣ�������SMSC��Ϣ���ȣ��������ֽڼ�
	sprintf(atCmd, "AT+CMGS=%d\r", nPduLength / 2 - nSmscLength);	// ��������
		
	//ͨ�����ڸ�GPRSģ�鷢��AT����
	if(gprs_send_cmd(atCmd,(uint_8*)">", NULL, 100, 10, NULL) != 0)
	{   
		return ret;
	}
	//_GPRS_DEBUG_STR("[gsm_send]:gsm send head ok!\r\n");

	//���Ͷ�������
	_GPRS_SEND_N(strlen((int_8*)pdu), pdu);
	//_GPRS_DEBUG_STR("[gsm_send]:send pdu data: ");
	//_GPRS_DEBUG_STR(pdu);
	//_GPRS_DEBUG_STR("\r\n");

	//�ȴ������жϽ�����ʱ (�ȴ������ַ�"OK")
	//s_GPRSIntTick = sys_tick;		
	_time_get_ticks(&mqx_tick_gprsint);
	while(is_timeout(mqx_tick_gprsint.TICKS[0], 5000) == FALSE);

	//��ʾ�Ѿ����յ����ݲ���������� ֡
	if(s_pNodePtr != NULL && s_pNodePtr->type == GPRSFRAME_CMD && s_pNodePtr->len > 1)
	{
		s_pNodePtr->pbuf[s_pNodePtr->len]='\0';//�ڽڵ���ջ�������ĩβ����'\0'�Ľ�β��ʶ��

		if(strstr((int_8*)s_pNodePtr->pbuf, "OK") != NULL)
		{
			ret = eGPRSRET_NONE;
		}
		
		//�黹�����еĽڵ�
		s_pNodePtr->len = 0;
		s_pNodePtr->type = GPRSFRAME_MAX;
		listnode_put(s_tGPRSInfo.pgprsfreelist, s_pNodePtr);
		s_pNodePtr = NULL;            //���㾲̬ȫ�ֱ���
	}
#else
    uint_8 ret = eGPRSRET_NONE;
#endif

	return ret;
}

//=========================================================================
//������: gsm_recv                                                        
//��  ��:����δ��ȡ�Ķ���Ϣ
//��  ��: ��
//��  ��: 0--����Ϣ���ճɹ�                                           
//		     ��0--����Ϣ����ʧ��                                        
//˵  ��: ÿ��ֻ����һ��
//=========================================================================
uint_8 gsm_recv(TGSM_PARAM* ptGsmParam)
{
#ifdef GPRS_NET
	uint_8* pstart = NULL;
	uint_8 ret = eGPRSRET_RECVGSM;
	uint_8 atret[1000];

	//�˳�͸��ģʽ
	/*if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gsm_recv]: gprs quit trans failed!\r\n");
			ret = eGPRSRET_TRANS;
			goto gsm_recv_exit;
		}
	}*/

	//�������ж���Ϣ�б�
	memset(atret, 0, 1000);
	if(gprs_send_cmd((uint_8*)"AT+CMGL=4\r",(uint_8*)"+CMGL: ", NULL, 500, 1, atret) != 0)
	{
		goto gsm_recv_exit;
	}

	//��ȡindex
	pstart = (uint_8*)strstr((int_8*)atret, (int_8*)"+CMGL: ");
	if(pstart != NULL)
	{
		pstart += strlen("+CMGL: ");
		
		//index����Ϊʮ������
		sscanf(pstart, "%d", &ptGsmParam->index);	// ��ȡ���

		//����pdu
		pstart = (uint_8*)strstr((int_8*)pstart, "\r\n");	// ����һ��
		if(pstart != NULL)
		{
			pstart += 2;		// ����"\r\n", ��λ��PDU
			gsm_decode_pdu(pstart, ptGsmParam);
			ret = eGPRSRET_NONE;
		}
	}

gsm_recv_exit:
	//�ٴν���͸��ģʽ
	//gprs_set_iptrans(TRUE);
#else
	uint_8 ret = eGPRSRET_NONE;
#endif

	return ret;
}

//=========================================================================
//������: gsm_del                                                        
//��  ��:���ݴӺ��������д���Ĳ���(��Ҫ���ص��ֻ�����,��Ҫ���صķ��͵����ݺ͸���ļ���ѡ��ֵ)                         
//��  ��: uint_8 delIndex:��ʾ��Ҫɾ���������ݵ�������
//              uint_8 delType:
//                         0--ɾ��ָ������λ�õĶ���
//                         1--ȫ��ɾ���洢���е��Ѷ�����
//                         2--ȫ��ɾ���洢���е��Ѷ����ѷ��Ͷ���
//                         3--ȫ��ɾ���洢���е��Ѷ����ѷ��ͺ�δ���Ͷ���
//                         4--ȫ��ɾ���洢���е��Ѷ���δ�����ѷ��ͺ�δ���Ͷ���
//��  ��:  0--����Ϣɾ���ɹ�                                           
//		     ��0--����Ϣɾ��ʧ��                                            
//˵  ��: 
//=========================================================================
uint_8 gsm_del(uint_8 delIndex, uint_8 delType)
{
#ifdef GPRS_NET
	uint_8 atCmd[20]="AT+CMGD=0,0";
	uint_8 i, ret;
	
	//�˳�͸��ģʽ
	/*if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gsm_del]: gprs quit trans failed!\r\n");
			ret =  eGPRSRET_TRANS;
			goto gsm_del_exit;
		}
	}*/

	if(delType > GSMDEL_ALL)
	{
		ret = eGPRSRET_PARAM;
	}
	else
	{
		//���ɾ�����ŵ�AT����
		//3λ
		if(delIndex >= 100)
		{
			atCmd[8] += delIndex / 100;
			delIndex %= 100;
			atCmd[9] = '0' + delIndex / 10;
			delIndex %= 10;
			atCmd[10] = '0' + delIndex;
			atCmd[11] = ',';
			atCmd[11] = '0' + delType;
			atCmd[13]='\r';
		}
		//2λ
		else if(delIndex >= 10)
		{
			atCmd[8] += delIndex / 10;
			delIndex %= 10;
			atCmd[9] = '0' + delIndex;
			atCmd[10] = ',';
			atCmd[11] = '0' + delType;
			atCmd[12]='\r';
		}
		//1λ
		else
		{
			atCmd[8] += delIndex;
			atCmd[10] += delType;
			atCmd[11]='\r';
		}

		//�������ж���Ϣ�б�
		if(gprs_send_cmd(atCmd,(uint_8*)"OK", NULL, 500, 10, NULL) != 0)
			ret = eGPRSRET_DELGSM;
		else
			ret = eGPRSRET_NONE;
	}

gsm_del_exit:
	//�ٴν���͸��ģʽ
	//gprs_set_iptrans(TRUE);
#else
		uint_8 ret = eGPRSRET_NONE;
#endif

	return ret;
}



