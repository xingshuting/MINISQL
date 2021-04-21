#pragma once
#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__
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
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include "API.h"
#include "Error.h"
//#include"Catalog.h"
using namespace std;

// ��ӡ�����д��ڣ�ʹ�ײ�ʵ�ֺ�GUI���룬������չ
#define PRINTLENGTH 63
class PrintWindow
{
public:
	void CreateTable(bool is_created);
	void CreateIndex(bool is_created);
	void DropIndex(bool is_dropped);
	void DropTable(bool is_dropped);

	void SelectTable(SelectPrintInfo select_table_print_info);
	void InsertRecord(bool is_inserted);
	void UpdateTable(bool isUpdated);
	void DeleteTable(bool isDeleted);
	void ShowAllTable(vector<string> sen_str, string path);

	void CreateDB(bool is_created);
	void DropDB(bool is_dropped);
	void ShowDB(std::vector<std::string> db_names);
	void UseDB(bool isUsed);


private:
	void Print(int len, string s); // ��ӡ |xxxx        | ���������ڳ���Ϊ len
	int GetColumnLength(std::string name, std::vector<std::string> col_name, std::vector<int> col_len);

};

/***********�������ַ�������Ϊ�����ִ�***************/
class SensefulStr
{
public:
	SensefulStr();
	void SetStr(string _srcstr);
	vector<string> GetSensefulStr()const;

private:
	void Parse();                                             // �����ַ���
	string src_str;                                           // ԭʼ�ַ���
	vector<string> sen_str;                                   // �������ַ���
	string key_char = ";,()=<>\012\015\040";
	bool IsKeyChar(char c);
};

// ������Ľ����ӿ� ����
void Interpreter(vector<string> sen_str, CmdType cmd_type, PrintWindow print_window);

CmdType GetOpType(vector<string> sen_str);

//// ����������Ϣ
//bool CreateShowTableInfo(std::vector<std::string> sen_str);
TB_Create_Info CreateTableInfo(std::vector<std::string> sen_str);          //�������������ַ����У��γ��´����ı�Ľṹ�壬������һ�㺯��
std::string DropTableInfo(std::vector<std::string> sen_str);  //ɾ��һ�ű�

TB_Insert_Info CreateInsertInfo(std::vector<std::string> sen_str);

TB_Select_Info TableSelectInfo(std::vector<std::string> sen_str);
// �Ƚϵ��ֶ����ƣ��Ƚϵ��ֶ����ͣ��ȽϹ�ϵ���Ƚϵ�ֵ
Cell_Compare CreateCmpCell(std::string column_name, Column_Type column_type, Operator_Type Optype, std::string value);

//���ɱ�ɾ���ṹ��
TB_Delete_Info TableDeleteInfo(std::vector<std::string> sen_str);

//���ɱ���½ṹ��
TB_Update_Info TableUpdateInfo(std::vector<std::string> sen_str);


//
//���ݿ������Ϣ����
string ShowDbInfo(std::vector<string> sen_str);// ��ʾ���ݿ�
string CreateDbInfo(std::vector<string> sen_str);// �������ݿ�,����Ҫ����������
string DeleteDbInfo(std::vector<string> sen_str);// ɾ�����ݿ�,����Ҫɾ��������
string UseDbInfo(std::vector<string> sen_str);// ʹ�����ݿ�

//������ز���
Idx_Create_Info CreateIndexInfo(std::vector<std::string> sen_str);
std::string DropIndexInfo(std::vector<std::string> sen_str);

#endif//__INTERPRETER_H__