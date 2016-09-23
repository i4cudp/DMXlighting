/*====================================================================
模块名  ： GPRS
文件名  ： gprs.c
相关文件： gprs.h
文件实现功能：实现GPRS模块底层驱动接口函数
作者    ：刘辉
版本    ：1.0.0.0.0
----------------------------------------------------------------------
修改记录    ：
日  期         版本        修改人        修改内容
2013/08/02       1.0             刘辉                  创建
====================================================================*/

#include "gprs.h"


//GPRS模块内部的全局变量
static TGPRSInfo s_tGPRSInfo;
static TBufNode* s_pNodePtr = NULL;
//static uint_32 s_GPRSIntTick = 0;
static uint_8 s_GSMCSCA[14];
static bool s_bIPTRANS = FALSE;
static bool s_bLinkBroken = FALSE;
static MQX_TICK_STRUCT mqx_tick_gprsint;



//-------------------------------------------------------------------------------------------
//函数名: gprs_send_cmd                                                        
//功  能: 发送命令给GPRS，返回回应是否超时了，并接收到的内容解析
//        是否有期望的回应数据                                                     
//参  数: uint_8* pcmd: 待发送的命令字符串 
//               uint_8 *responed1:期望的回应字符串1   
//               uint_8 *responed2:期望的回应字符串2   
//               uint_32 delayms:命令发送等待延时,ms
//               uint_8 sendtimes:命令发送失败的情况下，最大发送次数
//               uint_8* pret:返回接收到的应答数据
//返  回: 0--正确返回 
//        	     1--错误返回     
//说  明: 无
//-------------------------------------------------------------------------------------------
#ifdef GPRS_NET
static uint_8 gprs_send_cmd(uint_8* pcmd, uint_8* presponed1, uint_8* presponed2,
                                 uint_32 delayms, uint_8 sendtimes, uint_8* pret)
{
	uint_8 ret = 1;
	uint_8 i;

	for(i = 0; i < sendtimes; i++)
	{
		//通过串口发送函数向GPRS模块
		_GPRS_SEND_STR(pcmd); 
		
		//_GPRS_DEBUG_STR("[gprs_send_cmd]: send cmd: ");
	    //_GPRS_DEBUG_STR(pcmd);
		//_GPRS_DEBUG_STR("\r\n");
		
		//由于GPRS模块返回命令帧需要一定时间所以需要延时
		if(delayms > 0)
			delay_ms(delayms);
		
		//等待接收中断结束超时
		_time_get_ticks(&mqx_tick_gprsint);
		//s_GPRSIntTick = sys_tick;
		while(is_timeout(mqx_tick_gprsint.TICKS[0], 20) == FALSE);
		
		//表示已经接收到数据并且是命令返回 帧
		if(s_pNodePtr != NULL && s_pNodePtr->type == GPRSFRAME_CMD && s_pNodePtr->len > 1)
		{
			s_pNodePtr->pbuf[s_pNodePtr->len] = '\0';//在节点接收缓冲区的末尾添上'\0'的结尾标识符

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
			
			//将获取的节点放进链表pgprsfreelist
			s_pNodePtr->len = 0;
			s_pNodePtr->type = GPRSFRAME_MAX;
			listnode_put(s_tGPRSInfo.pgprsfreelist, s_pNodePtr);
			s_pNodePtr = NULL;   //清零静态全局变量
		}

		if(0 == ret)
			break;
	}

	return ret;
}
#endif

//=========================================================================
//函数名称：gprs_set_iptrans
//参数说明：bTrans！=0  进入透传模式       bTrans=0  退出透传模式  
//功能概要：进入和退出透明传输模式
//函数返回：0=设置成功          1=设置失败
//=========================================================================
uint_8 gprs_set_iptrans(uint_8 bTrans)
{
	uint_8 ret = 0;
	uint_8 atret[100];
	static uint_8 failtime = 0;   //连续3次进入透传模式，断链
	
	if(s_bIPTRANS != bTrans)
	{
#ifdef GPRS_NET
		//进入透明传输模式
		if(bTrans)
		{
			ret = gprs_send_cmd((uint_8*)"AT^IPENTRANS=0\r",(uint_8*)"OK", NULL, 50, 1, atret);
			if(0 == ret && strstr((int_8*)atret, (int_8*)"ERROR") == NULL)
			{
				s_bIPTRANS = true;                    //设置透明传输模式成功
				failtime = 0;
				s_bLinkBroken = FALSE;
			}
			else if(++failtime >= 3)
			{
				s_bLinkBroken = TRUE;
			}
		}
		else   //退出透明传输模式
		{
			delay_ms(1000);                     //等待1s，防止误认为为数据
			/*//先设置，为中断接收使用
			s_bIPTRANS = FALSE;
			ret = gprs_send_cmd((uint_8*)"+++",(uint_8*)"OK", NULL, 1000, 1, NULL);
			if(ret != 0)
			{
				s_bIPTRANS = TRUE;
			}*/
			//20140223不判断
			s_bIPTRANS = FALSE;
			gprs_send_cmd((uint_8*)"+++",(uint_8*)"OK", NULL, 1000, 1, NULL);
			delay_ms(1000);                     //等待1s，防止误认为为数据
		}
#else
        s_bIPTRANS = bTrans;
#endif
	}
	
	return ret;
}

//-------------------------------------------------------------------------------------------
//函数名:ip_to_str 											
//功  能: IPPort转换填写										
//参  数: 无
//内部调用:										
//函数返回:增加长度
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

	//ipaddr 循环多了一个'.'
	return --len;
}
#endif

