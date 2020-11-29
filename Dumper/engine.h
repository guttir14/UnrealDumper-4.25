#pragma once
#include <cstdint>

// Dead by Daylight
#define DBD 1

// RogueCompany2
#define RC2 1

#if DBD + RC2 > 1
#error "Define your game offsets here or set 1 only to one of these games"
#endif


#if DBD
inline struct {

	uint16_t Stride = 4; // alignof(FNameEntry)

	struct {
		uint16_t ComparisonIndex = 0;
		uint16_t Number = 8;
	} FName;

	struct {
		uint16_t InfoOffset = 4; // 2 bytes that contain info about type and len
		uint16_t WideBitOffset = 0;
		uint16_t LenBitOffset = 1; // bits offset
		uint16_t HeaderSize = 6;
	} FNameEntry;

	struct {
		uint16_t Index = 0xC;
		uint16_t Class = 0x10;
		uint16_t Name = 0x18;
		uint16_t Outer = 0x28;
	} UObject;

	struct {
		uint16_t Next = 0x30;
	} UField;

	struct {
		uint16_t SuperStruct = 0x48;
		uint16_t Children = 0x50;
		uint16_t ChildProperties = 0x58;
		uint16_t PropertiesSize = 0x60;
	} UStruct;

	struct {
		uint16_t Names = 0x48;
	} UEnum;

	struct {
		uint16_t Class = 0x8;
		uint16_t Next = 0x20;
		uint16_t Name = 0x28;
	} FField;


	struct {
		uint16_t ArrayDim = 0x38;
		uint16_t ElementSize = 0x3C;
		uint16_t Offset = 0x4C;
	} FProperty;

	struct
	{
		uint16_t Name = 0;
	} FFieldClass;


	struct
	{
		uint16_t Struct = 0x80;
	} FStructProperty;

	struct
	{
		uint16_t PropertyClass = 0x80;
	} FObjectPropertyBase;

	struct {
		uint16_t MetaClass = 0x88;
	} FClassProperty;


	struct
	{
		uint16_t Inner = 0x80;
	} FArrayProperty;

	struct {
		uint16_t Enum = 0x88;
	} FEnumProperty;


	struct {
		uint16_t ElementProp = 0x80;
	} FSetProperty;

	struct {
		uint16_t FieldSize = 0x80;
		uint16_t ByteOffset = 0x80 + 1;
		uint16_t ByteMask = 0x80 + 2;
		uint16_t FieldMask = 0x80 + 3;

	} FBoolProperty;

	struct {
		uint16_t Object = 0; // Offset to object
		uint16_t Size = 24; // sizeof(FUObjectItem)
	} FUObjectItem;

} defs;

#endif

#if RC2
inline struct {

	uint16_t Stride = 2; // alignof(FNameEntry)

	struct {
		uint16_t ComparisonIndex = 0;
		uint16_t Number = 4;
	} FName;

	struct {
		uint16_t InfoOffset = 0;
		uint16_t WideBitOffset = 0;
		uint16_t LenBitOffset = 6; // bits offset
		uint16_t HeaderSize = 2;
	} FNameEntry; 

	struct {
		uint16_t Index = 0xC;
		uint16_t Class = 0x10;
		uint16_t Name = 0x18;
		uint16_t Outer = 0x20;
	} UObject;

	struct  {
		uint16_t Next = 0x28;
	} UField;

	struct {
		uint16_t SuperStruct = 0x40;
		uint16_t Children = 0x48;
		uint16_t ChildProperties = 0x50;
		uint16_t PropertiesSize = 0x58;
	} UStruct;

	struct {
		uint16_t Names = 0x40;
	} UEnum;

	struct {
		uint16_t Class = 0x8;
		uint16_t Next = 0x20;
		uint16_t Name = 0x28;
	} FField;


	struct {
		uint16_t ArrayDim = 0x38;
		uint16_t ElementSize = 0x3C;
		uint16_t Offset = 0x4C;
	} FProperty;

	struct
	{
		uint16_t Name = 0;
	} FFieldClass;


	struct
	{
		uint16_t Struct = 0x78;
	} FStructProperty;

	struct
	{
		uint16_t PropertyClass = 0x78;
	} FObjectPropertyBase;

	struct {
		uint16_t MetaClass = 0x80;
	} FClassProperty;


	struct
	{
		uint16_t Inner = 0x78;
	} FArrayProperty;

	struct {
		uint16_t Enum = 0x80;
	} FEnumProperty;

	
	struct {
		uint16_t ElementProp = 0x78;
	} FSetProperty;

	struct {
		uint16_t FieldSize = 0x78;
		uint16_t ByteOffset = 0x78 + 1;
		uint16_t ByteMask = 0x78 + 2;
		uint16_t FieldMask = 0x78 + 3;

	} FBoolProperty;

	struct {
		uint16_t Object = 0; // Offset to object
		uint16_t Size = 24; // sizeof(FUObjectItem)
	} FUObjectItem;

} defs;
#endif