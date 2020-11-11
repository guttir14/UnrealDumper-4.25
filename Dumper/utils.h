#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>


struct ModuleInfo
{
	byte* base = nullptr;
	uint32_t size = 0;
};

uint32_t GetProcessIdByName(wchar_t* name);
bool GetProcessModule(uint32_t pid, wchar_t* modName, ModuleInfo& mod);
bool CompareByteArray(byte* data, byte* sig, size_t size);
byte* FindSignature(byte* start, byte* end, byte* sig, size_t size);
void* FindPointer(byte* start, byte* end, byte* sig, size_t size, int32_t addition = 0);
byte* GetExSection(byte* data);