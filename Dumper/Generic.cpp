#include "wrappers.h"


void FNamePool::DumpBlock(uint32_t BlockIdx, uint32_t BlockSize, std::function<void(std::string_view, int32_t)> callback)
{
	byte* It = Entries.Blocks[BlockIdx];
	byte* End = It + BlockSize - FNameEntry::GetDataOffset();
	FNameEntryHandle entryHandle = { BlockIdx, 0 };
	while (It < End)
	{
		auto entry = Read<FNameEntry>(It);
		auto header = entry.GetHeader();
		if (uint32_t Len = header.Len)
		{
			callback(entry.GetView(), entryHandle);
			auto size = FNameEntry::GetSize(Len, !header.bIsWide);
			entryHandle.Offset += size / offsets.stride;
			It += size;
		}
		else { break; };
	}
}

void FNamePool::Dump(std::function<void(std::string_view, int32_t)> callback)
{
	for (auto i = 0; i < Entries.CurrentBlock; i++) { DumpBlock(i, offsets.stride * 65536, callback); }
	DumpBlock(Entries.CurrentBlock, Entries.CurrentByteCursor, callback);
}


void TUObjectArray::Dump(std::function<void(UObject*)> callback)
{
	for (auto i = 0; i < NumElements; i++)
	{
		UObject* object = ObjObjects.GetObjectPtr(i);
		if (!object) { continue; }
		callback(object);
	}
}

UE_UClass TUObjectArray::FindClass(const std::string& const name)
{
	for (auto i = 0; i < NumElements; i++)
	{
		UE_UClass object = GetObjectPtr(i);
		if (object.GetFullName() == name) { return object; }
	}
	return nullptr;
}
