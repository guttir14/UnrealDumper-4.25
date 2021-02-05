#pragma once
#include <cstdint>
#include <functional>

struct Offsets {
	uint16_t Stride = 0; // alignof(FNameEntry)
	struct {
		uint16_t Size = 0;
	} FUObjectItem;
	struct {
		uint16_t Number = 0;
	} FName;
	struct {
		uint16_t Info		= 0; // Offset to Memory filled with info about type and size of string
		uint16_t WideBit	= 0; // Offset to bit which shows if string uses wide characters
		uint16_t LenBit		= 0; // Offset to bit which has lenght of string
		uint16_t HeaderSize = 0; // Size of FNameEntry header (offset where a string begins)
	} FNameEntry;
	struct {
		uint16_t Index	= 0; // Offset to index of this object in all objects array
		uint16_t Class	= 0; // Offset to UClass pointer (UClass* ClassPrivate)
		uint16_t Name	= 0; // Offset to FName structure
		uint16_t Outer	= 0; // (UObject* OuterPrivate)
	} UObject;
	struct {
		uint16_t Next = 0;
	} UField;
	struct {
		uint16_t SuperStruct		= 0;
		uint16_t Children			= 0;
		uint16_t ChildProperties	= 0;
		uint16_t PropertiesSize		= 0;
	} UStruct;
	struct {
		uint16_t Names = 0;
	} UEnum;
	struct {
		uint16_t FunctionFlags = 0;
		uint16_t Func = 0; // ue3-ue4, always +0x28 from flags location.
	} UFunction;
	struct {
		uint16_t Class	= 0;
		uint16_t Next	= 0;
		uint16_t Name	= 0;
	} FField;
	struct {
		uint16_t ArrayDim		= 0;
		uint16_t ElementSize	= 0;
		uint16_t PropertyFlags	= 0;
		uint16_t Offset			= 0;
		uint16_t Size			= 0; // sizeof(FProperty)
	} FProperty;
};

extern Offsets defs;

typedef char(*ansi_fn)(char* a1, int a2);
typedef char(*wide_fn)(wchar_t* a1, int a2);

extern ansi_fn Decrypt_ANSI;
extern wide_fn Decrypt_WIDE;

bool EngineInit(std::string game);