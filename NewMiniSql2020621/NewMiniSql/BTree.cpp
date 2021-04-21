#include "Btree.h"

//std::vector<Column_Head> ShowTable(std::string table_name, std::string path /*= std::string("./")*/)
//{
//	std::string idx_file = path + table_name + ".idx";
//	std::string dbf_file = path + table_name + ".dbf";
//	BTree tree(idx_file);
//	std::vector<Column_Head> vec_record_head;
//
//	auto data_fd = tree.GetPtrIndexHeadNode()->MostLeftNode;
//	while (data_fd.OffSet != 0)
//	{
//		auto pNode = tree.FileAddrToMemPtr(data_fd);
//
//		for (int i = 0; i < pNode->count_valid_key; i++)
//		{
//			auto tmp = GetDbfRecord(table_name, pNode->children[i], path);
//			vec_record_head.push_back(tmp);
//		}
//
//
//		data_fd = pNode->next;
//	}
//
//	return vec_record_head;
//}
//
//Column_Head GetDbfRecord(std::string table_name, FileAddress fd, std::string path /*= std::string("./")*/)
//{
//	std::string idx_file = path + table_name + ".idx";
//	std::string dbf_file = path + table_name + ".dbf";
//	BTree tree(idx_file);
//
//	Column_Head record_head;
//	// ��ȡ����ڴ��ַ
//	char* pRecTypeInfo = tree.GetPtrIndexHeadNode()->RecordTypeInfo;
//	//std::cout << pRecTypeInfo << std::endl;
//	auto pdata = (char*)GetGlobalBuffer().GetFile(dbf_file.c_str())->ReadRecord(&fd);
//	pdata += sizeof(FileAddress);  // ÿ����¼ͷ��Ĭ����Ӹü�¼�ĵ�ֵַ
//
//	auto vec_name_type = GetColumnAndTypeFromTable(table_name, path);
//	int index = 0;
//	while (*pRecTypeInfo != '\0')
//	{
//		Column_Cell cc;
//		switch (*pRecTypeInfo)
//		{
//		case 'I':
//			cc.column_type = Value_Type::Int;
//			cc.column_name = vec_name_type[index].first;
//			cc.column_value.IntValue = *(int*)pdata;
//			pdata += sizeof(int);
//			record_head.AddColumnCell(cc);
//			index++;
//			break;
//
//		case 'D':
//			cc.column_type = Value_Type::Float;
//			cc.column_name = vec_name_type[index].first;
//			cc.column_value.FloatValue = *(double*)pdata;
//			pdata += sizeof(double);
//			record_head.AddColumnCell(cc);
//			index++;
//			break;
//
//		case 'C':
//			cc.column_type = Value_Type::Char;
//			cc.column_name = vec_name_type[index].first;
//			// ��ȡ�ַ�������
//			int sz = 0;
//			sz = (*(pRecTypeInfo + 1) - '0') * 100 + (*(pRecTypeInfo + 2) - '0') * 10 + (*(pRecTypeInfo + 3) - '0');
//			auto pchar = (char*)malloc(sz);
//			memcpy(pchar, pdata, sz);
//			cc.column_value.CharValue = pchar;
//			pdata += sz;
//			record_head.AddColumnCell(cc);
//			index++;
//			break;
//		}
//		pRecTypeInfo++;
//	}
//
//	return record_head;
//}
//
BTree::BTree(string idx_name)
{
	str_idx_name = idx_name;
	if (GetGlobalBuffer()[idx_name.c_str()] == NULL) {
		throw Error("File Not Found!");
	}
	file_id = GetGlobalBuffer()[idx_name.c_str()]->FilePointer;
}
BTree::BTree(const string idx_name, int KeyTypeIndex, char(&_RecordTypeInfo)[RecordColumnCount],
	char(&_RecordColumnName)[RecordColumnCount / 4 * ColumnNameLength])
	:str_idx_name(idx_name) 
{
	auto &buffer = GetGlobalBuffer();
	auto pMemFile = buffer[str_idx_name.c_str()];

	// ��������ļ��������򴴽�
	if (!pMemFile)
	{
		// ���������ļ�
		buffer.CreateFile(str_idx_name.c_str());
		pMemFile = buffer[str_idx_name.c_str()];
		//printf("offset1:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		// ��ʼ�������ļ�������һ�������
		BTNode root_node;
		assert(sizeof(BTNode) < (PageSize - sizeof(PageHead)));
		//printf("offset2:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		root_node.node_type = NodeType::ROOT;
		root_node.count_valid_key = 0;
		root_node.next = FileAddress{ 0,0 };
		//printf("offset3:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		//printf("root_node:%d ", sizeof(root_node));
		FileAddress root_node_fd = buffer[str_idx_name.c_str()]->AddRecord(&root_node, sizeof(root_node));
		//printf("offset4:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		// ��ʼ�����������ļ�ͷ��Ϣ
		idx_head.root = root_node_fd;
		idx_head.MostLeftNode = root_node_fd;
		idx_head.KeyTypeIndex = KeyTypeIndex;

		
		//strcpy(idx_head.RecordTypeInfo, _RecordTypeInfo.c_str());
		memcpy(idx_head.RecordTypeInfo, _RecordTypeInfo, RecordColumnCount);
		//printf("offset5:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		//strcpy(idx_head.RecordColumnName, _RecordColumnName.c_str());
		memcpy(idx_head.RecordColumnName, _RecordColumnName, RecordColumnCount / 4 * ColumnNameLength);

		//printf("offset6:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		// �����ĵ�ַд���ļ�ͷ��Ԥ���ռ���
		memcpy(buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve, &idx_head, sizeof(idx_head));
		//for (int i = 0; i < FILECOND_RESERVE_SPACE; i++) {
		//	printf("%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve[i]);
		//}
	}
	file_id = pMemFile->FilePointer;

}

