/*
�ȴ��޸���
Update ��������� ����Unique���
Update ��������� �����������ͼ��
*/

#include "API.h"
#include <algorithm>
#include <iterator>
#pragma warning(disable:4996)
//�������ݿ�
bool Create_DB(std::string db_name, CatalogPosition& cp)
{
	std::string path = cp.GetRootPath() + db_name;
	//�ж����ݿ��Ƿ����
	if (_access(path.c_str(), 0) == -1)
	{
		_mkdir(path.c_str());
		return true;
	}
	else {
		std::cout << "Database already exists!" << std::endl;
		return false;
	}
}
//
//ѡ�����ݿ�
bool Use_DB(std::string db_name, CatalogPosition& cp)
{
	std::string path = cp.GetRootPath() + db_name;
	//�ж����ݿ��Ƿ����
	if (_access(path.c_str(), 0) == -1)
	{
		std::cout << "Database doesn't exist!" << std::endl;
		return false;
	}
	else {
		cp.SetCurrentPath(path + "/");//ת����������ݿ���Ŀ¼��
		cp.isInSpeDb = true;
		return true;
	}
}

//��ʾ��ǰ�������ݿ�
std::vector<std::string>Show_DB(CatalogPosition& cp)
{
	std::vector<std::string> db_names;
	_finddata_t File;//�ļ���ṹ
	intptr_t handle;
	std::string root = cp.GetRootPath() + "*.*";//�ļ���Ŀ¼�����ͨ���
	handle = _findfirst(root.c_str(), &File);    // ����Ŀ¼�еĵ�һ���ļ�
	if (handle == -1)
	{
		std::cout << "Failed to find first file!\n";
		return db_names;
	}
	do
	{
		if (File.attrib & _A_SUBDIR
			&& strcmp(File.name, ".") != 0
			&& strcmp(File.name, "..") != 0
			)    // �Ƿ�����Ŀ¼���Ҳ�Ϊ"."��".." ���������ͨ�ļ��о����
			db_names.push_back(File.name);
	} while (_findnext(handle, &File) == 0);    // ����Ŀ¼�е���һ���ļ�
	_findclose(handle);    // �ر��������
	return db_names;
}

//ɾ�����ݿ�
bool Drop_DB(std::string db_name, CatalogPosition& cp)
{
	std::string path = cp.GetRootPath() + db_name;
	//�ж����ݿ��Ƿ����
	if (_access(path.c_str(), 0) == -1)
	{
		std::cout << "Database Doesn't exist!" << std::endl;
		return false;
	}
	else {
		std::string tmp_path = path + "/";
		//Del_File(tmp_path);
		_rmdir(path.c_str());
	}

}

void Del_File(std::string folder_path)
{
	_finddata_t File;//�ļ���ṹ
	intptr_t handle;
	std::string root = folder_path + "*";     //�ļ�Ŀ¼�����ͨ���
	handle = _findfirst(root.c_str(), &File);    // ����Ŀ¼�еĵ�һ���ļ�
	if (handle == -1)
	{
		std::cout << "Failed to match folder path!" << std::endl;
		return;
	}
	else
	{
		do {
			//��ȵݹ�ɾ��
			if (File.attrib & _A_SUBDIR)//�Ƿ�����Ŀ¼
			{
				if ((strcmp(File.name, ".") != 0) && (strcmp(File.name, "..") != 0))
				{
					std::string new_path = folder_path + File.name;
					new_path += "/";
					Del_File(new_path);
					// ɾ�����ļ���
					_rmdir(new_path.c_str());
				}
			}
			else
			{
			    bool tmp = GetCp().IsInSpeDb();
				GetCp().SetIsInSpeDb(true);
				std::string name = folder_path + File.name;
				GetGlobalBuffer().CloseFile(name.c_str());
				remove(name.c_str());
				GetCp().SetIsInSpeDb(tmp);
			}
		} while (_findnext(handle, &File) == 0);

	}
	_findclose(handle);
}
/***********************************************************************************************
 ***                                   �������غ���                                        ***
 ***********************************************************************************************
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
																					   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/***********************************************************************************************
//������ ***************************************************************************************/
bool Create_Table(TB_Create_Info tb_create_info, std::string path)
{
	Check_TB_Create_Info(tb_create_info);//��������Ϣ�ĺϷ��Լ��
	if (GetCp().IsInSpeDb() == false)
	{
		std::cout << "Unselected database!" << std::endl;
		return false;
	}

	std::string table_name = tb_create_info.table_name;
	std::string idx_file = path + table_name + ".index";
	std::string dbs_file = path + table_name + ".dbs";

	int primary_key = 0; //Ĭ��-����Ϊ0���ֶ�
	for (int i = 0; i < tb_create_info.columns_info.size(); i++)
	{
		if (tb_create_info.columns_info[i].isPrimary)
		{
			primary_key = i;
			break;
		}
	}

	
	//��¼�ֶ���Ϣ
	char column_type[RecordColumnCount];
	char *type_ptr = column_type;
	char column_name[RecordColumnCount / 4 * ColumnNameLength];
	char* name_ptr = column_name;
	int unique_key[RecordColumnCount];
	int *unique_ptr = unique_key;

	std::vector<ColumnInfo>& CF = tb_create_info.columns_info;
	if (CF.size() > 32)
		throw Error("Atrribute is higher than 32");
	for (int i = 0; i < CF.size(); i++)
	{
		switch (CF[i].type)
		{
		case Column_Type::I:
			*type_ptr++ = 'I';
			break;
		case Column_Type::F:
			*type_ptr++ = 'F';
			break;
		case Column_Type::C:
			CF[i].RequiredLength++;
			*type_ptr++ = 'C';
			*type_ptr++ = CF[i].RequiredLength / 100 + '0';
			*type_ptr++ = (CF[i].RequiredLength % 100) / 10 + '0';
			*type_ptr++ = CF[i].RequiredLength % 10 + '0';
		default:
			break;
		}
		if (CF[i].isUnique) {
			*unique_ptr++ = i;
		}
		strcpy(name_ptr, CF[i].name.c_str());//д������
		name_ptr += ColumnNameLength;
	}
	*type_ptr = '\0';
	BTree tree(idx_file,primary_key, unique_key, column_type, column_name);//��������
	GetGlobalBuffer().CreateFile(dbs_file.c_str());//��������
	return true;
}

void Check_TB_Create_Info(const TB_Create_Info &tb_create_info)
{
	// ����
	std::string table_name = tb_create_info.table_name;
	std::string idx_file = GetCp().GetCurrentPath() + table_name + ".index";
	// �жϱ��Ƿ��Ѿ�����
	if (_access(idx_file.c_str(), 0) != -1) {    //�����
		throw Error("The table already exists!");
	}

	// ���ÿ���ֶε���Ϣ
	for (int i = 0; i < tb_create_info.columns_info.size(); i++)
	{
		auto &column = tb_create_info.columns_info[i];

		// ����ֶ����Ƴ���
		if (column.name.size() >= ColumnNameLength)
			throw Error("Column name length overflow");

		// ����ֶ�����
		if (column.type != Column_Type::C && column.type != Column_Type::F && column.type != Column_Type::I)
			throw Error("Column data type error!");
	}

	// ����Ƿ����ؼ���
	int primary_count = 0;
	for (auto &e : tb_create_info.columns_info)
		if (e.isPrimary) primary_count++;

	if (primary_count > 1)
		throw Error("More than one primary key!");


	// ����ֶθ���
	if (tb_create_info.columns_info.size() > RecordColumnCount)
		throw Error("Column count is overflow!");
}

