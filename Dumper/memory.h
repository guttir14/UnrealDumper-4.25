#pragma once
#include <windows.h>
#include <cstdint>

extern uint64_t Base;

bool Read(void* address, void* buffer, size_t size);

template<typename T>
T Read(void* address) 
{
	T buffer{};
	Read(address, &buffer, sizeof(T));
	return buffer;
}

bool ReaderInit(uint32_t pid);