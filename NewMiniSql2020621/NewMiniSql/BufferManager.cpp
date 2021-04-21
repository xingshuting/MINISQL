#include"BufferManager.h"
#include <iostream>
#include <string>
#pragma warning(disable:4996)
Clock* GetGlobalClock() {
	static Clock MemClock;
	return &MemClock;
}
Buffer& GetGlobalBuffer() {
	static Buffer MemBuffer;
	return MemBuffer;
}
File* Buffer::operator[](const char *fileName)
{
	return GetFile(fileName);
}

void PageHead::SetPageId(unsigned long PageId) {
	this->PageId = PageId;
}
void PageHead::SetFixed(bool IsFixed) {
	this->IsFixed = IsFixed;
}
void FileHead::Initialize() {
	this->TotalPage = 1;

	FileAddress temp1;      //���� DelFirst��DelLastλ�ó�ʼ��Ϊ0,0
	temp1.SetFileAddress(0, 0);
	this->DelFirst = temp1;
	this->DelLast = temp1;

	FileAddress temp2;    //���ÿ��Բ����¼��λ��Ϊ���ļ�ҳ���ļ�ͷ��ҳͷ֮���ƫ������λ��
	temp2.SetFileAddress(0, PageHeadSize + FileHeadSize);
	this->NewInsert = temp2;
	//�������ռ��ڴ���Ϊ0���Դ�����ʹ��
	memset(this->Reserve, 0, FILECOND_RESERVE_SPACE);
}
void PageHead::Initialize() {
	this->PageId = 0;
	this->IsFixed = 1;
}
void Buffer::CreateFile(const char *FileName)
{
	// �ļ����� ����ʧ��
	if (fopen(FileName, "rb") != NULL)
	{
		throw Error("File Is Existed");
	}
	//�����ļ�
	FILE* NewFile = fopen(FileName, "wb+"); // �½��ļ�(���ļ�) 

	void *ptr = malloc(PageSize);    //PageSizeΪ1ҳ�ļ�ҳ���涨�Ĵ�С
	memset(ptr, 0, PageSize);
	PageHead* PageHeadPointer = (PageHead *)(ptr);
	FileHead* FileHeadPointer = (FileHead *)((char*)ptr + PageHeadSize);
	PageHeadPointer->Initialize();
	FileHeadPointer->Initialize();
	// д��
	fwrite(ptr, PageSize, 1, NewFile);  /*size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
										ptr-- ����ָ��Ҫ��д���Ԫ�������ָ�롣
										size-- ����Ҫ��д���ÿ��Ԫ�صĴ�С�����ֽ�Ϊ��λ��
										nmemb-- ����Ԫ�صĸ�����ÿ��Ԫ�صĴ�СΪ size �ֽڡ�
										stream-- ����ָ�� FILE �����ָ�룬�� FILE ����ָ����һ���������*/
	fclose(NewFile);
	delete ptr;
	return;
}
char* File::GetFileName() {
	return this->FileName;
}
File* Buffer::GetFile(const char *FileName) {
	//�Ȳ鿴���ļ��Ƿ��Ѿ���
	for (int i = 0; i < this->MemFiles.size(); i++) {
		if ((strcmp(MemFiles[i]->GetFileName(), FileName) == 0))
			return MemFiles[i];
	}
	//��ѭ��������δreturn����˵�����ļ���δ�򿪣���򿪸��ļ�
	FILE* fp = fopen(FileName, "rb+");
	if (fp == NULL)
	{
		return NULL;
	}
	else {
		File* NewFile = new File(FileName, fp);
		//????��ȡ���̽�Ϊ����
		this->MemFiles.push_back(NewFile);
		return NewFile;
	}
}

File::File(const char *FileName, FILE* FilePointer)
{
	strcpy(this->FileName, FileName);
	this->FilePointer = FilePointer;
	this->TotalPage = GetGlobalClock()->GetMemAddr(this->FilePointer, 0)->GetFileHead()->TotalPage;
	//�˴�����ȫ��clock�࣬��ȡ���ļ��ĵ�0ҳ�ļ�ҳ���ڴ��еĶ�Ӧ�ڴ�ҳ��ָ�룬��ͨ���ڴ�ҳ�ҵ��ڴ�ҳҳͷ�ĵ�ַ���õ�TotalPage������ԣ�����ڴ���һ���ļ���ӳ��
}

