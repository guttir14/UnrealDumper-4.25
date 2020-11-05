#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "memory.h"

// this should be changed for most games to 0 
#define WITH_CASE_PRESERVING_NAME 1

struct FName
{
	int32_t ComparisonIndex;
#if WITH_CASE_PRESERVING_NAME
	int32_t DisplayIndex;
#endif
	int32_t Number;

};

template<class T>
class TArray
{
	friend class FString;

public:
	TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	size_t Num() const
	{
		return Count;
	};

	T& operator[](size_t i)
	{
		return Data[i];
	};

	const T& operator[](size_t i) const
	{
		return Data[i];
	};

	bool IsValidIndex(size_t i) const
	{
		return i < Num();
	}

private:
	T* Data;
	int32_t Count;
	int32_t Max;
};

template<typename KeyType, typename ValueType>
class TPair
{
public:
	KeyType   Key;
	ValueType Value;
};

class FString : public TArray<wchar_t>
{
public:
	std::string ToString() const
	{
		size_t size = std::wcslen(Data);
		std::string str(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, Data, Count, str.data(), size, nullptr, nullptr);
		return str;
	}
};

struct UStruct;
struct UClass;

struct UObject
{
	uint64_t VFTable; //0x0000
	int32_t ObjectFlags; //0x0008
	int32_t InternalIndex; //0x000C
	UClass* ClassPrivate; //0x0010
	FName NamePrivate; //0x0018
	UObject* OuterPrivate; //0x0020

}; //Size: 0x0028


struct UField : UObject
{
	UField* Next; //0x0028
}; //Size: 0x0030

struct UEnum : UField
{
	FString CppType; ; //0x0030
	TArray<TPair<FName, uint64_t>> Names; //0x0040
	char pad_0050[16]; //0x0050
}; //Size: 0x0060

struct FFieldClass {
	FName Name;
};

struct FField
{
	uint64_t VFTable; //0x0000
	FFieldClass* ClassPrivate; //0x0008
	UStruct* Owner; //0x0010
	uint64_t FlagsPrivate; //0x0018
	FField* Next; //0x0020
	FName Name; //0x0028
	char pad_0030[4]; //0x0030
}; //Size: 0x0038

struct FProperty : FField
{
	int32_t ArrayDim; //0x0038
	int32_t ElementSize; //0x003C
	uint64_t PropertyFlags; //0x0040
	char pad_0048[4]; //0x0048
	int32_t Offset_Internal; //0x004C
	char pad_0050[24]; //0x0050
}; //Size: 0x0068


struct UStruct : UField
{
	char pad_0030[0x10]; //0x0030
	UStruct* SuperStruct; //0x0040
	char pad_0048[8]; //0x0048
	FField* ChildProperties; //0x0050
	int32_t PropertiesSize; //0x0058
	int32_t MinAlignment; //0x005C
	char pad_0060[80]; //0x0060
}; //Size: 0x00B0

struct UScriptStruct : UStruct
{
	char pad_00B0[16]; //0x00B0
}; //Size: 0x00C0

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