/***********************************************************************************************
//ɾ���� ***************************************************************************************/
bool Drop_Table(std::string table_name, std::string path)
{
	std::string tmp_path = path + table_name;
	std::string idx = tmp_path + ".index";
	std::string dbs = tmp_path + ".dbs";
	Buffer &buffer = GetGlobalBuffer();
	Clock* Clock_ptr = GetGlobalClock();
	if (GetCp().IsInSpeDb() == false)
	{
		std::cout << "Unselected database!" << std::endl;
		return false;
	}
	if (_access(idx.c_str(), 0) == -1 || _access(dbs.c_str(), 0) == -1)
	{
		std::cout << "Table doesn't exist!" << std::endl;
		return false;
	}
	else
	{
		// ����ļ��Ѿ�����ʹ�� ��Ҫ�ȶ������ڴ��е��ļ�ҳ �����������ٴ�д��
		File *IdxMF_ptr = buffer.GetFile(idx.c_str());
		if (IdxMF_ptr)
		{
			for (int i = 1; i < MemPageCount; i++)
			{
				if (Clock_ptr->MemPages[i] && Clock_ptr->MemPages[i]->FilePointer == IdxMF_ptr->FilePointer)
				{
					Clock_ptr->MemPages[i]->FilePointer = NULL;
					Clock_ptr->MemPages[i]->IsModified = false;
				}
			}
		}
		File *DbfMF_ptr = buffer.GetFile(dbs.c_str());
		if (DbfMF_ptr)
		{
			for (int i = 1; i <= MemPageCount; i++)
			{
				if (Clock_ptr->MemPages[i] && Clock_ptr->MemPages[i]->FilePointer == DbfMF_ptr->FilePointer)
				{
					Clock_ptr->MemPages[i]->FilePointer = NULL;  // ������ҳ
					Clock_ptr->MemPages[i]->IsModified = false;
				}

			}
		}
		// ɾ�����ļ�
		fclose(IdxMF_ptr->FilePointer);
		fclose(DbfMF_ptr->FilePointer);
		buffer.CloseFile(idx.c_str());

		buffer.CloseFile(dbs.c_str());
		remove(idx.c_str());
		remove(dbs.c_str());
		return true;
	}
}

