#pragma once
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  2 0 2 0 D B S - G R O U P W O R K             ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : MiniSql		                                               *
 *                                                                                             *
 *                    File Name : API.H                                                        *
 *                                                                                             *
 *                   Programmer : Fantast(Lvkesheng Shen)&& Yuzhe Luo                          *
 *                                                                                             *
 *                   Start Date : 06/08/2020                                                   *
 *                                                                                             *
 *                  Last Update : 06/21/2020			                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
 /*
 * Copyright C Fantast
 * https://github.com/Fantast416
 */
#ifndef __API_H__
#define __API_H__
#include "Record.h"
#include <algorithm>
#include"Global.h"
#include"Catlog.h"
#include"Error.h"
#include"BTree.h"
#include"Interpreter.h"
#include"Timer.h"
//class Cell_Compare;
//
////��¼���ҹؼ���
//enum Operator_Type { B, BE, L, LE, E, NE };
//Operator_Type GetOperatorType(std::string str);


///************************************************************************
//*    ��������
//************************************************************************/
//�������ݿ�
bool Create_DB(std::string db_name, CatalogPosition& cp);

//ѡ�����ݿ�
bool Use_DB(std::string db_name, CatalogPosition& cp);

//��ʾ��ǰ�������ݿ�
std::vector<std::string> Show_DB(CatalogPosition& cp);

//ɾ�����ݿ�
bool Drop_DB(std::string db_name, CatalogPosition& cp);

void Del_File(std::string folder_path);

//������
bool Create_Table(TB_Create_Info tb_create_info, std::string path = std::string("./"));
void Check_TB_Create_Info(const TB_Create_Info &tb_create_info);

//ɾ����
bool Drop_Table(std::string table_name, std::string path = std::string("./"));

//�����¼
bool Insert_Record(TB_Insert_Info tb_insert_info, std::string path = std::string("./"));
void Check_TB_Insert_Info(const TB_Insert_Info &tb_insert_info);
void Check_Unique_Info(std::string table_name, std::string column_name, std::string column_value, char KeyType, int column_id, std::string path = std::string("./"));
void Check_Type_I(std::string value);
void Check_Type_F(std::string value);

//��������
bool Create_Index(Idx_Create_Info idx_create_info, std::string path = std::string("./"));

//ɾ������
bool Drop_Index(std::string index_name, std::string path = std::string("./"));

//*******************����/����/ɾ����¼ʱ��Ҫ�õ�����غ���*****************************************************************************************************
// ��Χ���ҵ����ֶ� ���ز��ҵ�ֵ�������ļ��еĵ�ַ																											//**
//������ѡ��																																				//**
std::vector<std::pair<Key_Attr, FileAddress>> Condition_Select(Cell_Compare cell_compare, std::string table_name, std::string path = std::string("./"));	//**
//�ؼ���ѡ��																																				//**
std::vector<std::pair<Key_Attr, FileAddress>> Key_Select(Cell_Compare cell_compare, std::string table_name, std::string path = std::string("./"));			//**
//��Χѡ��																																					//**
std::vector<std::pair<Key_Attr, FileAddress>> Range_Select(Cell_Compare cell_compare, std::string table_name, std::string path = std::string("./"));		//**
////ȡ��ָ����ַ������																																		//**
Column_Head Get_DB_Record(std::string table_name, FileAddress file_address, std::string path = std::string("./"));											//**
																																							//**
////���ر��и����ֶ�����������																																//**
std::vector<std::pair<std::string, Column_Type>> GetColumn_Name_Type(std::string table_name, std::string table_path);										//**
Column_Type GetType(std::string name, std::vector<std::pair<std::string, Column_Type>> pair);																//**
//**************************************************************************************************************************************************************

//*******************���Ҽ�¼*******************************************************************************
SelectPrintInfo Select_Record(TB_Select_Info tb_select_info, std::string path = std::string("./")); //	****
void Check_TB_Select_Info(const TB_Select_Info& tb_select_info);//�������Ϣ���						****
//**********************************************************************************************************


//*******************ɾ����¼*******************************************************************************
bool Delete_Record(TB_Delete_Info tb_delete_info, std::string path = std::string("./"));			//	****
void Check_TB_Delete_Info(const TB_Delete_Info &tb_delete_info);                                    //  ****
//**********************************************************************************************************


//*******************���¼�¼ ******************************************************************************
bool Update_Record(TB_Update_Info tb_update_info, std::string path = std::string("./"));			//	****
//**********************************************************************************************************


#endif