#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <vector>



uint32_t GetProcessIdByName(const wchar_t* name);
uint32_t GetProcessModules(uint32_t pid, uint32_t count, wchar_t* names[], MODULEENTRY32W mods[]);
bool CompareByteArray(byte* data, byte* sig, size_t size);
byte* FindSignature(byte* start, byte* end, byte* sig, size_t size);
void* FindPointer(byte* start, byte* end, byte* sig, size_t size, int32_t addition = 0);
bool GetExSections(byte* data, std::vector<std::pair<byte*, byte*>>& sections);