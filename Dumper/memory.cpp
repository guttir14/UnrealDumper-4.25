#include "memory.h"

HANDLE hProcess;
uint64 Base;

bool Read(void* address, void* buffer, uint64 size)
{
	return ReadProcessMemory(hProcess, address, buffer, size, nullptr);
}

bool GetMemoryInfo(void* address, PMEMORY_BASIC_INFORMATION info) 
{
	return VirtualQueryEx(hProcess, address, info, sizeof(PMEMORY_BASIC_INFORMATION)) > 0;
}

bool ReaderInit(uint32 pid)
{
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	return hProcess != nullptr;
}

