#pragma once
#include <windows.h>
#include <cstdint>

inline HANDLE g_Proc;

template<typename T>
T Read(void* address) {
	T buffer{};
	ReadProcessMemory(g_Proc, address, reinterpret_cast<void*>(&buffer), sizeof(T), nullptr);
	auto err = GetLastError();
	return buffer;
}

template<typename T>
bool Read(void* address, void* buffer) {
	return ReadProcessMemory(g_Proc, address, buffer, sizeof(T), nullptr);
}