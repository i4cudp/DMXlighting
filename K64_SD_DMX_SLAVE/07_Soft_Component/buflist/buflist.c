/*====================================================================
ģ����  �� BUF_LIST
�ļ���  �� buf_list.c
����ļ��� buf_list.h
�ļ�ʵ�ֹ��ܣ�ʵ��������
����    ������
�汾    ��1.0.0.0.0
----------------------------------------------------------------------
�޸ļ�¼    ��
��  ��      �汾        �޸���        �޸�����
2012/3/02   1.0         ����          ����
====================================================================*/
/* ͷ�ļ� */
#include "buflist.h"

//============================================================================
//������: buflist_init
//��  ��: �����ʼ��  
//��  ��: struct list_head* ptdatalist����������ָ��
//	      struct list_head* ptfreelist��������ָ��
//		  TBufNode* pnode��buf�ڵ������׵�ַ
//		  uint_32 dwnum��buf�ڵ��������
//��  ��: ������
//============================================================================
uint_8 buflist_init(struct list_head* ptdatalist, struct list_head* ptfreelist,TBufNode* pnode, uint_32 dwnum)
{
    uint_32 i;
	
	if(null == ptdatalist || null == ptfreelist || null == pnode || dwnum <= 0)
		return ERR_LIST_INIT;
	
	//�����ʼ��
	ptdatalist->next = ptdatalist;
	ptdatalist->prev = ptdatalist;
	ptfreelist->next = ptfreelist;
	ptfreelist->prev = ptfreelist;

	//������nodeȫ������freelist
	for(i = 0; i < dwnum; i++)
		list_add_tail(&pnode[i].list, ptfreelist);
		
    return ERR_NONE;
}

//============================================================================
//������: listnode_get
//��  ��: ��ȡ����һ���ڵ�  
//��  ��: struct list_head* ptlist������ָ��
//		  TBufNode** ppnode�����ؽڵ�ָ���ָ��
//��  ��: ������
//============================================================================
uint_8 listnode_get(struct list_head* ptlist,TBufNode** ppnode)
{
    struct list_head* plist = null;
	TBufNode* pnode = null;
	
	if(null == ptlist || list_empty(ptlist))
		return ERR_GET_ONE_NODE;
	
	//��ȡ������ڵ�
	plist = ptlist->next;
	list_del(plist);			
	pnode = list_entry(plist, TBufNode, list);
	if(null == pnode)
		return ERR_GET_ONE_NODE;

	*ppnode = pnode;
	
    return ERR_NONE;
}

//============================================================================
//������: listnode_put
//��  ��: �黹����һ���ڵ�  
//��  ��: struct list_head* ptlist������ָ��
//        TBufNode* pnode�����ؽڵ�ָ��
//��  ��: ������
//============================================================================
uint_8 listnode_put(struct list_head* ptlist,TBufNode* pnode)
{	
	if(null == ptlist || null == pnode)
		return ERR_PUT_ONE_NODE;
	
	list_add_tail(&pnode->list, ptlist);

    return ERR_NONE;
}

//============================================================================
//������: listnodenum_get
//��  ��: ��ȡ����ڵ�ĸ��� 
//��  ��: struct list_head* ptlist������ָ��
//        uint_8* pnum�����ؽڵ����
//��  ��: ������
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

