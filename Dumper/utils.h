#pragma once
#include <vector>
#include <string>
#include "defs.h"

uint32 GetProcessId(std::wstring name);
std::pair<uint8*, uint32> GetModuleInfo(uint32 pid, std::wstring name);
bool Compare(uint8* data, uint8* sig, uint64 size);
uint8* FindSignature(uint8* start, uint8* end, uint8* sig, uint64 size);
void* FindPointer(uint8* start, uint8* end, uint8* sig, uint64 size, int32 addition = 0);
std::vector<std::pair<uint8*, uint8*>> GetExSections(uint8* data);
uint32 GetProccessPath(uint32 pid, wchar_t* processName, uint32 size);