Page* Clock::GetMemAddr(FILE* FilePointer, unsigned long FilePageID) {  //�ȴ��ļ�clock�в��Ҹ�ҳ�Ƿ��Ѿ����ڴ��У��������ֱ�ӻ�ȡ��������Ӵ��̶���

	Page* MemPage = NULL;
	//�������е��ڴ����Ѿ��е��ļ�ҳ,�����ļ�ҳ�Ƿ��Ѿ����ڴ���������
	for (int i = 1; i <= MemPageCount; i++)
	{
		if (MemPages[i] && MemPages[i]->FilePointer == FilePointer && MemPages[i]->FilePageID == FilePageID) {
			MemPage = MemPages[i];
			break;
		}
	}
	if (MemPage != NULL)
		return MemPage;

	// ���򣬴Ӵ�������
	unsigned int NewPage = GetUseablePage();   //��Clock����һ�������滻��ҳ����
	//����ҳ���ж�Ӧ�ĳ�ʼ����Ϣ��¼
	MemPages[NewPage]->FilePointer = FilePointer;
	MemPages[NewPage]->FilePageID = FilePageID;
	MemPages[NewPage]->IsModified = false;
	MemPages[NewPage]->IsLastUsed = true;

	int temp = fseek(FilePointer, FilePageID*PageSize, SEEK_SET);       // ��λ����Ҫȡ�����ļ�ҳ���׵�ַ

	//if (temp != 0) {
	//	printf("fseek Error");
	//	return NULL;
	//}

	long byte_count = fread(MemPages[NewPage]->PtrtoPageBeginAddress, PageSize, 1, FilePointer);          // �����ڴ���
	/*ע�⣺fread ���سɹ���ȡ�Ķ�������������ִ���򵽴��ļ�ĩβ�������С��count��
		��size��countΪ�㣬��fread�������Ҳ���������������
		fread�������ļ�β�ʹ�����˵����߱�����feof��ferror�����жϷ�����ʲô��*/
		/*	if (byte_count == 0) {
			printf("fseek Error");
			return NULL;
		}*/
	return MemPages[NewPage];

}

unsigned int Clock::GetUseablePage()  //��һ������������¼��ҳ
{
	int flag = 0;
	// �Ȳ�����û��û������ڴ�ҳ�������ֱ�������þ�����
	for (int i = 1; i <= MemPageCount; i++)
	{
		if (MemPages[i] == NULL)
		{
			MemPages[i] = new Page();
			return i;
		}
		if (MemPages[i]->FilePointer == NULL) {
			flag = i;                    //�˴���ʱ��¼һ�����������ڴ�ҳ�������������û�пյ��ڴ�ҳ�Ļ�����ʹ�ñ��������ڴ�ҳ�������ٲ���һ����
		}
	}

	//ִ�е��˴���û��δ������ڴ�ҳ�ˣ��ж�һ��֮ǰ��ѭ������û���ҵ����������ڴ�ҳ

	if (flag == 0 && MemPages[0]->FilePointer != NULL) {   //��flagû�䣬�����ڴ�ҳ�е�0ҳδ��������˵����û���Ѿ����������ڴ�ҳ,Ҫʹ���û��㷨��

		//unsigned int i = ClockSwap();  //�����û��㷨�ҵ�һ�����û����ڴ�ҳ���±�
		unsigned int i = rand() % MemPageCount;   //Ŀǰ������û� 

		if (i == 0)i++;  //����պô��������˵�0ҳ�Ļ��ǲ��еģ�������1ҳ����Ϊʲô���У�ΪʲôҪ�ƣ���

		MemPages[i]->WriteMemToFile();  //������±��Ӧ���ڴ�ҳд���ļ���

		return i;

	}
	else {  //�б��������ڴ�ҳ
		return flag;     //�����������ڴ�ҳ����������������
	}
}

