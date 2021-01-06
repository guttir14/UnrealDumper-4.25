#pragma once
#include <cstdint>
#include <unordered_map>
#include <functional>

struct Offsets {
	uint16_t Stride = 0; // alignof(FNameEntry)

	struct {
		uint16_t ComparisonIndex = 0;
		uint16_t Number = 0;
	} FName;

	struct {
		uint16_t InfoOffset = 0; // 2 bytes that contain info about type and len
		uint16_t WideBitOffset = 0;
		uint16_t LenBitOffset = 0; // bits offset
		uint16_t HeaderSize = 0;
	} FNameEntry;

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
		uint16_t ChildProperties = 0;
		uint16_t PropertiesSize = 0;
	} UStruct;

	struct {
		uint16_t Names = 0;
	} UEnum;

	struct {
		uint16_t Class = 0;
		uint16_t Next = 0;
		uint16_t Name = 0;
	} FField;

	struct {
		uint16_t ArrayDim = 0;
		uint16_t ElementSize = 0;
		uint16_t PropertyFlags = 0;
		uint16_t Offset = 0;
	} FProperty;

	struct
	{
		uint16_t Name = 0;
	} FFieldClass;

	struct
	{
		uint16_t Struct = 0;
	} FStructProperty;

	struct
	{
		uint16_t PropertyClass = 0;
	} FObjectPropertyBase;

	struct {
		uint16_t MetaClass = 0;
	} FClassProperty;

	struct
	{
		uint16_t Inner = 0;
	} FArrayProperty;

	struct {
		uint16_t Enum = 0;
	} FEnumProperty;


	struct {
		uint16_t ElementProp = 0;
	} FSetProperty;

	struct {
		uint16_t KeyProp = 0;
		uint16_t ValueProp = 0;
	} FMapProperty;

	struct {
		uint16_t InterfaceClass = 0;
	} FInterfaceProperty;

	struct {
		uint16_t FieldSize = 0;
		uint16_t ByteOffset = 0;
		uint16_t ByteMask = 0;
		uint16_t FieldMask = 0;

	} FBoolProperty;

	struct {
		uint16_t Object = 0; // Offset to object
		uint16_t Size = 0; // sizeof(FUObjectItem)
	} FUObjectItem;
};

extern Offsets defs;

bool EngineInit(std::string game);