BTree::BTree(const string idx_name, int KeyTypeIndex, int(&_UniqueKeyIndex)[RecordColumnCount],char(&_RecordTypeInfo)[RecordColumnCount],
	char(&_RecordColumnName)[RecordColumnCount / 4 * ColumnNameLength])//�����ļ������ؼ�����ţ����ͣ�����
	:str_idx_name(idx_name)
{
	auto &buffer = GetGlobalBuffer();
	auto pMemFile = buffer[str_idx_name.c_str()];

	// ��������ļ��������򴴽�
	if (!pMemFile)
	{
		// ���������ļ�
		buffer.CreateFile(str_idx_name.c_str());
		pMemFile = buffer[str_idx_name.c_str()];
		//printf("offset1:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		// ��ʼ�������ļ�������һ�������
		BTNode root_node;
		assert(sizeof(BTNode) < (PageSize - sizeof(PageHead)));
		//printf("offset2:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		root_node.node_type = NodeType::ROOT;
		root_node.count_valid_key = 0;
		root_node.next = FileAddress{ 0,0 };
		//printf("offset3:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		//printf("root_node:%d ", sizeof(root_node));
		FileAddress root_node_fd = buffer[str_idx_name.c_str()]->AddRecord(&root_node, sizeof(root_node));
		//printf("offset4:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		// ��ʼ�����������ļ�ͷ��Ϣ
		idx_head.root = root_node_fd;
		idx_head.MostLeftNode = root_node_fd;
		idx_head.KeyTypeIndex = KeyTypeIndex;

		memcpy(idx_head.UniqueKeyIndex, _UniqueKeyIndex, RecordColumnCount);
		//strcpy(idx_head.RecordTypeInfo, _RecordTypeInfo.c_str());
		memcpy(idx_head.RecordTypeInfo, _RecordTypeInfo, RecordColumnCount);
		//printf("offset5:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		//strcpy(idx_head.RecordColumnName, _RecordColumnName.c_str());
		memcpy(idx_head.RecordColumnName, _RecordColumnName, RecordColumnCount / 4 * ColumnNameLength);

		//printf("offset6:%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->NewInsert.OffSet);
		// �����ĵ�ַд���ļ�ͷ��Ԥ���ռ���
		memcpy(buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve, &idx_head, sizeof(idx_head));
		//for (int i = 0; i < FILECOND_RESERVE_SPACE; i++) {
		//	printf("%02X ", buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve[i]);
		//}
	}
	file_id = pMemFile->FilePointer;
}

FileAddress BTree::Search(Key_Attr search_key)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(file_id, 0);
	auto pfilefd = (FileAddress*)pMemPage->GetFileHead()->Reserve;  // �ҵ������ĵ�ַ
	return Search(search_key, *pfilefd);
}

