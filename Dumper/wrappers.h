#pragma once
#include "Generic.h"
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class File {
private:
	FILE* file;
public:
	explicit File(const fs::path& path) { fopen_s(&file, path.string().c_str(), "w"); }
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
	UE_UObject(UObject* object) : object(object) {}
	UE_UObject() { object = nullptr; }
	operator bool() const { return object != nullptr; }
	bool operator==(const UE_UObject& obj) const { return obj.object == object; };
	bool operator!=(const UE_UObject& obj) const { return obj.object != object; };
	int32_t GetIndex() const;
	UE_UClass GetClass() const;
	UE_UObject GetOuter() const;
	UE_UObject GetPackageObject() const;
	std::string GetName() const;
	std::string GetFullName() const;
	std::string GetNameCPP() const;

	void* GetAddress() const { return object; }
	operator UObject* () const { return object; };

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

class UE_FField {
protected:
	FField* object;
public:
	UE_FField(FField* _object) : object(_object) {}
	operator bool() const { return object != nullptr; }
	UE_FField GetNext() const;
	std::string GetName() const;

	template<typename Base>
	Base Cast() const { return Base(object); }
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

class UE_UScriptStruct : public UE_UStruct
{
public:
	using UE_UStruct::UE_UStruct;

	static UE_UClass StaticClass();
};

class UE_UClass : public UE_UStruct {
public:
	using UE_UStruct::UE_UStruct;

	static UE_UClass StaticClass();
};


class UE_FProperty : public UE_FField {
public:
	using UE_FField::UE_FField;

	int32_t GetArrayDim() const;
	int32_t GetSize() const;
	int32_t GetOffset() const;

	std::string GetType() const;
};

class UE_FStructProperty : public UE_FProperty {
public:
	using UE_FProperty::UE_FProperty;

	std::string GetType();
};

class UE_FObjectPropertyBase : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;

	std::string GetType() const;
};

class UE_FArrayProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;

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
		std::string name;
		int32_t offset;
		int32_t size;
	};
	struct Struct
	{
		std::string fullname;
		std::string header;
		int32_t inherited;
		int32_t size;
		std::vector<Member> members;
	};
private:
	std::pair<UObject* const, std::vector<UE_UObject>>* package;
	std::vector<Struct> classes;
	std::vector<Struct> structures;
	std::vector<UObject*> dependences;
public:
	UE_UPackage(std::pair<UObject* const, std::vector<UE_UObject>>& package) : package(&package) {};

	void Process(std::unordered_map<UObject*, bool>& processedObjects);
	bool Save(const fs::path& dir);
private:
	void GenerateStruct(UE_UStruct object, std::unordered_map<UObject*, bool>& processedObjects, std::vector<Struct>& arr);
};