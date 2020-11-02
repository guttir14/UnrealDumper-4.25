#pragma once
#include "Engine.h"


enum { FNameMaxBlockBits = 13 };
enum { FNameBlockOffsetBits = 16 };
enum { FNameMaxBlocks = 1 << FNameMaxBlockBits };
enum { FNameBlockOffsets = 1 << FNameBlockOffsetBits };
struct FNameEntryHandle
{

	uint32_t Block = 0;
	uint32_t Offset = 0;

	FNameEntryHandle(uint32_t _Block, uint32_t _Offset)
		: Block(_Block)
		, Offset(_Offset)
	{}

	FNameEntryHandle(uint32_t Id)
		: Block(Id >> FNameBlockOffsetBits)
		, Offset(Id& (FNameBlockOffsets - 1))
	{}

	operator uint32_t() const
	{
		return (Block << FNameBlockOffsetBits | Offset);
	}
};


#if WITH_CASE_PRESERVING_NAME
enum { Stride = 4 };
#else
enum { Stride = 2 };
#endif



struct FNameEntryHeader {
	uint16_t bIsWide : 1;
#if WITH_CASE_PRESERVING_NAME
	uint16_t Len : 15;
#else
	static constexpr uint32_t ProbeHashBits = 5;
	uint16_t LowercaseProbeHash : ProbeHashBits;
	uint16_t Len : 10;
#endif
};

enum { NAME_SIZE = 1024 };
struct FNameEntry
{
#if WITH_CASE_PRESERVING_NAME
	uint32_t ComparisonId;
#endif
	FNameEntryHeader header;
	union
	{
		char	AnsiName[NAME_SIZE];
		wchar_t	WideName[NAME_SIZE];
	};

	std::string_view GetViewA() const {
		return std::string_view(AnsiName, header.Len);
	}

	std::wstring_view GetViewW() const {
		return std::wstring_view(WideName, header.Len);
	}

	std::string GetStringA() const
	{
		auto str = std::string(AnsiName, header.Len);
		return str;
	}

	std::wstring GetStringW() const
	{
		return std::wstring(WideName, header.Len);
	}

	std::string GetString() const
	{
		return GetStringA();
	}

	std::string_view GetView() const
	{
		return GetViewA();
	}

	static inline int32_t GetDataOffset()
	{
		return offsetof(FNameEntry, AnsiName);
	}

	static inline int32_t GetSize(int32_t Length, bool bIsPureAnsi)
	{
		int32_t Bytes = GetDataOffset() + Length * (bIsPureAnsi ? sizeof(char) : sizeof(wchar_t));
		return (Bytes + Stride - 1) & ~(Stride - 1);
	}

};


enum { BlockSizeBytes = Stride * FNameBlockOffsets };
struct FNameEntryAllocator
{
	char Lock[8];
	uint32_t CurrentBlock;
	uint32_t CurrentByteCursor;
	uint8_t* Blocks[FNameMaxBlocks];

	void DumpBlock(uint32_t BlockIdx, uint32_t BlockSize, std::vector<std::pair<FNameEntry*, uint32_t>>& Out)
	{
		uint8_t* It = Blocks[BlockIdx];
		uint8_t* End = It + BlockSize - FNameEntry::GetDataOffset();
		FNameEntryHandle entryHandle = { BlockIdx, 0 };
		while (It < End)
		{
			FNameEntryHeader header = Read<FNameEntryHeader>(It + offsetof(FNameEntry, FNameEntry::header));
			if (uint16_t Len = header.Len)
			{
				Out.push_back({ reinterpret_cast<FNameEntry*>(It), entryHandle / Stride });
				auto size = FNameEntry::GetSize(Len, !header.bIsWide);
				entryHandle.Offset += size;
				It += size;
			}
			else { break; };
		}
	}

	void Dump(std::vector<std::pair<FNameEntry*, uint32_t>>& Out)
	{
		for (uint32_t BlockIdx = 0; BlockIdx < CurrentBlock; ++BlockIdx)
		{
			DumpBlock(BlockIdx, BlockSizeBytes, Out);
		}
		DumpBlock(CurrentBlock, CurrentByteCursor, Out);
	}

};

struct FNamePool {
	FNameEntryAllocator Entries;

	FNameEntry* GetEntry(FNameEntryHandle Handle)
	{
		return reinterpret_cast<FNameEntry*>(Entries.Blocks[Handle.Block] + Stride * (uint64_t)Handle.Offset);
	}
};

class UE_UClass;
struct TUObjectArray
{
	enum
	{
		NumElementsPerChunk = 64 * 1024,
	};

	FUObjectItem** Objects;
	FUObjectItem* PreAllocatedObjects;
	int32_t MaxElements;
	int32_t NumElements;
	int32_t MaxChunks;
	int32_t NumChunks;

	UObject* GetObjectPtr(int32_t Index)
	{
		if (Index >= NumElements) return nullptr;
		const uint64_t ChunkIndex = Index / NumElementsPerChunk;
		if (ChunkIndex >= NumChunks) return nullptr;
		const uint32_t WithinChunkIndex = Index % NumElementsPerChunk;
		FUObjectItem* Chunk = Read<FUObjectItem*>(Objects + ChunkIndex * 8);
		if (!Chunk) return nullptr;
		auto item = Read<FUObjectItem>(Chunk + WithinChunkIndex);
		return item.Object;
	}


	UE_UClass FindClass(std::string name);
};


inline TUObjectArray ObjObjects;
inline FNamePool* NamePoolData = new FNamePool;
