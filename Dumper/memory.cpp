#include "memory.h"

HANDLE hProcess;

uint64_t Base;

bool Read(void* address, void* buffer, size_t size)
{
	return ReadProcessMemory(hProcess, address, buffer, size, nullptr);
}

bool ReaderInit(uint32_t pid)
{
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	return hProcess != nullptr;
}