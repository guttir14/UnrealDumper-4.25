#pragma once
#include <functional>
#include "defs.h"

bool Compare(uint8* data, uint8* sig, uint32 size);

uint8* FindSignature(void* start, void* end, const char* sig, uint32 size);

void* FindPointer(void* start, void* end, const char* sig, uint32 size, int32 addition = 0);

void IterateExSections(void* data, std::function<bool(void*, void*)> callback);

uint32 GetProccessPath(uint32 pid, wchar_t* processName, uint32 size);

uint64 GetTime();