FileAddress BTree::Search(Key_Attr search_key, FileAddress node_fd)
{
	BTNode* pNode = FileAddrToMemPtr(node_fd);

	if (pNode->node_type == NodeType::LEAF || pNode->node_type == NodeType::ROOT)
	{
		return SearchLeafNode(search_key, node_fd);
	}
	else
	{
		return SearchInnerNode(search_key, node_fd);
	}
}

FileAddress BTree::DeleteKeyAtInnerNode(FileAddress x, int i, Key_Attr key)
{
	auto px = FileAddrToMemPtr(x);
	auto py = FileAddrToMemPtr(px->children[i]);
	FileAddress fd_res;
	if (py->node_type == NodeType::LEAF)
	{
		fd_res = DeleteKeyAtLeafNode(x, i, key);
	}
	else
	{
		int j = py->count_valid_key - 1;
		while (py->key[j] > key)j--;
		//assert(j >= 0);
		fd_res = DeleteKeyAtInnerNode(px->children[i], j, key);
	}

	// �ж�ɾ����Ľ�����
	if (py->count_valid_key >= MaxKeyCount / 2)
		return fd_res;

	// ���ɾ����Ĺؼ��ָ���������B+���Ĺ涨�����ֵܽ�����key

	// ������ֵܴ������и���ؼ���
	if ((i <= px->count_valid_key - 2) && (FileAddrToMemPtr(px->children[i + 1])->count_valid_key > MaxKeyCount / 2))
	{
		auto RBrother = FileAddrToMemPtr(px->children[i + 1]);
		// �����Ĺؼ���
		auto key_bro = RBrother->key[0];
		auto fd_bro = RBrother->children[0];


		// �������ֵܵ��������
		px->key[i + 1] = RBrother->key[1];
		// �������ֵܽ��
		for (int j = 1; j <= RBrother->count_valid_key - 1; j++)
		{
			RBrother->key[j - 1] = RBrother->key[j];
			RBrother->children[j - 1] = RBrother->children[j];
		}
		RBrother->count_valid_key -= 1;

		// ���±�Ҷ�ӽ��
		py->key[py->count_valid_key] = key_bro;
		py->children[py->count_valid_key] = fd_bro;
		py->count_valid_key += 1;

		return fd_res;
	}

	// ������ֵܴ������и���ؼ���
	if (i > 0 && FileAddrToMemPtr(px->children[i - 1])->count_valid_key > MaxKeyCount / 2)
	{
		auto LBrother = FileAddrToMemPtr(px->children[i - 1]);
		// �����Ĺؼ���
		auto key_bro = LBrother->key[LBrother->count_valid_key - 1];
		auto fd_bro = LBrother->children[LBrother->count_valid_key - 1];

		// �������ֵܽ��
		LBrother->count_valid_key -= 1;

		// ���±����
		px->key[i] = key_bro;
		for (int j = py->count_valid_key - 1; j >= 0; j--)
		{
			py->key[j + 1] = py->key[j];
			py->children[j + 1] = py->children[j];
		}
		py->key[0] = key_bro;
		py->children[0] = fd_bro;

		py->count_valid_key += 1;

		return fd_res;
	}

	// ���ֵܽ����û�и����key,��ǰ�����ֵܽ��ϲ���һ���µ�Ҷ�ӽ�㣬��ɾ��������е�key

	// �����ֵܴ��ڽ���ϲ�
	if (i < px->count_valid_key - 1)
	{
		auto RBrother = FileAddrToMemPtr(px->children[i + 1]);
		for (int j = 0; j < RBrother->count_valid_key; j++)
		{
			py->key[py->count_valid_key] = RBrother->key[j];
			py->children[py->count_valid_key] = RBrother->children[j];
			py->count_valid_key++;
		}

		// ����next
		py->next = RBrother->next;
		// ɾ���ҽ��
		GetGlobalBuffer()[str_idx_name.c_str()]->DeleteRecord(&px->children[i + 1], sizeof(BTNode));
		// ���¸��ڵ�����
		for (int j = i + 2; j < px->count_valid_key; j++)
		{
			px->key[j - 1] = px->key[j];
			px->children[j - 1] = px->children[j];
		}
		px->count_valid_key--;
	}
	else
	{// ������ϲ�
		auto LBrother = FileAddrToMemPtr(px->children[i - 1]);
		for (int j = 0; j < py->count_valid_key; j++)
		{
			LBrother->key[LBrother->count_valid_key] = py->key[j];
			LBrother->children[LBrother->count_valid_key] = py->children[j];
			LBrother->count_valid_key++;
		}

		// ����next
		LBrother->next = py->next;

		// ɾ�������
		GetGlobalBuffer()[str_idx_name.c_str()]->DeleteRecord(&px->children[i], sizeof(BTNode));
		// ���¸��ڵ�����
		for (int j = i + 1; j < px->count_valid_key; j++)
		{
			px->key[j - 1] = px->key[j];
			px->children[j - 1] = px->children[j];
		}
		px->count_valid_key--;
	}
	return fd_res;
}

