//说明见工程文件夹下的Doc文件夹内Readme.txt文件

#define GLOBLE_VAR  //只需在main.c中定义一次，用来防止全局变量的重复定义

#include "includes.h"

extern const MQXLITE_INITIALIZATION_STRUCT  MQX_init_struct;

void main(void)
{
    // 初始化mqx
	_mqxlite_init((MQXLITE_INITIALIZATION_STRUCT_PTR) &MQX_init_struct);
    // 启动mqx
	_mqxlite();
}
