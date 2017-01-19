#include "VirtualMemory-scaner.h"
/*ВЫВОД:для страниц, имеющие состояние COMMIT:

- возможна смена атрибута  на значение AllocationBase(то есть на значение, которое задавалось в результате работы функции
VirtualAlloc, то есть при обеспечении страницы страничным кадром) для любой страницы
- при ОТКАЗЕ доступа страница приводится к атрибуту защиты AllocationBase;

- IMAGE: всегда имеет AllocationBase= (EXECUTE_...) , потому что IMAGE - DLL&EXE, ОТКАЗАНО при попытке получения доступа WRITE, преобразуется к WRITECOPY
- MAPPED: ОТКАЗАНО при попытке получения доступа EXECUTE(в файле может содержаться любая информация)
- PRIVATE: ОТКАЗАНО при любой попытке получения доступа, кроме значения AllocationBase;
*/
void ErrorMessage(DWORD error)
/*
Функция в качестве параметра содержит код системной ошибки
в качестве возращаемого значения фомируется сообщение о системной ошибке
форматирование проиходит с помощью функции FormatMessage
*/
{
	//задаем размер буфера под описание кода ошибки
	char* cFail = new char[100];

	FormatMessageA(										//функция форматирует строку сообщения 			
		FORMAT_MESSAGE_FROM_SYSTEM, 				    //ищет в системном ресурсе таблицы сообщений затребованное сообщение 
		NULL,											//расположение определения сообщения, игнорируется 			
		error,											//идентификатор сообщения 			
		0,												//идентификатор языка  			
		cFail,											//помещает указатель на буфер <64 KB 			
		100, 											//игнорируется, определяет размер буфера 			
		NULL);											//массив значений, используемых для вставки в сообщение			 
	cout << "Код ошибки:" << error << " - " << cFail << endl;

}
void cinInputINT(int& i)
//функция проверяет вводимое значение на корректность типа "int"
{
	cin.sync();
	cin >> i;
	if (!cin.good())
	{
		while (!cin)
		{
			cin.clear();
			cin.sync();
			cout << "Ошибка при вводе информации." << endl;
			cout << "Введите корректное значение" << endl;
			cin >> i;
		}
	}
}
/*
(Ниже описаны 3 функции, смысл которых преобразовать числовое значение состояния страницы к текстовому буферу
(Декодирование)
*/
char* GetMemState(DWORD dwState)