void Page::WriteMemToFile() const {       // ���ڴ��е�ҳд�ص��ļ���

		// ֻ����ҳ��Ҫд�أ���������ҳ��ҳ�ǲ���Ҫд�ص�
	if (this->IsModified && this->FilePointer != NULL)
	{
		int temp = 0;
		temp = fseek(this->FilePointer, this->FilePageID*PageSize, SEEK_SET);  //������ļ���ʼ��������ļ�ҳ * �ļ�ҳ��С���͵��˵�ǰ��Ҫд�ص��ļ�ҳ��ʼ

		//if (temp != 0) {
		//	printf("fseek Error");
		//	return;
		//}

		temp = fwrite(this->PtrtoPageBeginAddress, PageSize, 1, this->FilePointer); // д���ļ�
		//for (int i = 0; i < PageSize; i++) {
		//	printf("%2X", *((char*)(this->PtrtoPageBeginAddress)+i));
		//}
		//fflush(this->FilePointer);
	/*	if (temp != PageSize) {
			printf("WriteBack Error");
			return;
		}*/
		this->IsModified = false;
		this->IsLastUsed = true;
	}

}

FileHead* Page::GetFileHead() {
	return (FileHead*)((char*)this->PtrtoPageBeginAddress + PageHeadSize);
}

void Buffer::CloseFile(const char *FileName) {
	// ��Ҫ���ڴ������б�����ļ���ҳȫ��д��
	File* MemFile = GetFile(FileName);       //��ͨ���ļ����õ����ļ����ڴ��е�һϵ����Ϣ
	Clock* TempClock = GetGlobalClock();
	//���������ڴ�ҳ����������Щ�ڴ�ҳ���������Ҫд�ص��ļ���,������д��
	for (int i = 1; i <= MemPageCount; i++)
	{
		if (TempClock->MemPages[i] && TempClock->MemPages[i]->FilePointer == MemFile->FilePointer)
		{
			TempClock->MemPages[i]->WriteMemToFile();
			TempClock->MemPages[i]->IsLastUsed = 0;
			TempClock->MemPages[i]->IsModified = false;
			TempClock->MemPages[i]->FilePointer = NULL;
		}
	}
	std::vector<File*>::iterator it;
	//��Clock��¼�����Ѿ��򿪵��ļ���vector�У��ҵ�Ҫ�رյ��ļ��������رգ����Ұ�����vector���޳�
	for (it = MemFiles.begin(); it != MemFiles.end();)
	{
		if (strcmp((*it)->FileName, FileName) == 0)
		{
			fclose((*it)->FilePointer);
			delete (*it);
			MemFiles.erase(it);
			break;
		}
		it++;
	}
}
void Buffer::CloseAllFile() {
	int Len = this->MemFiles.size();
	for (int i = 0; i < Len; i++) {
		this->CloseFile(MemFiles[0]->FileName);
	}
}
Buffer::~Buffer()
{
	this->CloseAllFile();
}
Page* File::GetFileFirstPage()
{
	return GetGlobalClock()->GetMemAddr(this->FilePointer, 0);  // ��Clock���������ļ�ָ�룬��ȡ�����ļ����ڴ��ַ�ĵ�һҳ ��ҳ 
}

Page* File::AddExtraPage()
{
	Clock *MemClock = GetGlobalClock();
	//��ȡ�ļ���ҳ,���������Ϣ 
	Page* FirstPage = this->GetFileFirstPage();
	this->TotalPage = FirstPage->GetFileHead()->TotalPage + 1;  //��ȡ��һҳ��ҳͷ���ֵĸ��ļ�����ҳ��+1����Ϊ���ļ�����ҳ��
	FirstPage->GetFileHead()->TotalPage += 1; //ҳͷ�е���ϢҲ��Ҫ����
	FirstPage->SetModified(true);
	FirstPage->SetLastUsed(true);

	//�������ڴ�ҳ���ҷ���
	Page * NewPage = MemClock->CreateNewPage(this->FilePointer, FirstPage->GetFileHead()->TotalPage - 1);
	NewPage->SetModified(true);
	NewPage->SetLastUsed(true);

	return NewPage;
}

