#include "wrappers.h"
#include "memory.h"

byte* FNamePool::GetEntry(FNameEntryHandle handle) const
{
	return reinterpret_cast<byte*>(Blocks[handle.Block] + defs.Stride * static_cast<uint64_t>(handle.Offset));
}

void FNamePool::DumpBlock(uint32_t blockId, uint32_t blockSize, std::function<void(std::string_view, uint32_t)> callback) const
{
	byte* it = Blocks[blockId];
	byte* end = it + blockSize - defs.FNameEntry.HeaderSize;
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
			entryHandle.Offset += size / defs.Stride;
			it += size;
		}
		else { break; };
	}
}

void FNamePool::Dump(std::function<void(std::string_view, uint32_t)> callback) const
{
	for (auto i = 0u; i < CurrentBlock; i++) { DumpBlock(i, defs.Stride * 65536, callback); }
	DumpBlock(CurrentBlock, CurrentByteCursor, callback);
}

byte* TUObjectArray::GetObjectPtr(uint32_t id) const
{
	if (id >= NumElements) return nullptr;
	uint64_t chunkIndex = id / 65536;
	if (chunkIndex >= NumChunks) return nullptr;
	byte* chunk = Read<byte*>(Objects + chunkIndex);
	if (!chunk) return nullptr;
	uint32_t withinChunkIndex = id % 65536 * defs.FUObjectItem.Size;
	auto item = Read<byte*>(chunk + withinChunkIndex);
	return item;
}

void TUObjectArray::Dump(std::function<void(byte*)> callback) const
{
	for (auto i = 0u; i < NumElements; i++)
	{
		byte* object = GetObjectPtr(i);
		if (!object) { continue; }
		callback(object);
	}
}

UE_UClass TUObjectArray::FindObject(const std::string& name) const
{
	for (auto i = 0u; i < NumElements; i++)
	{
		UE_UClass object = GetObjectPtr(i);
		if (object && object.GetFullName() == name) { return object; }
	}
	return nullptr;
}

TUObjectArray ObjObjects;
FNamePool NamePoolData;