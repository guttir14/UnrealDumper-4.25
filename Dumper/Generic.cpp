#include "wrappers.h"
#include "memory.h"

byte* FNamePool::GetEntry(FNameEntryHandle Handle) const
{
	return reinterpret_cast<byte*>(Blocks[Handle.Block] + defs.Stride * static_cast<uint64_t>(Handle.Offset));
}

void FNamePool::DumpBlock(uint32_t BlockIdx, uint32_t BlockSize, std::function<void(std::string_view, uint32_t)> callback) const
{
	byte* it = Blocks[BlockIdx];
	byte* end = it + BlockSize - defs.FNameEntry.HeaderSize;
	FNameEntryHandle entryHandle = { BlockIdx, 0 };
	while (it < end)
	{
		auto entry = UE_FNameEntry(it);
		auto [wide, len] = entry.Info();
		uint16_t size = UE_FNameEntry::Size(wide, len);
		if (len)
		{
			char buf[1024]{};
			entry.String(buf, wide, len);
			callback(std::string_view(buf, len), entryHandle);
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
	const uint64_t ChunkIndex = id / 65536;
	if (ChunkIndex >= NumChunks) return nullptr;
	const uint32_t WithinChunkIndex = id % 65536 * defs.FUObjectItem.Size;
	byte* Chunk = Read<byte*>(Objects + ChunkIndex);
	if (!Chunk) return nullptr;
	auto item = Read<byte*>(Chunk + WithinChunkIndex + defs.FUObjectItem.Object);
	return item;
}

void TUObjectArray::Dump(std::function<void(byte*)> callback) const
{
	for (auto i = 0u; i < NumElements; i++)
	{
		byte* object = ObjObjects.GetObjectPtr(i);
		if (!object) { continue; }
		callback(object);
	}
}

UE_UClass TUObjectArray::FindObject(const std::string& name) const
{
	for (auto i = 0u; i < NumElements; i++)
	{
		UE_UClass object = GetObjectPtr(i);
		if (object.GetFullName() == name) { return object; }
	}
	return nullptr;
}