Page* Clock::CreateNewPage(FILE* FilePointer, unsigned long FilePageId)
{
	// ��ʼ���µ��ڴ�ҳ����
	int index = GetUseablePage();   //�ҵ�����ҳ
	memset(MemPages[index]->PtrtoPageBeginAddress, 0, PageSize);  //���ÿ���ҳ�����ݴ洢��������Ϊ0
	MemPages[index]->FilePointer = FilePointer;
	MemPages[index]->FilePageID = FilePageId;
	MemPages[index]->SetModified(true);  // ��ҳ����Ϊ��ҳ����Ҫд��

	// ��ʼ����ҳ��ҳͷ��Ϣ
	MemPages[index]->Pagehead->SetPageId(FilePageId); //����ҳͷ�� ҳ��id��Ϣ
	if (FilePageId != 0)
	{
		MemPages[index]->Pagehead->SetFixed(false);
	}
	else
	{
		MemPages[index]->Pagehead->SetFixed(true);     //����µ�ҳ��һ���ļ�����ҳ�Ļ���������������ڴ��У����ᱻ�ͷ�д��
		MemPages[index]->GetFileHead()->Initialize();  //�ٰ�����ļ�ҳ���ļ�ͷ����ϢҲ���г�ʼ��
	}
	return MemPages[index];

}

Clock::Clock()
{
	for (int i = 0; i <= MemPageCount; i++)
	{
		MemPages[i] = NULL;
	}
}

Clock::~Clock()
{
	for (int i = 0; i <= MemPageCount; i++)
	{
		if (MemPages[i] != nullptr)
			delete MemPages[i];
	}
}

void Page::SetModified(bool State) {
	this->IsModified = State;
}
void Page::SetLastUsed(bool State) {
	this->IsLastUsed = State;
}

void* File::ReadMem(FileAddress *ReadAddress) {  //��ȡ�ڴ��е��ļ������ݶ�Ӧ���ļ���ַ��,���ҷ���һ��ָ�룬ָ����ļ����ڴ��ַ
	Page* MemPage = GetGlobalClock()->GetMemAddr(this->FilePointer, ReadAddress->FilePageID);
	MemPage->SetLastUsed(true);
	return (char*)MemPage->PtrtoPageBeginAddress + ReadAddress->OffSet;
}

FileAddress File::WriteMem(const void* Source, unsigned int Length, FileAddress* Dest) {           // ��Dest��ַ��д�����ݣ����ҷ����ļ���ĳ����ַ����
	Page* MemPage = GetGlobalClock()->GetMemAddr(this->FilePointer, Dest->FilePageID);             //���ص�����ʵ�ļ���ַ��д�����ݺ�ĵ�ַ
	// �����ҳʣ��ռ䲻��
	if ((PageSize - Dest->OffSet) < Length)
	{
		return *Dest;
	}

	memcpy((void*)((char*)MemPage->PtrtoPageBeginAddress + Dest->OffSet), Source, Length);
	MemPage->SetLastUsed(true);
	MemPage->SetModified(true);

	//dest->offSet += length;
	FileAddress fd;
	fd.SetFileAddress(Dest->FilePageID, Dest->OffSet + Length);
	return fd;

}
// �ڿ�д���ַд�����ݣ����ռ䲻���������µ�ҳ, ��������д��ĵ�ַ
FileAddress File::WriteMem(const void* Source, unsigned int Length)
{
	// �Ӹ��ļ����ļ�ͷ�л�ȡ���ļ��п���д���ַ
	FileAddress InsertPos = GetGlobalClock()->GetMemAddr(this->FilePointer, 0)->GetFileHead()->NewInsert;

	// д��
	FileAddress write_res = WriteMem(Source, Length, &InsertPos);
	if (write_res.FilePageID == InsertPos.FilePageID && write_res.OffSet == InsertPos.OffSet)  //�ռ䲻�㣬��Ҫ�����µ�ҳ
	{
		this->AddExtraPage();
		InsertPos.SetFileAddress(InsertPos.FilePageID + 1, PageHeadSize);
		write_res = WriteMem(Source, Length, &InsertPos);  // ����д��
	}

	// ���¿�д��λ��
	GetGlobalClock()->GetMemAddr(this->FilePointer, 0)->GetFileHead()->NewInsert = write_res;
	GetGlobalClock()->GetMemAddr(this->FilePointer, 0)->SetModified(true);
	GetGlobalClock()->GetMemAddr(this->FilePointer, 0)->SetLastUsed(true);
	return InsertPos;
}
void FileAddress::SetFileAddress(unsigned long FilePageID, unsigned int  OffSet) {
	this->FilePageID = FilePageID;
	this->OffSet = OffSet;
}

