//============================================================================
//�ļ����ƣ�buflist.h
//���ܸ�Ҫ��ʵ��������Ľӿ�ͷ�ļ�
//��Ȩ���У����ݴ�ѧ��˼����Ƕ��ʽ����(sumcu.suda.edu.cn)
//�汾���£�2014-01-24	V2.0
//============================================================================

#ifndef __BUF_LIST_H__
#define __BUF_LIST_H__

#ifdef __cplusplus 
extern "C" { 
#endif // __cplusplus

// ͷ�ļ�
#include "list.h"
#include "common.h"

// ����ֵ�궨��
#define ERR_NONE               0
#define ERR_LIST_INIT          1 
#define ERR_GET_ONE_NODE       2 
#define ERR_PUT_ONE_NODE       3 

//// �ṹ����
//typedef struct
//{
//    uint_8*     pbuf;
//    uint_16     size;	 //buf�ռ��С����list�û�ȷ��
//	uint_8      len;      //ʵ�����ݳ��ȣ�����ʱȷ��
//	uint_8      type;     //node���ͣ���Ӧ�ò���ȷ
//    struct  list_head list;
//}TBufNode;

// �ṹ����
//#pragma pack(1)
typedef struct
{
	uint_8   CHNum;      //ͨ���ţ���1������0ͨ������
	uint_8   CHSeleted;  //ͨ���Ƿ�ѡ��
	uint_8   *pbuf; //��ͨ�����͵�������
    uint_32  size;	     //buf�ռ��С����list�û�ȷ��
	uint_32  len;      //ͨ������ֽ���
	uint_8   type;       //node���ͣ���Ӧ�ò���ȷ
    struct  list_head list;
}TBufNode;

//#pragma pack()

//============================================================================
//������: buflist_init
//��  ��: �����ʼ��  
//��  ��: struct list_head* ptdatalist����������ָ��
//	      struct list_head* ptfreelist��������ָ��
//		  TBufNode* pnode��buf�ڵ������׵�ַ
//		  uint_32 dwnum��buf�ڵ��������
//��  ��: ������
//============================================================================
uint_8 buflist_init(struct list_head* ptdatalist, struct list_head* ptfreelist,TBufNode* pnode, uint_32 dwnum);

//============================================================================
//������: listnode_get
//��  ��: ��ȡ����һ���ڵ�  
//��  ��: struct list_head* ptlist������ָ��
//		  TBufNode** ppnode�����ؽڵ�ָ���ָ��
//��  ��: ������
//============================================================================
uint_8 listnode_get(struct list_head* ptlist,TBufNode** ppnode);

//============================================================================
//������: listnode_put
//��  ��: �黹����һ���ڵ�  
//��  ��: struct list_head* ptlist������ָ��
//        TBufNode* pnode�����ؽڵ�ָ��
//��  ��: ������
//============================================================================
uint_8 listnode_put(struct list_head* ptlist,TBufNode* pnode);

//============================================================================
//������: listnodenum_get
//��  ��: ��ȡ����ڵ�ĸ��� 
//��  ��: struct list_head* ptlist������ָ��
//        uint_8* pnum�����ؽڵ����
//��  ��: ������
//============================================================================
uint_8 listnodenum_get(struct list_head* ptlist, uint_8* pnum);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __BUF_LIST_H__
