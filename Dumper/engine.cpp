#include <Windows.h>
#include "engine.h"
#include "generic.h"
#include "memory.h"
#include "utils.h"
#include "wrappers.h"

Offsets offsets;

ansi_fn Decrypt_ANSI = nullptr;
// wide_fn Decrypt_WIDE = nullptr;

struct {
  uint16 Stride = 2;
  struct {
    uint16 Size = 24;
  } FUObjectItem;
  struct {
    uint16 Number = 4;
  } FName;
  struct {
    uint16 Info = 0;
    uint16 WideBit = 0;
    uint16 LenBit = 6;
    uint16 HeaderSize = 2;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x20;
  } UObject;
  struct {
    uint16 Next = 0x28;
  } UField;
  struct {
    uint16 SuperStruct = 0x40;
    uint16 Children = 0x48;
    uint16 ChildProperties = 0x50;
    uint16 PropertiesSize = 0x58;
  } UStruct;
  struct {
    uint16 Names = 0x40;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB0;
    uint16 Func = 0xB0 + 0x28;
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x78;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} Default;
static_assert(sizeof(Default) == sizeof(Offsets));

struct
{
    uint16 Stride = 2;
    struct
    {
        uint16 Size = 24;
    } FUObjectItem;
    struct
    {
        uint16 Number = 4;
    } FName;
    struct
    {
        uint16 Info = 0;
        uint16 WideBit = 0;
        uint16 LenBit = 6;
        uint16 HeaderSize = 2;
    } FNameEntry;
    struct
    {
        uint16 Index = 0xC;
        uint16 Class = 0x10;
        uint16 Name = 0x18;
        uint16 Outer = 0x20;
    } UObject;
    struct
    {
        uint16 Next = 0x28;
    } UField;
    struct
    {
        uint16 SuperStruct = 0x40;
        uint16 Children = 0x48;
        uint16 ChildProperties = 0;
        uint16 PropertiesSize = 0x50;
    } UStruct;
    struct
    {
        uint16 Names = 0x40;
    } UEnum;
    struct
    {
        uint16 FunctionFlags = 0x98;
        uint16 Func = 0xC0;
    } UFunction;
    struct
    {
        uint16 Class = 0;
        uint16 Next = 0;
        uint16 Name = 0;
    } FField;
    struct
    {
        uint16 ArrayDim = 0;
        uint16 ElementSize = 0;
        uint16 PropertyFlags = 0;
        uint16 Offset = 0;
        uint16 Size = 0;
    } FProperty;
    struct
    {
        uint16 ArrayDim = 0x30;
        uint16 ElementSize = 0x34;
        uint16 PropertyFlags = 0x38;
        uint16 Offset = 0x44;
        uint16 Size = 0x70; // sizeof(UProperty)
    } UProperty;
} Squad;
static_assert(sizeof(Squad) == sizeof(Offsets));

struct {
  uint16 Stride = 4;
  struct {
    uint16 Size = 24;
  } FUObjectItem;
  struct {
    uint16 Number = 8;
  } FName;
  struct {
    uint16 Info = 4;
    uint16 WideBit = 0;
    uint16 LenBit = 1;
    uint16 HeaderSize = 6;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x28;
  } UObject;
  struct {
    uint16 Next = 0x30;
  } UField;
  struct {
    uint16 SuperStruct = 0x48;
    uint16 Children = 0x50;
    uint16 ChildProperties = 0x58;
    uint16 PropertiesSize = 0x60;
  } UStruct;
  struct {
    uint16 Names = 0x48;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB8;
    uint16 Func = 0xB8 + 0x28; // ue3-ue4, always +0x28 from flags location.
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x80;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} DeadByDaylight;
static_assert(sizeof(DeadByDaylight) == sizeof(Offsets));

struct {
  uint16 Stride = 2;
  struct {
    uint16 Size = 24;
  } FUObjectItem;
  struct {
    uint16 Number = 4;
  } FName;
  struct {
    uint16 Info = 0;
    uint16 WideBit = 0;
    uint16 LenBit = 6;
    uint16 HeaderSize = 2;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x20;
  } UObject;
  struct {
    uint16 Next = 0x28;
  } UField;
  struct {
    uint16 SuperStruct = 0x40;
    uint16 Children = 0x48;
    uint16 ChildProperties = 0x50;
    uint16 PropertiesSize = 0x58;
  } UStruct;
  struct {
    uint16 Names = 0x40;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB0;
    uint16 Func = 0xB0 + 0x30;
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x78;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} Scavengers;
static_assert(sizeof(Scavengers) == sizeof(Offsets));

struct {
  uint16 Stride = 2;
  struct {
    uint16 Size = 32;
  } FUObjectItem;
  struct {
    uint16 Number = 4;
  } FName;
  struct {
    uint16 Info = 0;
    uint16 WideBit = 0;
    uint16 LenBit = 6;
    uint16 HeaderSize = 2;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x20;
  } UObject;
  struct {
    uint16 Next = 0x28;
  } UField;
  struct {
    uint16 SuperStruct = 0x40;
    uint16 Children = 0x48;
    uint16 ChildProperties = 0x50;
    uint16 PropertiesSize = 0x58;
  } UStruct;
  struct {
    uint16 Names = 0x40;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB0;
    uint16 Func = 0xB0 + 0x28;
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x78;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} Brickadia;
static_assert(sizeof(Brickadia) == sizeof(Offsets));

struct {
  uint16 Stride = 4;
  struct {
    uint16 Size = 24;
  } FUObjectItem;
  struct {
    uint16 Number = 8;
  } FName;
  struct {
    uint16 Info = 4;
    uint16 WideBit = 0;
    uint16 LenBit = 1;
    uint16 HeaderSize = 6;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x28;
  } UObject;
  struct {
      uint16 Next = 0x30;
  } UField;
  struct {
    uint16 SuperStruct = 0x48;
    uint16 Children = 0x50;
    uint16 ChildProperties = 0x58;
    uint16 PropertiesSize = 0x60;
  } UStruct;
  struct {
    uint16 Names = 0x48;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB8;
    uint16 Func = 0xB8 + 0x28;
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x80;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} Core;
static_assert(sizeof(Core) == sizeof(Offsets));

struct {
  void* offsets; // address to filled offsets structure
  std::pair<const char*, uint32> names; // NamePoolData signature
  std::pair<const char*, uint32> objects; // ObjObjects signature
  std::function<bool(void*, void*)> callback;
} engines[] = {
  { // RogueCompany | PropWitchHuntModule-Win64-Shipping | Scum
    &Default,
    {"\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x01\x0F\x10\x03\x4C\x8D\x44\x24\x20\x48\x8B\xC8", 30},
    {"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x0C\xC8\x48\x8D\x04\xD1\xEB", 16},
    nullptr
  },
  { // Scavenger-Win64-Shipping
    &Scavengers,
    {"\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x01\x0F\x10\x03\x4C\x8D\x44\x24\x20\x48\x8B\xC8", 30},
    {"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x0C\xC8\x48\x8D\x04\xD1\xEB", 16},
    nullptr
  },
  { // DeadByDaylight-Win64-Shipping
    &DeadByDaylight,
    {"\x48\x8D\x35\x00\x00\x00\x00\xEB\x16", 9},
    {"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x0C\xC8\x48\x8D\x04\xD1\xEB", 16},
    nullptr
  },
  { // Brickadia-Win64-Shipping
    &Brickadia,
    {"\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x00\x85", 20},
    {"\x48\x8B\x05\x00\x00\x00\x00\x48\x63\x8C\x24\xE0", 12},
    nullptr
  },
  { // POLYGON-Win64-Shipping
    &Default,
    {"\x48\x8D\x35\x00\x00\x00\x00\xEB\x16", 9},
    {"\x48\x8d\x1d\x00\x00\x00\x00\x39\x44\x24\x68", 11},
    nullptr
  },
  { // FortniteClient-Win64-Shipping
    &Default,
    {"\x4C\x8D\x35\x00\x00\x00\x00\x0F\x10\x07\x83\xFB\x01", 13},
    {"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x0C\xC8\x48\x8D\x04\xD1\xEB", 16},
    [](void* start, void* end) {
      if (!Decrypt_ANSI) {
        auto decryptAnsi = FindPointer(start, end, "\xE8\x00\x00\x00\x00\x0F\xB7\x3F\x33\xF6\xC1\xEF\x06\x48\x89\x33\x48\x89\x73\x08\x85\xFF\x0F\x84\x00\x00\x00\x00\x40\x00\x00\x00\x00", 33);
        if (decryptAnsi) {
          /*
          mov [rsp +8], rbx
          push rdi
          sub rsp, 0x20
          mov ebx, edx
          mov rdi, rcx
          mov rax, 0xDEADBEEFDEADBEEF
          jmp rax
          */
          uint8 trampoline[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x89, 0xD3, 0x48, 0x89, 0xCF, 0x48, 0xB8, 0xEF, 0xBE, 0xAD, 0xDE, 0xEF, 0xBE, 0xAD, 0xDE, 0xFF, 0xE0 };
          *(uint64*)(trampoline + 17) = (uint64)((uint8*)decryptAnsi + 0x4A); // https://i.imgur.com/zWtMDar.png
          Decrypt_ANSI = (ansi_fn)VirtualAlloc(0, sizeof(trampoline), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
          if (Decrypt_ANSI) {
            memcpy((void*)Decrypt_ANSI, trampoline, sizeof(trampoline));
            return true;
          }
        }
      }
      return false;
    }
  },
  { // TheIsleClient-Win64-Shipping
    &Default,
    {"\x48\x8D\x05\x00\x00\x00\x00\xEB\x13", 9},
    {"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x0C\xC8\x48\x8D\x04\xD1\xEB\x03", 17},
    nullptr
  },
  { // PortalWars-Win64-Shipping
    &Default,
    {"\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x01\x0F\x10\x03\x4C\x8D\x44\x24\x20\x48\x8B\xC8", 30},
    {"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x0C\xC8\x48\x8D\x1C\xD1\xEB\x03\x49\x8B\xDD", 20},
    nullptr
  },
  { // Tiger-Win64-Shipping.exe
    &Default,
    {"\x48\x8D\x35\x00\x00\x00\x00\xEB\x16", 9},
    {"\x48\x8d\x1d\x00\x00\x00\x00\x39\x44\x24\x68", 11},
    nullptr
  },
  { // Platform-Win64-Shipping.exe
    &Core,
    {"\x48\x8D\x35\x00\x00\x00\x00\xEB\x16", 9},
    {"\x48\x8d\x1d\x00\x00\x00\x00\x39\x44\x24\x68", 11},
    nullptr
  },
  {//Beached-Win64-Test.exe
   &Default,
   {"\x48\x8D\x0D\x00\x00\x00\x00\xE8\xC6\x05\x00\x00\x00\x00\x0F\x10\x03\x4C\x8D\x44\x24\x20\x48\x8B\xC8",30},
   {"\x48\x8B\x05\x00\x00\x00\x00\xC1\xF9\x10\x48\x63\xC9\x48\x8B\x14\xC8\x4B\x8D\x0C\x40\x4C\x8D\x04\xCA\xEB\x03",27},
   nullptr
  },
  {//PromodClient-Win64-Shipping.exe
   &Default,
   {"\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x01\x0F\x10\x03\x4C\x8D\x44\x24\x20\x48\x8B\xC8",30},
   {"\x48\x8B\x15\x00\x00\x00\x00\x0F\x1F\x44\x00\x00",12},
   nullptr
  }, 
  {//Dauntless-Win64-Shipping.exe
   &Default,
	{"\x48\x8d\x0d\x00\x00\x00\x00\xe8\x00\x00\x00\x00\xc6\x05\x00\x00\x00\x00\x00\x0f\x10\x03", 22}, //GName
	{"\x48\x8B\x05\x00\x00\x00\x00\xC1\xF9", 9},//Gobject 
	nullptr
  },
  {//SquadGame.exe
   &Squad,
    {"\x4C\x8D\x05\x00\x00\x00\x00\xEB\x16\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00", 22}, //GName
    {"\x48\x8D\x1D\x00\x00\x00\x00\x8B\x05\x00\x00\x00\x00\x39\x45\x88\x7C\x20\x48\x8D\x45\x88\x48\x89\x85\x90\x05\x00\x00", 30}, //Gobject 
    nullptr
  }
};

std::unordered_map<std::string, decltype(&engines[0])> games = {
  {"RogueCompany", &engines[0]},
  {"SCUM", &engines[0]},
  {"PropWitchHuntModule-Win64-Shipping", &engines[0]},
  {"HLL-Win64-Shipping", &engines[0]},
  {"Scavenger-Win64-Shipping", &engines[1]},
  {"DeadByDaylight-Win64-Shipping", &engines[2]},
  {"Brickadia-Win64-Shipping", &engines[3]},
  {"POLYGON-Win64-Shipping", &engines[4]},
  {"FortniteClient-Win64-Shipping", &engines[5]},
  {"TheIsleClient-Win64-Shipping", &engines[6]},
  {"PortalWars-Win64-Shipping", &engines[7]},
  {"Tiger-Win64-Shipping", &engines[0]},
  {"Platform-Win64-Shipping", &engines[9]},
  {"Beached-Win64-Test",& engines[0]},
  {"PromodClient-Win64-Shipping",&engines[0]},
  {"Dauntless-Win64-Shipping",&engines[12]},
  {"ProjectBoundarySteam-Win64-Shipping",&engines[0]},
  {"Prospect-Win64-Shipping",&engines[0]},
  {"SquadGame", &engines[13]},
};

STATUS EngineInit(std::string game, void* image) {

  auto it = games.find(game);
  if (it == games.end()) { return STATUS::ENGINE_NOT_FOUND; }

  auto engine = it->second;
  offsets = *(Offsets*)(engine->offsets);

  void* names = nullptr; 
  void* objects = nullptr;

  uint8 found = 0;
  if (!engine->callback) {
    found |= 4;
  }

  IterateExSections(
    image,
    [&](void* start, void* end)->bool {
      if (!(found & 1)) if (names = FindPointer(start, end, engine->names.first, engine->names.second)) found |= 1;
      if (!(found & 2)) if (objects = FindPointer(start, end, engine->objects.first, engine->objects.second)) found |= 2;
      if (!(found & 4)) if (engine->callback(start, end)) found |= 4;
      if (found == 7) return 1;
      return 0;
    }
  );

  if (found != 7) return STATUS::ENGINE_FAILED;

  NamePoolData = *(decltype(NamePoolData)*)names;
  ObjObjects = *(decltype(ObjObjects)*)objects;

  auto entry = UE_FNameEntry(NamePoolData.GetEntry(0));

  // exception handler exclusively for Decrypt_ANSI
  try {
    if (*(uint32*)entry.String().data() != 'enoN') return STATUS::ENGINE_FAILED;
  }
  catch (...) {
    return STATUS::ENGINE_FAILED;
  }

  return STATUS::SUCCESS;
}
