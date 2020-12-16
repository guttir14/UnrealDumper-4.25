#pragma once
#include <windows.h>
#include <cstdint>

inline HANDLE _hProcess;

template<typename T>
inline T Read(void* address) 
{
	T buffer{};
	ReadProcessMemory(_hProcess, address, &buffer, sizeof(T), nullptr);
	return buffer;
}

template<typename T>
inline bool Read(void* address, T* buffer) 
{
	return ReadProcessMemory(_hProcess, address, buffer, sizeof(T), nullptr);
}

inline bool Read(void* address, void* buffer, size_t size) 
{
	return ReadProcessMemory(_hProcess, address, buffer, size, nullptr);
}

inline bool ReaderInit(uint32_t pid)
{
	_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	return _hProcess != nullptr;
}