/***********************************************************************************************
//�����¼ ***************************************************************************************/
bool Insert_Record(TB_Insert_Info tb_insert_info, std::string path)   //tb_insert_info �ṹ���£�
																	/*std::string table_name;
																	  std::vector<InsertInfo> insert_info;*/ 
																		/*typedef struct InsertInfo {
																		std::string column_name; //������ֶ�
																		std::string column_value;//�����ֵ
																	}InsertInfo;*/
{

	std::string idx_file = path + tb_insert_info.table_name + ".index";
	std::string dbs_file = path + tb_insert_info.table_name + ".dbs";
	BTree tree(idx_file);      //��idx_file ��ʼ������ һ��B+��
	TableIndexHeadInfo table_index_head_info(tree);   //������ʼ������һ���ṹ�壬���Ա��ֻ��һ��BTree,��Ϊ�˷��㺯����������
	IndexHeadNode* pHead = tree.GetPtrIndexHeadNode();    //��������ļ��е�Index�ڵ�ͷ
	Column_Head record_head;//����¼��Ϣ��װ�ɼ�¼���ݶ��� 

	if (tb_insert_info.IsOrder == true) {
		int column_id = 0;
		for (int i = 0; pHead->RecordTypeInfo[i] != '\0'; i++)
		{
			if (pHead->RecordTypeInfo[i] == 'I')
			{
				Column_Cell cell;
				char* pColumnName = pHead->RecordColumnName + column_id * ColumnNameLength; //�ҵ���Ӧ�ֶ�����
				int flag = -1;
				for (int j = 0; j < tb_insert_info.insert_info.size(); j++)
				{
					if (pColumnName == tb_insert_info.insert_info[j].column_name)
					{

						flag = j;
						break;
					}
				}
				if (flag != -1)//���յ��û�����        
				{
					Check_Type_I(tb_insert_info.insert_info[flag].column_value);
					
					if (table_index_head_info.IsUnique(tb_insert_info.insert_info[flag].column_name)) {
						Check_Unique_Info(tb_insert_info.table_name, tb_insert_info.insert_info[flag].column_name, tb_insert_info.insert_info[flag].column_value, 'I', column_id, path);
						  // Unique���
					}

					cell.column_type = Value_Type::I;
					cell.column_value.IntValue = stoi(tb_insert_info.insert_info[flag].column_value);//�ַ���תΪint
				}
				else
				{
					cell.column_type = Value_Type::I;
					cell.column_value.IntValue = 0;//Ϊ���룬����ΪĬ��ֵ
				}
				column_id++;
				record_head.attachCell(cell);
			}

			if (pHead->RecordTypeInfo[i] == 'F')
			{
				Column_Cell cell;
				char* pColumnName = pHead->RecordColumnName + column_id * ColumnNameLength;//�ҵ���Ӧ�ֶ�����
				int flag = -1;
				for (int j = 0; j < tb_insert_info.insert_info.size(); j++)
				{
					if (pColumnName == tb_insert_info.insert_info[j].column_name)
					{
						flag = j;
						break;
					}
				}
				if (flag != -1)//���յ��û�����
				{	
					Check_Type_F(tb_insert_info.insert_info[flag].column_value);
					
					if (table_index_head_info.IsUnique(tb_insert_info.insert_info[flag].column_name)) {
						Check_Unique_Info(tb_insert_info.table_name, tb_insert_info.insert_info[flag].column_name, tb_insert_info.insert_info[flag].column_value, 'F', column_id, path);
						 // Unique���
					}
					cell.column_type = Value_Type::F;
			
					cell.column_value.FloatValue = stod(tb_insert_info.insert_info[flag].column_value);
				}
				else
				{
					cell.column_type = Value_Type::F;
					cell.column_value.FloatValue = 0;//Ϊ���룬����ΪĬ��ֵ
				}
				column_id++;
				record_head.attachCell(cell);
			}

			if (pHead->RecordTypeInfo[i] == 'C')
			{
				Column_Cell cell;
				char* pColumnName = pHead->RecordColumnName + column_id * ColumnNameLength;//�ҵ���Ӧ�ֶ�����
				int flag = -1;
				for (int j = 0; j < tb_insert_info.insert_info.size(); j++)
				{
					if (pColumnName == tb_insert_info.insert_info[j].column_name)
					{
						flag = j;
						break;
					}
				}
				if (flag != -1)//���յ��û�����
				{

					if (table_index_head_info.IsUnique(tb_insert_info.insert_info[flag].column_name)) {
						Check_Unique_Info(tb_insert_info.table_name, tb_insert_info.insert_info[flag].column_name, tb_insert_info.insert_info[flag].column_value, 'C', column_id, path);
						  // Unique���
					}

					cell.column_type = Value_Type::C;
					cell.sz = table_index_head_info.GetColumnSizeByIndex(column_id);//szΪ�����ַ������ȵı���
					char* pChar = (char*)malloc(cell.sz);
					strcpy(pChar, tb_insert_info.insert_info[flag].column_value.c_str());
					cell.column_value.CharValue = pChar;
				}
				else
				{
					cell.column_type = Value_Type::C;
					cell.sz = table_index_head_info.GetColumnSizeByIndex(column_id);
					char* pChar = (char*)malloc(cell.sz);
					memset(pChar, 0, cell.sz);
					cell.column_value.CharValue = pChar;
				}
				column_id++;
				record_head.attachCell(cell);
			}


		}
	}
	else{
		int column_id = 0;
		for (int i = 0; pHead->RecordTypeInfo[i] != '\0'; i++)
		{
			if (pHead->RecordTypeInfo[i] == 'I')
			{
				Column_Cell cell;
				char* pColumnName = pHead->RecordColumnName + column_id * ColumnNameLength; //�ҵ���Ӧ�ֶ�����
				tb_insert_info.insert_info[column_id].column_name = pColumnName;	
				Check_Type_I(tb_insert_info.insert_info[column_id].column_value);
				if (tb_insert_info.insert_info[column_id].column_value != "") {
					if (table_index_head_info.IsUnique(pColumnName)) {
						Check_Unique_Info(tb_insert_info.table_name, tb_insert_info.insert_info[column_id].column_name, tb_insert_info.insert_info[column_id].column_value, 'I', column_id, path);
						 // Unique���
					}

					cell.column_type = Value_Type::I;
				
					cell.column_value.IntValue = stoi(tb_insert_info.insert_info[column_id].column_value);//�ַ���תΪint
				}
				else
				{
					cell.column_type = Value_Type::I;
					cell.column_value.IntValue = 0;//Ϊ���룬����ΪĬ��ֵ
				}
				column_id++;
				record_head.attachCell(cell);
			}
			if (pHead->RecordTypeInfo[i] == 'F')
			{
				Column_Cell cell;
				char* pColumnName = pHead->RecordColumnName + column_id * ColumnNameLength;//�ҵ���Ӧ�ֶ�����
				tb_insert_info.insert_info[column_id].column_name = pColumnName;	
				Check_Type_F(tb_insert_info.insert_info[column_id].column_value);
				
				if (tb_insert_info.insert_info[column_id].column_value != "") {
					if (table_index_head_info.IsUnique(pColumnName)) {
						Check_Unique_Info(tb_insert_info.table_name, tb_insert_info.insert_info[column_id].column_name, tb_insert_info.insert_info[column_id].column_value, 'F', column_id, path);
						  // Unique���
					}

					cell.column_type = Value_Type::F;
				
					cell.column_value.FloatValue = stod(tb_insert_info.insert_info[column_id].column_value);//�ַ���תΪint
				}
				else
				{
					cell.column_type = Value_Type::F;
					cell.column_value.FloatValue = 0;//Ϊ���룬����ΪĬ��ֵ
				}
				column_id++;
				record_head.attachCell(cell);
			}

			if (pHead->RecordTypeInfo[i] == 'C')
			{
				Column_Cell cell;
				char* pColumnName = pHead->RecordColumnName + column_id * ColumnNameLength;//�ҵ���Ӧ�ֶ�����
				tb_insert_info.insert_info[column_id].column_name = pColumnName;
				if (tb_insert_info.insert_info[column_id].column_value != "") {

					if (table_index_head_info.IsUnique(pColumnName)) {
						Check_Unique_Info(tb_insert_info.table_name, tb_insert_info.insert_info[column_id].column_name, tb_insert_info.insert_info[column_id].column_value, 'C', column_id, path);
						// Unique���
					}

					cell.column_type = Value_Type::C;
					cell.sz = table_index_head_info.GetColumnSizeByIndex(column_id);//szΪ�����ַ������ȵı���
					char* pChar = (char*)malloc(cell.sz);
					strcpy(pChar, tb_insert_info.insert_info[column_id].column_value.c_str());
					cell.column_value.CharValue = pChar;
				}
				else
				{
					cell.column_type = Value_Type::C;
					cell.sz = table_index_head_info.GetColumnSizeByIndex(column_id);
					char* pChar = (char*)malloc(cell.sz);
					memset(pChar, 0, cell.sz);
					cell.column_value.CharValue = pChar;
				}
				column_id++;
				record_head.attachCell(cell);
			}


		}
	}
	
	Key_Attr tmp_key;
	int tmp_key_index = 0;
	Column_Cell* pCell = record_head.firstCell();
	while (tmp_key_index != pHead->KeyTypeIndex)
	{
		pCell = pCell->next;
		tmp_key_index++;
	}
	tmp_key = *pCell;
	if (tree.Search(tmp_key) != FileAddress{0,0 })
	{
		std::cout << "primary key repeat" << std::endl;
		return false;
	}
	Check_TB_Insert_Info(tb_insert_info);  // ������

	//��������
	Record record;
	FileAddress db_file = record.InsertRecord(dbs_file, record_head);

	//������������
	int index = 0;
	pCell = record_head.firstCell();
	while (index != pHead->KeyTypeIndex)
	{
		pCell = pCell->next;
		index++;
	}
	Key_Attr key = *pCell;
	tree.Insert(key, db_file);

	//������������

	pCell = record_head.firstCell();
	int index2 = 0;
	std::vector<int> index_info = table_index_head_info.GetIndexInfo();

	for (int i = 0; i < index_info.size(); i++) {
		char* pIndexName = pHead->RecordIndexName + i * ColumnNameLength;//�ҵ�i��������
		std::string index_name = pIndexName;
		//if (index_name == "") {
		//	continue;
		//}
		std::string idx_file = path + index_name + ".index";
		BTree tree2(idx_file);//�ҵ�����

		//int index = 0;
		while (index2 != index_info[i])//�ҵ�����
		{
			pCell = pCell->next;
			index2++;
		}
		Key_Attr key = *pCell;
		tree2.Insert(key, db_file);
	}
	
	return true;
}
void Check_Unique_Info(std::string table_name, std::string column_name, std::string column_value, char KeyType, int column_id,  std::string path) {
	std::string idx_file = path + table_name + ".index";
	BTree tree(idx_file);
	TableIndexHeadInfo table_index_head_info(tree);
	IndexHeadNode* pHead = tree.GetPtrIndexHeadNode();

	SelectPrintInfo select_print_info;
	TB_Select_Info tb_select_info;
	tb_select_info.table_name =table_name;
	tb_select_info.name_select_column.push_back(column_name);

	if (table_index_head_info.hasIndex(column_name)) {
		int ff = table_index_head_info.GetIndex2(column_name);
		char* pIndexName = pHead->RecordIndexName + ff * ColumnNameLength;//�ҵ�i��������
		std::string index_name = pIndexName;
		std::string idx_file = path + index_name + ".index";
		BTree tree2(idx_file);//�ҵ�����
		Key_Attr tmp_key;
		int tmp_key_index = column_id;
		switch (KeyType) {
		case'C': {
			tmp_key.type = Value_Type::C;
			int sz = table_index_head_info.GetColumnSizeByIndex(column_id);//szΪ�����ַ������ȵı���
			char* pChar = (char*)malloc(sz);
			strcpy(pChar, column_value.c_str());
			strcpy(tmp_key.value.CharValue, pChar);
			break;
		}
		case'I': {
			tmp_key.type = Value_Type::I;
			tmp_key.value.IntValue = stoi(column_value);
			break;
		}
		case'F': {
			tmp_key.type = Value_Type::F;
			tmp_key.value.FloatValue = stod(column_value);
			break;
		}
		}
		if (tree2.Search(tmp_key) != FileAddress{ 0,0 })
		{
			throw Error("Unique repeat");
		}
	}
	else {
		select_print_info = Select_Record(tb_select_info, path);

		std::vector<std::string> out_name = select_print_info.name_selected_column;

		if (KeyType == 'I') {
			for (int i = 0; i < select_print_info.key_fd.size(); i++) {
				Column_Head record_head = Get_DB_Record(select_print_info.table_name, select_print_info.key_fd[i].second, GetCp().GetCurrentPath());
				auto pColumn = record_head.firstCell();
				while (pColumn) {
					auto it = find(out_name.begin(), out_name.end(), pColumn->column_name);
					if (it != out_name.end()) {
						if (pColumn->column_value.IntValue == stoi(column_value))
							throw Error("insert value is not unique");
					}
					pColumn = pColumn->next;
				}
			}
		}
		if (KeyType == 'F') {
			for (int i = 0; i < select_print_info.key_fd.size(); i++) {
				Column_Head record_head = Get_DB_Record(select_print_info.table_name, select_print_info.key_fd[i].second, GetCp().GetCurrentPath());
				auto pColumn = record_head.firstCell();
				while (pColumn) {
					auto it = find(out_name.begin(), out_name.end(), pColumn->column_name);
					if (it != out_name.end()) {
						if (pColumn->column_value.FloatValue == stod(column_value))
							throw Error("insert value is not unique");
					}
					pColumn = pColumn->next;
				}

			}
		}
		if (KeyType == 'C') {
			for (int i = 0; i < select_print_info.key_fd.size(); i++) {
				size_t sz = table_index_head_info.GetColumnSizeByIndex(column_id);//szΪ�����ַ������ȵı���
				char* pChar = (char*)malloc(sz);
				strcpy(pChar,column_value.c_str());

				Column_Head record_head = Get_DB_Record(select_print_info.table_name, select_print_info.key_fd[i].second, GetCp().GetCurrentPath());
				auto pColumn = record_head.firstCell();
				while (pColumn) {
					auto it = find(out_name.begin(), out_name.end(), pColumn->column_name);
					if (it != out_name.end()) {
						if (strcmp(pColumn->column_value.CharValue, pChar) == 0)
							throw Error("insert value is not unique");
					}
					pColumn = pColumn->next;
				}
			}
		}
	}
}

