#pragma once
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcessIdByName(wchar_t* name);

bool GetProcessModule(DWORD pid, wchar_t* modName, MODULEENTRY32W& mod);

FORCEINLINE bool CompareByteArray(BYTE* data, BYTE* sig, SIZE_T size);

FORCEINLINE BYTE* FindSignature(BYTE* start, BYTE* end, BYTE* sig, SIZE_T size);

void* FindPointer(BYTE* start, BYTE* end, BYTE* sig, SIZE_T size, int addition = 0);
