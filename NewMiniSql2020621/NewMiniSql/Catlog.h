/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  2 0 2 0 D B S - G R O U P W O R K             ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : MiniSql		                                               *
 *                                                                                             *
 *                    File Name : Timer.H                                                      *
 *                                                                                             *
 *                   Programmer : YuZhe Luo                                                    *
 *                                                                                             *
 *                   Start Date : 06/08/2020                                                   *
 *                                                                                             *
 *                  Last Update : 06/21/2020			                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef __CATLOG_H__
#define __CATLOG_H__
#include"BTree.h"
#include"Error.h"
class CatalogPosition;
// Ŀ¼��λ���л� �������ݿ�ͱ��ʹ��
class CatalogPosition
{
	friend bool Use_DB(std::string db_name, CatalogPosition& cp);//ѡ�����ݿ�

public:
	CatalogPosition();
	bool ResetRootCatalog(std::string root_new);//���ø�Ŀ¼
	void SwitchToDb();//ת�����ݿ��б�Ŀ¼��
	bool SwitchToDb(std::string db);//ת��ָ�������ݿ�
	std::string GetCurrentPath()const;//��õ�ǰ����·��
	std::string GetRootPath()const;//��ø�·��
	std::string SetCurrentPath(std::string path);//�趨·��

	bool IsInSpeDb()
	{
		return isInSpeDb;
	}
	bool SetIsInSpeDb(bool tmp)
	{
		isInSpeDb = tmp;
		return tmp;
	}

	static bool isInSpeDb;//�Ƿ��ھ�������ݿ�Ŀ¼��
	std::string root;//��Ŀ¼
	std::string current_catalog;
};
CatalogPosition& GetCp();

/************************************************************************
*    �����ļ�ͷ��Ϣ����
************************************************************************/
class TableIndexHeadInfo
{
private:
	BTree& tree;
public:
	TableIndexHeadInfo(BTree &in_tree):tree(in_tree){}//���캯��Ҫ��������B+��
	unsigned int GetColumnCount()const;                 //��ñ���ֶθ���
	unsigned int GetIndexCount()const;					//��ñ����������
	std::vector<std::string> GetIndexNames()const;		//������������
	std::vector<std::string> GetColumnNames()const;     //�����ֶε�����
	std::vector<Column_Type> GetColumnType()const;      //�����ֶε�����
	std::vector<int> GetColumnSize()const;              //�����ֶεĴ�С
	int GetPrimaryIndex()const;                         //�����ֶε�����
	std::vector<int> GetUniqueIndex()const;				//unique�ֶε����� 
	std::vector<int> GetIndexInfo()const;				//��ý��������ֶε�λ����Ϣ
	int GetColumnSizeByIndex(int i)const;               //��i���ֶε����ݴ�С
	bool IsColumnName(std::string column_name)const;    //�жϸ��ֶ��ǲ��Ǳ���ֶ�
	bool IsPrimary(std::string column_name)const;       //�ж��ֶ����Ƿ�Ϊ�����ֶ�
	bool IsUnique(std::string column_name)const;       //�ж��ֶ����Ƿ�Ϊunique�ֶ�
	bool hasIndex(std::string column_name)const;		//�жϸ��ֶ���û�н�����
	Column_Type GetColumnType(std::string column_name)const;//���ظ��ֶε�����
	int GetIndex(std::string column_name)const;         //���ظ��ֶ��������ֶ��е�����λ��
	int GetIndex2(std::string column_name)const;         //���ظ��ֶ��������е�����λ��
	int GetColumnOffset(std::string column_name);       //���ظ����ֶ����ֶξ�������ͷ��ַ��ƫ��
};
/************************************************************************
*    �����ֶ���Ϣ
************************************************************************/
struct Idx_Create_Info                                                         //
{                                                                             //
	std::string index_name;								//������
	std::string table_name;                            // ����          //
	std::string column_name;							//����    //
};

/************************************************************************
*    ���ֶ���Ϣ
************************************************************************/
//�����¼�����ֶε�����
//�ֶ����� I-- - int  C-- - char  F-- - float
/********************************************************************************/
//�½���Ҫ�õ��������ṹ��
typedef struct ColumnInfo               // �½�����ֶ���Ϣ                   //*
{																			  //*
	std::string name;                                                         //
	Column_Type type;                                                         //
	bool isPrimary;                                // �Ƿ�����                //
	bool isUnique;                                 // �Ƿ������ظ�            //
	int RequiredLength;                            // �û������ֶ����ݳ���    //
}ColumnInfo;																  //
/************************************************************************     //
*    �������Ϣ�ṹ��
************************************************************************/     //
struct TB_Create_Info                                                         //
{                                                                             //
	std::string table_name;                            // �½��ı���          //
	std::vector<ColumnInfo> columns_info;              // ��ĸ����ֶ�        //
};

/********************************************************************************/

/********************************************************************************/
//�����¼Ҫ�õ��������ṹ��
typedef struct InsertInfo {
	std::string column_name; //������ֶ�
	std::string column_value;//�����ֵ
}InsertInfo;