void Check_Type_I(std::string value) {
	for (int k = 0; k < value.size(); k++) {
		if ((value[k] > '9' ||value[k] < '0') && value[k] != '-') {
			throw Error("You have enter a String or Float to an Int Field");
		}
	}
}
void Check_Type_F(std::string value) {
	for (int k = 0; k < value.size(); k++) {
		if ((value[k] > '9' || value[k] < '0') && value[k] != '.' && value[k]!='-') {
			throw Error("You have enter a string to an Float Field");
		}
	}
}
void Check_TB_Insert_Info(const TB_Insert_Info &tb_insert_info)
{
	std::string idx_file = GetCp().GetCurrentPath() + tb_insert_info.table_name + ".index";

	// ������ھ������ݿ�Ŀ¼�£����ܲ����¼
	if (!GetCp().IsInSpeDb())
		throw Error("Error!Not use database!");

	// �жϱ��Ƿ��Ѿ�����
	if (_access(idx_file.c_str(), 0) == -1) {  //������
		throw Error("The table is not exists!");
	}

	BTree tree(idx_file);
	TableIndexHeadInfo table_index_head_info(tree);


	// �����������ֶε���Ϣ
	for (int i = 0; i < tb_insert_info.insert_info.size(); i++)
	{
		auto &cur_columu = tb_insert_info.insert_info[i];
			// �ж��ֶ��Ƿ�Ϸ�
			if (!table_index_head_info.IsColumnName(cur_columu.column_name))
				throw Error("Fields do not exist!");
		
		// ����ֶδ�С
		int index = table_index_head_info.GetIndex(cur_columu.column_name);
		Column_Type column_type = table_index_head_info.GetColumnType(cur_columu.column_name);
		int column_size = table_index_head_info.GetColumnSizeByIndex(index);
		if (column_type == Column_Type::C && cur_columu.column_value.size() > column_size)
			throw Error("Field length overflow!");

		// ��������������ַ����ֶ�
		if (table_index_head_info.IsPrimary(cur_columu.column_name))
		{
			if (column_type == Column_Type::C && (cur_columu.column_value.size() > column_size) || cur_columu.column_value.size() > ColumnNameLength)
				throw Error("Primary key field length overflow!");
		}
	

	}
}
/***********************************************************************************************
//ѡ���¼ ***************************************************************************************/
//
// ѡ���¼   Select_Record Ϊ�����������ڻ��õ������condition_select ��ѭ������ʽ���ã�
//                                    ���ڶ������������˵ �� һ��һ����������condition_select���в��ң�Ȼ�������ϵ���
//                                     ��conditon_select�����ڲ������������ֶ��Ƿ�Ϊ�������ùؼ��ֲ����ֻ��Ƿ�Χ����
//                                                                               ��Ӧ����ΪKey_Select �� 

SelectPrintInfo Select_Record(TB_Select_Info tb_select_info, std::string path)
{	
	GetGlobalTimer().Start();
	Check_TB_Select_Info(tb_select_info);

	std::vector<std::pair<Key_Attr, FileAddress>> result;
	std::vector<std::pair<Key_Attr, FileAddress>> tmp_result;
	std::vector<std::pair<Key_Attr, FileAddress>> tmp;
	//GetTime().Start();//��ʱ��
	if (tb_select_info.vec_cmp_cell.empty())//���������ң��������м�¼
	{
		std::string idx_file = path + tb_select_info.table_name + ".index";
		BTree tree(idx_file);
		IndexHeadNode* pHead = tree.GetPtrIndexHeadNode();
		FileAddress node = pHead->MostLeftNode;

		while (node.OffSet != 0)
		{
			BTNode *pNode = tree.FileAddrToMemPtr(node);
			for (int i = 0;i < pNode->count_valid_key;i++)
			{
				result.push_back({ pNode->key[i], pNode->children[i] });

			}
			node = tree.FileAddrToMemPtr(node)->next;
		}
	}
	else       
	{
		for (int i = 0;i < tb_select_info.vec_cmp_cell.size();i++)  //����ÿһ��������˵
		{
			//���ֶη�Χ����
			//����Condition_Select������ Cell_Compare�ṹ�壬�������Լ���ǰ·������ǰ�������ݿ⣬
			//��������std::vector<std::pair<Key_Attr, FileAddress>>
			tmp_result= Condition_Select(tb_select_info.vec_cmp_cell[i], tb_select_info.table_name, GetCp().GetCurrentPath());
			//�����
			if (result.empty())
			{
				result = tmp_result;
			}
			else
			{
				std::vector<std::pair<Key_Attr, FileAddress>> tmp;

				std::vector<std::pair<FileAddress, Key_Attr>> temp1;
				std::vector<std::pair<FileAddress, Key_Attr>> temp2;
				int i = 0;
				int j = 0;
				for (i = 0; i < result.size(); i++) {
					temp1.push_back( make_pair(result[i].second, result[i].first) );
				}
				for (j = 0; j < tmp_result.size(); j++) {
					temp2.push_back(make_pair(tmp_result[j].second, tmp_result[j].first));
				}
				sort(temp1.begin(), temp1.end());
				sort(temp2.begin(), temp2.end());
				//�����󽻣���Ϊ������Ϊand
				//����O(n)���ӶȽ�����
				int rsize = temp1.size();
				int trsize = temp2.size();
				int res = 0;
				i = j = 0;
				while (i!=rsize&&j!=trsize)
				{
					if (temp1[i].first > temp2[j].first) {
						j++;
					}
					else if (temp1[i].first < temp2[j].first) {
						i++;
					}
					else {
						tmp.push_back(make_pair(temp1[i].second, temp1[i].first));
						i++;
						j++;
					}

				}

				//Timer time1;
				//time1.Start();
				//std::vector<FileAddress> temp1;
				//std::vector<FileAddress> temp2;
				//std::vector<FileAddress> temp3;
				//for (int i = 0; i < result.size(); i++) {
				//	temp1.push_back(result[i].second);
				//}
				//for (int i = 0; i < tmp_result.size(); i++) {
				//	temp2.push_back(tmp_result[i].second);
				//}
				//sort(temp1.begin(), temp1.end());
				//sort(temp2.begin(), temp2.end());
				//set_intersection(temp1.begin(), temp1.end(), temp2.begin(), temp2.end(), std::back_inserter(temp3));
				//for (int i = 0; i < result.size(); i++) {
				//	for (int j = 0; j < temp3.size(); j++) {
				//		if (temp3[j] == result[i].second) {
				//			tmp.push_back(result[i]);
				//			break;
				//		}
				//	}
				//	
				//}
				//time1.End();
				//time1.PrintTime();


				//for (int i = 0; i < result.size(); i++) {
				//	for (int j = 0; j < tmp_result.size(); j++) {
				//		if (result[i].second == tmp_result[j].second) {
				//			tmp.push_back(result[i]);
				//		}
				//	}
				//}
				//set_intersection(tmp_result.begin(),tmp_result.end(), result.begin(), result.end(), std::back_inserter(tmp));
				result = tmp;
			}
		}
	}

	SelectPrintInfo out;
	out.table_name = tb_select_info.table_name;
	out.name_selected_column = tb_select_info.name_select_column;
	out.key_fd = result;
	GetGlobalTimer().End();
	return out;
}

