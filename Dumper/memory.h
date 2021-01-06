#pragma once
#include <windows.h>
#include <cstdint>

bool Read(void* address, void* buffer, size_t size);

template<typename T>
T Read(void* address) 
{
	T buffer{};
	Read(address, &buffer, sizeof(T));
	return buffer;
}

bool ReaderInit(uint32_t pid);