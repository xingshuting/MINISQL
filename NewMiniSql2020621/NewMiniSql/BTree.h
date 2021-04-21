#pragma once
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  2 0 2 0 D B S - G R O U P W O R K             ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : MiniSql		                                               *
 *                                                                                             *
 *                    File Name : Timer.H                                                      *
 *                                                                                             *
 *                   Programmer : Shuting Xing                                                 *
 *                                                                                             *
 *                   Start Date : 06/08/2020                                                   *
 *                                                                                             *
 *                  Last Update : 06/21/2020			                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef __BPTREE_H__
#define __BPTREE_H__
#include "BufferManager.h"
#include "Record.h"
#include"Error.h"
#include <queue>
#include <assert.h>

class IndexHeadNode
{
public:
	FileAddress    root;                                       // the address of the root
	FileAddress    MostLeftNode;                               // the address of the most left node
	int         KeyTypeIndex;                               // �ؼ����ֶε�λ��  ���ڼ���
	char        RecordTypeInfo[RecordColumnCount];          // ��¼�ֶ�������Ϣ
	char        RecordColumnName[RecordColumnCount / 4 * ColumnNameLength];
	int			UniqueKeyIndex[RecordColumnCount];							//��¼Uniqueλ��

	char        RecordIndexName[RecordColumnCount / 4 * ColumnNameLength]; //��¼������
	int			RecordIndexInfo[RecordColumnCount];					//��¼����λ��
};

// define B+tree Node
enum class NodeType { ROOT, INNER, LEAF };
class BTNode
{
public:
	NodeType node_type;                              // �ڵ�����
	int count_valid_key;                             // �ýڵ��д���Ĺؼ�����Ŀ

	Key_Attr key[MaxKeyCount];                        // array of keys
	FileAddress children[MaxChildCount];             // if the node is not a leaf node, children store the children pointer
													 // otherwise it store record address;

	FileAddress next;                                 // if leaf node
	void PrintSelf();
};

class BTree
{
	//friend vector<Column_Head> ShowTable(string table_name, string path);
	//friend Column_Head GetDbfRecord(std::string table_name, FileAddress fd, string path);
public:
	// �����������ļ����ƣ� �ؼ������ͣ� ��¼����������Ϣ���飬 ��¼�����ֶ�������Ϣ����
	BTree(string idx_name);
	BTree(const string idx_name, int KeyTypeIndex, int(&_UniqueKeyIndex)[RecordColumnCount], char(&_RecordTypeInfo)[RecordColumnCount], char(&_RecordColumnName)[RecordColumnCount / 4 * ColumnNameLength]);          // ���������ļ���B+��
	BTree(const string idx_name, int KeyTypeIndex, char(&_RecordTypeInfo)[RecordColumnCount], char(&_RecordColumnName)[RecordColumnCount / 4 * ColumnNameLength]);          // ���������ļ���B+��
	~BTree() { }
	FileAddress Search(Key_Attr search_key);                                        // ���ҹؼ����Ƿ��Ѿ�����
	bool Insert(Key_Attr k, FileAddress k_fd);                                      // ����ؼ���k
	FileAddress UpdateKey(Key_Attr k, Key_Attr k_new);                               // ���عؼ��ֶ�Ӧ�ļ�¼��ַ
	FileAddress Delete(Key_Attr k);                                                 // ���ظùؼ��ּ�¼�������ļ��еĵ�ַ
	void PrintBTreeStruct();                                                    // �����ӡ���н����Ϣ
	void PrintAllLeafNode();
	IndexHeadNode *GetPtrIndexHeadNode();
	BTNode *FileAddrToMemPtr(FileAddress node_fd);                                 // �ļ���ַת��Ϊ�ڴ�ָ��
	//void InsertIndexName();

private:
	FileAddress DeleteKeyAtInnerNode(FileAddress x, int i, Key_Attr key);              // x���±�Ϊi�Ľ��ΪҶ�ӽ��
	FileAddress DeleteKeyAtLeafNode(FileAddress x, int i, Key_Attr key);               // x���±�Ϊi�Ľ��ΪҶ�ӽ��
	void InsertNotFull(FileAddress x, Key_Attr k, FileAddress k_fd);					// ��һ��������� x, ����ؼ��� k, k�����ݵ�ַΪ k_fd
	void SplitChild(FileAddress x, int i, FileAddress y);                             // ����x�ĺ��ӽ��x.children[i] , y
	FileAddress Search(Key_Attr search_key, FileAddress node_fd);                      // �жϹؼ����Ƿ����
	FileAddress SearchInnerNode(Key_Attr search_key, FileAddress node_fd);             // ���ڲ��ڵ����
	FileAddress SearchLeafNode(Key_Attr search_key, FileAddress node_fd);              // ��Ҷ�ӽ�����


private:
	FILE* file_id;
	string str_idx_name;
	IndexHeadNode idx_head;
};


#endif