void Check_TB_Select_Info(const TB_Select_Info& tb_select_info)//�������Ϣ���
{
    std::string idx_file = GetCp().GetCurrentPath() + tb_select_info.table_name + ".index";
    BTree tree(idx_file);
    TableIndexHeadInfo table_index_head_info(tree);
    // ������ھ������ݿ�Ŀ¼�£����ܲ����¼
    if (!GetCp().IsInSpeDb())
    {
        std::cout << "Error!Not use database!" << std::endl;
        return;
    }
    // �жϱ��Ƿ��Ѿ�����
    if (_access(idx_file.c_str(), 0) == -1)   //������
    {
        std::cout << "The table is not exists!" << std::endl;
        return;
    }
    //����ֶ����Ƿ�Ϸ�
    if (!(tb_select_info.name_select_column.size() == 1 && tb_select_info.name_select_column[0] == "*"))
    {
        for (int j = 0; j < tb_select_info.name_select_column.size(); j++)
        {
            std::string name = tb_select_info.name_select_column[j];
            if (!table_index_head_info.IsColumnName(name))
            {
                std::cout << "Selected fields do not exist!" << std::endl;
                return;
            }
        }
    }
    // ����������
    for (int j = 0; j < tb_select_info.vec_cmp_cell.size(); j++)
    {
        auto& cmp = tb_select_info.vec_cmp_cell[j];
        if (!table_index_head_info.IsColumnName(cmp.cmp_value.column_name))
        {
            std::cout << "Where Expr Fields do not exist!" << std::endl;
            return;
        }
		Value_Type Temp = cmp.cmp_value.column_type;
		Column_Type Res = Column_Type::Error;
		if (Temp == Value_Type::I) {
			Res = Column_Type::I;
		}
		else if(Temp == Value_Type::C){
			Res = Column_Type::C;
		}
		else if (Temp == Value_Type::F) {
			Res = Column_Type::F;
		}
        if (table_index_head_info.GetColumnType(cmp.cmp_value.column_name) != Res)
        {
            std::cout << "Where Expr Fields type error!" << std::endl;
            return;
        }
        if (cmp.operator_type != Operator_Type::B
            && cmp.operator_type != Operator_Type::BE
            && cmp.operator_type != Operator_Type::E
            && cmp.operator_type != Operator_Type::L
            && cmp.operator_type != Operator_Type::LE
            && cmp.operator_type != Operator_Type::NE)
        {
            std::cout << "Where Expr relational operator error!" << std::endl;
            return;
        }
    }
}
//������ ѡ��
std::vector<std::pair<Key_Attr, FileAddress>> Condition_Select(Cell_Compare cell_compare, std::string table_name, std::string path)
{
	std::vector<std::pair<Key_Attr, FileAddress>> result;
	std::string file_idx = path + table_name + ".index";
	BTree tree(file_idx);
	TableIndexHeadInfo table_index_head_info(tree);
	IndexHeadNode* pHead = tree.GetPtrIndexHeadNode();    //��������ļ��е�Index�ڵ�ͷ

	//�ж��Ƿ�Ϊ����
	bool isPrimary = table_index_head_info.IsPrimary(cell_compare.cmp_value.column_name);
	if (isPrimary)
	{
		result = Key_Select(cell_compare, table_name, path);
	}
	else
	{	//�ж��Ƿ�����
		bool hasIndex = table_index_head_info.hasIndex(cell_compare.cmp_value.column_name);
		if (hasIndex) {
			
			char* pIndexName = pHead->RecordIndexName + table_index_head_info.GetIndex2(cell_compare.cmp_value.column_name) * ColumnNameLength; //�ҵ���Ӧ�ֶ�����
			std::string index_name = pIndexName;
			result = Key_Select(cell_compare, index_name, path);
		}
		else
		{
			result = Range_Select(cell_compare, table_name, path);
		}
		
	}
	return result;
}

std::vector<std::pair<Key_Attr, FileAddress>> Key_Select(Cell_Compare cell_compare, std::string table_name, std::string path)
{
	std::vector<std::pair<Key_Attr, FileAddress>> result;
	std::string file_idx = path + table_name + ".index";
	BTree tree(file_idx);
	IndexHeadNode* pHead = tree.GetPtrIndexHeadNode();

	//�������ֵ
	if (cell_compare.operator_type == Operator_Type::E)
	{
		FileAddress tmp = tree.Search(cell_compare.cmp_value);
		if (tmp.OffSet != 0)
		{
			result.push_back({ cell_compare.cmp_value, tmp });

		}
	}
	else
	{
		FileAddress node = pHead->MostLeftNode;//��һ�����ݽڵ�
		while (node.OffSet != 0)
		{
			BTNode* pNode = tree.FileAddrToMemPtr(node);
			for (int j = 0;j < pNode->count_valid_key;j++)
			{
				Column_Cell cell(pNode->key[j]);
				if (cell_compare(cell))
				{
					result.push_back({ pNode->key[j],pNode -> children[j] });
				}
			}
			node = tree.FileAddrToMemPtr(node)->next;
		}
	}
	return result;
}

//��Χѡ��
std::vector<std::pair<Key_Attr, FileAddress>> Range_Select(Cell_Compare cell_compare, std::string table_name, std::string path)
{
	std::vector<std::pair<Key_Attr, FileAddress>> result;
	std::string file_idx = path + table_name + ".index";
	BTree tree(file_idx);
	IndexHeadNode* pHead = tree.GetPtrIndexHeadNode();

	FileAddress node = pHead->MostLeftNode;
	while (node.OffSet != 0)
	{
		BTNode* pNode = tree.FileAddrToMemPtr(node);
		for (int i = 0;i < pNode->count_valid_key;i++)
		{
			Column_Head record = Get_DB_Record(table_name,pNode->children[i], path);
			Column_Cell* pColumn = record.firstCell();
			while (pColumn && pColumn->column_name != cell_compare.cmp_value.column_name)
			{
				pColumn = pColumn->next;
			}
			if (cell_compare(*pColumn))
			{
				result.push_back({ pNode->key[i],pNode->children[i] });
			}
		}
		node = tree.FileAddrToMemPtr(node)->next;
	}
	return result;
}

