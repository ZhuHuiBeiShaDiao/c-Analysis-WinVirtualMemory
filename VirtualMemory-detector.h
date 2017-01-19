#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <winbase.h>
#include <iomanip> //std::setw
#include <math.h>
#include <string>
#include <tchar.h>

typedef void (WINAPI *importFunction)(LPSYSTEM_INFO);// тип данных "функция GetNativeSystemInfo"(определение, какой тип данных будет получать адрес функции GetNativeSystemInfo )
using std::cout;
using std::cin;
using std::endl;
using std::setw;
using std::pow;
using std::string;

/*ПРОТОТИПЫ ФУНКЦИЙ*/
void ErrorMessage(DWORD);
HANDLE CrFile(char*);
string WriteInFile(HANDLE);
HANDLE CreateMapping(HANDLE);
BOOL HClose(HANDLE);
LPVOID MapViewFile(HANDLE);
BOOL UnMapViewFile(LPVOID);
void ImportFunctionFromDLL(HMODULE);
void storozhVM();

/*//Проецирование файла с заранее известной информацией
hFile = CrFile("123.txt");
WriteInFile(hFile);
hFileMapping = CreateMapping(hFile);
if (!HClose(hFile)) return;
lpAddress = MapViewFile(hFileMapping);
if (lpAddress != lpAddressBUF)
cout << "WARNING! Back up(FILE) HAS been change!" << endl;
else
cout << "Allright, Back up(FILE) has not been change" << endl;

system("pause");

//Проецирование файла библиотеки
hFileDLL = CrFile("D://kernel32.dll");
hFileMappingDLL = CreateMapping(hFileDLL);
if (!HClose(hFileDLL)) return;
lpAddressDLL = MapViewFile(hFileMappingDLL);
if (lpAddressDLL != lpAddressDLLBUF)
cout << "WARNING! Back up(DLL) HAS been change!" << endl;
else
cout << "Allright, Back up(DLL) has not been change" << endl;

system("pause");
if (!UnMapViewFile(lpAddressDLL)) return;
if (!HClose(hFileMappingDLL)) return;
if (!UnMapViewFile(lpAddress)) return;
if (!HClose(hFileMapping)) return;
lpContainBUF = MapViewFile(hFileMappingBUF);
char* chContain = new char[sizeof(lpContainBUF)];
strcpy(chContain,(char*)lpContainBUF);
string ssContain = string(chContain);
*/