struct TB_Insert_Info
{
	bool IsOrder;
	std::string table_name;
	std::vector<InsertInfo> insert_info;
};
/********************************************************************************/

/********************************************************************************/
//ɾ����¼Ҫ�õ��������ṹ��
typedef struct Expr {			//����ƥ����ʽ     where name = 'name1';   �� field = ��name��  op = ��=��value = 'name1'
	std::string field;			//�ֶ���
	std::string op;				//����
	std::string value;			//ֵ
}Expr;

struct TB_Delete_Info
{
	std::string table_name;
	std::vector<Expr> expr;             // ɾ�����ֶ�����
};
/********************************************************************************/

/********************************************************************************/
//���Ҽ�¼Ҫ�õ��Ľṹ��
enum Operator_Type { B, BE, L, LE, E, NE };
Operator_Type GetOperatorType(std::string s);  //�������< > >=����ת���� L B ����ĺ���
//�ֶαȽϵ�Ԫ�� 
class Cell_Compare  
{
public:
	Cell_Compare(Operator_Type ot, Column_Cell cc) :operator_type(ot),cmp_value(cc){}
	Operator_Type operator_type;
	Column_Cell cmp_value;
	bool operator()(const Column_Cell &cc)//������бȽϵ������ֶ�
{
	switch (cmp_value.column_type)
	{
	case Value_Type::I:
		switch(operator_type)
		{
		case B:
			return cc.column_value.IntValue > cmp_value.column_value.IntValue;
			break;
		case BE:
			return cc.column_value.IntValue >= cmp_value.column_value.IntValue;
			break;
		case L:
			return cc.column_value.IntValue < cmp_value.column_value.IntValue;
			break;
		case LE:
			return cc.column_value.IntValue <= cmp_value.column_value.IntValue;
			break;
		case E:
			return cc.column_value.IntValue == cmp_value.column_value.IntValue;
			break;
		case NE:
			return cc.column_value.IntValue != cmp_value.column_value.IntValue;
			break;
		default:
			return false;
			break;
		}
		break;
	case Value_Type::F:
		switch (operator_type)
		{
		case B:
			return cc.column_value.FloatValue > cmp_value.column_value.FloatValue;
			break;
		case BE:
			return cc.column_value.FloatValue >= cmp_value.column_value.FloatValue;
			break;
		case L:
			return cc.column_value.FloatValue < cmp_value.column_value.FloatValue;
			break;
		case LE:
			return cc.column_value.FloatValue <= cmp_value.column_value.FloatValue;
			break;
		case E:
			return cc.column_value.FloatValue == cmp_value.column_value.FloatValue;
			break;
		case NE:
			return cc.column_value.FloatValue != cmp_value.column_value.FloatValue;
			break;
		default:
			return false;
			break;
		}
		break;
	case Value_Type::C:
		switch (operator_type)
		{
		case B:
			return std::string(cc.column_value.CharValue) > std::string(cmp_value.column_value.CharValue);
			break;
		case BE:
			return std::string(cc.column_value.CharValue) >= std::string(cmp_value.column_value.CharValue);
			break;
		case L:
			return std::string(cc.column_value.CharValue) < std::string(cmp_value.column_value.CharValue);
			break;
		case LE:
			return std::string(cc.column_value.CharValue) <= std::string(cmp_value.column_value.CharValue);
			break;
		case E:
			return std::string(cc.column_value.CharValue) == std::string(cmp_value.column_value.CharValue);
			break;
		case NE:
			return std::string(cc.column_value.CharValue) != std::string(cmp_value.column_value.CharValue);
			break;
		default:
			return false;
			break;
		}
		break;
	default:
		return false;
		break;
	}
	return false;
  }
};
//��interpreter���õ�����һ��sql��������Ϊ��ôһ���ṹ�壬����api�У����в�����ִ��
struct TB_Select_Info
{
	std::string table_name; //ѡ��ı���
	std::vector <std::string> name_select_column;//ѡ����ֶ�����
	std::vector<Cell_Compare> vec_cmp_cell;//ѡ������
};
//api�в���ִ�����Ժ󣬽��������һ��SelectPrintInfo �ṹ�壬���´���interpreter��printwindow���У������������Ĵ�ӡ
struct SelectPrintInfo
{
	std::string table_name;
	std::vector<std::string> name_selected_column;
	std::vector<std::pair<Key_Attr, FileAddress>> key_fd;  // keys �� fds �����Ŷ�Ӧ�Ĺؼ����Լ��ùؼ��ֶ�Ӧ�ļ�¼��ַ

};

/********************************************************************************/
//���¼�¼Ҫ�õ��Ľṹ��
typedef struct NewValue {
	std::string field;
	std::string value;
}NewValue;
//Expr ������ɾ��ʱ����Ľṹ��,���ڼ�¼ һ�� ���ʽ���磺  name = 'name1'    field op value
struct TB_Update_Info
{
	std::string table_name;
	std::vector<NewValue> field_value;  // �ֶΡ���ֵ
	std::vector<Expr> expr;             // ���µ��ֶ�����
};

#endif
