#pragma once
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  2 0 2 0 D B S - G R O U P W O R K             ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : MiniSql		                                               *
 *                                                                                             *
 *                    File Name : BufferManager.H                                              *
 *                                                                                             *
 *                   Programmer : Fantast(Lvkesheng Shen)                                      *
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

#ifndef _BUFFER_H_
#define _BUFFER_H_
#include<sys/types.h>
#include<sys/stat.h>
#include <vector>
#include "global.h"
#include "Error.h"
#include <io.h>
#include <fcntl.h>

#define PageHeadSize 8   // Ϊ8���ֽ� 
#define FileHeadSize 540   // Ϊ540���ֽ�
#define FileAddressSize 8 // Ϊ8���ֽ�
//һ���ļ��������ҳ����һ���ļ�ͷ��ÿһҳ��һ��ҳͷ
//�ļ�ͷ�����ڵ�һ���ļ�ҳ��
class Clock;
class Buffer;
Clock* GetGlobalClock(); //��ȡȫ�ֵ��ڴ�ҳ������ķ���
Buffer& GetGlobalBuffer();
class FileHead;
class PageHead {
public:
	void SetPageId(unsigned long PageId);                         //�ļ�ҳͷ  �ļ�ͷ  
	void SetFixed(bool IsFixed);
	void Initialize();
private:
	unsigned long PageId;
	bool IsFixed;  //�ڴ�ҳ�Ƿ�����������������򲻻ᱻ�ر�
};

class Page {
	friend class Buffer;
	friend class Clock;
	friend class File;
public:
	PageHead* Pagehead;                // ҳͷָ��
	FILE* FilePointer;                 // �ļ�ָ�룬������ΪNULLʱ����ҳ������
	unsigned long FilePageID;          // �ļ�ҳ��
	mutable bool IsLastUsed;           // ���һ�η����ڴ��Ƿ�ʹ�ã�����Clock�㷨
	mutable bool IsModified;           // �Ƿ���ҳ
	void *PtrtoPageBeginAddress;       // �ڴ���ʵ�ʱ��������ļ����ݵ���ʼ��ַ
public:
	//function()
	void WriteMemToFile() const;            // ���ڴ��е�ҳд�ص��ļ���
	FileHead* GetFileHead();           // ��ȡ�ļ�ͷָ�루���ļ�ҳΪ0��ʱ����ô˺����������ض�Ӧ���ļ�ͷָ�룬��ָ���ļ�ͷ��ʼ�ĵ�ַ��
	Page();
	~Page();
	void SetModified(bool State);
	void SetLastUsed(bool State);

};

const unsigned int FILECOND_RESERVE_SPACE = 512;  // �ļ�ͷԤ���ռ�

class FileAddress {         //�ļ���ĳ���ݵĵ�ַ = �ļ���ҳ��� + ҳ��ƫ���� 
	friend class File;
public:
	void SetFileAddress(unsigned long FilePageID, unsigned int  OffSet);   //����FileAddress�ı���ֵ
	bool operator==(const FileAddress &rhs) const;
	bool operator!=(const FileAddress &rhs) const;
	bool operator<(const FileAddress &rhs)const;
	bool operator>(const FileAddress &rhs)const;
	unsigned long FilePageID;     // �ļ�ҳ���
	unsigned int  OffSet;         // ҳ��ƫ����

};

class FileHead {
public:
	void Initialize();                           //��ʼ���µ��ļ����ļ�ͷ��Ϣ
	FileAddress DelFirst;                         // ��һ����ɾ����¼��ַ  
	FileAddress DelLast;                          // ���һ����ɾ����¼��ַ  
	FileAddress NewInsert;                        // �ļ�ĩβ�ɲ��������ݵĵ�ַ

	unsigned long TotalPage;                     // Ŀǰ�ļ��й���ҳ��
	char Reserve[FILECOND_RESERVE_SPACE];         // Ԥ���ռ�
};
#define MAX_FILENAME_LEN 100
class File {
	friend class Buffer;
	friend class Page;
public:
	char* GetFileName();
	char FileName[MAX_FILENAME_LEN];
	FILE* FilePointer;                                                 // �ļ�ָ��
	unsigned long TotalPage;                                      // Ŀǰ�ļ��й���ҳ��

//function()
	File(const char *FileName, FILE* FilePointer);
	Page* AddExtraPage();                                       // ��ǰ�ļ����һҳ�ռ�
	Page* GetFileFirstPage();                                    // �õ��ļ���ҳ

	void* ReadMem(FileAddress *ReadAddress);                           // ��ȡ�ڴ��ļ�,���ض�ȡλ��ָ��
	FileAddress WriteMem(const void* Source, unsigned int Length);           // �ڿ�д���ַд������
	FileAddress WriteMem(const void* Source, unsigned int Length, FileAddress* Dest);
	//void WipeMem(void*Source, unsigned int WipeSize, FileAddress *WipePos);

public:  //����ӿں���
	const void* ReadRecord(FileAddress *RecordAddress)const;         // ��ȡĳ����¼,���ؼ�¼ָ��(������¼��ַ����)
	//void* ReadWriteRecord(FileAddress *RecordAddress);              // ��ȡĳ����¼,���ؼ�¼ָ��(������¼��ַ����)
	FileAddress AddRecord(const void* RecordSource, unsigned int SizeofRecord);                    // ���ؼ�¼����ӵ�λ��
	FileAddress DeleteRecord(FileAddress *DeleteAddress, unsigned int SizeofRecord);               // ����ɾ����λ��
	bool UpdateRecord(FileAddress *DeleteAddress, void *UpdateRecordData, unsigned int SizeofRecord);

};


class Clock {  //�ڴ�ҳ������
	friend class File;
	friend class Buffer;
public:
	Clock();
	~Clock();
	Page* MemPages[MemPageCount + 1];  // �ڴ�ҳ��������  MemPageCount���ڴ�ҳ����

//function()
	Page* GetMemAddr(FILE* FilePointer, unsigned long FilePageID);  //��������ļ�ҳ��ĳһ�����ļ�ҳ���ļ�ָ���ҳ��ű�ʾ�����ڴ��еĵ�ַ

	// ����һ�����滻���ڴ�ҳ����
	// ԭҳ�����ݸ�д����д��
	unsigned int GetUseablePage();

	// ������ҳ�������ڴ������ļ����������ҳ�������
	Page* CreateNewPage(FILE* FilePointer, unsigned long FilePageId);
};

class Buffer {       //��д�ļ��õ���ײ���
	friend class Clock;

public:
	~Buffer();
	File* operator[](const char *fileName);      // ���ļ�����ʧ�ܷ��� nullptr
	void CreateFile(const char *FileName);
	File* GetFile(const char *FileName);

	void CloseFile(const char *FileName);
	void CloseAllFile();
private:
	std::vector<File*> MemFiles;  // �����Ѿ��򿪵��ļ��б�,������ļ�ָ��

};

#endif