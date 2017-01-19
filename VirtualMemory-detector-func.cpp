#include "VirtualMemory-detector.h"
void ErrorMessage(DWORD error)
// Функция в качестве параметра содержит код системной ошибки
// в качестве возращаемого значения фомируется сообщение о системной ошибке
// форматирование проиходит с помощью функции FormatMessage
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
	delete[]cFail;

}
HANDLE CrFile(char *cBuf)
//Создание дескриптора файла, параметр функции передает указатель на строку, в которой указан путь к создаваемому файлу
{
	HANDLE FileHandle;										     //создание переменной под дескриптор файла

	FileHandle = CreateFileA(cBuf,								//в качестве первого параметра передается путь к создаваемому файлу, по умолчанию используется директория проекта программы
		GENERIC_READ | GENERIC_WRITE,							//тип доступа к объекту (указывается на запись или чтение)
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, //режим совместного доступа(допускаются последующие операции чтения, записи и удаления объекта)
		NULL,													//объект получает заданный по умолчанию дескриптор безопасности
		OPEN_ALWAYS,											//выполняемое дейтсвие с файлом(создается новый файл)
		FILE_ATTRIBUTE_NORMAL,									//файл не ммеет предустановленных атрибутов
		0);														//файл шаблона, который предоставляет атрибуты файла и дополнительные атрибуты для создающегося файла
	if (FileHandle == INVALID_HANDLE_VALUE)						//если возникла ошибка с возвращаемым значением функции CreateFile
		return INVALID_HANDLE_VALUE;											  //возвращается эта ошибка

	return FileHandle;										  //иначе возвращается дескриптор
}
string WriteInFile(HANDLE hFile)
{
	string sBuf;																	  //буфер под массив символов, добавляемых в файл 
	char *k = new char[5];															  //переменная под отдельный символ
	for (int i = 0; i < 5; i++)													  
	{																				  
		_itoa_s(i,																	   //символ, который будет преобразован
			k,																		   //переменная, куда записывается сивол
			5, 																		   //размер буфера		
			10);														  			   //система исчисления
		sBuf += k;																	  
	}

	if (!WriteFile(hFile,															 //дескриптор
		sBuf.c_str(),																 //буфер записываемой информации
		sBuf.size(),															     //размер буфера
		NULL, 																		 //число записанных байтов
		NULL))
	{            																     //если не удалось переместить файл, то происходит передача управления на функцию 
		ErrorMessage(GetLastError());												 //обработки ошибок API
		return 0;
	}
	delete[]k;																		  //освобождение памяти
	return sBuf;
}																					  
HANDLE CreateMapping(HANDLE FileHandle)
/*Функция создает или открывает объект отображенного в памяти (проецируемого) файла для заданного файла.*/
{
	HANDLE hFileMapping;
	hFileMapping = CreateFileMapping(FileHandle, //дескриптор файла
		NULL,									 //защита
		PAGE_READWRITE,							 //атрибуты защиты
		NULL,									 //младшее слово
		NULL,									 //старшее слово
		_T("FileNameOject")						 //имя проецируемого файла
		);									 
	if (!hFileMapping)							 //проверка на ошибки
	{
		ErrorMessage(GetLastError());
		return INVALID_HANDLE_VALUE;
	}
	return hFileMapping;
}
BOOL HClose(HANDLE handleF)
//Закрывает дескриптор
{
	if (!CloseHandle(handleF))
	{
		ErrorMessage(GetLastError());
		return FALSE;
	}
	return TRUE;
}
LPVOID MapViewFile(HANDLE hFileMapping)
//Функция отображает представление проецируемого файла в адресное пространство вызывающего процесса
{
	LPVOID lpAddress;
	lpAddress = MapViewOfFile(hFileMapping, //дескриптор проецируемого файла
		FILE_MAP_ALL_ACCESS,			   //режим доступа
		0, 								   //старшее слово
		0,								   //младшее слово
		0);						   		   //число отображаемых байтов(0 - весь файл)
	if (!lpAddress)						   //проверка на ошибки
	{
		ErrorMessage(GetLastError());
		return 0;
	}
	return lpAddress;
}
BOOL UnMapViewFile(LPVOID lpAddress)
//Функция отменяет отображение представления файла из адресного пространства вызывающего процесса
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
		//проверка работы функции
	{
		getSysInfo(&system_info);
		cout << "(DLL) не изменен." << endl;
	}
}
//Главная функция
void storozhVM()
{
	HANDLE hFile, hFileMapping;
	LPVOID lpBaseAddressFile;
	string sContain;
	HMODULE  hProcessDLL;
	//Файл
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
	//Проверка на неизменность
	while (1)
	{
		lpBaseAddressFile = MapViewFile(hFileMapping);
		if (strcmp(sContain.c_str(),(char*)lpBaseAddressFile)!= 0)
			cout << "Ошибка проверки файла!" << endl;
		else
			cout << "(FILE) не изменен." << endl;
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