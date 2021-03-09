#pragma once
#include <windows.h>
#include "defs.h"

extern uint64 Base;

bool Read(void* address, void* buffer, uint64 size);
template<typename T>
T Read(void* address) 
{
	T buffer{};
	Read(address, &buffer, sizeof(T));
	return buffer;
}

bool GetMemoryInfo(void* address, PMEMORY_BASIC_INFORMATION info);

bool ReaderInit(uint32 pid);