#include "engine.h"
#include "memory.h"
#include "wrappers.h"

uint8* FNamePool::GetEntry(FNameEntryHandle handle) const {
  if (handle.Block >= 8192) return nullptr;
  return (uint8*)(Blocks[handle.Block] + offsets.Stride * (uint64)(handle.Offset));
}

void FNamePool::DumpBlock(uint32 blockId, uint32 blockSize, std::function<void(std::string_view, uint32)> callback) const {
  uint8* it = Blocks[blockId];
  uint8* end = it + blockSize - offsets.FNameEntry.HeaderSize;
  FNameEntryHandle entryHandle = {blockId, 0};
  while (it < end) {
    auto entry = UE_FNameEntry(it);
    auto [wide, len] = entry.Info();
    if (len) {
      char buf[1024];
      entry.String(buf, wide, len);
      callback(std::string_view(buf, len), entryHandle);
      uint16 size = UE_FNameEntry::Size(wide, len);
      entryHandle.Offset += size / offsets.Stride;
      it += size;
    } else {
      break;
    };
  }
}

void FNamePool::Dump(std::function<void(std::string_view, uint32)> callback) const {
  for (uint32 i = 0; i < CurrentBlock; i++) {
    DumpBlock(i, offsets.Stride * 65536, callback);
  }
  DumpBlock(CurrentBlock, CurrentByteCursor, callback);
}

uint8* TUObjectArray::GetObjectPtr(uint32 id) const {
  if (id >= NumElements) return nullptr;
  uint64 chunkIndex = id / 65536;
  if (chunkIndex >= NumChunks) return nullptr;
  uint8 *chunk = Read<uint8*>(Objects + chunkIndex);
  if (!chunk) return nullptr;
  uint32 withinChunkIndex = id % 65536 * offsets.FUObjectItem.Size;
  auto item = Read<uint8*>(chunk + withinChunkIndex);
  return item;
}

void TUObjectArray::Dump(std::function<void(uint8 *)> callback) const {
  for (uint32 i = 0; i < NumElements; i++) {
    uint8* object = GetObjectPtr(i);
    if (!object) continue;
    callback(object);
  }
}

UE_UObject TUObjectArray::FindObject(const std::string &name) const {
  for (uint32 i = 0; i < NumElements; i++) {
    UE_UObject object = GetObjectPtr(i);
    if (object && object.GetFullName() == name) {
      return object;
    }
  }
  return nullptr;
}

void TUObjectArray::ForEachObjectOfClass(const UE_UClass cmp, std::function<bool(uint8*)> callback) const {
  for (uint32 i = 0; i < NumElements; i++) {
    UE_UObject object = GetObjectPtr(i);
    if (object && object.IsA(cmp) && object.GetName().find("_Default") == std::string::npos) {
      if (callback(object)) return;
    }
  }
}

bool TUObjectArray::IsObject(UE_UObject address) const {
  for (uint32 i = 0; i < NumElements; i++) {
    UE_UObject object = GetObjectPtr(i);
    if (address == object) {
      return true;
    }
  }
  return false;
}

TUObjectArray ObjObjects;
FNamePool NamePoolData;