//ȡ��ָ����ַ������
Column_Head Get_DB_Record(std::string table_name, FileAddress file_address, std::string path)
{
	std::string idx_file = path + table_name + ".index";
	std::string dbs_file = path + table_name + ".dbs";
	BTree tree(idx_file);

	Column_Head record_head;
	char *pTypeInfo = tree.GetPtrIndexHeadNode()->RecordTypeInfo;
	char* pData = (char*)GetGlobalBuffer()[dbs_file.c_str()]->ReadRecord(&file_address);
	//��¼ͷ����ӵ�ֵַ
	pData += sizeof(FileAddress);
	std::vector<std::pair<std::string, Column_Type>>Column_Name_Type = GetColumn_Name_Type(table_name, path);
	int index = 0;
	while (*pTypeInfo != '\0')
	{
		Column_Cell cell;
		if (*pTypeInfo == 'I') 
		{
			cell.column_type = Value_Type::I;
			cell.column_name = Column_Name_Type[index].first;
			cell.column_value.IntValue = *(int*)pData;
			pData += sizeof(int);
			record_head.attachCell(cell);
			index++;
		}
		else if (*pTypeInfo == 'F')
		{
			cell.column_type = Value_Type::F;
			cell.column_name = Column_Name_Type[index].first;
			cell.column_value.FloatValue = *(double*)pData;
			pData += sizeof(double);
			record_head.attachCell(cell);
			index++;
		}
		else if (*pTypeInfo == 'C')
		{
			cell.column_type = Value_Type::C;
			cell.column_name = Column_Name_Type[index].first;
			//�ַ�������
			int off = (*(pTypeInfo + 1) - '0') * 100 + (*(pTypeInfo + 2) - '0') * 10 + (*(pTypeInfo + 3) - '0');
			auto pChar = (char*)malloc(off);
			memcpy(pChar, pData, off);
			cell.column_value.CharValue = pChar;
			pData += off;
			record_head.attachCell(cell);
			index++;
		}
		pTypeInfo++;
	}
	return record_head;
}
std::vector<std::pair<std::string, Column_Type>> GetColumn_Name_Type(std::string table_name, std::string table_path)
{
	std::string idx_file = table_path + table_name + ".index";
	std::string dbs_file = table_path + table_name + ".dbs";
	BTree tree(idx_file);
	IndexHeadNode *pHead = tree.GetPtrIndexHeadNode();
	std::vector<Column_Type> types;
	std::vector<std::string> names;
	std::vector<std::pair<std::string, Column_Type >> result;

	int sum = 0;//�ֶθ���
	for (int i = 0; pHead->RecordTypeInfo[i] != '\0'; i++)
	{
		if (pHead->RecordTypeInfo[i] == 'I')
		{
			types.push_back(Column_Type::I);
			sum++;
		}
		else if (pHead->RecordTypeInfo[i] == 'F')
		{
			types.push_back(Column_Type::F);
			sum++;
		}
		else if (pHead->RecordTypeInfo[i] == 'C')
		{
			types.push_back(Column_Type::C);
			sum++;
		}
	}
	char* pName = pHead->RecordColumnName;
	for (int i = 0;i < sum;i++)
	{
		names.push_back(pName);
		pName += ColumnNameLength;
	}
	for (int i = 0;i < names.size();i++)
	{
		result.push_back({ names[i],types[i] });
	}
	return result;
}

Column_Type GetType(std::string name, std::vector<std::pair<std::string, Column_Type>> pair)
{
	for (int i = 0;i < pair.size();i++)
	{
		if (pair[i].first == name)
		{
			return pair[i].second;
		}
	}
	return Column_Type::I;//Ĭ�Ϸ���
}


/***********************************************************************************************
//ɾ����¼ ***************************************************************************************/

bool Delete_Record(TB_Delete_Info tb_delete_info, std::string path) {

	Check_TB_Delete_Info(tb_delete_info);
	std::string file_idx = path + tb_delete_info.table_name + ".index";
	std::string file_dbf = path + tb_delete_info.table_name + ".dbs";
	BTree tree(file_idx);      //��idx_file ��ʼ������ һ��B+��
	TableIndexHeadInfo table_index_head_info(tree);   //������ʼ������һ���ṹ�壬���Ա��ֻ��һ��BTree,��Ϊ�˷��㺯����������
	IndexHeadNode* pHead = tree.GetPtrIndexHeadNode();    //��������ļ��е�Index�ڵ�ͷ
	

	// ��ɾ��������expr������װ�ɲ�������
	std::vector<Cell_Compare> cmp_cells;
	auto fields_name = GetColumn_Name_Type(tb_delete_info.table_name, GetCp().GetCurrentPath());

	for (int i = 0; i < tb_delete_info.expr.size(); i++)
	{
		Cell_Compare cmp_cell = CreateCmpCell(tb_delete_info.expr[i].field, GetType(tb_delete_info.expr[i].field, fields_name)
			, GetOperatorType(tb_delete_info.expr[i].op), tb_delete_info.expr[i].value);
		cmp_cells.push_back(cmp_cell);
	}

	// ������������������ֶ�
	std::vector<std::pair<Key_Attr, FileAddress>> res;
	std::vector<std::pair<Key_Attr, FileAddress>> fds;

	for (int i = 0; i < cmp_cells.size(); i++)
	{
		// �������㵥���ֶεļ�¼
		fds = Condition_Select(cmp_cells[i], tb_delete_info.table_name, GetCp().GetCurrentPath());
     	// �µĽ����֮ǰ�Ľ���󽻼�
		if (res.empty())
		{
			res = fds;
		}
		else
		{
		/*	std::vector<std::pair<Key_Attr, FileAddress>> v;
			sort(fds.begin(), fds.end());
			sort(res.begin(), res.end());
     		set_intersection(fds.begin(), fds.end(), res.begin(), res.end(), std::back_inserter(v));
			res = v;*/
			std::vector<std::pair<Key_Attr, FileAddress>> tmp;
			std::vector<std::pair<FileAddress, Key_Attr>> temp1;
			std::vector<std::pair<FileAddress, Key_Attr>> temp2;
			int i = 0;
			int j = 0;
			for (i = 0; i < res.size(); i++) {
				temp1.push_back(make_pair(res[i].second, res[i].first));
			}
			for (j = 0; j < fds.size(); j++) {
				temp2.push_back(make_pair(fds[j].second, fds[j].first));
			}
			sort(temp1.begin(), temp1.end());
			sort(temp2.begin(), temp2.end());
			//�����󽻣���Ϊ������Ϊand
			//����O(n)���ӶȽ�����
			int rsize = temp1.size();
			int trsize = temp2.size();
			i = j = 0;
			while (i != rsize && j != trsize)
			{
				if (temp1[i].first > temp2[j].first) {
					j++;
				}
				else if (temp1[i].first < temp2[j].first) {
					i++;
				}
				else {
					tmp.push_back(make_pair(temp1[i].second, temp1[i].first));
					i++;
					j++;
				}

			}
			res = tmp;
		}
	}

	// ɾ������ɾ���Ľ��
	//BTree tree(file_idx);
	Record record;
	for (int i = 0; i < res.size(); i++)
	{
		tree.Delete(res[i].first);
		record.DeleteRecord(file_dbf, res[i].second, 0);
	}

	int index2 = 0;
	std::vector<int> index_info = table_index_head_info.GetIndexInfo();
	for (int i = 0; i < index_info.size(); i++) {
		char* pIndexName = pHead->RecordIndexName + i * ColumnNameLength;//�ҵ�i��������
		std::string index_name = pIndexName;
		std::string idx_file = path + index_name + ".index";
		BTree tree2(idx_file);//�ҵ�����
		for (int i = 0; i < res.size(); i++) {
			Key_Attr Temp;
			Column_Head record_head = Get_DB_Record(tb_delete_info.table_name, res[i].second, GetCp().GetCurrentPath());
			auto pColumn = record_head.firstCell();
			while (pColumn)
			{
				switch (pColumn->column_type)
				{
				case Value_Type::I:
					Temp.type = Value_Type::I;
					Temp.value.IntValue = pColumn->column_value.IntValue;
					break;
				case Value_Type::F:
					Temp.type = Value_Type::F;
					Temp.value.FloatValue = pColumn->column_value.FloatValue;
					break;
				case Value_Type::C:
					Temp.type = Value_Type::C;
					strcpy(Temp.value.CharValue, pColumn->column_value.CharValue);
					break;
				default:
					break;
				}
				pColumn = pColumn->next;
			}
			tree2.Delete(Temp);
		}
		
	}


	
	return true;
}
void Check_TB_Delete_Info(const TB_Delete_Info &tb_delete_info)
{
	std::string idx_file = GetCp().GetCurrentPath() + tb_delete_info.table_name + ".index";
	BTree tree(idx_file);
	TableIndexHeadInfo table_index_head_info(tree);

	// ������ھ������ݿ�Ŀ¼�£����ܲ����¼
	if (!GetCp().IsInSpeDb())
		throw Error("Error!Not use database!");

	// �жϱ��Ƿ��Ѿ�����
	if (_access(idx_file.c_str(), 0) == -1) {  //������
		throw Error("The table is not exists!");
	}

	// ���ɾ������
	for (int j = 0; j < tb_delete_info.expr.size(); j++)
	{
		auto &expr_tmp = tb_delete_info.expr[j];
		// ����ֶ�����
		if (!table_index_head_info.IsColumnName(expr_tmp.field))
			throw Error("Where Expr Fields do not exist!");
		if (expr_tmp.op != ">" && expr_tmp.op != "=" && expr_tmp.op != "<" && expr_tmp.op != ">=" && expr_tmp.op != "<=" && expr_tmp.op != "!=")
			throw Error("Where Expr relational operator error!");
	}
}
/***********************************************************************************************
//���¼�¼ ***************************************************************************************/

