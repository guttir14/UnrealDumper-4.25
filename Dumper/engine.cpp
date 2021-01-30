#include "engine.h"

using namespace std;

Offsets defs;

struct SeaOfThieves {
	struct {
		uint16_t HeaderSize = 0x10;
	} FNameEntry;
	struct {
		uint16_t Index = 0xC;
		uint16_t Class = 0x10;
		uint16_t Name = 0x18;
		uint16_t Outer = 0x20;
	} UObject;
	struct {
		uint16_t Next = 0x28;
	} UField;
	struct {
		uint16_t SuperStruct = 0x30;
		uint16_t Children = 0x38;
		uint16_t PropertiesSize = 0x40;
	} UStruct;
	struct {
		uint16_t Names = 0x40;
		uint16_t NamesElementSize = 0xC;
	} UEnum;
	struct {
		uint16_t FunctionFlags = 0x88;
		uint16_t Func = 0xB0;
	} UFunction;
	struct {
		uint16_t ArrayDim = 0x30;
		uint16_t ElementSize = 0x34;
		uint16_t PropertyFlags = 0x38;
		uint16_t Offset = 0x4C;
		uint16_t Size = 0x70;
	} UProperty;
};
static_assert(sizeof(Offsets) == sizeof(SeaOfThieves));

struct ContraReboot {
	struct {
		uint16_t HeaderSize = 0x10;
	} FNameEntry;
	struct {
		uint16_t Index = 0xC;
		uint16_t Class = 0x10;
		uint16_t Name = 0x18;
		uint16_t Outer = 0x20;
	} UObject;
	struct {
		uint16_t Next = 0x28;
	} UField;
	struct {
		uint16_t SuperStruct = 0x30;
		uint16_t Children = 0x38;
		uint16_t PropertiesSize = 0x40;
	} UStruct;
	struct {
		uint16_t Names = 0x40;
		uint16_t NamesElementSize = 0x10;
	} UEnum;
	struct {
		uint16_t FunctionFlags = 0x88;
		uint16_t Func = 0xB0;
	} UFunction;
	struct {
		uint16_t ArrayDim = 0x30;
		uint16_t ElementSize = 0x34;
		uint16_t PropertyFlags = 0x38;
		uint16_t Offset = 0x44;
		uint16_t Size = 0x70;
	} UProperty;
};
static_assert(sizeof(Offsets) == sizeof(ContraReboot));

unordered_map<string, function<void()>> games = {
	{
		{
			"SoTGame",
			[]() {
				SeaOfThieves buf{};
				defs = *reinterpret_cast<Offsets*>(&buf);
			}
		},
		{
			"ContraReboot-Win64-Shipping",
			[]() {
			ContraReboot buf{};
			defs = *reinterpret_cast<Offsets*>(&buf);
			}
		}
	}
};

bool EngineInit(std::string game)
{
	auto& fn = games[game];
	if (!fn) { return false; }
	fn();
	return true;
}