//-------------------------------------------------------------------------------------------
//函数名:port_to_str 											
//功  能: uint32PortTostrPort转换填写										
//参  数: 无
//内部调用:										
//函数返回:增加长度
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
//函数名称：gprs_start
//参数说明：pDataList：接收数据链表头指针地址；
//                           pFreeList:空闲链表头指针地址；
//函数返回：0=初始化成功;
//                         非0，错误返回
//功能概要：初始化用到的GPRS模块引脚，启动并设置设置GPRS模块，连接目标服务器
//       GPRS接收数据链表与接收空闲链表指针传递
//说明：
//=========================================================================
uint_8 gprs_start(struct list_head* pDataList,struct list_head* pFreeList, GPRSRECVDATACallBack pdatacallback)
{
	uint_8 ret = eGPRSRET_NONE;
	uint_8 atcmd[50];
	uint_8 i;

	//参数检查
	if(NULL == pDataList || NULL == pFreeList || NULL == pdatacallback)
	{
		ret = eGPRSRET_PARAM;
		goto gprs_start_exit;
	}	

	//链表初始化
	s_tGPRSInfo.pgprsdatalist = pDataList;
	s_tGPRSInfo.pgprsfreelist = pFreeList;
	s_tGPRSInfo.pgprsdatacb = pdatacallback;
	s_bIPTRANS = FALSE;

	//硬件管脚启动
	_GPRS_DEBUG_STR("[gprs_start]:gprs power off\r\n");
	uart_init(GPRS_UART, GPRS_UART_BAUD);      // 初始化串口 
    uart_enable_re_int(GPRS_UART);                              //开串口接收中断

#ifdef GPRS_NET
	gpio_init(GPRS_POWER,1,GPRS_POWER_OFF);
	gpio_init(GPRS_TERMON,1,GPRS_TTERMON_OFF);
	//gpio_init(GPRS_RESET_PORT,GPRS_RESET_PIN,1,GPRS_RESET_OFF);
	delay_ms(2000);    //断电2s，电容放电
	_GPRS_DEBUG_STR("[gprs_start]:gprs power on\r\n");
	gpio_set(GPRS_POWER,GPRS_POWER_ON);
	delay_ms(1000);
	gpio_set(GPRS_TERMON,GPRS_TTERMON_ON);
	delay_ms(3000);

	//等待开机完成
	while(1)
	{
		if(s_pNodePtr != NULL && s_pNodePtr->type == GPRSFRAME_CMD && s_pNodePtr->len > 8)
		{
			s_pNodePtr->pbuf[s_pNodePtr->len] = '\0';//在节点接收缓冲区的末尾添上'\0'的结尾标识符

			if(strstr((const int_8*)s_pNodePtr->pbuf, (const int_8*)"^SYSSTART") != NULL)
			{
				//将获取的节点放进链表pgprsfreelist
				s_pNodePtr->len = 0;
				s_pNodePtr->type = GPRSFRAME_MAX;
				listnode_put(s_tGPRSInfo.pgprsfreelist, s_pNodePtr);
				s_pNodePtr = NULL;   //清零静态全局变量
				break;
			}
		}
	}	
	
	_GPRS_DEBUG_STR("[gprs_start]:gprs start ok!\r\n");

	//开机
	if(gprs_send_cmd((uint_8*)"AT\r", 
			         (uint_8*)"OK", NULL, 100, 50, NULL) != 0) 
	{
		ret = eGPRSRET_START;
		goto gprs_start_exit;
	}
	_GPRS_DEBUG_STR("[gprs_start]:gprs AT ok!\r\n");
	
	//关闭回显
	if(gprs_send_cmd((uint_8*)"ATE0\r", 
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)	
	{
		ret = eGPRSRET_CLOSEECHO;
		goto gprs_start_exit;
	}
	_GPRS_DEBUG_STR("[gprs_start]:gprs close echo ok!\r\n");
	
	//检测SIM卡
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
//函数名: gprs_ready                                                        
//功  能: 得到当前gprs是否上线                                                    
//参  数: 无
//说  明:无                                                       
//=========================================================================
#ifdef GPRS_NET
static bool gprs_ready()
{
	uint_8* pstart = NULL;
	uint_8 atret[100];
	bool bret = FALSE;
	uint_8 tmp8;
	
	//获取CSQ
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
		//CSQ返回为十进制数
		sscanf(pstart, "%d", &tmp8);	// 读取CSQ
		if(0 == tmp8)
		{
			_GPRS_DEBUG_STR("[gprs_ready]: gprs csq failed:0\r\n");
			goto gprs_ready_exit;
		}
	}

	//查询网络注册状态
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
	//查询网络注册状态返回的参数
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
//函数名称：gprs_init 
//参数说明：linktype:链接类型，移动或联通
//                           pIpPort：目标服务器的IP地址和端口号；
//函数返回：0=初始化成功;
//                         非0，错误返回
//功能概要：初始化用到的GPRS模块引脚，启动并设置设置GPRS模块，连接目标服务器
//       GPRS接收数据链表与接收空闲链表指针传递
//说明：参数IP_PORT(目标服务器的IP地址和端口号)格式例：202.195.128.106:8100，
//       SET_IP_PORT为定义的展开宏
//=========================================================================
uint_8 gprs_init(uint_8 linktype, const uint_8* ip, uint_16 port)
{
	uint_8 ret = eGPRSRET_NONE;
	
#ifdef GPRS_NET
	uint_8 atcmd[50];
	uint_8 i;
	uint_32 ticks;

	//参数检查
	if(linktype >= GPRSLINK_MAX || NULL == ip)
	{
		ret = eGPRSRET_PARAM;
		goto gprs_init_exit;
	}	

	//注册网络
	//将ID为0的Profile的 连接方式 设置为GPRS0
	if(gprs_send_cmd((uint_8*)"AT^SICS=0,conType,GPRS0\r",
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{
		ret = eGPRSRET_GPRS0;
		_GPRS_DEBUG_STR("[gprs_init]:gprs set gprs0 failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs set gprs0 ok!\r\n");

	//将ID为0的Profile的 接入点名称 设置为cmnet /uninet
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
		
	//将ID为0的Profile服务类型 设置为Socket
	if(gprs_send_cmd((uint_8*)"AT^SISS=0,srvType,Socket\r", 
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{
		ret = eGPRSRET_SOCKET;
		_GPRS_DEBUG_STR("[gprs_init]:gprs set socket failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs set socket ok!\r\n");
	
	//将ID为0的Profile的Internet链接号 设置为0
	if(gprs_send_cmd((uint_8*)"AT^SISS=0,conId,0\r", 
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{
		ret = eGPRSRET_CONID;
		_GPRS_DEBUG_STR("[gprs_init]:gprs set conid failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs set conid ok!\r\n");
	
	//打开链接
	//设置IP地址和端口号
	strcpy((int_8*)atcmd, "AT^SISS=0,address,\"socktcp://");
	i = (uint_8)strlen("AT^SISS=0,address,\"socktcp://");
    //添加打开链接的服务器的IP地址
    i += ip_to_str(ip, &atcmd[i]);
	//添加打开链接的服务器端口
	atcmd[i++] = ':';
    i += port_to_str(port, &atcmd[i]);
	atcmd[i++] = '\"';
	atcmd[i++] = '\r';
	//通过串口发送AT 命令
	//"OK"表示打开成功，"ERROR: 2\r"表示链接已经存在
	if(gprs_send_cmd(atcmd,(uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{   
		ret = eGPRSRET_LINKIP;
		_GPRS_DEBUG_STR("[gprs_init]:gprs link ip port failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs link ip port ok!\r\n");

	//等待系统上线
	while(1)
	{
		if(gprs_ready())
		{
			break;
		}
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs ready!\r\n");
	
	//打开一个Internet服务,0为服务标识（取值范围0-9）
	if(gprs_send_cmd((uint_8*)"AT^SISO=0\r", 
			         (uint_8*)"OK", NULL, 4000, 10, NULL) != 0)
	{   
		ret = eGPRSRET_SISO;
		_GPRS_DEBUG_STR("[gprs_init]:gprs set siso failed!\r\n");
		goto gprs_init_exit;
	}
	_GPRS_DEBUG_STR("[gprs_init]:gprs set siso ok!\r\n");

	//查询分配的IP地址
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
//函数名: gprs_get_csq                                                        
//功  能: 得到目前的信号质量                                                    
//参  数: pcsq:  表示信号质量的结果放在入口参数中 
//说  明:无                                                       
//=========================================================================
uint_8 gprs_get_csq(uint_8* pcsq)
{
#ifdef GPRS_NET
	uint_8* pstart = NULL;
	uint_8 ret = eGPRSRET_GETCSQ;
	uint_8 atret[100];
	
	//退出透传模式
	/*if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gprs_get_csq]: gprs quit trans failed!\r\n");
			goto gprs_get_csq_exit;
		}
	}*/

	//获取CSQ
	memset(atret, 0, 100);
	if(gprs_send_cmd((uint_8*)"AT+CSQ\r", 
			         (uint_8*)"OK", NULL, 100, 10, atret) != 0)	
	{
		_GPRS_DEBUG_STR("[gprs_get_csq]: gprs get csq failed!\r\n");
		goto gprs_get_csq_exit;
	}

	//取得能量值
	pstart = (uint_8*)strstr((int_8*)atret, (int_8*)"+CSQ: ");
	if(pstart != NULL)
	{
		pstart += strlen("+CSQ: ");
		//CSQ返回为十进制数
		sscanf(pstart, "%d", pcsq);	// 读取CSQ
		ret = eGPRSRET_NONE;
	}

gprs_get_csq_exit:
	//再次进入透传模式
	//gprs_set_iptrans(TRUE);

#else
	uint_8 ret = eGPRSRET_NONE;
	*pcsq = 31;
#endif

	return ret;
}

//=========================================================================
//函数名称：gprs_send
//参数说明：pdata:发送数据首地址  ，len:发送数据长度
//函数返回：无
//功能概要：发送数据给目标服务器。条件是目标服务器已链接成功       
//=========================================================================
uint_8 gprs_send(uint_8* pdata,uint_16 len)
{
	uint_8 ret = eGPRSRET_NONE;
	
#ifdef GPRS_NET
	//已断链
	if(s_bLinkBroken)
	{
		ret = eGPRSRET_BROKEN;
	    goto gprs_snd_exit;
	}
	
	//透传模式
	if(!s_bIPTRANS && gprs_set_iptrans(true))
	{
		ret = eGPRSRET_TRANS;
		goto gprs_snd_exit;
	}
#endif

	//数据发送
	if(_GPRS_SEND_N(len, pdata))
	{
		ret = eGPRSRET_SNDDATA;
	}
	
gprs_snd_exit:
	return ret;
}

//=========================================================================
//函数名称：gprs_isbroken
//参数说明：判断gprs是否断链
//函数返回：无
//功能概要：发送数据给目标服务器。条件是目标服务器已链接成功       
//=========================================================================
bool gprs_isbroken()
{
	return s_bLinkBroken;
}


//=========================================================================
//函数名称：gprs_recv_intproc 
//参数说明：无
//函数返回：无
//功能概要：中断处理接收数据
//=========================================================================
void gprs_recv_intproc()	                    
{
	uint_8 flag;
	uint_8 ch;

	//正确接收到数据
	ch = _GPRS_RECV_1(&flag);
	if(flag == 0)
	{
		//上帧未接收完数据丢弃
		if(is_timeout(mqx_tick_gprsint.TICKS[0], 1000) && s_pNodePtr != NULL)
		{
			//数据清空
			s_pNodePtr->len = 0;
#ifdef GPRS_NET
			s_pNodePtr->type = s_bIPTRANS ? GPRSFRAME_DATA : GPRSFRAME_CMD;
#else
            s_pNodePtr->type = GPRSFRAME_DATA;
#endif
		}

		//从全局链表"pgprsfreelist" 中取出节点
		if(NULL == s_pNodePtr)
		{	
			//从全局链表"pgprsfreelist" 中取出节点
			listnode_get(s_tGPRSInfo.pgprsfreelist, &s_pNodePtr);
			if(s_pNodePtr != NULL)                  //取到空闲节点
			{
				s_pNodePtr->len = 0;                //长度初始化
#ifdef GPRS_NET
							s_pNodePtr->type = s_bIPTRANS ? GPRSFRAME_DATA : GPRSFRAME_CMD;
#else
							s_pNodePtr->type = GPRSFRAME_DATA;
#endif
			}
		}

		//接收数据
		if(s_pNodePtr != NULL)
		{
			//接收数据
			if(s_pNodePtr->len < s_pNodePtr->size)
				s_pNodePtr->pbuf[s_pNodePtr->len++] = ch;

			
			//数据帧应用层判断是否结束
			if(GPRSFRAME_DATA == s_pNodePtr->type)
			{
#ifdef GPRS_NET
				//检测是否断链
				if(10 == s_pNodePtr->len)
				{
					ch = s_pNodePtr->pbuf[10];
					s_pNodePtr->pbuf[10] = '\0';
					if(strstr((int_8*)s_pNodePtr->pbuf,(int_8*)"ERROR") != NULL)
					{
						s_bLinkBroken = TRUE;
						s_pNodePtr->len = 0;                //长度初始化
						s_bIPTRANS = FALSE;                 //自动退出透传模式
						s_pNodePtr->type = GPRSFRAME_CMD;
					}
					else
						s_pNodePtr->pbuf[10] = ch;
				}
#endif
				
				//数据正确接收,其余不处理
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
//函数名: gsm_get_csca                                                        
//功     能: 得到目前的中心号码                                                    
//参     数: pcsq:  表示信号质量的结果放在入口参数中 
//函数返回：0=设置成功          1=设置失败
//说          明:无                                                       
//=========================================================================
#ifdef GPRS_NET
static uint_8 gsm_get_csca(void)
{
	uint_8* pstart = NULL;
	uint_8 ret = 1;
	uint_8 atret[100];
	
	//退出透传模式
	if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gprs_get_csca]: gprs quit trans failed!\r\n");
			return ret;
		}
	}

	//获取CSMA
	if(gprs_send_cmd((uint_8*)"AT+CSCA?\r", 
			         (uint_8*)"+CSCA:", (uint_8*)"OK", 100, 10, atret) != 0)	
	{
		_GPRS_DEBUG_STR("[gprs_get_csca]: gprs get csca failed!\r\n");
		return ret;
	}

	//找到第一个8的位置
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
//函数名: gsm_encode_7bit                                                   
//功  能:  短消息7bit编码
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度   
//               pDst - 目标数据指针
//返  回: 目标编码串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_encode_7bit(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 nSrc;		// 源字符串的计数值
	uint_32 nDst;		// 目标编码串的计数值
	uint_32 nChar;		// 当前正在处理的组内字符字节的序号，范围是0-7
	uint_8 nLeft;	    // 上一字节残余的数据

	// 计数值初始化
	nSrc = 0;
	nDst = 0;

	// 将源串每8个字节分为一组，压缩成7个字节
	// 循环该处理过程，直至源串被处理完
	// 如果分组不到8字节，也能正确处理
	while (nSrc < nSrcLen)
	{
		// 取源字符串的计数值的最低3位
		nChar = nSrc & 7;

		// 处理源串的每个字节
		if(nChar == 0)
		{
			// 组内第一个字节，只是保存起来，待处理下一个字节时使用
			nLeft = *pSrc;
		}
		else
		{
			// 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
			*pDst = (*pSrc << (8-nChar)) | nLeft;

			// 将该字节剩下的左边部分，作为残余数据保存起来
			nLeft = *pSrc >> nChar;

			// 修改目标串的指针和计数值
			pDst++;
			nDst++;
		}

		// 修改源串的指针和计数值
		pSrc++;
		nSrc++;
	}

	// 返回目标串长度
	return nDst;
}
#endif

//=========================================================================
//函数名: gsm_decode_7bit                                                   
//功  能:  短消息7bit解码
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度   
//               pDst - 目标数据指针
//返  回: 目标编码串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_decode_7bit(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 nSrc;		// 源字符串的计数值
	uint_32 nDst;		// 目标解码串的计数值
	uint_32 nByte;		// 当前正在处理的组内字节的序号，范围是0-6
	uint_8 nLeft;	    // 上一字节残余的数据

	// 计数值初始化
	nSrc = 0;
	nDst = 0;
	
	// 组内字节序号和残余数据初始化
	nByte = 0;
	nLeft = 0;

	// 将源数据每7个字节分为一组，解压缩成8个字节
	// 循环该处理过程，直至源数据被处理完
	// 如果分组不到7字节，也能正确处理
	while(nSrc < nSrcLen)
	{
		// 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;

		// 将该字节剩下的左边部分，作为残余数据保存起来
		nLeft = *pSrc >> (7-nByte);

		// 修改目标串的指针和计数值
		pDst++;
		nDst++;

		// 修改字节计数值
		nByte++;

		// 到了一组的最后一个字节
		if(nByte == 7)
		{
			// 额外得到一个目标解码字节
			*pDst = nLeft;

			// 修改目标串的指针和计数值
			pDst++;
			nDst++;

			// 组内字节序号和残余数据初始化
			nByte = 0;
			nLeft = 0;
		}

		// 修改源串的指针和计数值
		pSrc++;
		nSrc++;
	}

	// 输出字符串加个结束符
	*pDst = '\0';

	// 返回目标串长度
	return nDst;
}
#endif

//=========================================================================
//函数名: gsm_encode_8bit                                                 
//功  能:  短消息8bit编码
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度   
//               pDst - 目标数据指针
//返  回: 目标编码串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_encode_8bit(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	// 简单复制
	memcpy(pDst, pSrc, nSrcLen);

	return nSrcLen;
}
#endif

//=========================================================================
//函数名: gsm_decode_8bit                                                 
//功  能:  短消息8bit解码
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度   
//               pDst - 目标数据指针
//返  回: 目标编码串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_decode_8bit(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	// 简单复制
	memcpy(pDst, pSrc, nSrcLen);

	// 输出字符串加个结束符
	*pDst = '\0';

	return nSrcLen;
}
#endif

//=========================================================================
//函数名: gsm_encode_ucs2                                                 
//功  能:  短消息UCS2编码
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度   
//               pDst - 目标数据指针
//返  回: 目标编码串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_encode_ucs2(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	/*int nDstLength;		// UNICODE宽字符数目
	WCHAR wchar[128];	// UNICODE串缓冲区

	// 字符串-->UNICODE串
	nDstLength = MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);

	// 高低字节对调，输出
	for(int i=0; i<nDstLength; i++)
	{
		*pDst++ = wchar[i] >> 8;		// 先输出高位字节
		*pDst++ = wchar[i] & 0xff;		// 后输出低位字节
	}
	// 返回目标编码串长度
	return nDstLength * 2;
	*/
	memcpy(pDst, pSrc, nSrcLen);
	return nSrcLen;
}
#endif

//=========================================================================
//函数名: gsm_decode_ucs2                                                 
//功  能:   短消息UCS2解码
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度   
//               pDst - 目标数据指针
//返  回: 目标编码串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_decode_ucs2(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	/*int nDstLength;		// UNICODE宽字符数目
	WCHAR wchar[128];	// UNICODE串缓冲区

	// 高低字节对调，拼成UNICODE
	for(int i=0; i<nSrcLength/2; i++)
	{
		wchar[i] = *pSrc++ << 8;	// 先高位字节
		wchar[i] |= *pSrc++;		// 后低位字节
	}

	// UNICODE串-->字符串
	nDstLength = WideCharToMultiByte(CP_ACP, 0, wchar, nSrcLength/2, pDst, 160, NULL, NULL);

	// 输出字符串加个结束符
	pDst[nDstLength] = '\0';

	// 返回目标字符串长度
	return nDstLength;
	*/
	
	memcpy(pDst, pSrc, nSrcLen);
	pDst[nSrcLen] = '\0';    // 输出字符串加个结束符
	return nSrcLen;
}
#endif

//=========================================================================
//函数名: gsm_invert_num                                                 
//功  能:  正常顺序的字符串转换为两两颠倒的字符串，
//               若长度为奇数，补'F'凑成偶数
//                如："8613851872468" --> "683158812764F8"
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度   
//               pDst - 目标数据指针
//返  回: 目标字符串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_invert_num(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 nDstLength;		// 目标字符串长度
	uint_8 ch;			    // 用于保存一个字符
	uint_32 i;

	// 复制串长度
	nDstLength = nSrcLen;

	// 两两颠倒
	for(i=0; i<nSrcLen;i+=2)
	{
		ch = *pSrc++;		// 保存先出现的字符
		*pDst++ = *pSrc++;	// 复制后出现的字符
		*pDst++ = ch;		// 复制先出现的字符
	}

	// 源串长度是奇数吗？
	if(nSrcLen & 1)
	{
		*(pDst-2) = 'F';	// 补'F'
		nDstLength++;		// 目标串长度加1
	}

	// 输出字符串加个结束符
	*pDst = '\0';

	// 返回目标字符串长度
	return nDstLength;
}
#endif

//=========================================================================
//函数名: gsm_serialize_num                                                 
//功  能:  两两颠倒的字符串转换为正常顺序的字符串
//              如："683158812764F8" --> "8613851872468"
//参  数: pSrc - 源字符串指针
//               nSrcLen - 源字符串长度   
//               pDst - 目标数据指针
//返  回: 目标字符串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_serialize_num(const uint_8* pSrc, uint_32 nSrcLen, uint_8* pDst)
{
	uint_32 nDstLength;		// 目标字符串长度
	uint_8 ch;			// 用于保存一个字符
	uint_32 i;

	// 复制串长度
	nDstLength = nSrcLen;

	// 两两颠倒
	for(i=0; i<nSrcLen;i+=2)
	{
		ch = *pSrc++;		// 保存先出现的字符
		*pDst++ = *pSrc++;	// 复制后出现的字符
		*pDst++ = ch;		// 复制先出现的字符
	}

	// 最后的字符是'F'吗？
	if(*(pDst-1) == 'F')
	{
		pDst--;
		nDstLength--;		// 目标字符串长度减1
	}

	// 输出字符串加个结束符
	*pDst = '\0';

	// 返回目标字符串长度
	return nDstLength;
}
#endif

//=========================================================================
//函数名: gsm_encode_pdu                                                 
//功  能:  短消息PDU编码，用于编制、发送短消息
//参  数: pSrc - 源PDU参数指针
//               pDst - 目标PDU串指针
//返  回: 目标字符串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_encode_pdu(const TGSM_PARAM* pSrc, uint_8* pDst)
{
	uint_32 nLength;			// 内部用的串长度
	uint_32 nDstLength;	    // 目标PDU串长度
	uint_8 buf[256];	        // 内部用的缓冲区

	// SMSC地址信息段
	nLength = strlen((int_8*)pSrc->sca);	// SMSC地址字符串的长度	
	buf[0] = (uint_8)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	// SMSC地址信息长度
	buf[1] = 0x91;		           // 固定: 用国际格式号码
	nDstLength = hex_to_string(buf, 2, pDst);		// 转换2个字节到目标PDU串
	nDstLength += gsm_invert_num(pSrc->sca, nLength, &pDst[nDstLength]);	// 转换SMSC号码到目标PDU串

	// TPDU段基本参数、目标地址等
	nLength = strlen((int_8*)pSrc->tpa);	// TP-DA地址字符串的长度
	buf[0] = 0x11;					// 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
	buf[1] = 0;						// TP-MR=0
	buf[2] = (uint_8)nLength;			// 目标地址数字个数(TP-DA地址字符串真实长度)
	buf[3] = 0x91;					// 固定: 用国际格式号码
	nDstLength += hex_to_string(buf, 4, &pDst[nDstLength]);		// 转换4个字节到目标PDU串
	nDstLength += gsm_invert_num(pSrc->tpa, nLength, &pDst[nDstLength]);	// 转换TP-DA到目标PDU串

	// TPDU段协议标识、编码方式、用户信息等
	//Unicode由上层确定
	nLength = strlen((int_8*)pSrc->tp_ud);	// 用户信息字符串的长度
	buf[0] = pSrc->tp_pid;			// 协议标识(TP-PID)
	buf[1] = pSrc->tp_dcs;			// 用户信息编码方式(TP-DCS)
	buf[2] = 0;						// 有效期(TP-VP)为5分钟
	// 7-bit编码方式
	if(pSrc->tp_dcs == GSM_7BIT)	
	{
		buf[3] = nLength;			// 编码前长度
		nLength = gsm_encode_7bit(pSrc->tp_ud, nLength+1, &buf[4]) + 4;	// 转换TP-DA到目标PDU串
	}
	// UCS2编码方式
	else if(pSrc->tp_dcs == GSM_UCS2)
	{
		buf[3] = gsm_encode_ucs2(pSrc->tp_ud, nLength, &buf[4]);	// 转换TP-DA到目标PDU串		
		nLength = buf[3] + 4;		// nLength等于该段数据长度
	}
	// 8-bit编码方式
	else if(pSrc->tp_dcs == GSM_8BIT)
	{
		buf[3] = gsm_encode_8bit(pSrc->tp_ud, nLength, &buf[4]);	// 转换TP-DA到目标PDU串
		nLength = buf[3] + 4;		// nLength等于该段数据长度
	}
	//原始数据
	else if(pSrc->tp_dcs == GSM_ORIG)
	{
		buf[1] = GSM_UCS2;
		buf[3] = nLength >> 1;
		nLength = 4;
	}

	nDstLength += hex_to_string(buf, nLength, &pDst[nDstLength]);		// 转换该段数据到目标PDU串

	//原始数据
	if(pSrc->tp_dcs == GSM_ORIG)
	{
		strcat((int_8*)&pDst[nDstLength], (int_8*)pSrc->tp_ud);
		nDstLength += buf[3] << 1;
	}
	
	// 返回目标字符串长度
	return nDstLength;
}
#endif

//=========================================================================
//函数名: gsm_decode_pdu                                                 
//功  能:  短消息PDU解码，用于接收、阅读短消息
//参  数: pSrc - 源PDU参数指针
//               pDst - 目标PDU串指针
//返  回: 目标字符串长度                                                       
//说  明:     
//=========================================================================
#ifdef GPRS_NET
static uint_32 gsm_decode_pdu(const uint_8* pSrc, TGSM_PARAM* pDst)
{
	uint_32 nDstLength;			// 目标PDU串长度
	uint_8 tmp;		            // 内部用的临时字节变量
	uint_8 buf[500];	        // 内部用的缓冲区
	uint_32 ntmp;

	// SMSC地址信息段
	string_to_hex(pSrc, 2, &tmp);   // 取长度
	tmp = (tmp - 1) * 2;	       // SMSC号码串长度
	pSrc += 4;			           // 指针后移，忽略了SMSC地址格式
	gsm_serialize_num(pSrc, tmp, pDst->sca);	// 转换SMSC号码到目标PDU串
	pSrc += tmp;		           // 指针后移

	// TPDU段基本参数
	string_to_hex(pSrc, 2, &tmp);   // 取基本参数
	pSrc += 2;		               // 指针后移

	// 取回复号码
	string_to_hex(pSrc, 2, &tmp);   // 取长度
	if(tmp & 1) tmp += 1;	       // 调整奇偶性
	pSrc += 4;			           // 指针后移，忽略了回复地址(TP-RA)格式
	gsm_serialize_num(pSrc, tmp, pDst->tpa);	// 取TP-RA号码
	pSrc += tmp;		           // 指针后移

	// TPDU段协议标识、编码方式、用户信息等
	string_to_hex(pSrc, 2, (uint_8*)&pDst->tp_pid);	// 取协议标识(TP-PID)
	pSrc += 2;		// 指针后移
	string_to_hex(pSrc, 2, (uint_8*)&pDst->tp_dcs);	// 取编码方式(TP-DCS)
	pSrc += 2;		// 指针后移
	gsm_serialize_num(pSrc, 14, pDst->tp_scts);	// 服务时间戳字符串(TP_SCTS) 
	pSrc += 14;		// 指针后移
	string_to_hex(pSrc, 2, &tmp);	// 用户信息长度(TP-UDL)
	pSrc += 2;		// 指针后移
	// 7-bit解码
	if(pDst->tp_dcs == GSM_7BIT)	
	{
		//7位编码有效数据长度
		ntmp = (tmp * 7 + 8) >> 3;
		ntmp <<= 1;
		nDstLength = string_to_hex(pSrc, ntmp, buf);	// 格式转换
		gsm_decode_7bit(buf, nDstLength, pDst->tp_ud);	// 转换到TP-DU
		nDstLength = tmp;
	}
	// UCS2解码
	else if(pDst->tp_dcs == GSM_UCS2)
	{
		//nDstLength = String2Bytes(pSrc, tmp << 1, buf);			    // 格式转换
		nDstLength = tmp << 1;
		nDstLength = gsm_decode_ucs2(buf, nDstLength, pDst->tp_ud);	// 转换到TP-DU
	}
	// 8-bit解码
	else if(pDst->tp_dcs == GSM_8BIT)
	{
		nDstLength = string_to_hex(pSrc, tmp << 1, buf);			    // 格式转换
		nDstLength = gsm_decode_8bit(buf, nDstLength, pDst->tp_ud);	// 转换到TP-DU
	}

	// 返回目标字符串长度
	return nDstLength;
}
#endif

//=========================================================================
//函数名: gsm_init                                                        
//功  能:初始化GPRS模块的短信功能                            
//参  数: void,无需传输任何参数
//返  回:0--表示GPRS模块的短信功能初始化成功                                           
//		    非0--表示GPRS模块的短信功能初始化失败
//说  明: 
//=========================================================================
uint_8 gsm_init()
{
	uint_8 ret = eGPRSRET_NONE;
	
#ifdef GPRS_NET
	uint_8 i;

	//退出透传模式
	/*if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gsm_init]: gprs quit trans failed!\r\n");
			goto gsm_init_exit;
		}
	}*/

	//设置SM为存储器，提高存取效率
	if(gprs_send_cmd((uint_8*)"AT+CPMS=\"SM\"\r", 
			         (uint_8*)"OK", NULL, 2000, 10, NULL) != 0)
	{   
		ret = eGPRSRET_SETCPSM;
		_GPRS_DEBUG_STR("[gsm_init]:gsm set sm memorizer failed!\r\n");
		goto gsm_init_exit;
	}
	_GPRS_DEBUG_STR("[gsm_init]:gsm set sm memorizer ok!\r\n");

    
	//设置pdu模式
	if(gprs_send_cmd((uint_8*)"AT+CMGF=0\r", 
			         (uint_8*)"OK", NULL, 100, 10, NULL) != 0)
	{   
		ret = eGPRSRET_SETCMGF;
		_GPRS_DEBUG_STR("[gsm_init]:gsm set pdu mode failed!\r\n");
		goto gsm_init_exit;
	}
	_GPRS_DEBUG_STR("[gsm_init]:gsm set pdu mode ok!\r\n");

	//获取服务中心号码
	if(gsm_get_csca() != 0)
	{   
		ret = eGPRSRET_GETCSCA;
		_GPRS_DEBUG_STR("[gsm_init]:gsm get csca failed!\r\n");
		goto gsm_init_exit;
	}
	_GPRS_DEBUG_STR("[gsm_init]:gsm init ok!\r\n");

gsm_init_exit:
	//进入透传模式
	//gprs_set_iptrans(TRUE);
#endif

	return ret;
}

//=========================================================================
//函数名: gsm_send                                                        
//功  能:短信发送(手机号码,发送的内容,号码长度和内容长度)                         
//参  数: 
//返  回:0--发送成功                                           
//		    非0--发送失败
//说  明: 
//=========================================================================
uint_8 gsm_send(const uint_8* const phone, uint_8* psrc,uint_16 srclen)
{
#ifdef GPRS_NET
	uint_8 i, ret = eGPRSRET_SNDGSM;
	uint_8 atCmd[16];
	TGSM_PARAM tGsmParam;
	uint_8 pdu[512];		// PDU串
	uint_16 nPduLength;		// PDU串长度
	uint_8 nSmscLength;	    // SMSC串长度

	if(phone == NULL || psrc == NULL || srclen == 0)
		return eGPRSRET_PARAM;

	//退出透传模式
	/*if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gsm_send]: gprs quit trans failed!\r\n");
			return eGPRSRET_TRANS;
		}
	}*/

	// 填充短消息结构
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

	// 根据PDU参数，编码PDU串
	nPduLength = gsm_encode_pdu(&tGsmParam, pdu);	

	//strcat(pdu, "\x01A");		// 以Ctrl-Z结束
	pdu[nPduLength] = 0x1A;  // 以Ctrl-Z结束
	pdu[nPduLength + 1] = '\r'; 
	
	string_to_hex(pdu, 2, &nSmscLength);	// 取PDU串中的SMSC信息长度
	nSmscLength++;		// 加上长度字节本身

	// 命令中的长度，不包括SMSC信息长度，以数据字节计
	sprintf(atCmd, "AT+CMGS=%d\r", nPduLength / 2 - nSmscLength);	// 生成命令
		
	//通过串口给GPRS模块发送AT命令
	if(gprs_send_cmd(atCmd,(uint_8*)">", NULL, 100, 10, NULL) != 0)
	{   
		return ret;
	}
	//_GPRS_DEBUG_STR("[gsm_send]:gsm send head ok!\r\n");

	//发送短信内容
	_GPRS_SEND_N(strlen((int_8*)pdu), pdu);
	//_GPRS_DEBUG_STR("[gsm_send]:send pdu data: ");
	//_GPRS_DEBUG_STR(pdu);
	//_GPRS_DEBUG_STR("\r\n");

	//等待接收中断结束超时 (等待接收字符"OK")
	//s_GPRSIntTick = sys_tick;		
	_time_get_ticks(&mqx_tick_gprsint);
	while(is_timeout(mqx_tick_gprsint.TICKS[0], 5000) == FALSE);

	//表示已经接收到数据并且是命令返回 帧
	if(s_pNodePtr != NULL && s_pNodePtr->type == GPRSFRAME_CMD && s_pNodePtr->len > 1)
	{
		s_pNodePtr->pbuf[s_pNodePtr->len]='\0';//在节点接收缓冲区的末尾添上'\0'的结尾标识符

		if(strstr((int_8*)s_pNodePtr->pbuf, "OK") != NULL)
		{
			ret = eGPRSRET_NONE;
		}
		
		//归还链接中的节点
		s_pNodePtr->len = 0;
		s_pNodePtr->type = GPRSFRAME_MAX;
		listnode_put(s_tGPRSInfo.pgprsfreelist, s_pNodePtr);
		s_pNodePtr = NULL;            //清零静态全局变量
	}
#else
    uint_8 ret = eGPRSRET_NONE;
#endif

	return ret;
}

//=========================================================================
//函数名: gsm_recv                                                        
//功  能:接收未读取的短消息
//参  数: 无
//返  回: 0--短消息接收成功                                           
//		     非0--短消息接收失败                                        
//说  明: 每次只接收一条
//=========================================================================
uint_8 gsm_recv(TGSM_PARAM* ptGsmParam)
{
#ifdef GPRS_NET
	uint_8* pstart = NULL;
	uint_8 ret = eGPRSRET_RECVGSM;
	uint_8 atret[1000];

	//退出透传模式
	/*if(s_bIPTRANS)
	{
		if(gprs_set_iptrans(FALSE) != eGPRSRET_NONE)
		{
			_GPRS_DEBUG_STR("[gsm_recv]: gprs quit trans failed!\r\n");
			ret = eGPRSRET_TRANS;
			goto gsm_recv_exit;
		}
	}*/

	//接收所有短消息列表
	memset(atret, 0, 1000);
	if(gprs_send_cmd((uint_8*)"AT+CMGL=4\r",(uint_8*)"+CMGL: ", NULL, 500, 1, atret) != 0)
	{
		goto gsm_recv_exit;
	}

	//获取index
	pstart = (uint_8*)strstr((int_8*)atret, (int_8*)"+CMGL: ");
	if(pstart != NULL)
	{
		pstart += strlen("+CMGL: ");
		
		//index返回为十进制数
		sscanf(pstart, "%d", &ptGsmParam->index);	// 读取序号

		//查找pdu
		pstart = (uint_8*)strstr((int_8*)pstart, "\r\n");	// 找下一行
		if(pstart != NULL)
		{
			pstart += 2;		// 跳过"\r\n", 定位到PDU
			gsm_decode_pdu(pstart, ptGsmParam);
			ret = eGPRSRET_NONE;
		}
	}

gsm_recv_exit:
	//再次进入透传模式
	//gprs_set_iptrans(TRUE);
#else
	uint_8 ret = eGPRSRET_NONE;
#endif

	return ret;
}

//=========================================================================
//函数名: gsm_del                                                        
//功  能:根据从函数参数中传入的参数(需要返回的手机号码,需要返回的发送的内容和给点的检索选项值)                         
//参  数: uint_8 delIndex:表示需要删除短信内容的索引号
//              uint_8 delType:
//                         0--删除指定索引位置的短信
//                         1--全部删除存储器中的已读短信
//                         2--全部删除存储器中的已读和已发送短信
//                         3--全部删除存储器中的已读、已发送和未发送短信
//                         4--全部删除存储器中的已读、未读、已发送和未发送短信
//返  回:  0--短消息删除成功                                           
//		     非0--短消息删除失败                                            
//说  明: 
//=========================================================================
uint_8 gsm_del(uint_8 delIndex, uint_8 delType)
{
#ifdef GPRS_NET
	uint_8 atCmd[20]="AT+CMGD=0,0";
	uint_8 i, ret;
	
	//退出透传模式
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
		//组成删除短信的AT命令
		//3位
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
		//2位
		else if(delIndex >= 10)
		{
			atCmd[8] += delIndex / 10;
			delIndex %= 10;
			atCmd[9] = '0' + delIndex;
			atCmd[10] = ',';
			atCmd[11] = '0' + delType;
			atCmd[12]='\r';
		}
		//1位
		else
		{
			atCmd[8] += delIndex;
			atCmd[10] += delType;
			atCmd[11]='\r';
		}

		//接收所有短消息列表
		if(gprs_send_cmd(atCmd,(uint_8*)"OK", NULL, 500, 10, NULL) != 0)
			ret = eGPRSRET_DELGSM;
		else
			ret = eGPRSRET_NONE;
	}

gsm_del_exit:
	//再次进入透传模式
	//gprs_set_iptrans(TRUE);
#else
		uint_8 ret = eGPRSRET_NONE;
#endif

	return ret;
}



