#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <vector>
#undef GetObject

uint32_t GetProcessIdByName(const wchar_t* name);
uint32_t GetProcessModules(uint32_t pid, uint32_t count, const wchar_t* names[], MODULEENTRY32W mods[]);
bool Compare(byte* data, byte* sig, size_t size);
byte* FindSignature(byte* start, byte* end, byte* sig, size_t size);
void* FindPointer(byte* start, byte* end, byte* sig, size_t size, int32_t addition = 0);
bool GetExSections(byte* data, std::vector<std::pair<byte*, byte*>>& sections);
uint32_t GetProccessPath(uint32_t pid, wchar_t* processName, uint32_t size);