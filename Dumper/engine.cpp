#include "engine.h"
#include "utils.h"
#include "generic.h"
#include "memory.h"

Offsets offsets;

ansi_fn Decrypt_ANSI = nullptr;
wide_fn Decrypt_WIDE = nullptr;

struct DeadByDaylight {
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
};
static_assert(sizeof(DeadByDaylight) == sizeof(Offsets));

struct RogueCompany {
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
};
static_assert(sizeof(RogueCompany) == sizeof(Offsets));

struct Brickadia {
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
};
static_assert(sizeof(Brickadia) == sizeof(Offsets));

struct Fortnite {
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
};
static_assert(sizeof(Fortnite) == sizeof(Offsets));

bool FindData(std::vector<std::pair<uint8*, uint8*>>* sections, std::pair<uint8*, uint8> namePoolDataSig, std::pair<uint8*, uint8> objObjectsSig, std::function<bool(std::pair<uint8*, uint8*>&)> callback) {
	
	void* a1 = nullptr;
	void* a2 = nullptr;
	bool b1 = false;
	bool b2 = false;
	if (!callback) b1 = true;
	for (auto i = 0; i < sections->size(); i++)
	{
		auto& s = sections->at(i);

		if (!a1) { a1 = FindPointer(s.first, s.second, namePoolDataSig.first, namePoolDataSig.second); }
		if (!a2) { a2 = FindPointer(s.first, s.second, objObjectsSig.first, objObjectsSig.second); }
		if (!b1) (b1 = callback(s));
		if (a1 && a2 && b1) { b2 = true; break; }
	}
	if (!b2) return false;
	NamePoolData = *reinterpret_cast<decltype(NamePoolData)*>(a1);
	ObjObjects = *reinterpret_cast<decltype(ObjObjects)*>(a2);
	return b2;
}

std::unordered_map<std::string, std::function<STATUS(std::vector<std::pair<uint8*, uint8*>>*)>> games = {
	{
		{
			"DeadByDaylight-Win64-Shipping",
			[](std::vector<std::pair<uint8*, uint8*>>* sections) {
				uint8 namePoolDataSig[] = { 0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00, 0xeb, 0x16 };
				uint8 objObjectsSig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0C, 0xC8, 0x48, 0x8D, 0x04, 0xD1, 0xEB };

				if (!FindData(sections, { namePoolDataSig, sizeof(namePoolDataSig) }, { objObjectsSig, sizeof(objObjectsSig) }, nullptr))
				{
					return STATUS::ENGINE_FAILED;
				};

				DeadByDaylight buf{};
				offsets = *reinterpret_cast<Offsets*>(&buf);

				return STATUS::SUCCESS;
			}
		},
		{
			"RogueCompany",
			[](std::vector<std::pair<uint8*, uint8*>>* sections) {
				
				uint8 namePoolDataSig[] = { 0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00, 0xeb, 0x16 };
				uint8 objObjectsSig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0C, 0xC8, 0x48, 0x8D, 0x04, 0xD1, 0xEB };

				if (!FindData(sections, { namePoolDataSig, sizeof(namePoolDataSig) }, { objObjectsSig, sizeof(objObjectsSig) }, nullptr))
				{
					return STATUS::ENGINE_FAILED;
				};

				RogueCompany buf{};
				offsets = *reinterpret_cast<Offsets*>(&buf);

				return STATUS::SUCCESS;
			}
		},
		{
			"PropWitchHuntModule-Win64-Shipping",
			[](std::vector<std::pair<uint8*, uint8*>>* sections) {
				uint8 namePoolDataSig[] = { 0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00, 0xeb, 0x16 };
				uint8 objObjectsSig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0C, 0xC8, 0x48, 0x8D, 0x04, 0xD1, 0xEB };

				if (!FindData(sections, { namePoolDataSig, sizeof(namePoolDataSig) }, { objObjectsSig, sizeof(objObjectsSig) }, nullptr))
				{
					return STATUS::ENGINE_FAILED;
				};

				RogueCompany buf{};
				offsets = *reinterpret_cast<Offsets*>(&buf);

				return STATUS::SUCCESS;
			}
		},
		{
			"POLYGON-Win64-Shipping",
			[](std::vector<std::pair<uint8*, uint8*>>* sections) {
				uint8 namePoolDataSig[] = { 0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00, 0xeb, 0x16 };
				uint8 objObjectsSig[] = { 0x48, 0x8d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x39, 0x44, 0x24, 0x68 };

				if (!FindData(sections, { namePoolDataSig, sizeof(namePoolDataSig) }, { objObjectsSig, sizeof(objObjectsSig) }, nullptr))
				{
					return STATUS::ENGINE_FAILED;
				};

				RogueCompany buf{};
				offsets = *reinterpret_cast<Offsets*>(&buf);

				return STATUS::SUCCESS;
			}
		},
		{
			"Brickadia-Win64-Shipping",
			[](std::vector<std::pair<uint8*, uint8*>>* sections) {
				uint8 namePoolDataSig[] = { 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0xE9, 0x73, 0xAB, 0xFF, 0xFF };
				uint8 objObjectsSig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x63, 0x8C, 0x24, 0xE0 };

				if (!FindData(sections, { namePoolDataSig, sizeof(namePoolDataSig) }, { objObjectsSig, sizeof(objObjectsSig) }, nullptr))
				{
					return STATUS::ENGINE_FAILED;
				};

				Brickadia buf{};
				offsets = *reinterpret_cast<Offsets*>(&buf);

				return STATUS::SUCCESS;
			}
		},
		{
			"FortniteClient-Win64-Shipping",
			[](std::vector<std::pair<uint8*, uint8*>>* sections) {
				void* decryptAnsiAdr = nullptr;
				uint8 namePoolDataSig[] = { 0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00, 0xeb, 0x16 };
				uint8 objObjectsSig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0C, 0xC8, 0x48, 0x8D, 0x04, 0xD1, 0xEB };
				uint8 decryptAnsiSig[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x8B, 0xFA, 0x48, 0x8B, 0xD9, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x85, 0xFF };
				
				auto callback = [&decryptAnsiAdr, &decryptAnsiSig](std::pair<uint8*, uint8*>& s) {
					if (!decryptAnsiAdr) { decryptAnsiAdr = FindSignature(s.first, s.second, decryptAnsiSig, sizeof(decryptAnsiSig)); }
					if (decryptAnsiAdr) { return true; }
					return false;
				};

				if (!FindData(sections, { namePoolDataSig, sizeof(namePoolDataSig) }, { objObjectsSig, sizeof(objObjectsSig) }, callback))
				{
					return STATUS::ENGINE_FAILED;
				};

				Decrypt_ANSI = (ansi_fn)VirtualAlloc(0, 77, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				if (!Decrypt_ANSI) return STATUS::ENGINE_FAILED;
				memcpy(Decrypt_ANSI, decryptAnsiAdr, 15);
				memcpy((uint8*)Decrypt_ANSI + 15, (uint8*)decryptAnsiAdr + 20, 62);

				Fortnite buf{};
				offsets = *reinterpret_cast<Offsets*>(&buf);

				return STATUS::SUCCESS;
			}
		}
	}

};

STATUS EngineInit(std::string game, std::vector<std::pair<uint8*, uint8*>>* sections)
{
	auto& fn = games[game];
	if (!fn) { return STATUS::ENGINE_NOT_FOUND; }
	return fn(sections);
}