bool BTree::Insert(Key_Attr k, FileAddress k_fd)
{
	// ����ùؼ����Ѿ����������ʧ��
	try
	{
		auto key_fd = Search(k);
		if (key_fd != FileAddress{ 0,0 })
			throw Error("(KEY_INSERT_FAILED) Key Word Insert Failed! The record that to inset has been excisted!");
	    }
	catch (const Error &error)
	{
		DispatchError(error);
		std::cout << std::endl;
		return false;
	}

	// �õ�������fd
	FileAddress root_fd = *(FileAddress*)GetGlobalBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve;
	auto proot = FileAddrToMemPtr(root_fd);
	if (proot->count_valid_key == MaxKeyCount)
	{
		// �����µĽ�� s ,��Ϊ�����
		BTNode s;
		s.node_type = NodeType::INNER;  // ֻ�г�ʼ����ʹ�� NodeType::ROOT
		s.count_valid_key = 1;
		s.key[0] = proot->key[0];
		s.children[0] = root_fd;
		FileAddress s_fd = GetGlobalBuffer()[str_idx_name.c_str()]->AddRecord(&s, sizeof(BTNode));

		// ���µĸ��ڵ��ļ���ַд��
		*(FileAddress*)GetGlobalBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve = s_fd;
		GetGlobalBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->IsModified = true;

		//���ɵĸ��������ΪҶ�ӽ��
		auto pOldRoot = FileAddrToMemPtr(root_fd);
		if (pOldRoot->node_type == NodeType::ROOT)
			pOldRoot->node_type = NodeType::LEAF;

		// �ȷ����ٲ���
		SplitChild(s_fd, 0, s.children[0]);
		InsertNotFull(s_fd, k, k_fd);
	}
	else
	{
		InsertNotFull(root_fd, k, k_fd);
	}
	return true;
}

FileAddress BTree::UpdateKey(Key_Attr k, Key_Attr k_new)
{
	// ��������ɾ���ɵĹؼ���
	auto data_fd = Delete(k);  // ����ؼ��ֶ�Ӧ�ļ�¼��ַ

	//�����µĹؼ���
	Insert(k_new, data_fd);
	return data_fd;
}

FileAddress BTree::Delete(Key_Attr key)
{
	auto search_res = Search(key);
	if (search_res.OffSet == 0)
		return FileAddress{ 0,0 };

	// �õ�������fd
	FileAddress root_fd = *(FileAddress*)GetGlobalBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve;
	auto proot = FileAddrToMemPtr(root_fd);


	// ���ڵ�ΪROOT ���� LEAF ֱ��ɾ��
	if (proot->node_type == NodeType::ROOT || proot->node_type == NodeType::LEAF)
	{
		// ֱ��ɾ��
		int j = proot->count_valid_key - 1;
		while (proot->key[j] != key)j--;
		//if (j < 0) return;
		assert(j >= 0);
		/*assert���ԭ�Ͷ�����<assert.h>�У�����������������������ش�������ֹ����ִ�С�
ԭ�Ͷ��壺
void assert(int expression);*/
		FileAddress fd_res = proot->children[j];
		for (j++; j < proot->count_valid_key; j++)
		{
			proot->key[j - 1] = proot->key[j];
			proot->children[j - 1] = proot->children[j];
		}
		proot->count_valid_key--;
		return fd_res;
	}

	int i = proot->count_valid_key - 1;
	while (proot->key[i] > key)i--;
	//assert(i >= 0);
	//auto px = FileAddrToMemPtr(root_fd);
	//auto py = FileAddrToMemPtr(px->children[i]);

	auto fd_delete = DeleteKeyAtInnerNode(root_fd, i, key);


	if (proot->count_valid_key == 1)
	{
		// ���µĸ��ڵ��ļ���ַд��
		*(FileAddress*)GetGlobalBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve = proot->children[0];
		GetGlobalBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->IsModified = true;

		GetGlobalBuffer()[str_idx_name.c_str()]->DeleteRecord(&root_fd, sizeof(BTNode));
	}

	return fd_delete;
}