//void File::WipeMem(void* Source, unsigned int WipeSize, FileAddress *WipePos)
//{
//	auto pMemPage = GetGlobalClock()->GetMemAddr(this->FilePointer, WipePos->FilePageID);
//	// wipe
//	memcpy((char*)pMemPage->PtrtoPageBeginAddress + WipePos->OffSet, Source, WipeSize);
//	pMemPage->SetModified(true);
//	pMemPage->SetLastUsed(true);
//}

bool FileAddress::operator==(const FileAddress &rhs) const
{
	return (this->FilePageID == rhs.FilePageID && this->OffSet == rhs.OffSet);
}
bool FileAddress::operator!=(const FileAddress &rhs) const
{
	return !(this->FilePageID == rhs.FilePageID && this->OffSet == rhs.OffSet);
}
bool FileAddress::operator<(const FileAddress &rhs)const
{
	return (this->FilePageID < rhs.FilePageID) || ((this->FilePageID == rhs.FilePageID) && (this->OffSet < rhs.OffSet));
}
bool FileAddress::operator>(const FileAddress &rhs)const
{
	return (this->FilePageID > rhs.FilePageID) || ((this->FilePageID == rhs.FilePageID) && (this->OffSet > rhs.OffSet));
}


Page::Page()
{
	this->PtrtoPageBeginAddress = malloc(PageSize);
	this->Pagehead = (PageHead*)(this->PtrtoPageBeginAddress);
	this->FilePointer = NULL;
	this->SetModified(false);
	this->SetLastUsed(false);
}

Page::~Page()
{
	// ��ҳ�Ҳ���������ҳ��Ҫд��
	if (this->IsModified && this->FilePointer != NULL)
		this->WriteMemToFile();
	delete this->PtrtoPageBeginAddress;
}


const void* File::ReadRecord(FileAddress *RecordAddress)const {
	Page* MemPage = GetGlobalClock()->GetMemAddr(this->FilePointer, RecordAddress->FilePageID);    //���ڴ���ͨ���ļ�ָ���ҳ�ţ��ҵ�Ҫ��ѯ�ļ�¼���ڴ�ҳ��ָ���ַ
	return (char*)(MemPage->PtrtoPageBeginAddress) + RecordAddress->OffSet;  //Ȼ���ô��ڴ�ҳ������¼�Ĵ����ݵĵ�ַ + ҳ������ƫ����  ==  ĳ����¼�ĵ�ַ
}

//void* File::ReadWriteRecord(FileAddress *RecordAddress) {                   // ��ȡĳ����¼,���ؼ�¼ָ��(������¼��ַ����)
//	Page* MemPage = GetGlobalClock()->GetMemAddr(this->FilePointer, RecordAddress->FilePageID);
//	MemPage->SetModified(true);
//	return (char*)(MemPage->PtrtoPageBeginAddress) + RecordAddress->OffSet;
//}