{
	char *chState = "Undefined";

	switch (dwState)
	{
	case MEM_FREE:   //страница свободна, недоступна для вызывающего процесса
		chState = "Free    ";
		break;
	case MEM_RESERVE://зарезервирована, но нет физической памяти
		chState = "Reserve ";
		break;
	case MEM_COMMIT: //передана процессу, ей соответсвуеи страница физической памяти
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
	case MEM_IMAGE://исполняемый код,спроецированный образ (.DLL или .EXE)
		chType = "Image   ";
		break;
	case MEM_MAPPED://файл данных, проецируемый в память
		chType = "Mapped  ";
		break;
	case MEM_PRIVATE://страница скрыта для других процессов
		chType = "Private ";
		break;
	}
	return chType;
}
void GetProtectPageInformation(DWORD dwProtect,ofstream& f)
{
	cout << std::setw(18);
	f << std::setw(18);
	if (dwProtect&PAGE_NOACCESS)				   //запрещено: чтение, запись или выполнение кода 
	{
		cout << "NOACCESS";
		f << "NOACCESS";
	}
	if (dwProtect&PAGE_READONLY)				   //разрешено: чтение 
	{
		cout << "READONLY";
		f << "READONLY";
	}
	if (dwProtect&PAGE_READWRITE)				   //разрешено: чтение и запись
	{
		cout << "READWRITE";
		f << "READWRITE";
	}
	if (dwProtect&PAGE_WRITECOPY)				   //любая попытка записи на этой странице заставляет систему выдать процессу
	{
		cout << "WRITECOPY";					   //закрытую копию данной станицы - при выполнении кода произойдет нарушение доступа 
		f << "WRITECOPY";
	}
	if (dwProtect&PAGE_EXECUTE)					   //разрешено: выполнение кода
	{
		cout << "EXECUTE";
		f << "EXECUTE";
	}
	if (dwProtect&PAGE_EXECUTE_READ)			   //разрешено: чтение и выполнение кода
	{
		cout << "EXECUTE_READ";
		f << "EXECUTE_READ";
	}
	if (dwProtect&PAGE_EXECUTE_READWRITE)		   //разрешено: чтение, запись и выполнение кода
	{
		cout << "EXECUTE_READWRITE";
		f << "EXECUTE_READWRITE";
	}
	if (dwProtect&PAGE_EXECUTE_WRITECOPY)		   //выдается копия страницы вызывающему процессу, разрешено:чтение и выполнение кода без копии,+ запись с копией
	{
		cout << "EXECUTE_WRITECOPY";
		f << "EXECUTE_WRITECOPY";
	}
	
	if (dwProtect&PAGE_WRITECOMBINE)			//разрешает комбинированную запись на страницу памяти
	{
		cout << std::setw(6) << "WRITECOMBINE";
		f << std::setw(6) << "WRITECOMBINE";
	}
	
	if (dwProtect&PAGE_GUARD)					//при попытке чтения или записи этот флаг вызывает исключение EXCEPTION_GUARD_PAGE
	{
		cout << std::setw(6) << "GUARD";		//и страница перестает быть сторожевой		
		f << std::setw(6) << "GUARD";
	}
		/*
		EXCEPTION_GUARD_PAGE:
		этот флаг используется для определения момента, когда процессу необходимо выделить допвиртпамять
		*/
	if (dwProtect&PAGE_NOCACHE)					//используется некэшируемая фищическая память(проецирование буфера видеокадра без кэширования)
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
	if (dwProtect&PAGE_NOACCESS)				   //запрещено: чтение, запись или выполнение кода 
	{
		cout << "NOACCESS";
		f << "NOACCESS";
	}
	if (dwProtect&PAGE_READONLY)				   //разрешено: чтение 
	{
		cout << "READONLY";
		f << "READONLY";
	}
	if (dwProtect&PAGE_READWRITE)				   //разрешено: чтение и запись
	{
		cout << "READWRITE";
		f << "READWRITE";
	}
	if (dwProtect&PAGE_WRITECOPY)				   //любая попытка записи на этой странице заставляет систему выдать процессу
	{
		cout << "WRITECOPY";					   //закрытую копию данной станицы - при выполнении кода произойдет нарушение доступа 
		f << "WRITECOPY";
	}
	if (dwProtect&PAGE_EXECUTE)					   //разрешено: выполнение кода
	{
		cout << "EXECUTE";
		f << "EXECUTE";
	}
	if (dwProtect&PAGE_EXECUTE_READ)			   //разрешено: чтение и выполнение кода
	{
		cout << "EXECUTE_READ";
		f << "EXECUTE_READ";
	}
	if (dwProtect&PAGE_EXECUTE_READWRITE)		   //разрешено: чтение, запись и выполнение кода
	{
		cout << "EXECUTE_READWRITE";
		f << "EXECUTE_READWRITE";
	}
	if (dwProtect&PAGE_EXECUTE_WRITECOPY)		   //выдается копия страницы вызывающему процессу, разрешено:чтение и выполнение кода без копии,+ запись с копией
	{
		cout << "EXECUTE_WRITECOPY";
		f << "EXECUTE_WRITECOPY";
	}

	if (dwProtect&PAGE_WRITECOMBINE)			//разрешает комбинированную запись на страницу памяти
	{
		cout << std::setw(6) << "WRITECOMBINE";
		f << std::setw(6) << "WRITECOMBINE";
	}

	if (dwProtect&PAGE_GUARD)					//при попытке чтения или записи этот флаг вызывает исключение EXCEPTION_GUARD_PAGE
	{
		cout << std::setw(6) << "GUARD";		//и страница перестает быть сторожевой		
		f << std::setw(6) << "GUARD";
	}
	/*
	EXCEPTION_GUARD_PAGE:
	этот флаг используется для определения момента, когда процессу необходимо выделить допвиртпамять
	*/
	if (dwProtect&PAGE_NOCACHE)					//используется некэшируемая фищическая память(проецирование буфера видеокадра без кэширования)
	{
		cout << std::setw(6) << "NOCACHE";
		f << std::setw(6) << "NOCACHE";
	}
}
/*
(1)
Описанные ниже 4 функции являются локальными функциям PageAndModuleInformation, пункт 1.1 и 1.2 ЛР
*/
void GetPageInformationProcess(HANDLE hProcess)
/*
Функция GetPageInformationProcess получает информацию о состоянии страниц, находящиеся в ВАП заданного процесса
*/
{
	ofstream fFile;
	string path;
	MEMORY_BASIC_INFORMATION memory_information64;
	unsigned char *address = 0;
	cout << "Введите путь к фалу" << endl;
	cin.ignore()		;
	getline(cin, path);
	
	fFile.open(path.c_str(), std::ios::out);
	/*
	цикл использует VirtualQuery (возвращает информацию о блоке, начинающемся по указанному начальному адресу address)
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
Функция GetModulesInformationProcess получает информация о списке модулей, загруженных в ВАП заданного процесса
*/
{
	DWORD cbN;
	HMODULE hMods[1024]; //брать количество модулей > 1024 нет смысла, так как с процессом больше 1024 трудно работать, в контексте лабораторной работы не актуально
	char ch[MAX_PATH];
	MODULEINFO inf;
	int i;

	TCHAR module_name[MAX_PATH];
	/*
	Другой способ получения модулей - через функцию VirtualQueryEx, с выделением страниц с атрибутом State MEM_COMMIT и выводом их на экран
	разница не принципиальна
	Функция EnumProcessModules возвращает заголовки для каждого модуля в массив hMods в заданном процессе
	*/
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbN))
	{
		for (i = 0; i < (cbN / sizeof(HMODULE)); i++)
		{
			if (GetModuleFileNameEx(hProcess, hMods[i], module_name, sizeof(module_name) / sizeof(TCHAR)))
			{
				CharToOem(module_name, ch);//преобразование  LPWSTR к char
				cout << ch << endl;
				//получение информации о модуле
				if (GetModuleInformation(hProcess, hMods[i], &inf, sizeof(inf)))
				{
					cout << "Размер модуля:			" << inf.SizeOfImage << " bytes" << endl;
					cout << "Заголовок системной библиотеки:	" << inf.lpBaseOfDll << endl << endl;
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
Локальная функция GetHandleProc, которая возвращает выбранный вариант ответа
*/
{
	int n;
	cout << "Выберите способ отображения информации" << endl;
	cout << "1 - По текущему процессу" << endl;
	cout << "2 - По заданному PID процесса" << endl;
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
			cout << "Введите предложенный вариант" << endl;
			break;
		}
	}
}
HANDLE GetHandleProc()
//Возвращает HANDLE открытого процесса, в зависимости от желания пользователя, либо текущий, либо задаваемый
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
		cout << "Введите PID процесса" << endl;
		cin >> processID;
		/*
		Используемый флаги при открытии процесса нужны для возможности работы с памятью процесса:
		получение информации
		проведения операций
		чтения
		записи
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
Данная функция получает HANDLE процесса, затем занимается выводом информации о состоянии страниц и списка модулей
*/
{
	HANDLE hProcess = GetHandleProc();
	GetModulesInformationProcess(hProcess);
	GetPageInformationProcess(hProcess);
	return hProcess;
}
/*
(2)
Описанные ниже 5 функций являются локальными функциями ChangePageAttributeProtection, которая занимается сменой атрибуты защиты страницы
и выводом полученного результата
*/
int ChoiceViewPage()
/*
Локальная функция  GetPageBaseAddress, которая возвращает выбранный вариант ответа
*/
{
	int n;
	cout << "Выберите страницу для смены атрибутов защиты" << endl;
	cout << "1 - Страница, в которой содержится контролируемый программой-сторожем заголовок системной библиотеки" << endl;
	cout << "2 - Страница задается пользователем" << endl;
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
			cout << "Введите предложенный вариант" << endl;
			break;
		}
	}
}
LPVOID GetPageBaseAddress(HANDLE hProcess)
/*
Данная функция в зависимости от выбора пользователя  возвращает адрес заголовка вводимой страницы, либо адрес системной библиотеки,
а также выводит информацию о странице функцией GetOnePageInformation для сравнения результатов после смены атрибута
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
		cout << "Введите адрес страницы" << endl;
		cin >> lpBaseAddress;
	}
	GetOnePageInformation(hProcess, lpBaseAddress);
	return lpBaseAddress;
}
DWORD GetNewAttributeProtect()
/*
Локальная функция ChangePageAttributeProtection, назначение которой выбора нужного атрибута защиты
*/
{
	int iFlag;
	DWORD flNewProtect;
	cout << "Выберите новый атрибут защиты:" << endl;
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
			cout << "Введите предложенный вариант" << endl;
			break;
		}
	}


}
void GetOnePageInformation(HANDLE hProcess, LPVOID lpBaseAddress)
/*
Получение информации об одной странице
*/
{
	MEMORY_BASIC_INFORMATION memory_information64;
	cout << "Обзор страницы" << endl;
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
Смена атрибута защиты(flNewProtect) получаемой страницы(lpBaseAddress) функцией VirtualProtectEx
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
Данная функция получает адрес задаваемой пользователем страницы заданного процесса(GetPageBaseAddress), новый атрибут защиты(GetNewAttributeProtect),
изменяет атрибут защиты для заданной страницы(ChangeVirtualProtect) и выводит измененную страницу на экран
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
Описанная ниже функция  копирует данные из указанного буфера в текущий процесс по адресу страницы указанного процесса.
Процесс должен иметь фалги доступа PROCESS_VM_WRITE и PROCESS_VM_OPERATION
Использование функции WriteProcessMemory не принципиально, подойдет любая другая функция, например CopyMemory
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
		cout << "Модификация памяти прошла успешно" << endl;
}
/*
(4)
Описанная ниже функция импортирует функцию GetNativeSystemInfo(ее адрес) из указанной ранее библиотеки, а именно kernel32.dll,
в случае удачного импортирования переменная system_info поучает структуру SYSTEM_INFO и выводится размер страницы
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
		//проверка работы функции
	{
		getSysInfo(&system_info);
		cout << "Импорт функции прошел успешно:" << endl;
		cout << "Размер страницы - " << system_info.dwPageSize << endl;
	}
}
/*
(5)
Описанная ниже функция проецирует и модифицирует участок памяти файла
*/
void ChangeMemoryFile()
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hMapFile;
	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("FileNameOject"));//получение HANDLE объекта проецированный файл по его имени
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
	//копирование участка памяти в проекцию
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
	cout << "Введите путь к первому файлу:" << endl;
	//cin >> path1;
	f1 = fopen("default.txt", "r");
	cout << "Введите путь ко второму файлу:" << endl;
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
//менюшка
{
	int n;
	HANDLE hProcess = NULL;
	LPVOID lpBaseAddress = nullptr;
	while (true)
	{
		//system("cls");
		cout << "---------------------------------------------------------" << endl;
		cout << "Выберите интересующий пункт меню:(рекомендуется выполнять по порядку)" << endl;
		cout << "1 - Вывод информации о состоянии страниц и списка модулей" << endl;
		cout << "2 - Смена атрибута защиты страницы" << endl;
		cout << "3 - Модификация участка памяти страницы" << endl;
		cout << "4 - Импорт функции из библиотеки" << endl;
		cout << "5 - Проецирование и модификация участка памяти файла" << endl;
		cout << "6 - Выход" << endl;
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
			cout << "Введите предложенный вариант" << endl;
			system("pause");
			break;
		}
	}
}

