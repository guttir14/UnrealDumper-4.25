#include "wrappers.h"
#include "memory.h"

byte* TNameEntryArray::GetEntry(uint32_t id) const
{
	uint32_t ChunkIndex = id / 16384;
	uint32_t WithinChunkIndex = id % 16384;
	byte** Chunk = Chunks[ChunkIndex];
	return Read<byte*>(Chunk + WithinChunkIndex);
}

void TNameEntryArray::Dump(std::function<void(std::string_view name, uint32_t id)> callback) const
{
	for (auto i = 0; i < NumElements; i++)
	{
		auto entry = UE_FNameEntry(GetEntry(i));
		if (!entry) { continue; }
		auto name = entry.String();
		callback(name, i);
	}
}

byte* TUObjectArray::GetObjectPtr(uint64_t id) const
{
	if (id >= NumElements) { return nullptr; }
	return Read<byte*>(ObjObjects.Objects + id * 24);
}

void TUObjectArray::Dump(std::function<void(byte*)> callback) const
{
	for (uint32_t i = 0u; i < NumElements; i++)
	{
		byte* object = GetObjectPtr(i);
		if (!object) { continue; }
		callback(object);
	}
}

UE_UClass TUObjectArray::FindObject(const std::string& name) const
{
	for (uint32_t i = 0u; i < NumElements; i++)
	{
		UE_UClass object = GetObjectPtr(i);
		if (object && object.GetFullName() == name) { return object; }
	}
	return nullptr;
}

TNameEntryArray GlobalNames;
TUObjectArray ObjObjects;
