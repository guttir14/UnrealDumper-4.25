#pragma once
#include "Generic.h"
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class File {
private:
	FILE* file;
public:
	File(const fs::path& const path) { fopen_s(&file, path.string().c_str(), "w"); }
	~File() { fclose(file); }
	operator bool() { return file != nullptr; }
	operator FILE* () { return file; }
};

class UE_UClass;
class UE_FField;

class UE_UObject {
protected:
	UObject* object;
public:
	UE_UObject(UObject* _object) : object(_object) {}
	UE_UObject() { object = nullptr; }
	operator bool() { return object != nullptr; }
	bool operator==(UE_UObject& obj) { return obj.object == object; };
	int32_t GetIndex() { return Read<int32_t>(reinterpret_cast<char*>(object) + offsetof(UObject, UObject::InternalIndex)); };
	UE_UClass GetClass();
	UE_UObject GetOuter(){ 
		return UE_UObject(Read<UObject*>(reinterpret_cast<char*>(object) + offsetof(UObject, UObject::OuterPrivate) + offsets.addition));
	}
	UE_UObject GetPackageObject() { 
		UE_UObject package(nullptr);
		for (auto outer = GetOuter(); outer; outer = outer.GetOuter())
		{
			package = outer;
		}
		return package;
	}
	std::string GetName()
	{
		FName fname = Read<FName>(reinterpret_cast<char*>(object) + offsetof(UObject, UObject::NamePrivate));
		auto entry = Read<FNameEntry>(NamePoolData.GetEntry(fname.GetIndex()));
		auto number = fname.GetNumber();
		auto name = entry.GetString();
		if (number > 0)
		{
			name += '_' + std::to_string(number);
		}

		auto pos = name.rfind('/');
		if (pos == std::string::npos)
		{
			return name;
		}

		return name.substr(pos + 1);
	}
	std::string GetFullName();
	void* GetAddress() { return object; }

	template<typename Base>
	Base Cast() const
	{
		return Base(object);
	}

	template<typename T>
	bool IsA();

	static UE_UClass StaticClass();
};

class UE_UField : public UE_UObject 
{
public:
	using UE_UObject::UE_UObject;

	UE_UField GetNext() { return UE_UField(Read<UField*>(reinterpret_cast<char*>(object) + offsetof(UField, UField::Next))); }

	static UE_UClass StaticClass();
};

class UE_FField {
protected:
	FField* object;
public:
	UE_FField(FField* _object) : object(_object) {}
	operator bool() { return object != nullptr; }
	UE_FField GetNext() { return UE_FField(Read<FField*>(reinterpret_cast<char*>(object) + offsetof(FField, FField::Next))); };
	std::string GetName() {
		FName fname = Read<FName>(reinterpret_cast<char*>(object) + offsetof(FField, FField::Name));
		auto entry = Read<FNameEntry>(NamePoolData.GetEntry(fname.GetIndex()));
		auto name = entry.GetString();
		
		if (!name.size()) { return "None"; }
		auto number = fname.GetNumber();
		if (number) { name += "_" + std::to_string(number); }
		return name;
	}

	template<typename Base>
	Base Cast() const
	{
		return Base(object);
	}
};

class UE_UStruct : public UE_UField
{
public:
	using UE_UField::UE_UField;

	UE_UStruct GetSuper() { return UE_UStruct(Read<UObject*>(reinterpret_cast<char*>(object) + offsetof(UStruct, UStruct::SuperStruct) + offsets.addition)); };

	UE_FField GetChildProperties() { return UE_FField(Read<FField*>(reinterpret_cast<char*>(object) + offsetof(UStruct, UStruct::ChildProperties) + offsets.addition)); }
	UE_UField GetChildren() { return UE_UField(Read<UField*>(reinterpret_cast<char*>(object) + offsetof(UStruct, UStruct::Children) + offsets.addition)); };

	int32_t GetSize() { { return Read<int32_t>(reinterpret_cast<char*>(object) + offsetof(UStruct, UStruct::PropertiesSize) + offsets.addition); }; };

	static UE_UClass StaticClass();
};

class UE_UScriptStruct : public UE_UStruct
{
public:
	using UE_UStruct::UE_UStruct;

	static UE_UClass StaticClass();
};

class UE_UClass : public UE_UStruct {
public:
	using UE_UStruct::UE_UStruct;

	static UE_UClass StaticClass() {
		static auto obj = ObjObjects.FindClass("Class CoreUObject.Class");
		return UE_UClass(obj);
	};
};


class UE_FProperty : public UE_FField {
public:
	using UE_FField::UE_FField;

	int32_t GetSize() { return Read<int32_t>(reinterpret_cast<char*>(object) + offsetof(FProperty, FProperty::ElementSize)); }

	int32_t GetOffset() { return Read<int32_t>(reinterpret_cast<char*>(object) + offsetof(FProperty, FProperty::Offset_Internal)); }

	std::string GetType();
};

class UE_FStructProperty : public UE_FProperty {
public:
	using UE_FProperty::UE_FProperty;

	std::string GetType() {
		auto obj = Read<UE_UObject>(reinterpret_cast<char*>(object) + offsetof(FStructProperty, FStructProperty::Struct));
		return obj.GetName();
	}
};

class UE_FObjectPropertyBase : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;

	std::string GetType() {
		auto obj = Read<UE_UObject>(reinterpret_cast<char*>(object) + offsetof(FStructProperty, FStructProperty::Struct));
		return obj.GetName() + "*";
	}
};

class UE_FArrayProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;

	std::string GetType() {
		auto obj = UE_FProperty(Read<FProperty*>(reinterpret_cast<char*>(object) + offsetof(FArrayProperty, FArrayProperty::Inner)));
		return "Array<" + obj.GetType() + ">";
	}
};

class UE_FBoolProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;

	uint8_t GetFieldSize()
	{
		return Read<uint8_t>(reinterpret_cast<char*>(object) + offsetof(FBoolProperty, FBoolProperty::FieldSize));
	}

	uint8_t GetByteOffset()
	{
		return Read<uint8_t>(reinterpret_cast<char*>(object) + offsetof(FBoolProperty, FBoolProperty::ByteOffset));
	}

	uint8_t GetFieldMask()
	{
		return Read<uint8_t>(reinterpret_cast<char*>(object) + offsetof(FBoolProperty, FBoolProperty::FieldMask));
	}

	std::string GetType() {
		if (GetFieldMask() == 0xFF) { return "bool"; };
		return "char";
	}
};
template<typename T>
bool UE_UObject::IsA()
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
		std::string name;
		std::string type;
		uint32_t offset;
		uint32_t size;
	};
	struct Class
	{
		std::string fullname;
		std::string header;
		size_t inherited;
		size_t size;
		std::vector<Member> members;
	};
private:
	std::pair<UE_UObject, std::vector<UE_UObject>>* package;
	std::vector<Class> classes;
	std::vector<Class> structures;
public:
	UE_UPackage(std::pair<UE_UObject, std::vector<UE_UObject>>& package) : package(&package) {};

	void Process(std::vector<void*>& processedObjects);
	bool Save(fs::path& dir);
private:
	static void GenerateStruct(UE_UStruct object, std::vector<void*>& processedObjects, std::vector<Class>& classes);	
};
