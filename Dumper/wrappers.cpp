#include "wrappers.h"

UE_UClass UE_UObject::GetClass()
{
	return UE_UClass(Read<UObject*>(reinterpret_cast<char*>(object) + offsetof(UObject, UObject::ClassPrivate)));
}

std::string UE_UObject::GetFullName()
{
	auto classObj = GetClass();
	if (!classObj) return "(null)";
	std::string temp;

	for (auto outer = GetOuter(); outer; outer = outer.GetOuter())
	{
		temp = outer.GetName() + "." + temp;
	}

	std::string name = classObj.GetName();
	name += " ";
	name += temp;
	name += GetName();

	return name;
}

UE_UClass UE_UObject::StaticClass()
{
	static auto obj = ObjObjects.FindClass("Class CoreUObject.Object");
	return UE_UClass(obj);
};

UE_UClass UE_UStruct::StaticClass()
{
	static auto obj = ObjObjects.FindClass("Class CoreUObject.Struct");
	return UE_UClass(obj);
};

UE_UClass UE_UField::StaticClass()
{
	static auto obj = ObjObjects.FindClass("Class CoreUObject.Field");
	return UE_UClass(obj);
};