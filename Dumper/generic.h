#pragma once
#include <functional>
#include "windows.h"
#undef GetObject

struct TArray {
	byte* Data;
	uint32_t Count;
	uint32_t Max;
};

struct TNameEntryArray
{
	byte** Chunks[128];
	int32_t NumElements;
	int32_t NumChunks;

	byte* GetEntry(uint32_t id) const;
	void Dump(std::function<void(std::string_view name, uint32_t id)> callback) const;
};

struct TUObjectArray
{
	byte* Objects;
	uint32_t MaxElements;
	uint32_t NumElements;

	byte* GetObjectPtr(uint64_t id) const;
	void Dump(std::function<void(byte*)> callback) const;
	class UE_UClass FindObject(const std::string& name) const;
};

extern TNameEntryArray GlobalNames;
extern TUObjectArray ObjObjects;
