
#include"Api.h"
bool CatalogPosition::isInSpeDb = false;
bool CatalogPosition::ResetRootCatalog(std::string root_new)//���ø�Ŀ¼
{
	if (root_new[root_new.size() - 1] == '/')
	{
		root = root_new;
		current_catalog = root_new;
		isInSpeDb = false;
		return true;
	}
	else {
		return false;
	}
}

void CatalogPosition::SwitchToDb()//ת�����ݿ��б�Ŀ¼��
{
	current_catalog = root;
	isInSpeDb = false;
}

bool CatalogPosition::SwitchToDb(std::string db)//ת��ָ�������ݿ�
{
	if (_access((root + db).c_str(), 0) == -1)//���ݿⲻ����
	{
		return false;
	}
	else {
		current_catalog = root + db + "/";
		isInSpeDb = true;
		return true;
	}
}

std::string CatalogPosition::GetCurrentPath()const//��õ�ǰ����·��
{
	return current_catalog;
}

std::string CatalogPosition::GetRootPath()const//��ø�·��
{
	return root;
}

std::string CatalogPosition::SetCurrentPath(std::string path)//�趨·��
{
	current_catalog = path;
	return path;
}

CatalogPosition::CatalogPosition()
	:root("./DB/"), current_catalog("./DB/")
{
	// �����ǰĿ¼��û�� DB �ļ����򴴽�
	std::string tmp_path = "./DB";

	if (_access(tmp_path.c_str(), 0) == -1)
	{
		_mkdir(tmp_path.c_str());
	}

	/*
	������: _mkdir
�� ��: ����һ��Ŀ¼
�� ��: int _mkdir( const char *dirname );
ͷ�ļ��⣺direct.h
����ֵ������һ��Ŀ¼�����ɹ��򷵻�0�����򷵻�-1*/
}

CatalogPosition& GetCp()
{
	static CatalogPosition cp;
	return cp;
}

/************************************************************************
*    �����ļ�ͷ��Ϣ����
************************************************************************/

unsigned int TableIndexHeadInfo::GetColumnCount()const               //��ñ���ֶθ���
{
	int num = 0;
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	char* pColumnTypeInfo = pHeadNode->RecordTypeInfo;
	while ((*pColumnTypeInfo) != '\0')
	{
		char c = *pColumnTypeInfo;
		if (c == 'I' || c == 'F' || c == 'C')  num++;
		pColumnTypeInfo++;
	}
	return num;
}
unsigned int  TableIndexHeadInfo::GetIndexCount()const					//��ñ����������
{
	int num = 0;
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	int* pIndexInfo = pHeadNode->RecordIndexInfo;
	while ((*pIndexInfo) > 0)
	{
		num++;
		pIndexInfo++;
	}
	return num;

}
/*
std::vector<int> TableIndexHeadInfo::GetUniqueIndex()const 
{
	std::vector<int>unique_index;
	int column_count = GetColumnCount();
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
}
*/
std::vector<std::string> TableIndexHeadInfo::GetIndexNames()const {
	std::vector<std::string>index_names;
	int index_count = GetIndexCount();
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	char* pIndexName = pHeadNode->RecordIndexName;
	for (int i = 0; i < index_count; i++)
	{
		index_names.push_back(pIndexName);
		pIndexName += ColumnNameLength;
	}
	return index_names;
}
std::vector<std::string> TableIndexHeadInfo::GetColumnNames()const    //�����ֶε�����
{
	std::vector<std::string>column_names;
	int column_count = GetColumnCount();
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	char* pColumnName = pHeadNode->RecordColumnName;
	for (int i = 0; i < column_count; i++)
	{
		column_names.push_back(pColumnName);
		pColumnName += ColumnNameLength;
	}
	return column_names;
}

std::vector<Column_Type> TableIndexHeadInfo::GetColumnType()const      //�����ֶε�����
{
	std::vector<Column_Type> column_types;
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	char* pColumnTypeInfo = pHeadNode->RecordTypeInfo;
	while ((*pColumnTypeInfo) != '\0')
	{
		switch (*pColumnTypeInfo)
		{
		case'I':
			column_types.push_back(Column_Type::I);
			break;
		case'F':
			column_types.push_back(Column_Type::F);
			break;
		case'C':
			column_types.push_back(Column_Type::C);
			break;
		default:
			break;
		}
		pColumnTypeInfo++;
	}
	return column_types;
}


std::vector<int> TableIndexHeadInfo::GetColumnSize()const              //�����ֶεĴ�С
{
	std::vector<int> column_sizes;
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	char* pColumnTypeInfo = pHeadNode->RecordTypeInfo;
	while ((*pColumnTypeInfo) != '\0')
	{
		int tmp = 0;
		switch (*pColumnTypeInfo)
		{
		case'I':
			column_sizes.push_back(sizeof(int));
			break;
		case'F':
			column_sizes.push_back(sizeof(double));
			break;
		case'C':
			tmp = (pColumnTypeInfo[1] - '0') * 100 + (pColumnTypeInfo[2] - '0') * 10 + (pColumnTypeInfo[3] - '0') * 1; //??
			column_sizes.push_back(tmp);
			break;
		default:
			break;
		}
		pColumnTypeInfo++;
	}
	return column_sizes;
}

