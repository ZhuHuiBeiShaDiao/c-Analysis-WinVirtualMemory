//#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <fstream>
#include <iomanip> 
#include <string>

typedef void (WINAPI *importFunction)(LPSYSTEM_INFO);// тип данных "функция GetNativeSystemInfo"(определение, какой тип данных будет получать адрес функции GetNativeSystemInfo )
using std::cin;
using std::endl;
using std::cout;
using std::string;
using std::ofstream;
using std::ifstream;
using std::getline;
void ErrorMessage(DWORD error);
void cinInputINT(int& i);
char* GetMemState(DWORD dwState);
char* GetMemType(DWORD dwType);
void GetProtectPageInformation(DWORD dwProtect,ofstream&);
void GetProtectPageInformation1(DWORD dwProtect);
void GetPageInformationProcess(HANDLE hProcess);
void GetModulesInformationProcess(HANDLE hProcess);
int ChoiceViewPID();
HANDLE GetHandleProc();
HANDLE PageAndModuleInformation();
int ChoiceViewPage();
LPVOID GetPageBaseAddress(HANDLE hProcess);
DWORD GetNewAttributeProtect();
void GetOnePageInformation(HANDLE hProcess, LPVOID lpBaseAddress);
LPVOID ChangePageAttributeProtection(HANDLE hProcess);
void ChangeProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress);
void ImportFunctionFromDLL(LPVOID lpBaseAddress);
void letsgotostart();