// ��������Ӽ�¼�ĵ�ַquit
FileAddress File::AddRecord(const void* const Source, unsigned int SizeofRecord)
{
	Page* MemPage = GetGlobalClock()->GetMemAddr(this->FilePointer, 0);
	FileHead* Filehead = MemPage->GetFileHead();
	FileAddress fd; // AddRecordд���λ��
	void *Temp;
	if (Filehead->DelFirst.OffSet == 0 && Filehead->DelLast.OffSet == 0)
	{
		// û�б�ɾ�����Ŀ���ռ䣬ֱ�����ļ�β��������
		// ����ӵ��µ�ַ��Ϊ��¼���ݵ�һ����д��
		Temp = malloc(sizeof(FileAddress) + SizeofRecord);
		memcpy(Temp, &Filehead->NewInsert, sizeof(FileAddress));              //Temp == (��¼���ļ�ĩβ�ɲ��������ݵ�FileAddress)�ĵ�ַ + ��¼������
		memcpy((char*)Temp + sizeof(FileAddress), Source, SizeofRecord);
		FileAddress Pos = WriteMem(Temp, SizeofRecord + sizeof(FileAddress));  //����¼��Ϣд����ڴ��ļ��п���Ĳ���,���ҷ���д��������ļ��еĵ�ַ��Ϣ
		WriteMem(&Pos, sizeof(FileAddress), &Pos);  //��Pos�а��µ�ַҲ��Ϊ��¼���ݵ�һ����д�� ��
		fd = Pos;                //��������
	}
	else if (Filehead->DelFirst == Filehead->DelLast)
	{
		// �ڵ�һ����ɾ�������ݴ������������
		Temp = malloc(SizeofRecord + sizeof(FileAddress));
		memcpy(Temp, &Filehead->DelFirst, sizeof(FileAddress));
		memcpy((char*)Temp + sizeof(FileAddress), Source, SizeofRecord);
		WriteMem(Temp, SizeofRecord + sizeof(FileAddress), &Filehead->DelFirst);
		fd = Filehead->DelFirst;
		Filehead->DelFirst.OffSet = 0;
		Filehead->DelLast.OffSet = 0;
	}
	else
	{
		FileAddress Pos = Filehead->DelFirst;
		fd = Filehead->DelFirst;
		Filehead->DelFirst = *(FileAddress*)ReadMem(&Filehead->DelFirst);

		Temp = malloc(SizeofRecord + sizeof(FileAddress));
		memcpy(Temp, &Pos, sizeof(FileAddress));
		memcpy((char*)Temp + sizeof(FileAddress), Source, SizeofRecord);
		WriteMem(Temp, SizeofRecord + sizeof(FileAddress), &Pos);  //������д�����ɾ���ļ�¼��
	}
	delete Temp;
	MemPage->SetModified(true);
	return fd;
}

FileAddress File::DeleteRecord(FileAddress *DeleteAddress, unsigned int SizeofRecord)
{
	Page* MemPage = GetGlobalClock()->GetMemAddr(this->FilePointer, 0);
	FileHead* Filehead = MemPage->GetFileHead();

	// �����ɾ�����ݵ�ַ�ĵ�ַ��ʶ�ͱ����ַ���ȣ������Ѿ�ɾ���������� 
	//���ص�fd����Ӧ���Ƿ��ص�һ���ڴ�ĵ�ַ
	FileAddress fd = *(FileAddress*)ReadMem(DeleteAddress);
	if (fd != *DeleteAddress)
	{
		FileAddress tmp;
		tmp.SetFileAddress(0, 0);
		return tmp;
	}
	else if (Filehead->DelFirst.OffSet == 0 && Filehead->DelLast.OffSet == 0)  // ֮ǰû��ɾ������¼
	{
		Filehead->DelFirst = Filehead->DelLast = *DeleteAddress;
		FileAddress tmp;
		tmp.SetFileAddress(0, 0);
		WriteMem(&tmp, sizeof(FileAddress), &Filehead->DelLast);
	}
	else
	{
		// ɾ����¼
		WriteMem(DeleteAddress, sizeof(FileAddress), &Filehead->DelLast);
		Filehead->DelLast = *DeleteAddress;
		FileAddress tmp;
		tmp.SetFileAddress(0, 0);
		WriteMem(&tmp, sizeof(FileAddress), &Filehead->DelLast);
	}

	MemPage->SetModified(true);
	return *DeleteAddress;
}

bool File::UpdateRecord(FileAddress *address, void *UpdateRecordData, unsigned int SizeofRecord)
{
	auto MemPage = GetGlobalClock()->GetMemAddr(this->FilePointer, address->FilePageID);
	auto pdest = (char*)MemPage->PtrtoPageBeginAddress + address->OffSet + sizeof(FileAddress);
	memcpy(pdest, UpdateRecordData, SizeofRecord);
	MemPage->SetModified(true);
	return true;
}