void BTree::PrintBTreeStruct()
{
	std::queue<FileAddress> fds;
	//int n = 0;
	// �õ�������fd
	FileAddress root_fd = *(FileAddress*)GetGlobalBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve;
	auto pRoot = FileAddrToMemPtr(root_fd);
	if (pRoot->node_type == NodeType::ROOT || pRoot->node_type == NodeType::LEAF)
	{
		if (pRoot->count_valid_key > 0)
		{
			pRoot->PrintSelf();
		}
		return;
	}

	fds.push(root_fd);
	while (!fds.empty())
	{
		// ��ӡ�ý������еĹؼ���
		FileAddress tmp = fds.front();
		fds.pop();
		auto pNode = FileAddrToMemPtr(tmp);
		std::cout << "Node File Address:" << tmp.FilePageID << " " << tmp.OffSet << std::endl;
		pNode->PrintSelf();
		std::cout << std::endl;

		if (pNode->node_type != NodeType::LEAF)
		{
			for (int i = 0; i < pNode->count_valid_key; i++)
			{
				fds.push(pNode->children[i]);
			}
		}

	}
	//std::cout << "total nodes: " << n << std::endl;
}

void BTree::PrintAllLeafNode()
{
	auto phead = (IndexHeadNode*)GetGlobalBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve;
	auto pRoot = FileAddrToMemPtr(phead->root);
	if (pRoot->count_valid_key <= 0)
	{
		std::cout << "Record is empty" << std::endl;
		return;
	}

	auto pNode = FileAddrToMemPtr(phead->MostLeftNode);


	static int n = 0;
	while (pNode->next.OffSet != 0)
	{
		for (int i = 0; i < pNode->count_valid_key; i++)
		{
			n++;
			std::cout << pNode->key[i];
		}

		pNode = FileAddrToMemPtr(pNode->next);
	}
	for (int i = 0; i < pNode->count_valid_key; i++)
	{
		n++;
		std::cout << pNode->key[i];
	}
	std::cout << std::endl << n << std::endl;
}

IndexHeadNode * BTree::GetPtrIndexHeadNode()
{
	auto phead = (IndexHeadNode*)GetGlobalBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileHead()->Reserve;
	return phead;
}

FileAddress BTree::SearchInnerNode(Key_Attr search_key, FileAddress node_fd)
{
	FileAddress fd_res{ 0,0 };

	BTNode* pNode = FileAddrToMemPtr(node_fd);
	for (int i = pNode->count_valid_key - 1; i >= 0; i--)
	{
		if (pNode->key[i] <= search_key)
		{
			fd_res = pNode->children[i];
			break;
		}
	}

	if (fd_res == FileAddress{ 0,0 })
	{
		return fd_res;
	}
	else
	{
		BTNode* pNextNode = FileAddrToMemPtr(fd_res);
		if (pNextNode->node_type == NodeType::LEAF)
			return SearchLeafNode(search_key, fd_res);
		else
			return SearchInnerNode(search_key, fd_res);
	}
	//return fd_res;
}

FileAddress BTree::SearchLeafNode(Key_Attr search_key, FileAddress node_fd)
{

	BTNode* pNode = FileAddrToMemPtr(node_fd);
	for (int i = 0; i < pNode->count_valid_key; i++)
	{
		if (pNode->key[i] == search_key)
		{
			return pNode->children[i];
		}
	}
	return FileAddress{ 0,0 };
}

BTNode * BTree::FileAddrToMemPtr(FileAddress node_fd)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(file_id, node_fd.FilePageID);
	pMemPage->IsModified = true;
	return (BTNode*)((char*)pMemPage->PtrtoPageBeginAddress + node_fd.OffSet + sizeof(FileAddress));
}

