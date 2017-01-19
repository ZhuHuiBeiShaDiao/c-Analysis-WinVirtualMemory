#include "VirtualMemory-scaner.h"

int main()
{
	setlocale(LC_ALL, "");
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);

	MEMORYSTATUS memory_status;
	GlobalMemoryStatus(&memory_status);
	letsgotostart();
}