#pragma once
#include <functional>
#include "Engine.h"

struct FNameEntryHandle
{
	uint32_t Block = 0;
	uint32_t Offset = 0;
	FNameEntryHandle(uint32_t _Block, uint32_t _Offset)
	{
		Block = _Block;
		Offset = _Offset;
	};
	FNameEntryHandle(uint32_t Id)
	{
		Block = Id >> 16;
		Offset = Id & 65535;
	}
	operator uint32_t() const
	{
		return (Block << 16 | Offset);
	}
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
			header = *(FNameEntryHeader*)((char*)this + offsets.header);
		}
		else {
			auto head = *(FNameEntryHeader1*)(this);
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
	char Lock[8];
	uint32_t CurrentBlock;
	uint32_t CurrentByteCursor;
	uint8_t* Blocks[8192];
};

struct FNamePool {
	FNameEntryAllocator Entries;

	FNameEntry* GetEntry(FNameEntryHandle Handle) { return reinterpret_cast<FNameEntry*>(Entries.Blocks[Handle.Block] + offsets.stride * (uint64_t)Handle.Offset); }

	void DumpBlock(uint32_t BlockIdx, uint32_t BlockSize, std::function<void(std::string_view, int32_t)> callback)
	{
		uint8_t* It = Entries.Blocks[BlockIdx];
		uint8_t* End = It + BlockSize - FNameEntry::GetDataOffset();
		FNameEntryHandle entryHandle = { BlockIdx, 0 };
		while (It < End)
		{
			auto entry = Read<FNameEntry>(It);
			auto header = entry.GetHeader();
			if (uint32_t Len = header.Len)
			{
				callback(entry.GetView(), entryHandle / offsets.stride);
				auto size = FNameEntry::GetSize(Len, !header.bIsWide);
				entryHandle.Offset += size;
				It += size;
			}
			else { break; };
		}
	}


	void Dump(std::function<void(std::string_view, int32_t)> callback)
	{
		for (uint32_t BlockIdx = 0; BlockIdx < Entries.CurrentBlock; ++BlockIdx)
		{
			DumpBlock(BlockIdx, offsets.stride * 65536, callback);
		}
		DumpBlock(Entries.CurrentBlock, Entries.CurrentByteCursor, callback);
	}
};

class UE_UClass;
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


	UE_UClass FindClass(std::string name);
};


inline TUObjectArray ObjObjects;
inline FNamePool NamePoolData;