// �����ɾ���Ĺؼ����Ѿ�����
FileAddress BTree::DeleteKeyAtLeafNode(FileAddress x, int i, Key_Attr key)
{
	auto px = FileAddrToMemPtr(x);
	auto py = FileAddrToMemPtr(px->children[i]);
	FileAddress fd_res;
	int j = py->count_valid_key - 1;
	while (py->key[j] != key)j--;
	//assert(j >= 0);
	fd_res = py->children[j];
	// ɾ��Ҷ�ڵ�����С�Ĺؼ��֣����¸��ڵ�
	if (j == 0)
	{
		px->key[i] = py->key[j + 1];
	}

	j++;
	while (j <= py->count_valid_key - 1)
	{
		py->children[j - 1] = py->children[j];
		py->key[j - 1] = py->key[j];
		j++;
	}
	py->count_valid_key -= 1;
	return fd_res;
}


// ��һ��������� x, ����ؼ��� k, k�����ݵ�ַΪ k_fd
void BTree::InsertNotFull(FileAddress x, Key_Attr k, FileAddress k_fd)
{
	auto px = FileAddrToMemPtr(x);
	int i = px->count_valid_key - 1;

	// ����ý����Ҷ�ӽ�㣬ֱ�Ӳ���
	if (px->node_type == NodeType::LEAF || px->node_type == NodeType::ROOT)
	{
		while (i >= 0 && k < px->key[i])
		{
			px->key[i + 1] = px->key[i];
			px->children[i + 1] = px->children[i];
			i--;
		}
		px->key[i + 1] = k;
		px->children[i + 1] = k_fd;
		px->count_valid_key += 1;
	}
	else
	{
		while (i >= 0 && k < px->key[i])  i = i - 1;

		// ��������ֵ���ڽڵ��ֵ��С
		if (i < 0) {
			i = 0;
			px->key[i] = k;
		}
		if (i < 0) return;
		//assert(i >= 0);

		FileAddress ci = px->children[i];
		auto pci = FileAddrToMemPtr(ci);
		if (pci->count_valid_key == MaxKeyCount)
		{
			SplitChild(x, i, ci);
			if (k >= px->key[i + 1]) 
				i += 1;
		}
		InsertNotFull(px->children[i], k, k_fd);
	}
}


// ��x�±�Ϊi�ĺ�����������
void BTree::SplitChild(FileAddress x, int i, FileAddress y)
{
	auto pMemPageX = GetGlobalClock()->GetMemAddr(file_id, x.FilePageID);
	auto pMemPageY = GetGlobalClock()->GetMemAddr(file_id, y.FilePageID);
	pMemPageX->IsModified = true;
	pMemPageY->IsModified = true;

	BTNode*px = FileAddrToMemPtr(x);
	BTNode*py = FileAddrToMemPtr(y);
	BTNode z;         // ���ѳ������½��
	FileAddress z_fd;    // �½����ļ��ڵ�ַ

	z.node_type = py->node_type;
	z.count_valid_key = MaxKeyCount / 2;

	// ��y����һ������ת�Ƶ��½��
	for (int k = MaxKeyCount / 2; k < MaxKeyCount; k++)
	{
		z.key[k - MaxKeyCount / 2] = py->key[k];
		z.children[k - MaxKeyCount / 2] = py->children[k];
	}
	py->count_valid_key = MaxKeyCount / 2;

	// ��y�ĸ��ڵ�x������´������ӽ�� z
	int j;
	for (j = px->count_valid_key - 1; j > i; j--)
	{
		px->key[j + 1] = px->key[j];
		px->children[j + 1] = px->children[j];
	}

	j++; // after j++, j should be i+1;
	px->key[j] = z.key[0];

	if (py->node_type == NodeType::LEAF)
	{
		z.next = py->next;
		z_fd = GetGlobalBuffer()[str_idx_name.c_str()]->AddRecord(&z, sizeof(z));
		py->next = z_fd;
	}
	else
		z_fd = GetGlobalBuffer()[str_idx_name.c_str()]->AddRecord(&z, sizeof(z));

	px->children[j] = z_fd;
	px->count_valid_key++;
}

void BTNode::PrintSelf()
{
	using std::cout;
	using std::endl;
	std::cout << "Node Type: ";
	switch (node_type)
	{
	case NodeType::ROOT:
		cout << "ROOT";
		break;
	case NodeType::INNER:
		cout << "INNER";
		break;
	case NodeType::LEAF:
		cout << "LEAF";
		break;
	default:
		break;
	}
	cout << "\tcount_valid_key: " << count_valid_key << endl;

	for (int i = 0; i < count_valid_key; i++)
	{
		cout << "index: " << i << " key: " << key[i] << "\t" << "child addr: " << children[i].FilePageID << " " << children[i].OffSet << endl;
	}

}