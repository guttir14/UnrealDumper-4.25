#pragma once
#include "Generic.h"
#include <unordered_map>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// Wrapper for 'FILE*' that closes the file handle when it goes out of scope
class File 
{
private:
	FILE* file;
public:
	File(fs::path path, const char* mode) { fopen_s(&file, path.string().c_str(), mode); }
	~File() { fclose(file); }
	operator bool() const { return file != nullptr; }
	operator FILE* () { return file; }
};

// Wrapper for array unit in global names array
class UE_FNameEntry 
{
protected:
	byte* object;
public:
	UE_FNameEntry(byte* object) : object(object) {}
	UE_FNameEntry() : object(nullptr) {}
	// Gets info about contained string (bool wide, uint16_t len) depending on 'defs.FNameEntry' info
	std::pair<bool, uint16_t> Info() const; 
	// Gets string out of array unit
	std::string String(bool wide, uint16_t len) const;
	// Gets string out of array unit
	void String(char* buf, bool wide, uint16_t len) const;
	// Calculates the unit size depending on 'defs.FNameEntry' and information about string
	static uint16_t Size(bool wide, uint16_t len);
};

class UE_FName 
{
protected: 
	byte* object;
public:
	UE_FName(byte* object) : object(object) {}
	UE_FName() : object(nullptr) {}
	std::string GetName() const;
};

class UE_UClass;
class UE_FField;

class UE_UObject 
{
protected:
	byte* object;
public:
	UE_UObject(byte* object) : object(object) {}
	UE_UObject() : object(nullptr) {}
	bool operator==(const UE_UObject& obj) const { return obj.object == object; };
	bool operator!=(const UE_UObject& obj) const { return obj.object != object; };
	uint32_t GetIndex() const;
	UE_UClass GetClass() const;
	UE_UObject GetOuter() const;
	UE_UObject GetPackageObject() const;
	std::string GetName() const;
	std::string GetFullName() const;
	std::string GetCppName() const;
	void* GetAddress() const { return object; }
	operator byte* () const { return object; };
	operator bool() const { return object != nullptr; }

	template<typename Base>
	Base Cast() const { return Base(object); }

	template<typename T>
	bool IsA() const;

	static UE_UClass StaticClass();
};

class UE_UField : public UE_UObject 
{
public:
	using UE_UObject::UE_UObject;
	UE_UField GetNext() const;
	static UE_UClass StaticClass();
};

class UE_UProperty : public UE_UField
{
public:
	using UE_UField::UE_UField;
	static UE_UClass StaticClass();
};

class UE_UStruct : public UE_UField
{
public:
	using UE_UField::UE_UField;
	UE_UStruct GetSuper() const;
	UE_FField GetChildProperties() const;
	UE_UField GetChildren() const;
	int32_t GetSize() const;
	static UE_UClass StaticClass();
};

enum class UEFunctionFlags : uint32_t
{
	Final = 0x00000001,
	RequiredAPI = 0x00000002,
	BlueprintAuthorityOnly = 0x00000004,
	BlueprintCosmetic = 0x00000008,
	Net = 0x00000040,
	NetReliable = 0x00000080,
	NetRequest = 0x00000100,
	Exec = 0x00000200,
	Native = 0x00000400,
	Event = 0x00000800,
	NetResponse = 0x00001000,
	Static = 0x00002000,
	NetMulticast = 0x00004000,
	MulticastDelegate = 0x00010000,
	Public = 0x00020000,
	Private = 0x00040000,
	Protected = 0x00080000,
	Delegate = 0x00100000,
	NetServer = 0x00200000,
	HasOutParms = 0x00400000,
	HasDefaults = 0x00800000,
	NetClient = 0x01000000,
	DLLImport = 0x02000000,
	BlueprintCallable = 0x04000000,
	BlueprintEvent = 0x08000000,
	BlueprintPure = 0x10000000,
	Const = 0x40000000,
	NetValidate = 0x80000000
};

inline bool operator&(UEFunctionFlags lhs, UEFunctionFlags rhs)
{
	return (static_cast<std::underlying_type_t<UEFunctionFlags>>(lhs) & static_cast<std::underlying_type_t<UEFunctionFlags>>(rhs)) == static_cast<std::underlying_type_t<UEFunctionFlags>>(rhs);
}

class UE_UFunction : public UE_UStruct
{
public:
	using UE_UStruct::UE_UStruct;
	static UE_UClass StaticClass();
	UEFunctionFlags GetFunctionFlags() const;
	size_t GetFunctionPtr() const;
	std::string GetFlagsStringified(UEFunctionFlags flags) const;
};

class UE_UScriptStruct : public UE_UStruct
{
public:
	using UE_UStruct::UE_UStruct;
	static UE_UClass StaticClass();
};

class UE_UClass : public UE_UStruct 
{
public:
	using UE_UStruct::UE_UStruct;
	static UE_UClass StaticClass();
};

class UE_UEnum : public UE_UField 
{
public:
	using UE_UField::UE_UField;
	TArray GetNames() const;
	static UE_UClass StaticClass();
};

