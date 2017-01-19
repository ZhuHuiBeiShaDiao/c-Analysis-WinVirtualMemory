#include "VirtualMemory-scaner.h"
/*�����:��� �������, ������� ��������� COMMIT:

- �������� ����� ��������  �� �������� AllocationBase(�� ���� �� ��������, ������� ���������� � ���������� ������ �������
VirtualAlloc, �� ���� ��� ����������� �������� ���������� ������) ��� ����� ��������
- ��� ������ ������� �������� ���������� � �������� ������ AllocationBase;

- IMAGE: ������ ����� AllocationBase= (EXECUTE_...) , ������ ��� IMAGE - DLL&EXE, �������� ��� ������� ��������� ������� WRITE, ������������� � WRITECOPY
- MAPPED: �������� ��� ������� ��������� ������� EXECUTE(� ����� ����� ����������� ����� ����������)
- PRIVATE: �������� ��� ����� ������� ��������� �������, ����� �������� AllocationBase;
*/
void ErrorMessage(DWORD error)
/*
������� � �������� ��������� �������� ��� ��������� ������
� �������� ������������ �������� ���������� ��������� � ��������� ������
�������������� ��������� � ������� ������� FormatMessage
*/
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

}
void cinInputINT(int& i)
//������� ��������� �������� �������� �� ������������ ���� "int"
{
	cin.sync();
	cin >> i;
	if (!cin.good())
	{
		while (!cin)
		{
			cin.clear();
			cin.sync();
			cout << "������ ��� ����� ����������." << endl;
			cout << "������� ���������� ��������" << endl;
			cin >> i;
		}
	}
}
/*
(���� ������� 3 �������, ����� ������� ������������� �������� �������� ��������� �������� � ���������� ������
(�������������)
*/
char* GetMemState(DWORD dwState)