//// ���¼�¼ 
bool Update_Record(TB_Update_Info tb_update_info, std::string path) {
		//Check_TB_Update_Info(tb_update_info);
	
		std::string file_idx = path + tb_update_info.table_name + ".index";
		std::string file_dbf = path + tb_update_info.table_name + ".dbs";
		BTree tree(file_idx);
		TableIndexHeadInfo table_index_head_info(tree);
		IndexHeadNode* pHead = tree.GetPtrIndexHeadNode();    //��������ļ��е�Index�ڵ�ͷ
	
		// �����²�����expr������װ�ɲ�������
		std::vector<Cell_Compare> cmp_cells;
		auto fields_name = GetColumn_Name_Type(tb_update_info.table_name, GetCp().GetCurrentPath());
	
		for (int i = 0; i < tb_update_info.expr.size(); i++)
		{
			Cell_Compare cmp_cell = CreateCmpCell(tb_update_info.expr[i].field, GetType(tb_update_info.expr[i].field, fields_name)
				, GetOperatorType(tb_update_info.expr[i].op), tb_update_info.expr[i].value);
	
			cmp_cells.push_back(cmp_cell);
		}
	
		// ������������������ֶ�
		std::vector<std::pair<Key_Attr, FileAddress>> res;
		std::vector<std::pair<Key_Attr, FileAddress>> fds;
		// ����
		for (int i = 0; i < cmp_cells.size(); i++)
		{
			// �������㵥���ֶεļ�¼
			fds = Condition_Select(cmp_cells[i], tb_update_info.table_name, GetCp().GetCurrentPath());
			// �µĽ����֮ǰ�Ľ���󽻼�
			if (res.empty())
			{
				res = fds;
			}
			else
			{
				std::vector<std::pair<Key_Attr, FileAddress>> tmp;
				std::vector<std::pair<FileAddress, Key_Attr>> temp1;
				std::vector<std::pair<FileAddress, Key_Attr>> temp2;
				int i = 0;
				int j = 0;
				for (i = 0; i < res.size(); i++) {
					temp1.push_back(make_pair(res[i].second, res[i].first));
				}
				for (j = 0; j < fds.size(); j++) {
					temp2.push_back(make_pair(fds[j].second, fds[j].first));
				}
				sort(temp1.begin(), temp1.end());
				sort(temp2.begin(), temp2.end());
				//�����󽻣���Ϊ������Ϊand
				//����O(n)���ӶȽ�����
				int rsize = temp1.size();
				int trsize = temp2.size();
				i = j = 0;
				while (i != rsize && j != trsize)
				{
					if (temp1[i].first > temp2[j].first) {
						j++;
					}
					else if (temp1[i].first < temp2[j].first) {
						i++;
					}
					else {
						tmp.push_back(make_pair(temp1[i].second, temp1[i].first));
						i++;
						j++;
					}

				}
				res = tmp;
			}
			//for (auto e : res)
				//PrintRecord(tb_update_info.table_name, e.first, e.second, path);
		}
	
		//���¼�¼
		for (int i = 0; i < res.size(); i++)
		{
			// �ֶμ�¼
			char* pdata = (char*)(GetGlobalBuffer()[file_dbf.c_str()]->ReadRecord(&res[i].second));  // ��¼������ָ��
			GetGlobalClock()->GetMemAddr(GetGlobalBuffer()[file_dbf.c_str()]->FilePointer, (&(res[i].second))->FilePageID)->IsModified = true; //������Ϊ��ҳ
			pdata += sizeof(FileAddress);  // ������ַ����
	
			//����ÿ��Ҫ���µ��ֶ�ֵ
			for (int j = 0; j < tb_update_info.field_value.size(); j++)
			{
				char *p = pdata + table_index_head_info.GetColumnOffset(tb_update_info.field_value[j].field);
				Column_Type column_type = table_index_head_info.GetColumnType(tb_update_info.field_value[j].field);
				int column_id = table_index_head_info.GetIndex(tb_update_info.field_value[j].field);
				switch (column_type)
				{
				case Column_Type::I:
					Check_Type_I(tb_update_info.field_value[j].value);
					Check_Unique_Info(tb_update_info.table_name, tb_update_info.field_value[j].field, tb_update_info.field_value[j].value, 'I', column_id, path);
					*(int*)p = stoi(tb_update_info.field_value[j].value);
					break;
	
				case Column_Type::F:
					Check_Type_F(tb_update_info.field_value[j].value);
					Check_Unique_Info(tb_update_info.table_name, tb_update_info.field_value[j].field, tb_update_info.field_value[j].value, 'F', column_id, path);
					*(double*)p = stod(tb_update_info.field_value[j].value);
					break;
	
				case Column_Type::C:
					Check_Unique_Info(tb_update_info.table_name, tb_update_info.field_value[j].field, tb_update_info.field_value[j].value, 'C', column_id, path);
					strcpy(p, tb_update_info.field_value[j].value.c_str());
					break;
	
				default:
					break;
				}
			}
	
	
			// �ж�������ֵ��û�б�����
			bool isPrimary = false;
			std::string new_primary_key;
			int index = 0; // ����������޸ģ�������ֵ��λ��
			for (int j = 0; j < tb_update_info.field_value.size(); j++)
			{
				if (table_index_head_info.IsPrimary(tb_update_info.field_value[j].field))
				{
					isPrimary = true;
					new_primary_key = tb_update_info.field_value[j].value;
					index = j;
					break;
				}
			}
	
			// �޸���������
			if (isPrimary)
			{
				// TODO		
				Column_Type old_key_type = (Column_Type)res[i].first.type;
				
				tree.Delete(res[i].first);
				Key_Attr new_key;
				new_key.type = (Value_Type)old_key_type;
				switch (new_key.type)
				{
				case Value_Type::I:
					new_key.value.IntValue = stoi(new_primary_key);
					break;
				case Value_Type::F:
					new_key.value.FloatValue = stod(new_primary_key);
					break;
				case Value_Type::C:
					strcpy(new_key.value.CharValue, new_primary_key.c_str());
					break;
				default:
					break;
				}
				tree.Insert(new_key, res[i].second);
			}


			//�ж�������û�б��޸�
			int index2 = 0; // ����������޸ģ�������ֵ��λ��
			for (int j = 0; j < tb_update_info.field_value.size(); j++)
			{
				bool hasIndex = false;
				std::string new_index_key;
				if (table_index_head_info.hasIndex(tb_update_info.field_value[j].field))
				{
					hasIndex= true;
					new_index_key = tb_update_info.field_value[j].value;
					index2 = j;
					//break;
				}
				if (hasIndex)
				{
					int tmp = table_index_head_info.GetIndex2(tb_update_info.field_value[j].field);
					char* pIndexName = pHead->RecordIndexName + tmp * ColumnNameLength;//�ҵ�i��������
					std::string index_name = pIndexName;
					std::string idx_file = path + index_name + ".index";
					BTree tree2(idx_file);//�ҵ�����
					Column_Type old_key_type = (Column_Type)res[i].first.type;

					Key_Attr new_key;
					new_key.type = (Value_Type)old_key_type;
					switch (new_key.type)
					{
					case Value_Type::I:
						new_key.value.IntValue = stoi(new_index_key);
						break;
					case Value_Type::F:
						new_key.value.FloatValue = stod(new_index_key);
						break;
					case Value_Type::C:
						strcpy(new_key.value.CharValue, new_index_key.c_str());
						break;
					default:
						break;
					}
					tree2.Delete(new_key);
					tree2.Insert(new_key, res[i].second);
				}
			}

		}
		return true;
	}


	/***********************************************************************************************
	//�������� ***************************************************************************************/

	//��������
	bool Create_Index(Idx_Create_Info idx_create_info, std::string path) {

		std::string idx_file = path + idx_create_info.index_name + ".index";
		std::string file_idx = path + idx_create_info.table_name + ".index";
		if (_access(idx_file.c_str(), 0) != -1)
		{
			std::cout << "Index already exists!" << std::endl;
			return false;
		}
		BTree tree(file_idx);
		TableIndexHeadInfo table_index_head_info(tree);
		IndexHeadNode* pHead = tree.GetPtrIndexHeadNode();

		//�жϸ��ֶ��Ƿ��Ǳ����ֶ�,���Ƿ�Ϊ����
		if (!table_index_head_info.IsColumnName(idx_create_info.column_name)) {
			cout << "Is not table fields" << endl;
			return false;
		}

		if (table_index_head_info.IsPrimary(idx_create_info.column_name)){
			cout << "Is primary key" << endl;
			return false;
		}
		if (!table_index_head_info.IsUnique(idx_create_info.column_name)) {
			cout << "Is not unique" << endl;
				return false;
		}

		char column_name[RecordColumnCount / 4 * ColumnNameLength];
		char column_type[RecordColumnCount];
		int i = 0;
		int j = 0;
		for (i = 0; pHead->RecordTypeInfo[i] != '\0'; i++) {
			if (pHead->RecordTypeInfo[i] == 'I' || pHead->RecordTypeInfo[i] == 'F' || pHead->RecordTypeInfo[i] == 'C') {
				char* pColumnName = pHead->RecordColumnName + j * ColumnNameLength; //�ҵ���Ӧ�ֶ�����
				strcpy(&column_name[j*ColumnNameLength], pColumnName);
				j++;
			}
			column_type[i] = pHead->RecordTypeInfo[i];
		}
		column_type[i] = '\0';
		int column_key;

		column_key = table_index_head_info.GetIndex(idx_create_info.column_name);//���ظ��ֶ��������ֶ��е�����λ��

		BTree tree2(idx_file, column_key, column_type, column_name);//��������
	
		SelectPrintInfo select_print_info;
		TB_Select_Info tb_select_info;
		tb_select_info.table_name = idx_create_info.table_name;
		tb_select_info.name_select_column.push_back(idx_create_info.column_name);
		
		select_print_info = Select_Record(tb_select_info, path);
		std::vector<std::string> out_name = select_print_info.name_selected_column;//ѡ�������
		auto key_fd = select_print_info.key_fd;//ѡ�������ֶ���Ϣ�͵�ַ
		
		for (int i = 0; i < select_print_info.key_fd.size(); i++) {
			Key_Attr Temp;
			Column_Head record_head = Get_DB_Record(select_print_info.table_name, key_fd[i].second, GetCp().GetCurrentPath());
			auto pColumn = record_head.firstCell();
			while (pColumn)
			{   
				auto it = find(out_name.begin(), out_name.end(), pColumn->column_name);
				if (it != out_name.end())
				{
					switch (pColumn->column_type)
					{
					case Value_Type::I:
						Temp.type = Value_Type::I;
						Temp.value.IntValue = pColumn->column_value.IntValue;
						break;
					case Value_Type::F:
						Temp.type = Value_Type::F;
						Temp.value.FloatValue = pColumn->column_value.FloatValue;
						break;
					case Value_Type::C:
						Temp.type = Value_Type::C;
						strcpy(Temp.value.CharValue, pColumn->column_value.CharValue);
						break;
					default:
						break;
					}
				}
				pColumn = pColumn->next;
			}
			tree2.Insert(Temp, select_print_info.key_fd[i].second);
		}
		
		char index_name[ColumnNameLength];
		char* index_ptr = index_name;
		strcpy(index_ptr, idx_create_info.index_name.c_str());//д������
		index_ptr += ColumnNameLength;
		int num = table_index_head_info.GetIndexCount();//�õ����е���������
		memcpy(pHead->RecordIndexName+ num * ColumnNameLength, index_name, ColumnNameLength);
		memcpy(pHead->RecordIndexInfo+ num * sizeof(int), &column_key, sizeof(int));

		return true;
	}
	/***********************************************************************************************
	//ɾ������ ***************************************************************************************/

	//ɾ������
	bool Drop_Index(std::string index_name, std::string path) {
		std::string tmp_path = path + index_name;
		std::string idx = tmp_path + ".index";
		
		Buffer &buffer = GetGlobalBuffer();
		Clock* Clock_ptr = GetGlobalClock();
		
		if (_access(idx.c_str(), 0) == -1 )
		{
			std::cout << "Index doesn't exist!" << std::endl;
			return false;
		}
		else
		{
			// ����ļ��Ѿ�����ʹ�� ��Ҫ�ȶ������ڴ��е��ļ�ҳ �����������ٴ�д��
			File *IdxMF_ptr = buffer.GetFile(idx.c_str());
			if (IdxMF_ptr)
			{
				for (int i = 1; i < MemPageCount; i++)
				{
					if (Clock_ptr->MemPages[i] && Clock_ptr->MemPages[i]->FilePointer == IdxMF_ptr->FilePointer)
					{
						Clock_ptr->MemPages[i]->FilePointer = NULL;
						Clock_ptr->MemPages[i]->IsModified = false;
					}
				}
			}
		 
			// ɾ�����ļ�
			fclose(IdxMF_ptr->FilePointer);
			buffer.CloseFile(idx.c_str());
			remove(idx.c_str());
		
			return true;
		}

	}