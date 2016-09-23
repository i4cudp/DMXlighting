/*====================================================================
模块名  ： BUF_LIST
文件名  ： buf_list.c
相关文件： buf_list.h
文件实现功能：实现链表缓冲
作者    ：刘辉
版本    ：1.0.0.0.0
----------------------------------------------------------------------
修改记录    ：
日  期      版本        修改人        修改内容
2012/3/02   1.0         刘辉          创建
====================================================================*/
/* 头文件 */
#include "buflist.h"

//============================================================================
//函数名: buflist_init
//功  能: 链表初始化  
//参  数: struct list_head* ptdatalist，数据链表指针
//	      struct list_head* ptfreelist，空链表指针
//		  TBufNode* pnode，buf节点数组首地址
//		  uint_32 dwnum，buf节点数组个数
//返  回: 错误码
//============================================================================
uint_8 buflist_init(struct list_head* ptdatalist, struct list_head* ptfreelist,TBufNode* pnode, uint_32 dwnum)
{
    uint_32 i;
	
	if(null == ptdatalist || null == ptfreelist || null == pnode || dwnum <= 0)
		return ERR_LIST_INIT;
	
	//链表初始化
	ptdatalist->next = ptdatalist;
	ptdatalist->prev = ptdatalist;
	ptfreelist->next = ptfreelist;
	ptfreelist->prev = ptfreelist;

	//将数据node全部加入freelist
	for(i = 0; i < dwnum; i++)
		list_add_tail(&pnode[i].list, ptfreelist);
		
    return ERR_NONE;
}

//============================================================================
//函数名: listnode_get
//功  能: 获取链表一个节点  
//参  数: struct list_head* ptlist，链表指针
//		  TBufNode** ppnode，返回节点指针的指针
//返  回: 错误码
//============================================================================
uint_8 listnode_get(struct list_head* ptlist,TBufNode** ppnode)
{
    struct list_head* plist = null;
	TBufNode* pnode = null;
	
	if(null == ptlist || list_empty(ptlist))
		return ERR_GET_ONE_NODE;
	
	//获取空链表节点
	plist = ptlist->next;
	list_del(plist);			
	pnode = list_entry(plist, TBufNode, list);
	if(null == pnode)
		return ERR_GET_ONE_NODE;

	*ppnode = pnode;
	
    return ERR_NONE;
}

//============================================================================
//函数名: listnode_put
//功  能: 归还链表一个节点  
//参  数: struct list_head* ptlist，链表指针
//        TBufNode* pnode，返回节点指针
//返  回: 错误码
//============================================================================
uint_8 listnode_put(struct list_head* ptlist,TBufNode* pnode)
{	
	if(null == ptlist || null == pnode)
		return ERR_PUT_ONE_NODE;
	
	list_add_tail(&pnode->list, ptlist);

    return ERR_NONE;
}

//============================================================================
//函数名: listnodenum_get
//功  能: 获取链表节点的个数 
//参  数: struct list_head* ptlist，链表指针
//        uint_8* pnum，返回节点个数
//返  回: 错误码
//============================================================================
uint_8 listnodenum_get(struct list_head* ptlist, uint_8* pnum)
{	
	uint_32 num = 0;
	struct list_head* plisttmp = null;
	
	if(null == ptlist)
		return ERR_PUT_ONE_NODE;
	
	list_for_each(plisttmp, ptlist)
        num++;
        
    *pnum = num;
        
    return ERR_NONE;
}

