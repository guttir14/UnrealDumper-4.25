#pragma once
#include <windows.h>
#include <cstdint>

inline HANDLE G_hProcess;

template<typename T>
inline T Read(void* address) {
	T buffer{};
	ReadProcessMemory(G_hProcess, address, reinterpret_cast<void*>(&buffer), sizeof(T), nullptr);
	return buffer;
}

template<typename T>
inline bool Read(void* address, T* buffer) {
	return ReadProcessMemory(G_hProcess, address, buffer, sizeof(T), nullptr);
}


inline bool Read(void* address, void* buffer, size_t size) {
	return ReadProcessMemory(G_hProcess, address, buffer, size, nullptr);
}