int TableIndexHeadInfo::GetPrimaryIndex()const                        //�����ֶε�����
{
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	return pHeadNode->KeyTypeIndex;
}
std::vector<int> TableIndexHeadInfo::GetUniqueIndex()const
{

	std::vector<int> unique_index;
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	int* pUniqueIndexInfo = pHeadNode->UniqueKeyIndex;
	while ((*pUniqueIndexInfo) > 0)
	{
		unique_index.push_back(*pUniqueIndexInfo);
		pUniqueIndexInfo++;
	}
	return unique_index;
}
std::vector<int> TableIndexHeadInfo::GetIndexInfo()const
{
	std::vector<int> index_info;
	IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	int* pIndexInfo = pHeadNode->RecordIndexInfo;
	while ((*pIndexInfo)>0)
	{
		index_info.push_back(*pIndexInfo);
		pIndexInfo++;
	}
	return index_info;

}
int TableIndexHeadInfo::GetColumnSizeByIndex(int i)const               //��i���ֶε����ݴ�С
{
	std::vector<int> tmp = GetColumnSize();
	return tmp[i];
}

bool TableIndexHeadInfo::IsColumnName(std::string column_name)const    //�жϸ��ֶ��ǲ��Ǳ���ֶ�
{
	std::vector<std::string> names = GetColumnNames();
	return (std::find(names.begin(), names.end(), column_name) != names.end());
}

bool TableIndexHeadInfo::hasIndex(std::string column_name)const
{
	int key_index = GetIndex(column_name);//�õ�������λ��
	std::vector<int> index_info =  GetIndexInfo();//�õ�������������λ��
	bool flag = false;
	for (int i = 0; i < index_info.size(); i++) {

		if (key_index == index_info[i]) {
			flag = true;
		}
	}
	return flag;
}

bool TableIndexHeadInfo::IsPrimary(std::string column_name)const       //�ж��ֶ����Ƿ�Ϊ�����ֶ�
{
	std::vector<std::string> names = GetColumnNames();
	int index = GetPrimaryIndex();
	if (!names.empty())
	{
		return names[index] == column_name;
	}
	return false;
}

bool TableIndexHeadInfo::IsUnique(std::string column_name)const     //�ж��ֶ����Ƿ�Ϊunique�ֶ�
{
	std::vector<std::string> names = GetColumnNames();
	std::vector<int> unique_index = GetUniqueIndex();
	bool flag = false;
	if (!names.empty())
	{
		for (int i = 0; i < unique_index.size(); i++) {
			//cout << unique_index[i] << endl;
			if (names[unique_index[i]] == column_name)
				flag = true;

		}
	}
	return flag;
}

Column_Type TableIndexHeadInfo::GetColumnType(std::string column_name)const//���ظ��ֶε�����
{
	std::vector<std::string> names = GetColumnNames();
	int tmp = 0;
	for (int i = 0; i < names.size(); i++)
	{
		if (names[i] == column_name)
		{
			tmp = i;
			break;
		}
	}
	std::vector<Column_Type> types = GetColumnType();
	return types[tmp];
}

int TableIndexHeadInfo::GetIndex(std::string column_name)const         //���ظ��ֶ��������ֶ��е�����λ��
{
	std::vector<std::string> names = GetColumnNames();
	for (int i = 0; i < names.size(); i++)
	{
		if (names[i] == column_name)
			return i;
	}
	return 0;
}

int TableIndexHeadInfo::GetIndex2(std::string column_name)const         //���ظ��ֶ��������ֶ��е�����λ��
{
	std::vector<std::string> names = GetIndexNames();
	for (int i = 0; i < names.size(); i++)
	{
		if (names[i] == column_name)
			return i;
	}
	return 0;
}

int TableIndexHeadInfo::GetColumnOffset(std::string column_name)      //���ظ����ֶ����ֶξ�������ͷ��ַ��ƫ��
{
	std::vector<int> sizes = GetColumnSize();
	int index = GetIndex(column_name);
	int sum = 0;
	for (int i = 0; i < index; i++)
	{
		sum += sizes[i];
	}
	return sum;
}

//bool TableIndexHeadInfo::IsUnique(std::string column_name)const       //�ж��ֶ����Ƿ�Ϊunique�ֶ�
//{
//    IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
//    std::vector<int> Find = pHeadNode->UniqueIndex;
//    if (std::find(Find.begin(), Find.end(), GetIndex(column_name)) == Find.end()) {
//        return false;
//    }
//    return true;
//}
Operator_Type GetOperatorType(std::string s)
{
	s = tolower(s);
	if (s == ">")
	{
		return Operator_Type::B;
	}
	else if (s == ">=")
	{
		return Operator_Type::BE;
	}
	else if (s == "<")
	{
		return Operator_Type::L;
	}
	else if (s == "<=")
	{
		return Operator_Type::LE;
	}
	else if (s == "=")
	{
		return Operator_Type::E;
	}
	else if (s == "!=")
	{
		return Operator_Type::NE;
	}
	else
	{
		return Operator_Type::B;
	}
}