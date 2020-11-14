#pragma once
#include <functional>
#include "Engine.h"

struct FNameEntryHandle
{
	uint32_t Block = 0;
	uint32_t Offset = 0;
	FNameEntryHandle(uint32_t Block, uint32_t Offset) : Block(Block), Offset(Offset) {};
	FNameEntryHandle(int32_t Id) : Block(Id >> 16), Offset(Id & 65535) {}; 
	operator int32_t() const { return (Block << 16 | Offset); }
};

struct FNameEntryHeader1 {
	uint16_t bIsWide : 1;
	uint16_t LowercaseProbeHash : 5;
	uint16_t Len : 10;
};

struct FNameEntryHeader {
	uint16_t bIsWide : 1;
	uint16_t Len : 15;
};

struct FNameEntry
{
	int32_t ComparisonId;
	FNameEntryHeader header;
	union
	{
		char	AnsiName[1024];
		wchar_t	WideName[1024];
	};

	static inline int32_t GetDataOffset()
	{
		return offsets.header + 2;
	}

	static inline int32_t GetSize(int32_t Length, bool bIsPureAnsi)
	{
		int32_t Bytes = GetDataOffset() + Length * (bIsPureAnsi ? sizeof(char) : sizeof(wchar_t));
		return (Bytes + offsets.stride - 1) & ~(offsets.stride - 1);
	}

	char* GetNameA() const
	{
		return ((char*)this + GetDataOffset());
	}

	wchar_t* GetNameW() const
	{
		return (wchar_t*)((char*)this + GetDataOffset());
	}

	FNameEntryHeader GetHeader() const
	{
		FNameEntryHeader header;
		if (offsets.addition) {
			header = *reinterpret_cast<const FNameEntryHeader*>(reinterpret_cast<const byte*>(this) + offsets.header);
		}
		else {
			auto head = *reinterpret_cast<const FNameEntryHeader1*>(this);
			header.bIsWide = head.bIsWide;
			header.Len = head.Len;
		}
		return header;
	}


	std::string_view GetViewA() const {
		return std::string_view(GetNameA(), GetHeader().Len);
	}

	std::wstring_view GetViewW() const {
		return std::wstring_view(GetNameW(), GetHeader().Len);
	}

	std::string GetStringA() const
	{
		auto str = std::string(GetNameA(), GetHeader().Len);
		return str;
	}

	std::wstring GetStringW() const
	{
		return std::wstring(GetNameW(), GetHeader().Len);
	}

	std::string GetString() const
	{
		return GetStringA();
	}

	std::string_view GetView() const
	{
		return GetViewA();
	}

};

struct FNameEntryAllocator
{
	byte Lock[8];
	uint32_t CurrentBlock;
	uint32_t CurrentByteCursor;
	byte* Blocks[8192];
};

struct FNamePool {
	FNameEntryAllocator Entries;

	FNameEntry* GetEntry(FNameEntryHandle Handle) { return reinterpret_cast<FNameEntry*>(Entries.Blocks[Handle.Block] + offsets.stride * static_cast<uint64_t>(Handle.Offset)); }
	void DumpBlock(uint32_t BlockIdx, uint32_t BlockSize, std::function<void(std::string_view, uint32_t)> callback);
	void Dump(std::function<void(std::string_view, uint32_t)> callback);
};

struct TUObjectArray
{
	FUObjectItem** Objects;
	FUObjectItem* PreAllocatedObjects;
	int32_t MaxElements;
	int32_t NumElements;
	int32_t MaxChunks;
	int32_t NumChunks;

	UObject* GetObjectPtr(int32_t Index)
	{
		if (Index >= NumElements) return nullptr;
		const uint64_t ChunkIndex = Index / 65536;
		if (ChunkIndex >= NumChunks) return nullptr;
		const uint32_t WithinChunkIndex = Index % 65536;
		FUObjectItem* Chunk = Read<FUObjectItem*>(Objects + ChunkIndex * 8);
		if (!Chunk) return nullptr;
		auto item = Read<FUObjectItem>(Chunk + WithinChunkIndex);
		return item.Object;
	}

	void Dump(std::function<void(UObject*)> callback);

	class UE_UClass FindClass(const std::string& const name);
};


inline TUObjectArray ObjObjects;
inline FNamePool NamePoolData;
