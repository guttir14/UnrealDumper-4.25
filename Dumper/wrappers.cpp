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

std::string UE_FProperty::GetType()
{
	auto classObj = Read<FFieldClass*>(reinterpret_cast<char*>(object) + offsetof(FProperty, FProperty::ClassPrivate));
	FName fname = Read<FName>(classObj + offsetof(FFieldClass, FFieldClass::Name));
	auto entry = Read<FNameEntry>(NamePoolData.GetEntry(fname.GetIndex()));
	auto str = entry.GetString();

	
	if (str == "StructProperty")
	{
		auto obj = this->Cast<UE_FStructProperty>();
		return obj.GetType();
	}
	else if (str == "ObjectProperty")
	{
		auto obj = this->Cast<UE_FObjectPropertyBase>();
		return obj.GetType();
	}
	else if (str == "FloatProperty")
	{
		return "float";
	}
	else if (str == "ByteProperty")
	{
		return "char";
	}
	else if (str == "BoolProperty")
	{
		auto obj = this->Cast<UE_FBoolProperty>();
		return obj.GetType();
	}
	else if (str == "IntProperty")
	{
		return "int";
	}
	else if (str == "ArrayProperty")
	{
		auto obj = this->Cast<UE_FArrayProperty>();
		return obj.GetType();
	}
	else if (str == "WeakObjectProperty")
	{
		auto obj = this->Cast<UE_FStructProperty>();
		return "WeakObjectPtr<" + obj.GetType() + ">";
	}

	return  str;
}

UE_UClass UE_UScriptStruct::StaticClass()
{
	static auto obj = ObjObjects.FindClass("Class CoreUObject.ScriptStruct");
	return UE_UClass(obj);
};

void UE_UPackage::Process(std::unordered_map<int32_t, bool>& processedObjects)
{
	auto objects = package->second;
	for (auto& object : objects)
	{
		if (object.IsA<UE_UClass>())
		{
			GenerateStruct(object.Cast<UE_UStruct>(), processedObjects, classes);
		}
		else if (object.IsA<UE_UScriptStruct>())
		{
			GenerateStruct(object.Cast<UE_UStruct>(), processedObjects, structures);
		}
	}
}


void UE_UPackage::GenerateStruct(UE_UStruct object, std::unordered_map<int32_t, bool>& processedObjects, std::vector<Class>& classes)
{
	auto objId = object.GetIndex();
	if (processedObjects[objId]) { return; };
	processedObjects[objId] = true;

	auto super = object.GetSuper();
	if (super)
	{
		GenerateStruct(super.Cast<UE_UStruct>(), processedObjects, classes);
	}

	Class c;
	c.fullname = object.GetFullName();
	c.header = "struct " + object.GetName();
	c.size = object.GetSize();
	c.inherited = 0;

	if (c.size == 0) { return; }

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

bool UE_UPackage::Save(const fs::path& dir)
{
	if (!classes.size() && !structures.size())
	{ 
		return false;
	}
	auto packageName = UE_UObject(package->first).GetName();
	static auto SaveStruct = [](std::vector<Class>& v, File file)
	{
		for (auto& c : v)
		{
			fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{", c.fullname, c.size, c.inherited, c.header);
			for (auto& p : c.members)
			{
				fmt::print(file, "\n\t{} {}; // {:#04x}({:#04x})", p.type, p.name, p.offset, p.size);
			}
			fmt::print(file, "\n}}\n\n");
		}
	};
	
	{
		if (classes.size())
		{
			File file(dir / (packageName + "_classes.h"));
			if (!file) { return false; }
			SaveStruct(classes, file);
		}
	}
	{
		if (structures.size())
		{
			File file(dir / (packageName + "_struct.h"));
			if (!file) { return false; }
			SaveStruct(structures, file);
		}
	}
	
	return true;
}