class UE_FFieldClass 
{
protected:
	byte* object;
public:
	UE_FFieldClass(byte* object) : object(object) {};
	UE_FFieldClass() : object(nullptr) {};
	std::string GetName() const;
};

class UE_FField 
{
protected:
	byte* object;
public:
	UE_FField(byte* object) : object(object) {}
	UE_FField() : object(nullptr) {}
	operator bool() const { return object != nullptr; }
	UE_FField GetNext() const;
	std::string GetName() const;

	template<typename Base>
	Base Cast() const { return Base(object); }
};

enum class PropertyType {
	Unknown,
	StructProperty,
	ObjectProperty, 
	SoftObjectProperty,
	FloatProperty,
	ByteProperty,
	BoolProperty,
	IntProperty,
	Int8Property,
	Int16Property,
	Int64Property,
	UInt16Property,
	UInt32Property, 
	UInt64Property,
	NameProperty,
	DelegateProperty,
	SetProperty,
	ArrayProperty,
	WeakObjectProperty,
	StrProperty,
	TextProperty,
	MulticastSparseDelegateProperty,
	EnumProperty,
	DoubleProperty,
	MulticastDelegateProperty,
	ClassProperty,
	MulticastInlineDelegateProperty,
	MapProperty,
	InterfaceProperty
};

class UE_FProperty : public UE_FField 
{
public:
	using UE_FField::UE_FField;
	// Gets amount of same properties at current offset
	int32_t GetArrayDim() const;
	int32_t GetSize() const;
	int32_t GetOffset() const;
	uint64_t GetPropertyFlags() const;
	std::pair<PropertyType, std::string> GetType() const;
};

class UE_FStructProperty : public UE_FProperty 
{
public:
	using UE_FProperty::UE_FProperty;
	UE_UStruct GetStruct() const;
	std::string GetType() const;
};

class UE_FObjectPropertyBase : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_UClass GetPropertyClass() const;
	std::string GetType() const;
};

class UE_FArrayProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_FProperty GetInner() const;
	std::string GetType() const;
};

class UE_FBoolProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	uint8_t GetFieldSize() const;
	uint8_t GetByteOffset() const;
	uint8_t GetFieldMask() const;
	std::string GetType() const;
};

class UE_FEnumProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_UClass GetEnum() const;
	std::string GetType() const;
};

class UE_FClassProperty : public UE_FObjectPropertyBase
{
public:
	using UE_FObjectPropertyBase::UE_FObjectPropertyBase;
	UE_UClass GetMetaClass() const;
	std::string GetType() const;
};

class UE_FSetProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_FProperty GetElementProp() const;
	std::string GetType() const;
};

class UE_FMapProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_FProperty GetKeyProp() const;
	UE_FProperty GetValueProp() const;
	std::string GetType() const;
};

class UE_FInterfaceProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_FProperty GetInterfaceClass() const;
	std::string GetType() const;
};

class UE_FSoftClassProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	std::string GetType() const;
};

template<typename T>
bool UE_UObject::IsA() const
{
	auto cmp = T::StaticClass();
	if (!cmp) { return false; }

	for (auto super = GetClass(); super; super = super.GetSuper().Cast<UE_UClass>())
	{
		if (super.object == cmp.object)
		{
			return true;
		}
	}

	return false;
}

class UE_UPackage
{
private:
	struct Member
	{
		std::string Name;
		int32_t Offset = 0;
		int32_t Size = 0;
	};
	struct Function
	{
		std::string FullName;
		std::string CppName;
		std::string Params;
		std::string FlagsString;
		uint32_t Flags;
		size_t FuncPtr;
	};
	struct Struct
	{
		std::string FullName;
		std::string CppName;
		int32_t Inherited = 0;
		int32_t Size = 0;
		std::vector<Member> Members;
		std::vector<Function> Functions;
	};
	struct Enum
	{
		std::string FullName;
		std::string CppName;
		std::vector<std::string> Members;
	};
private:
	std::pair<byte* const, std::vector<UE_UObject>>* Package;
	std::vector<Struct> Classes;
	std::vector<Struct> Structures;
	std::vector<Enum> Enums;
	size_t ModuleBase;
private:
	static void GenerateBitPadding(std::vector<Member>& members, int32_t offset, int16_t bitOffset, int16_t size);
	static void GeneratePadding(std::vector<Member>& members, int32_t& minOffset, int32_t& bitOffset, int32_t maxOffset);
	static void GenerateStruct(UE_UStruct object, std::vector<Struct>& arr);
	static void GenerateEnum(UE_UEnum object, std::vector<Enum>& arr);
	void SaveStruct(std::vector<Struct>& arr, File file);
public:
	UE_UPackage(std::pair<byte* const, std::vector<UE_UObject>>& package) : Package(&package) {};
	void Process(size_t ModuleBase);
	bool Save(const fs::path& dir);
	UE_UObject GetObject() const;
};