{
	char *chState = "Undefined";

	switch (dwState)
	{
	case MEM_FREE:   //�������� ��������, ���������� ��� ����������� ��������
		chState = "Free    ";
		break;
	case MEM_RESERVE://���������������, �� ��� ���������� ������
		chState = "Reserve ";
		break;
	case MEM_COMMIT: //�������� ��������, �� ������������ �������� ���������� ������
		chState = "Commit  ";
		break;
	}
	return chState;
}
char* GetMemType(DWORD dwType)
{

	char *chType = "Undefined";
	switch (dwType)
	{
	case MEM_IMAGE://����������� ���,��������������� ����� (.DLL ��� .EXE)
		chType = "Image   ";
		break;
	case MEM_MAPPED://���� ������, ������������ � ������
		chType = "Mapped  ";
		break;
	case MEM_PRIVATE://�������� ������ ��� ������ ���������
		chType = "Private ";
		break;
	}
	return chType;
}
void GetProtectPageInformation(DWORD dwProtect,ofstream& f)
{
	cout << std::setw(18);
	f << std::setw(18);
	if (dwProtect&PAGE_NOACCESS)				   //���������: ������, ������ ��� ���������� ���� 
	{
		cout << "NOACCESS";
		f << "NOACCESS";
	}
	if (dwProtect&PAGE_READONLY)				   //���������: ������ 
	{
		cout << "READONLY";
		f << "READONLY";
	}
	if (dwProtect&PAGE_READWRITE)				   //���������: ������ � ������
	{
		cout << "READWRITE";
		f << "READWRITE";
	}
	if (dwProtect&PAGE_WRITECOPY)				   //����� ������� ������ �� ���� �������� ���������� ������� ������ ��������
	{
		cout << "WRITECOPY";					   //�������� ����� ������ ������� - ��� ���������� ���� ���������� ��������� ������� 
		f << "WRITECOPY";
	}
	if (dwProtect&PAGE_EXECUTE)					   //���������: ���������� ����
	{
		cout << "EXECUTE";
		f << "EXECUTE";
	}
	if (dwProtect&PAGE_EXECUTE_READ)			   //���������: ������ � ���������� ����
	{
		cout << "EXECUTE_READ";
		f << "EXECUTE_READ";
	}
	if (dwProtect&PAGE_EXECUTE_READWRITE)		   //���������: ������, ������ � ���������� ����
	{
		cout << "EXECUTE_READWRITE";
		f << "EXECUTE_READWRITE";
	}
	if (dwProtect&PAGE_EXECUTE_WRITECOPY)		   //�������� ����� �������� ����������� ��������, ���������:������ � ���������� ���� ��� �����,+ ������ � ������
	{
		cout << "EXECUTE_WRITECOPY";
		f << "EXECUTE_WRITECOPY";
	}
	
	if (dwProtect&PAGE_WRITECOMBINE)			//��������� ��������������� ������ �� �������� ������
	{
		cout << std::setw(6) << "WRITECOMBINE";
		f << std::setw(6) << "WRITECOMBINE";
	}
	
	if (dwProtect&PAGE_GUARD)					//��� ������� ������ ��� ������ ���� ���� �������� ���������� EXCEPTION_GUARD_PAGE
	{
		cout << std::setw(6) << "GUARD";		//� �������� ��������� ���� ����������		
		f << std::setw(6) << "GUARD";
	}
		/*
		EXCEPTION_GUARD_PAGE:
		���� ���� ������������ ��� ����������� �������, ����� �������� ���������� �������� �������������
		*/
	if (dwProtect&PAGE_NOCACHE)					//������������ ������������ ���������� ������(������������� ������ ���������� ��� �����������)
	{
		cout << std::setw(6) << "NOCACHE";
		f << std::setw(6) << "NOCACHE";
	}
}
void GetProtectPageInformation1(DWORD dwProtect)
{

	ofstream f;
	f.open("D:\\111.txt", std::ios::out);
	cout << std::setw(18);
	f << std::setw(18);
	if (dwProtect&PAGE_NOACCESS)				   //���������: ������, ������ ��� ���������� ���� 
	{
		cout << "NOACCESS";
		f << "NOACCESS";
	}
	if (dwProtect&PAGE_READONLY)				   //���������: ������ 
	{
		cout << "READONLY";
		f << "READONLY";
	}
	if (dwProtect&PAGE_READWRITE)				   //���������: ������ � ������
	{
		cout << "READWRITE";
		f << "READWRITE";
	}
	if (dwProtect&PAGE_WRITECOPY)				   //����� ������� ������ �� ���� �������� ���������� ������� ������ ��������
	{
		cout << "WRITECOPY";					   //�������� ����� ������ ������� - ��� ���������� ���� ���������� ��������� ������� 
		f << "WRITECOPY";
	}
	if (dwProtect&PAGE_EXECUTE)					   //���������: ���������� ����
	{
		cout << "EXECUTE";
		f << "EXECUTE";
	}
	if (dwProtect&PAGE_EXECUTE_READ)			   //���������: ������ � ���������� ����
	{
		cout << "EXECUTE_READ";
		f << "EXECUTE_READ";
	}
	if (dwProtect&PAGE_EXECUTE_READWRITE)		   //���������: ������, ������ � ���������� ����
	{
		cout << "EXECUTE_READWRITE";
		f << "EXECUTE_READWRITE";
	}
	if (dwProtect&PAGE_EXECUTE_WRITECOPY)		   //�������� ����� �������� ����������� ��������, ���������:������ � ���������� ���� ��� �����,+ ������ � ������
	{
		cout << "EXECUTE_WRITECOPY";
		f << "EXECUTE_WRITECOPY";
	}

	if (dwProtect&PAGE_WRITECOMBINE)			//��������� ��������������� ������ �� �������� ������
	{
		cout << std::setw(6) << "WRITECOMBINE";
		f << std::setw(6) << "WRITECOMBINE";
	}

	if (dwProtect&PAGE_GUARD)					//��� ������� ������ ��� ������ ���� ���� �������� ���������� EXCEPTION_GUARD_PAGE
	{
		cout << std::setw(6) << "GUARD";		//� �������� ��������� ���� ����������		
		f << std::setw(6) << "GUARD";
	}
	/*
	EXCEPTION_GUARD_PAGE:
	���� ���� ������������ ��� ����������� �������, ����� �������� ���������� �������� �������������
	*/
	if (dwProtect&PAGE_NOCACHE)					//������������ ������������ ���������� ������(������������� ������ ���������� ��� �����������)
	{
		cout << std::setw(6) << "NOCACHE";
		f << std::setw(6) << "NOCACHE";
	}
}
/*
(1)
��������� ���� 4 ������� �������� ���������� �������� PageAndModuleInformation, ����� 1.1 � 1.2 ��
*/
void GetPageInformationProcess(HANDLE hProcess)
/*
������� GetPageInformationProcess �������� ���������� � ��������� �������, ����������� � ��� ��������� ��������
*/
{
	ofstream fFile;
	string path;
	MEMORY_BASIC_INFORMATION memory_information64;
	unsigned char *address = 0;
	cout << "������� ���� � ����" << endl;
	cin.ignore()		;
	getline(cin, path);
	
	fFile.open(path.c_str(), std::ios::out);
	/*
	���� ���������� VirtualQuery (���������� ���������� � �����, ������������ �� ���������� ���������� ������ address)
	*/
	for (address = NULL; VirtualQueryEx(hProcess, address, &memory_information64, sizeof(memory_information64)) == sizeof(memory_information64); address += memory_information64.RegionSize)
	{
		if (VirtualQueryEx(hProcess, address, &memory_information64, sizeof(memory_information64)) == NULL)
		{
			ErrorMessage(GetLastError());
			return;
		}
		cout << memory_information64.BaseAddress;
		fFile << memory_information64.BaseAddress;
		cout << std::setw(11);
		cout << memory_information64.RegionSize;
		cout << " ";
		fFile << std::setw(11);
		fFile << memory_information64.RegionSize;
		fFile << " ";
		fFile << GetMemState(memory_information64.State);
		if (memory_information64.State == MEM_RESERVE)
		{
			cout << GetMemType(memory_information64.Type);
			cout << std::setw(18);
			fFile << GetMemType(memory_information64.Type);
			fFile << std::setw(18);
			GetProtectPageInformation(memory_information64.AllocationProtect,fFile);
		}
		else if (memory_information64.State == MEM_COMMIT)
		{
			cout << GetMemType(memory_information64.Type);
			fFile << GetMemType(memory_information64.Type);
			GetProtectPageInformation(memory_information64.AllocationProtect,fFile);
			GetProtectPageInformation(memory_information64.Protect,fFile);			
		}
		cout << endl;
		fFile << '\n';
		

	}
}
void GetModulesInformationProcess(HANDLE hProcess)
/*
������� GetModulesInformationProcess �������� ���������� � ������ �������, ����������� � ��� ��������� ��������
*/
{
	DWORD cbN;
	HMODULE hMods[1024]; //����� ���������� ������� > 1024 ��� ������, ��� ��� � ��������� ������ 1024 ������ ��������, � ��������� ������������ ������ �� ���������
	char ch[MAX_PATH];
	MODULEINFO inf;
	int i;

	TCHAR module_name[MAX_PATH];
	/*
	������ ������ ��������� ������� - ����� ������� VirtualQueryEx, � ���������� ������� � ��������� State MEM_COMMIT � ������� �� �� �����
	������� �� �������������
	������� EnumProcessModules ���������� ��������� ��� ������� ������ � ������ hMods � �������� ��������
	*/
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbN))
	{
		for (i = 0; i < (cbN / sizeof(HMODULE)); i++)
		{
			if (GetModuleFileNameEx(hProcess, hMods[i], module_name, sizeof(module_name) / sizeof(TCHAR)))
			{
				CharToOem(module_name, ch);//��������������  LPWSTR � char
				cout << ch << endl;
				//��������� ���������� � ������
				if (GetModuleInformation(hProcess, hMods[i], &inf, sizeof(inf)))
				{
					cout << "������ ������:			" << inf.SizeOfImage << " bytes" << endl;
					cout << "��������� ��������� ����������:	" << inf.lpBaseOfDll << endl << endl;
				}
				else
				{
					ErrorMessage(GetLastError());
					return;
				}
			}
			else
			{
				ErrorMessage(GetLastError());
				return;
			}
		}
	}
	else
	{
		ErrorMessage(GetLastError());
		return;
	}
}
int ChoiceViewPID()
/*
��������� ������� GetHandleProc, ������� ���������� ��������� ������� ������
*/
{
	int n;
	cout << "�������� ������ ����������� ����������" << endl;
	cout << "1 - �� �������� ��������" << endl;
	cout << "2 - �� ��������� PID ��������" << endl;
	while (true)
	{
		cinInputINT(n);
		switch (n)
		{
		case 1:
			return 1;
		case 2:
			return 2;
		default:
			cout << "������� ������������ �������" << endl;
			break;
		}
	}
}
HANDLE GetHandleProc()
//���������� HANDLE ��������� ��������, � ����������� �� ������� ������������, ���� �������, ���� ����������
{
	HANDLE hProcess = NULL;
	int n = ChoiceViewPID();
	if (n == 1)
	{
		hProcess = GetCurrentProcess();
	}
	else if (n == 2)
	{
		DWORD processID;
		cout << "������� PID ��������" << endl;
		cin >> processID;
		/*
		������������ ����� ��� �������� �������� ����� ��� ����������� ������ � ������� ��������:
		��������� ����������
		���������� ��������
		������
		������
		*/
		if (!(hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, processID)))
		{
			ErrorMessage(GetLastError());
			return 0;
		}
	}
	return hProcess;
}
HANDLE PageAndModuleInformation()
/*
������ ������� �������� HANDLE ��������, ����� ���������� ������� ���������� � ��������� ������� � ������ �������
*/
{
	HANDLE hProcess = GetHandleProc();
	GetModulesInformationProcess(hProcess);
	GetPageInformationProcess(hProcess);
	return hProcess;
}
/*
(2)
��������� ���� 5 ������� �������� ���������� ��������� ChangePageAttributeProtection, ������� ���������� ������ �������� ������ ��������
� ������� ����������� ����������
*/
int ChoiceViewPage()
/*
��������� �������  GetPageBaseAddress, ������� ���������� ��������� ������� ������
*/
{
	int n;
	cout << "�������� �������� ��� ����� ��������� ������" << endl;
	cout << "1 - ��������, � ������� ���������� �������������� ����������-�������� ��������� ��������� ����������" << endl;
	cout << "2 - �������� �������� �������������" << endl;
	while (true)
	{
		cinInputINT(n);
		switch (n)
		{
		case 1:
			return 1;
		case 2:
			return 2;
		default:
			cout << "������� ������������ �������" << endl;
			break;
		}
	}
}
LPVOID GetPageBaseAddress(HANDLE hProcess)
/*
������ ������� � ����������� �� ������ ������������  ���������� ����� ��������� �������� ��������, ���� ����� ��������� ����������,
� ����� ������� ���������� � �������� �������� GetOnePageInformation ��� ��������� ����������� ����� ����� ��������
*/
{
	LPVOID lpBaseAddress = nullptr;
	int n = ChoiceViewPage();
	if (n == 1)
	{
		lpBaseAddress = GetModuleHandle(L"kernel32.dll");
		//	cout << lpBaseAddress << endl;
		if (lpBaseAddress == NULL)
		{
			ErrorMessage(GetLastError());
			return 0;
		}
	}
	else if (n == 2)
	{
		cout << "������� ����� ��������" << endl;
		cin >> lpBaseAddress;
	}
	GetOnePageInformation(hProcess, lpBaseAddress);
	return lpBaseAddress;
}
DWORD GetNewAttributeProtect()
/*
��������� ������� ChangePageAttributeProtection, ���������� ������� ������ ������� �������� ������
*/
{
	int iFlag;
	DWORD flNewProtect;
	cout << "�������� ����� ������� ������:" << endl;
	cout << "1  - PAGE_NOACCESS" << endl;
	cout << "2  - PAGE_READONLY " << endl;
	cout << "3  - PAGE_READWRITE" << endl;
	cout << "4  - PAGE_WRITECOPY" << endl;
	cout << "5  - PAGE_EXECUTE" << endl;
	cout << "6  - PAGE_EXECUTE_READ" << endl;
	cout << "7  - PAGE_EXECUTE_READWRITE" << endl;
	cout << "8  - PAGE_EXECUTE_WRITECOPY" << endl;
	cout << "9  - PAGE_GUARD" << endl;
	cout << "10 - PAGE_NOCACHE" << endl;
	while (true)
	{
		cinInputINT(iFlag);
		switch (iFlag)
		{
		case 1:
			flNewProtect = PAGE_NOACCESS;
			return flNewProtect;
		case 2:
			flNewProtect = PAGE_READONLY;
			return flNewProtect;
		case 3:
			flNewProtect = PAGE_READWRITE;
			return flNewProtect;
		case 4:
			flNewProtect = PAGE_WRITECOPY;
			return flNewProtect;
		case 5:
			flNewProtect = PAGE_EXECUTE;
			return flNewProtect;
		case 6:
			flNewProtect = PAGE_EXECUTE_READ;
			return flNewProtect;
		case 7:
			flNewProtect = PAGE_EXECUTE_READWRITE;
			return flNewProtect;
		case 8:
			flNewProtect = PAGE_EXECUTE_WRITECOPY;
			return flNewProtect;
		case 9:
			flNewProtect = PAGE_GUARD;
			return flNewProtect;
		case 10:
			flNewProtect = PAGE_NOCACHE;
			return flNewProtect;
		default:
			cout << "������� ������������ �������" << endl;
			break;
		}
	}


}
void GetOnePageInformation(HANDLE hProcess, LPVOID lpBaseAddress)
/*
��������� ���������� �� ����� ��������
*/
{
	MEMORY_BASIC_INFORMATION memory_information64;
	cout << "����� ��������" << endl;
	if (VirtualQueryEx(hProcess, lpBaseAddress, &memory_information64, sizeof(memory_information64)) == NULL)
	{
		ErrorMessage(GetLastError());
		return;
	}
	cout << memory_information64.BaseAddress;
	cout << std::setw(11);
	cout << memory_information64.RegionSize;
	cout << " ";
	cout << GetMemState(memory_information64.State);
	if (memory_information64.State == MEM_RESERVE)
	{
		cout << GetMemType(memory_information64.Type);
		cout << std::setw(18);
		GetProtectPageInformation1(memory_information64.AllocationProtect);
	}
	else if (memory_information64.State == MEM_COMMIT)
	{
		cout << GetMemType(memory_information64.Type);
		GetProtectPageInformation1(memory_information64.AllocationProtect);
		GetProtectPageInformation1(memory_information64.Protect);
	}
	cout << endl;
}
void ChangeVirtualProtect(HANDLE hProcess, LPVOID lpBaseAddress, DWORD flNewProtect)
/*
����� �������� ������(flNewProtect) ���������� ��������(lpBaseAddress) �������� VirtualProtectEx
*/
{
	DWORD  lpflOldProtect;
	if (!VirtualProtectEx(hProcess, lpBaseAddress, sizeof(lpBaseAddress), flNewProtect, &lpflOldProtect))
	{
		ErrorMessage(GetLastError());
		return;
	}
}
LPVOID ChangePageAttributeProtection(HANDLE hProcess)
/*
������ ������� �������� ����� ���������� ������������� �������� ��������� ��������(GetPageBaseAddress), ����� ������� ������(GetNewAttributeProtect),
�������� ������� ������ ��� �������� ��������(ChangeVirtualProtect) � ������� ���������� �������� �� �����
*/
{
	LPVOID lpBaseAddress = GetPageBaseAddress(hProcess);
	DWORD flNewProtect = GetNewAttributeProtect();
	ChangeVirtualProtect(hProcess, lpBaseAddress, flNewProtect);
	GetOnePageInformation(hProcess, lpBaseAddress);
	return lpBaseAddress;
}
/*
(3)
��������� ���� �������  �������� ������ �� ���������� ������ � ������� ������� �� ������ �������� ���������� ��������.
������� ������ ����� ����� ������� PROCESS_VM_WRITE � PROCESS_VM_OPERATION
������������� ������� WriteProcessMemory �� �������������, �������� ����� ������ �������, �������� CopyMemory
*/
void ChangeProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress)
{
	int data = 10000;
	if (!WriteProcessMemory(hProcess, lpBaseAddress, &data, sizeof(int), NULL))
	{
		ErrorMessage(GetLastError());
		return;
	}
	else
		cout << "����������� ������ ������ �������" << endl;
}
/*
(4)
��������� ���� ������� ����������� ������� GetNativeSystemInfo(�� �����) �� ��������� ����� ����������, � ������ kernel32.dll,
� ������ �������� �������������� ���������� system_info ������� ��������� SYSTEM_INFO � ��������� ������ ��������
*/
void ImportFunctionFromDLL(LPVOID lpBaseAddress)
{
	SYSTEM_INFO system_info;
	importFunction getSysInfo;
	getSysInfo = (importFunction)GetProcAddress((HMODULE)lpBaseAddress, "GetNativeSystemInfo");
	if (getSysInfo == NULL)
	{
		ErrorMessage(GetLastError());
		return;
	}
	else
		//�������� ������ �������
	{
		getSysInfo(&system_info);
		cout << "������ ������� ������ �������:" << endl;
		cout << "������ �������� - " << system_info.dwPageSize << endl;
	}
}
/*
(5)
��������� ���� ������� ���������� � ������������ ������� ������ �����
*/
void ChangeMemoryFile()
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hMapFile;
	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("FileNameOject"));//��������� HANDLE ������� �������������� ���� �� ��� �����
	if (hMapFile == NULL)
	{
		ErrorMessage(GetLastError());
		return;
	}

	LPVOID lpAddress;
	lpAddress = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, NULL, NULL, NULL);
	if (!lpAddress)
	{
		ErrorMessage(GetLastError());
		return;
	}
	GetOnePageInformation(hProcess, lpAddress);
	//����������� ������� ������ � ��������
	TCHAR pbData[] = TEXT("Message from process");
	SIZE_T sz = _tcslen(pbData);
	CopyMemory(lpAddress, pbData, sz);
	GetOnePageInformation(hProcess, lpAddress);
	//UnmapViewOfFile(lpAddress);
	//CloseHandle(hMapFile);
}
void CompareFiles()
{

	string path1, path2;
	FILE* f1;
	FILE* f2;
	char buf[100];
	char buf2[100];
	memset(buf,'\0',100);
	memset(buf2,'\0',100);
	int i = 0;
	char b, b1;
	cout << "������� ���� � ������� �����:" << endl;
	//cin >> path1;
	f1 = fopen("default.txt", "r");
	cout << "������� ���� �� ������� �����:" << endl;
	//cin >> path2;
	f2 = fopen("change.txt", "r");
	cout << "-----------------------" << endl;
	while (fread(&b, sizeof(char),1,f1))
		{
		buf[i] = b;
		fread(&b1, sizeof(char), 1, f2);
		buf2[i] = b1;
		i++;
		if (b == '\n')
		{
			if (strcmp(buf, buf2) != 0)
			{
				while (b1 != '\n')
				{
					buf2[i] = b;
					fread(&b1, sizeof(char), 1, f2);
					i++;
				}
				cout << buf2 << endl;
				memset(buf, '\0', 100);
				fread(&b, sizeof(char), 1, f1);
				i = 0;
				while (b != '\n')
				{					
					buf[i] = b;
					fread(&b, sizeof(char), 1, f1);
					i++;
				}
				while (strcmp(buf, buf2) != 0)
				{
					
					memset(buf2, '\0', 100);
					fread(&b1, sizeof(char), 1, f2);
					i = 0;
					while (b1 != '\n')
					{
						buf2[i] = b1;
						fread(&b1, sizeof(char), 1, f2);
						i++;
					}
					cout << buf2 << endl;
				}

			}
			memset(buf, '\0', 100);
			memset(buf2, '\0', 100);
			i = 0;

		}
		}

}
void letsgotostart()
//�������
{
	int n;
	HANDLE hProcess = NULL;
	LPVOID lpBaseAddress = nullptr;
	while (true)
	{
		//system("cls");
		cout << "---------------------------------------------------------" << endl;
		cout << "�������� ������������ ����� ����:(������������� ��������� �� �������)" << endl;
		cout << "1 - ����� ���������� � ��������� ������� � ������ �������" << endl;
		cout << "2 - ����� �������� ������ ��������" << endl;
		cout << "3 - ����������� ������� ������ ��������" << endl;
		cout << "4 - ������ ������� �� ����������" << endl;
		cout << "5 - ������������� � ����������� ������� ������ �����" << endl;
		cout << "6 - �����" << endl;
		cout << "---------------------------------------------------------" << endl;
		cinInputINT(n);
		switch (n)
		{
		case 1:
			hProcess = PageAndModuleInformation();
			system("pause");
			break;
		case 2:
			lpBaseAddress = nullptr;
			lpBaseAddress = ChangePageAttributeProtection(hProcess);
			system("pause");
			break;
		case 3:
			ChangeProcessMemory(hProcess, lpBaseAddress);
			system("pause");
			break;
		case 4:
			ImportFunctionFromDLL(lpBaseAddress);
			system("pause");
			break;
		case 5:
			ChangeMemoryFile();
			system("pause");
			break;
		case 6:
			CloseHandle(hProcess);
			exit(1);
			break;
		case 7:
			CompareFiles();
			break;
		default:
			cout << "������� ������������ �������" << endl;
			system("pause");
			break;
		}
	}
}

