#include "engine.h"
#include "generic.h"
#include "memory.h"
#include "utils.h"
#include "wrappers.h"
#define USE_ZYDIS 0
#if USE_ZYDIS
#define ZYDIS_STATIC_DEFINE
#define ZYCORE_STATIC_DEFINE
#include <Zydis/Zydis.h>
#endif

Offsets offsets;

ansi_fn Decrypt_ANSI = nullptr;
// wide_fn Decrypt_WIDE = nullptr;

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
} RogueCompany;
static_assert(sizeof(RogueCompany) == sizeof(Offsets));

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
} Fortnite;
static_assert(sizeof(Fortnite) == sizeof(Offsets));

struct {
  void* offsets; // address to filled offsets structure
  std::pair<const char*, uint32> names; // NamePoolData signature
  std::pair<const char*, uint32> objects; // ObjObjects signature
  std::function<bool(std::pair<uint8*, uint8*>*)> callback;
} engines[] = {
  { // RogueCompany | PropWitchHuntModule-Win64-Shipping | Scum
    &RogueCompany,
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
    {"\x48\x8D\x0D\x00\x00\x00\x00\xE9\x73\xAB\xFF\xFF", 12},
    {"\x48\x8B\x05\x00\x00\x00\x00\x48\x63\x8C\x24\xE0", 12},
    nullptr
  },
  { // POLYGON-Win64-Shipping
    &RogueCompany,
    {"\x48\x8D\x35\x00\x00\x00\x00\xEB\x16", 9},
    {"\x48\x8d\x1d\x00\x00\x00\x00\x39\x44\x24\x68", 11},
    nullptr
  },
  { // FortniteClient-Win64-Shipping
    &Fortnite,
    {"\x4C\x8D\x35\x00\x00\x00\x00\x0F\x10\x07\x83\xFB\x01", 13},
    {"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x0C\xC8\x48\x8D\x04\xD1\xEB", 16},
    [](std::pair<uint8*, uint8*>* s) {
      if (!Decrypt_ANSI) {
        auto var = FindPointer(s->first, s->second, "\x7F\x0B\x8B\x05\x00\x00\x00\x00\x48\x83\xC4\x28\xC3", 14);
        if (var) {
          auto decryptAnsi = FindPointer(s->first, s->second, "\xE8\x00\x00\x00\x00\x0F\xB7\x1B\xC1\xEB\x06\x4C\x89\x36\x4C\x89\x76\x08\x85\xDB\x74\x48", 22);
          if (decryptAnsi) {
            Decrypt_ANSI = (ansi_fn)VirtualAlloc(0, 150, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            if (Decrypt_ANSI) {
              /*
              mov [rsp+8], rbx
              push rdi
              sub rsp, 0x20
              mov ebx, edx
              mov rdi, rcx
              mov eax, 0
              */
              uint8 ins[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x89, 0xD3, 0x48, 0x89, 0xCF, 0xB8, 0x00, 0x00, 0x00, 0x00 };

#if USE_ZYDIS
#pragma comment(lib, "Zydis.lib")
              ZydisDecoder decoder;
              if (ZYAN_SUCCESS(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64))) {
                ZydisDecodedInstruction previousInstruction{};
                ZydisDecodedInstruction instruction{};
                int32 fnSize = 0;
                char callsCount = 0;
                auto buffer = (uint8*)decryptAnsi;

                ZydisRegister lenReg = ZYDIS_REGISTER_NONE;
                ZydisRegister bufReg = ZYDIS_REGISTER_NONE;
                int offset = 0;

                while (instruction.mnemonic != ZYDIS_MNEMONIC_RET) {
                  previousInstruction = instruction;

                  if (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, buffer, 15, &instruction))) { // infinite loop on fail :)
                    fnSize += instruction.length;
                    buffer += instruction.length;

                    switch (instruction.mnemonic) {
                    case ZYDIS_MNEMONIC_CALL:
                      callsCount++;
                      break;
                    case ZYDIS_MNEMONIC_TEST:
                      lenReg = instruction.operands[0].reg.value;
                      break;
                    }

                    if (!bufReg && callsCount == 1) {
                      bufReg = previousInstruction.operands[1].reg.value;
                    } else if (!offset && callsCount == 2) {
                      offset = fnSize;
                    }
                  }
                };

                switch (lenReg) {
                case ZYDIS_REGISTER_EDI:
                  ins[11] = 0xD7;
                  break;
                case ZYDIS_REGISTER_EBX:
                  ins[11] = 0xD3;
                  break;
                }

                switch (bufReg) {
                case ZYDIS_REGISTER_RDI:
                  ins[14] = 0xCF;
                  break;
                case ZYDIS_REGISTER_RBX:
                  ins[14] = 0xCB;
                  break;
                }

                ((uint32 *)ins)[4] = *((uint32 *)var);
                memcpy(Decrypt_ANSI, ins, sizeof(ins));
                memcpy((uint8 *)Decrypt_ANSI + sizeof(ins), (uint8 *)decryptAnsi + offset, 150 - sizeof(ins));
                return true;
              }

#else
              ((uint32 *)ins)[4] = *((uint32 *)var);
              memcpy(Decrypt_ANSI, ins, sizeof(ins));
              memcpy((uint8 *)Decrypt_ANSI + sizeof(ins), (uint8 *)decryptAnsi + 0x2F, 150 - sizeof(ins));
              return true;
#endif
            }
          }
        }
      }
      return false;
    }
  }
};

std::unordered_map<std::string, decltype(&engines[0])> games = {
    {"RogueCompany", &engines[0]},
    {"SCUM", &engines[0]},
    {"PropWitchHuntModule-Win64-Shipping", &engines[0]},
    {"HLL-Win64-Shipping", &engines[0]},
    {"DeadByDaylight-Win64-Shipping", &engines[1]},
    {"Brickadia-Win64-Shipping", &engines[2]},
    {"POLYGON-Win64-Shipping", &engines[3]},
    {"FortniteClient-Win64-Shipping", &engines[4]}
};

STATUS EngineInit(std::string game, void* image) {
  auto sections = GetExSections(image);
  auto it = games.find(game);
  if (it == games.end()) { return STATUS::ENGINE_NOT_FOUND; }

  auto engine = it->second;
  offsets = *(Offsets*)(engine->offsets);

  void* names = nullptr; 
  void* objects = nullptr;
  bool callback = false;

  uint8 found = 0;
  if (!engine->callback) {
    callback = true;
    found |= 4;
  }

  for (auto i = 0; i < sections.size(); i++) {
    auto &s = sections.at(i);
    if (!names) if (names = FindPointer(s.first, s.second, engine->names.first, engine->names.second)) found |= 1;
    if (!objects) if (objects = FindPointer(s.first, s.second, engine->objects.first, engine->objects.second)) found |= 2;
    if (!callback) if (callback = engine->callback(&s)) found |= 4;
    if (found == 7) break;
  }

  if (found != 7) return STATUS::ENGINE_FAILED;

  NamePoolData = *(decltype(NamePoolData)*)names;
  ObjObjects = *(decltype(ObjObjects)*)objects;

  auto entry = UE_FNameEntry(NamePoolData.GetEntry(0));
  if (entry.String() != "None") return STATUS::ENGINE_FAILED;

  return STATUS::SUCCESS;
}
