//============================================================================
//文件名称：buflist.h
//功能概要：实现链表缓冲的接口头文件
//版权所有：苏州大学飞思卡尔嵌入式中心(sumcu.suda.edu.cn)
//版本更新：2014-01-24	V2.0
//============================================================================

#ifndef __BUF_LIST_H__
#define __BUF_LIST_H__

#ifdef __cplusplus 
extern "C" { 
#endif // __cplusplus

// 头文件
#include "list.h"
#include "common.h"

// 返回值宏定义
#define ERR_NONE               0
#define ERR_LIST_INIT          1 
#define ERR_GET_ONE_NODE       2 
#define ERR_PUT_ONE_NODE       3 

//// 结构声明
//typedef struct
//{
//    uint_8*     pbuf;
//    uint_16     size;	 //buf空间大小，由list用户确定
//	uint_8      len;      //实际数据长度，操作时确定
//	uint_8      type;     //node类型，由应用层明确
//    struct  list_head list;
//}TBufNode;

// 结构声明
//#pragma pack(1)
typedef struct
{
	uint_8   CHNum;      //通道号，从1计数，0通道保留
	uint_8   CHSeleted;  //通道是否被选中
	uint_8   *pbuf; //该通道发送的数据区
    uint_32  size;	     //buf空间大小，由list用户确定
	uint_32  len;      //通道输出字节数
	uint_8   type;       //node类型，由应用层明确
    struct  list_head list;
}TBufNode;

//#pragma pack()

//============================================================================
//函数名: buflist_init
//功  能: 链表初始化  
//参  数: struct list_head* ptdatalist，数据链表指针
//	      struct list_head* ptfreelist，空链表指针
//		  TBufNode* pnode，buf节点数组首地址
//		  uint_32 dwnum，buf节点数组个数
//返  回: 错误码
//============================================================================
uint_8 buflist_init(struct list_head* ptdatalist, struct list_head* ptfreelist,TBufNode* pnode, uint_32 dwnum);

//============================================================================
//函数名: listnode_get
//功  能: 获取链表一个节点  
//参  数: struct list_head* ptlist，链表指针
//		  TBufNode** ppnode，返回节点指针的指针
//返  回: 错误码
//============================================================================
uint_8 listnode_get(struct list_head* ptlist,TBufNode** ppnode);

//============================================================================
//函数名: listnode_put
//功  能: 归还链表一个节点  
//参  数: struct list_head* ptlist，链表指针
//        TBufNode* pnode，返回节点指针
//返  回: 错误码
//============================================================================
uint_8 listnode_put(struct list_head* ptlist,TBufNode* pnode);

//============================================================================
//函数名: listnodenum_get
//功  能: 获取链表节点的个数 
//参  数: struct list_head* ptlist，链表指针
//        uint_8* pnum，返回节点个数
//返  回: 错误码
//============================================================================
uint_8 listnodenum_get(struct list_head* ptlist, uint_8* pnum);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __BUF_LIST_H__
