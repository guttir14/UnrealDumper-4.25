#include "wrappers.h"
#include "engine.h"
#include "memory.h"

uint8* FNamePool::GetEntry(FNameEntryHandle handle) const
{
	return reinterpret_cast<uint8*>(Blocks[handle.Block] + offsets.Stride * static_cast<uint64_t>(handle.Offset));
}

void FNamePool::DumpBlock(uint32 blockId, uint32 blockSize, std::function<void(std::string_view, uint32)> callback) const
{
	uint8* it = Blocks[blockId];
	uint8* end = it + blockSize - offsets.FNameEntry.HeaderSize;
	FNameEntryHandle entryHandle = { blockId, 0 };
	while (it < end)
	{
		auto entry = UE_FNameEntry(it);
		auto [wide, len] = entry.Info();
		if (len)
		{
			char buf[1024]{};
			entry.String(buf, wide, len);
			callback(std::string_view(buf, len), entryHandle);
			uint16_t size = UE_FNameEntry::Size(wide, len);
			entryHandle.Offset += size / offsets.Stride;
			it += size;
		}
		else { break; };
	}
}

void FNamePool::Dump(std::function<void(std::string_view, uint32)> callback) const
{
	for (uint32 i = 0u; i < CurrentBlock; i++) { DumpBlock(i, offsets.Stride * 65536, callback); }
	DumpBlock(CurrentBlock, CurrentByteCursor, callback);
}

uint8* TUObjectArray::GetObjectPtr(uint32 id) const
{
	if (id >= NumElements) return nullptr;
	uint64_t chunkIndex = id / 65536;
	if (chunkIndex >= NumChunks) return nullptr;
	uint8* chunk = Read<uint8*>(Objects + chunkIndex);
	if (!chunk) return nullptr;
	uint32 withinChunkIndex = id % 65536 * offsets.FUObjectItem.Size;
	auto item = Read<uint8*>(chunk + withinChunkIndex);
	return item;
}

void TUObjectArray::Dump(std::function<void(uint8*)> callback) const
{
	for (uint32 i = 0u; i < NumElements; i++)
	{
		uint8* object = GetObjectPtr(i);
		if (!object) { continue; }
		callback(object);
	}
}

UE_UClass TUObjectArray::FindObject(const std::string& name) const
{
	for (uint32 i = 0u; i < NumElements; i++)
	{
		UE_UClass object = GetObjectPtr(i);
		if (object && object.GetFullName() == name) { return object; }
	}
	return nullptr;
}

TUObjectArray ObjObjects;
FNamePool NamePoolData;