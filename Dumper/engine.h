#pragma once
#include <cstdint>
#include <functional>

struct Offsets {
	struct {
		uint16_t HeaderSize = 0;
	} FNameEntry;
	struct {
		uint16_t Size = 0;
	} FUObjectItem;
	struct {
		uint16_t Index = 0;
		uint16_t Class = 0;
		uint16_t Name = 0;
		uint16_t Outer = 0;
	} UObject;
	struct {
		uint16_t Next = 0;
	} UField;
	struct {
		uint16_t SuperStruct = 0;
		uint16_t Children = 0;
		uint16_t PropertiesSize = 0;
	} UStruct;
	struct {
		uint16_t Names = 0;
	} UEnum;
	struct {
		uint16_t FunctionFlags = 0;
		uint16_t Func = 0;
	} UFunction;
	struct {
		uint16_t ArrayDim = 0;
		uint16_t ElementSize = 0;
		uint16_t PropertyFlags = 0;
		uint16_t Offset = 0;
		uint16_t Size = 0;
	} UProperty;
};

extern Offsets defs;

bool EngineInit(std::string game);


/*
struct UByteProperty : public UProperty
{
	struct UEnum* Enum; // 0x88
};

struct UArrayProperty : public UProperty
{
	UProperty* Inner;
};

struct UBoolProperty : public UProperty
{
	uint8_t FieldSize;
	uint8_t ByteOffset;
	uint8_t ByteMask;
	uint8_t FieldMask;
};

struct UObjectPropertyBase : public UProperty
{
	UClass* PropertyClass;
};

struct UClassProperty : public UObjectPropertyBase
{
	UClass* MetaClass;
};

struct UInterfaceProperty : public UProperty
{
	UClass* InterfaceClass;
};

struct UAssetClassProperty : public UObjectPropertyBase
{
	UClass* MetaClass;
};

struct UStructProperty : public UProperty
{
	struct UScriptStruct* Struct;
};
*/