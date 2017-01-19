#include "VirtualMemory-detector.h"
void ErrorMessage(DWORD error)
// ������� � �������� ��������� �������� ��� ��������� ������
// � �������� ������������ �������� ���������� ��������� � ��������� ������
// �������������� ��������� � ������� ������� FormatMessage
{
	//������ ������ ������ ��� �������� ���� ������
	char* cFail = new char[100];

	FormatMessageA(										//������� ����������� ������ ��������� 			
		FORMAT_MESSAGE_FROM_SYSTEM, 				    //���� � ��������� ������� ������� ��������� ������������� ��������� 
		NULL,											//������������ ����������� ���������, ������������ 			
		error,											//������������� ��������� 			
		0,												//������������� �����  			
		cFail,											//�������� ��������� �� ����� <64 KB 			
		100, 											//������������, ���������� ������ ������ 			
		NULL);											//������ ��������, ������������ ��� ������� � ���������			 
	cout << "��� ������:" << error << " - " << cFail << endl;
	delete[]cFail;

}
HANDLE CrFile(char *cBuf)
//�������� ����������� �����, �������� ������� �������� ��������� �� ������, � ������� ������ ���� � ������������ �����
{
	HANDLE FileHandle;										     //�������� ���������� ��� ���������� �����

	FileHandle = CreateFileA(cBuf,								//� �������� ������� ��������� ���������� ���� � ������������ �����, �� ��������� ������������ ���������� ������� ���������
		GENERIC_READ | GENERIC_WRITE,							//��� ������� � ������� (����������� �� ������ ��� ������)
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, //����� ����������� �������(����������� ����������� �������� ������, ������ � �������� �������)
		NULL,													//������ �������� �������� �� ��������� ���������� ������������
		OPEN_ALWAYS,											//����������� �������� � ������(��������� ����� ����)
		FILE_ATTRIBUTE_NORMAL,									//���� �� ����� ����������������� ���������
		0);														//���� �������, ������� ������������� �������� ����� � �������������� �������� ��� ������������ �����
	if (FileHandle == INVALID_HANDLE_VALUE)						//���� �������� ������ � ������������ ��������� ������� CreateFile
		return INVALID_HANDLE_VALUE;											  //������������ ��� ������

	return FileHandle;										  //����� ������������ ����������
}
string WriteInFile(HANDLE hFile)
{
	string sBuf;																	  //����� ��� ������ ��������, ����������� � ���� 
	char *k = new char[5];															  //���������� ��� ��������� ������
	for (int i = 0; i < 5; i++)													  
	{																				  
		_itoa_s(i,																	   //������, ������� ����� ������������
			k,																		   //����������, ���� ������������ �����
			5, 																		   //������ ������		
			10);														  			   //������� ����������
		sBuf += k;																	  
	}

	if (!WriteFile(hFile,															 //����������
		sBuf.c_str(),																 //����� ������������ ����������
		sBuf.size(),															     //������ ������
		NULL, 																		 //����� ���������� ������
		NULL))
	{            																     //���� �� ������� ����������� ����, �� ���������� �������� ���������� �� ������� 
		ErrorMessage(GetLastError());												 //��������� ������ API
		return 0;
	}
	delete[]k;																		  //������������ ������
	return sBuf;
}																					  
HANDLE CreateMapping(HANDLE FileHandle)
/*������� ������� ��� ��������� ������ ������������� � ������ (�������������) ����� ��� ��������� �����.*/
{
	HANDLE hFileMapping;
	hFileMapping = CreateFileMapping(FileHandle, //���������� �����
		NULL,									 //������
		PAGE_READWRITE,							 //�������� ������
		NULL,									 //������� �����
		NULL,									 //������� �����
		_T("FileNameOject")						 //��� ������������� �����
		);									 
	if (!hFileMapping)							 //�������� �� ������
	{
		ErrorMessage(GetLastError());
		return INVALID_HANDLE_VALUE;
	}
	return hFileMapping;
}
BOOL HClose(HANDLE handleF)
//��������� ����������
{
	if (!CloseHandle(handleF))
	{
		ErrorMessage(GetLastError());
		return FALSE;
	}
	return TRUE;
}
LPVOID MapViewFile(HANDLE hFileMapping)
//������� ���������� ������������� ������������� ����� � �������� ������������ ����������� ��������
{
	LPVOID lpAddress;
	lpAddress = MapViewOfFile(hFileMapping, //���������� ������������� �����
		FILE_MAP_ALL_ACCESS,			   //����� �������
		0, 								   //������� �����
		0,								   //������� �����
		0);						   		   //����� ������������ ������(0 - ���� ����)
	if (!lpAddress)						   //�������� �� ������
	{
		ErrorMessage(GetLastError());
		return 0;
	}
	return lpAddress;
}
BOOL UnMapViewFile(LPVOID lpAddress)
//������� �������� ����������� ������������� ����� �� ��������� ������������ ����������� ��������
{
	if (!UnmapViewOfFile(lpAddress))
	{
		ErrorMessage(GetLastError());
		return FALSE;
	}
	return TRUE;
}
void ImportFunctionFromDLL(HMODULE lpBaseAddress)
{
	SYSTEM_INFO system_info;
	importFunction getSysInfo;
	getSysInfo = (importFunction)GetProcAddress(lpBaseAddress, "GetNativeSystemInfo");
	if (getSysInfo == NULL)
	{
		ErrorMessage(GetLastError());
		return;
	}
	else
		//�������� ������ �������
	{
		getSysInfo(&system_info);
		cout << "(DLL) �� �������." << endl;
	}
}
//������� �������
void storozhVM()
{
	HANDLE hFile, hFileMapping;
	LPVOID lpBaseAddressFile;
	string sContain;
	HMODULE  hProcessDLL;
	//����
	hFile = CrFile("123.txt");
	sContain = WriteInFile(hFile);
	hFileMapping = CreateMapping(hFile);
	if (!CloseHandle(hFile))
	{
		ErrorMessage(GetLastError());
		return;
	}
	//DLL
	hProcessDLL = GetModuleHandle(L"kernel32.dll");
	//�������� �� ������������
	while (1)
	{
		lpBaseAddressFile = MapViewFile(hFileMapping);
		if (strcmp(sContain.c_str(),(char*)lpBaseAddressFile)!= 0)
			cout << "������ �������� �����!" << endl;
		else
			cout << "(FILE) �� �������." << endl;
		if (!UnMapViewFile(lpBaseAddressFile))
		{
			ErrorMessage(GetLastError());
			return;
		}
		Sleep(100);
		
		ImportFunctionFromDLL(hProcessDLL);
		Sleep(100);
	}
	if (!FreeLibrary(hProcessDLL))
	{
		ErrorMessage(GetLastError());
		return;
	}
	if (!CloseHandle(hFileMapping))
	{
		ErrorMessage(GetLastError());
		return;
	}
	
}