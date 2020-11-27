#pragma once
#include "Engine.h"
#include <windows.h>
#include <functional>


struct TArray {
	byte* Data;
	uint32_t Count;
	uint32_t Max;
};

struct FNameEntryHandle
{
	uint32_t Block = 0;
	uint32_t Offset = 0;
	FNameEntryHandle(uint32_t Block, uint32_t Offset) : Block(Block), Offset(Offset) {};
	FNameEntryHandle(uint32_t Id) : Block(Id >> 16), Offset(Id & 65535) {}; 
	operator uint32_t() const { return (Block << 16 | Offset); }
};

struct FNamePool {

	byte Lock[8];
	uint32_t CurrentBlock;
	uint32_t CurrentByteCursor;
	byte* Blocks[8192];

	byte* GetEntry(FNameEntryHandle Handle) const;
	void DumpBlock(uint32_t BlockIdx, uint32_t BlockSize, std::function<void(std::string_view, uint32_t)> callback) const;
	void Dump(std::function<void(std::string_view, uint32_t)> callback) const;
};

struct TUObjectArray
{
	byte** Objects;
	byte* PreAllocatedObjects;
	uint32_t MaxElements;
	uint32_t NumElements;
	uint32_t MaxChunks;
	uint32_t NumChunks;

	byte* GetObjectPtr(uint32_t Index) const;

	void Dump(std::function<void(byte*)> callback) const;

	class UE_UClass FindObject(const std::string& name) const;
};


inline TUObjectArray ObjObjects;
inline FNamePool NamePoolData;
