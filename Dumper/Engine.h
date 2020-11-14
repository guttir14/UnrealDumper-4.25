#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "memory.h"


inline struct Offsets {
	int addition = 0;
	int stride = 2;
	int header = 0;
} offsets;

struct FName
{
	uint32_t ComparisonIndex;
	uint32_t DisplayIndex;
	uint32_t Number;
	uint32_t GetNumber() { if (offsets.addition) { return Number; } else { return DisplayIndex; } }
	uint32_t GetIndex() { return ComparisonIndex; }
};


struct UStruct;
struct UClass;

struct UObject
{
	uint64_t VFTable; //0x0000
	int32_t ObjectFlags; //0x0008
	int32_t InternalIndex; //0x000C
	UClass* ClassPrivate; //0x0010
	uint64_t NamePrivate; //0x0018
	UObject* OuterPrivate; //0x0020
}; //Size: 0x0028

struct UField : UObject
{
	UField* Next; //0x0028
}; //Size: 0x0030

class UProperty : public UField
{
public:
	int32_t ArrayDim; //0x0030
	int32_t ElementSize; //0x0034
	uint64_t PropertyFlags; //0x0038
	char pad_0040[4]; //0x0040
	int32_t Offset; //0x0044
	char pad_0048[40]; //0x0048
}; //Size: 0x0070

/*
struct UEnum : UField
{
	FString CppType; ; //0x0030
	TArray<TPair<FName, uint64_t>> Names; //0x0040
	char pad_0050[16]; //0x0050
}; //Size: 0x0060
*/

struct FFieldClass {
	FName Name;
	uint64_t Id;
	uint64_t CastFlags;
	uint64_t ClassFlags;
	FFieldClass* SuperClass;
};

struct FFieldVariant
{
	union FFieldObjectUnion
	{
		class FField* Field;
		UObject* Object;
	} Container;

	bool bIsUObject;

};

struct FField
{
	uint64_t VFTable; //0x0000
	FFieldClass* ClassPrivate; //0x0008
	FFieldVariant Owner; //0x0010
	FField* Next; //0x0020
	uint64_t Name; //0x0028
	char pad_0030[8]; //0x0030
}; //Size: 0x0038

struct FProperty : FField
{
	int32_t ArrayDim; //0x0038
	int32_t ElementSize; //0x003C
	uint64_t PropertyFlags; //0x0040
	char pad_0048[4]; //0x0048
	int32_t Offset_Internal; //0x004C
	uint64_t RepNotifyFunc; //0x0050
	FProperty* PropertyLinkNext; //0x0058
	FProperty* NextRef; //0x0060
	FProperty* DestructorLinkNext; //0x0068
	FProperty* PostConstructLinkNext; //0x0070
}; //Size: 0x0078

struct UStruct : UField
{
	char pad_0030[0x10]; //0x0030
	UStruct* SuperStruct; //0x0040
	UField* Children; // 0x48
	FField* ChildProperties; //0x0050
	int32_t PropertiesSize; //0x0058
	int32_t MinAlignment; //0x005C
	char pad_0060[80]; //0x0060
}; //Size: 0x00B0

struct UScriptStruct : UStruct
{
	char pad_00B0[16]; //0x00B0
}; //Size: 0x00C0


struct FStructProperty : public FProperty
{
	UScriptStruct* Struct;
};


struct FObjectPropertyBase : public FProperty
{
	class UClass* PropertyClass;
};


struct FArrayProperty : public FProperty
{
	FProperty* Inner;
};

struct FBoolProperty : public FProperty 
{
	/** Size of the bitfield/bool property. Equal to ElementSize but used to check if the property has been properly initialized (0-8, where 0 means uninitialized). */
	uint8_t FieldSize;
	/** Offset from the memeber variable to the byte of the property (0-7). */
	uint8_t ByteOffset;
	/** Mask of the byte with the property value. */
	uint8_t ByteMask;
	/** Mask of the field with the property value. Either equal to ByteMask or 255 in case of 'bool' type. */
	uint8_t FieldMask;
};

struct UFunction : UStruct
{
	char pad_00B0[40]; //0x00B0
	uint64_t Func; //0x00D8
}; //Size: 0x00E0

struct UClass : UStruct
{
	char pad_00B0[400]; //0x00B0
}; //Size: 0x0240

struct FUObjectItem
{
	UObject* Object;
	int32_t Flags;
	int32_t ClusterIndex;
	int32_t SerialNumber;
	uint32_t StatID;
};