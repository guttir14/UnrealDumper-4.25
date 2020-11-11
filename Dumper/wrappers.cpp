#include "wrappers.h"
#include <algorithm>
#include <fmt/core.h>


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

UE_UClass UE_UScriptStruct::StaticClass()
{
	static auto obj = ObjObjects.FindClass("Class CoreUObject.ScriptStruct");
	return UE_UClass(obj);
};

void UE_UPackage::Process(std::vector<void*>& processedObjects)
{
	auto objects = package->second;
	for (auto object : objects)
	{
		if (object.IsA<UE_UClass>())
		{
			GenerateClass(object.Cast<UE_UStruct>(), processedObjects);
		}
		else if (object.IsA<UE_UScriptStruct>())
		{
			GenerateStruct(object.Cast<UE_UStruct>(), processedObjects);
		}
	}
}


void UE_UPackage::GenerateStruct(UE_UStruct object, std::vector<void*>& processedObjects)
{
	auto it = std::find_if(processedObjects.begin(), processedObjects.end(), [&object](auto& obj) { return object.GetAddress() == obj; });
	if (it != processedObjects.end()) { return; }
	processedObjects.push_back(object.GetAddress());

	auto super = object.GetSuper();
	if (super)
	{
		GenerateStruct(super.Cast<UE_UStruct>(), processedObjects);
	}

	Class c;
	c.fullname = object.GetFullName();
	c.header = "struct " + object.GetName();
	c.size = object.GetSize();
	if (super)
	{
		c.header += " : public " + super.GetName();
		c.inherited = super.GetSize();
	}

	for (auto prop = object.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
	{
		Member m;
		m.name = prop.GetName();
		m.offset = prop.GetOffset();
		m.size = prop.GetSize();
		m.type = prop.GetType();
		c.members.push_back(m);
	}

	structures.push_back(c);

}

void UE_UPackage::GenerateClass(UE_UStruct object, std::vector<void*>& processedObjects)
{
	auto it = std::find_if(processedObjects.begin(), processedObjects.end(), [&object](auto& obj) { return object.GetAddress() == obj; });
	if (it != processedObjects.end()) { return; }
	processedObjects.push_back(object.GetAddress());

	auto super = object.GetSuper();
	if (super)
	{
		GenerateClass(super.Cast<UE_UStruct>(), processedObjects);
	}

	Class c;
	c.fullname = object.GetFullName();
	c.header = "struct " + object.GetName();
	c.size = object.GetSize();
	if (super)
	{
		c.header += " : public " + super.GetName();
		c.inherited = super.GetSize();
	}

	for (auto prop = object.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
	{
		Member m;
		m.name = prop.GetName();
		m.offset = prop.GetOffset();
		m.size = prop.GetSize();
		m.type = prop.GetType();
		c.members.push_back(m);
	}

	classes.push_back(c);

}

bool UE_UPackage::Save(fs::path& dir)
{
	auto packageName = package->first.GetName();
	{
		File file(dir / (packageName + "_classes.h"));
		if (!file) { return false; }
		for (auto& c : classes)
		{
			fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{", c.fullname, c.size, c.inherited, c.header);
			for (auto& p : c.members)
			{
				fmt::print(file, "\n\t{} {}; // {:#04x}({:#04x})", p.type, p.name, p.offset, p.size);
			}
			fmt::print(file, "\n}}\n\n");
		}
	}
	{
		File file(dir / (packageName + "_struct.h"));
		if (!file) { return false; }
		for (auto& c : structures)
		{
			fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{", c.fullname, c.size, c.inherited, c.header);
			for (auto& p : c.members)
			{
				fmt::print(file, "\n\t{} {}; // {:#04x}({:#04x})", p.type, p.name, p.offset, p.size);
			}
			fmt::print(file, "\n}}\n\n");
		}
	}
	
	return true;
}


