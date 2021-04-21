#pragma once
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  2 0 2 0 D B S - G R O U P W O R K             ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : MiniSql		                                               *
 *                                                                                             *
 *                    File Name : Timer.H                                                      *
 *                                                                                             *
 *                   This Work is Programmed by four person in the DBS CLASS                   * 
 *                  Shen Lvkesheng /Lu Ziyi/ Xing Shuting/Luo Yuzhe                            *
 *                                                                                             *
 *                   Start Date : 06/08/2020                                                   *
 *                                                                                             *
 *                  Last Update : 06/21/2020			                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <chrono>
#include <iomanip>
#include <direct.h>
#include <cstdlib>
#include "Error.h"

//#define NDEBUG
using namespace std::chrono;
enum class Column_Type { I, C, F,Error=-1};
enum class CmdType
{
	TABLE_CREATE, INDEX_CREATE, TABLE_DROP, INDEX_DROP, TABLE_SHOW, TABLE_SELECT, TABLE_INSERT, TABLE_UPDATE, TABLE_DELETE,
	DB_CREATE, DB_DROP, DB_SHOW, DB_USE,
	QUIT, HELP,
	FILE

};

constexpr int RecordColumnCount = 48;              // ��¼�ֶ���������,���������ֶζ����ַ����飬һ���ַ������ֶ���Ҫ4���ַ�->CXXX
constexpr int ColumnNameLength = 16;                   // �����ֶ����Ƴ�������
constexpr int bptree_t = 40;                            // B+tree's degree, bptree_t >= 2
constexpr int MaxKeyCount = 2 * bptree_t;              // the max number of keys in a b+tree node
constexpr int MaxChildCount = 2 * bptree_t;            // the max number of child in a b+tree node

/********************************************************  Buffer Module  ***************************************************************/

constexpr int PageSize = 8192;	                    // �ڴ�ҳ(==�ļ�ҳ)��С
constexpr int MemPageCount = 4096;                    // �ڴ�ҳ����
constexpr int MAX_FILENAME_LEN = 256;                   // �ļ���������·������󳤶�

std::string tolower(std::string str);

#endif