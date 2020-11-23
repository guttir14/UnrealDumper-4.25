#include "wrappers.h"
#include <algorithm>
#include <fmt/core.h>
#include <mutex>

int32_t UE_UObject::GetIndex() const
{
	return Read<int32_t>(reinterpret_cast<char*>(object) + offsetof(UObject, UObject::InternalIndex));
};

UE_UClass UE_UObject::GetClass() const
{
	return UE_UClass(Read<UObject*>(reinterpret_cast<char*>(object) + offsetof(UObject, UObject::ClassPrivate)));
}

UE_UObject UE_UObject::GetOuter() const
{
	return UE_UObject(Read<UObject*>(reinterpret_cast<char*>(object) + offsetof(UObject, UObject::OuterPrivate) + offsets.addition));
}

UE_UObject UE_UObject::GetPackageObject() const
{
	UE_UObject package(nullptr);
	for (auto outer = GetOuter(); outer; outer = outer.GetOuter())
	{
		package = outer;
	}
	return package;
}

std::string UE_UObject::GetName() const

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

std::string UE_UObject::GetFullName() const
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

std::string UE_UObject::GetNameCPP() const
{
	static auto actorObj = ObjObjects.FindObject("Class Engine.Actor");
	std::string name;
	if (this->IsA<UE_UClass>())
	{
		for (auto c = this->Cast<UE_UStruct>(); c; c = c.GetSuper())
		{
			if (c == actorObj)
			{
				name = "A";
				break;
			}
			else if (c == UE_UObject::StaticClass())
			{
				name = "U";
				break;

			}
		}
	}
	else
	{
		name = "F";
	}

	name += GetName();
	return name;
}

UE_UClass UE_UObject::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Object");
	return UE_UClass(obj);
};

UE_UStruct UE_UStruct::GetSuper() const
{
	return UE_UStruct(Read<UObject*>(reinterpret_cast<char*>(object) + offsetof(UStruct, UStruct::SuperStruct) + offsets.addition));
}
UE_FField UE_UStruct::GetChildProperties() const
{
	return UE_FField(Read<FField*>(reinterpret_cast<char*>(object) + offsetof(UStruct, UStruct::ChildProperties) + offsets.addition));
}

UE_UField UE_UStruct::GetChildren() const
{
	return UE_UField(Read<UField*>(reinterpret_cast<char*>(object) + offsetof(UStruct, UStruct::Children) + offsets.addition));
}
int32_t UE_UStruct::GetSize() const
{
	return Read<int32_t>(reinterpret_cast<char*>(object) + offsetof(UStruct, UStruct::PropertiesSize) + offsets.addition);
};

UE_UClass UE_UStruct::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Struct");
	return UE_UClass(obj);
};

UE_UField UE_UField::GetNext() const
{
	return UE_UField(Read<UField*>(reinterpret_cast<char*>(object) + offsetof(UField, UField::Next)));
}

UE_UClass UE_UField::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Field");
	return UE_UClass(obj);
};

int32_t UE_FProperty::GetSize() const
{
	return Read<int32_t>(reinterpret_cast<char*>(object) + offsetof(FProperty, FProperty::ElementSize));
}

int32_t UE_FProperty::GetOffset() const
{
	return Read<int32_t>(reinterpret_cast<char*>(object) + offsetof(FProperty, FProperty::Offset_Internal));
}

std::string UE_FProperty::GetType() const
{
	auto classObj = Read<FFieldClass*>(reinterpret_cast<char*>(object) + offsetof(FProperty, FProperty::ClassPrivate));
	FName fname = Read<FName>(classObj + offsetof(FFieldClass, FFieldClass::Name));
	auto entry = Read<FNameEntry>(NamePoolData.GetEntry(fname.GetIndex()));
	auto str = entry.GetString();


	static std::unordered_map<std::string, std::function<void(decltype(this), std::string&)>> types;
	static std::once_flag called;
	std::call_once(called, []() {
		
		
		types["StructProperty"] = [](decltype(this) prop, std::string& type) {
			auto obj = prop->Cast<UE_FStructProperty>();
			type = obj.GetType();
		};

		types["ObjectProperty"] = [](decltype(this) prop, std::string& type) {

			auto obj = prop->Cast<UE_FObjectPropertyBase>();
			type = obj.GetType();
		};

		types["FloatProperty"] = [](decltype(this) prop, std::string& type) {
			type = "float";
		};


		types["ByteProperty"] = [](decltype(this) prop, std::string& type) {
			type = "char";
		};

		types["BoolProperty"] = [](decltype(this) prop, std::string& type) {
			auto obj = prop->Cast<UE_FBoolProperty>();
			type = obj.GetType();
		};

		types["IntProperty"] = [](decltype(this) prop, std::string& type) {
			type = "int32_t";
		};

		types["UInt16Property"] = [](decltype(this) prop, std::string& type) {
			type = "uint16_t";
		};

		types["NameProperty"] = [](decltype(this) prop, std::string& type) {
			type = "FName";
		};

		types["ArrayProperty"] = [](decltype(this) prop, std::string& type) {
			auto obj = prop->Cast<UE_FArrayProperty>();
			type = obj.GetType();
		};

		types["WeakObjectProperty"] = [](decltype(this) prop, std::string& type) {
			auto obj = prop->Cast<UE_FStructProperty>();
			type = "FWeakObjectPtr<" + obj.GetType() + ">";
		};
		
		});

	auto& fn = types[str];

	if (fn) { fn(this, str); }

	return  str;
}

UE_UClass UE_UScriptStruct::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.ScriptStruct");
	return UE_UClass(obj);
};

void UE_UPackage::Process(std::unordered_map<UObject*, bool>& processedObjects)
{
	auto &objects = package->second;
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
void UE_UPackage::GenerateStruct(UE_UStruct object, std::unordered_map<UObject*, bool>& processedObjects, std::vector<Struct>& arr)
{
	auto curPackage = object.GetPackageObject();
	if (curPackage != UE_UObject(package->first))
	{
		dependences.push_back(curPackage);
		return;
	}
	
	auto& processed = processedObjects[object];
	if (processed) { return; };
	processed = true;

	auto outer = object.GetOuter();
	if (outer)
	{
		GenerateStruct(outer.Cast<UE_UStruct>(), processedObjects, arr);
	}
	
	auto super = object.GetSuper();
	
	if (super)
	{
		GenerateStruct(super.Cast<UE_UStruct>(), processedObjects, arr);
	}
	

	Struct s;
	s.size = object.GetSize();
	if (s.size == 0) { return; }
	s.fullname = object.GetFullName();
	s.header = "struct " + object.GetNameCPP();
	s.inherited = 0;
	if (super)
	{
		s.header += " : public " + super.GetNameCPP();
		s.inherited = super.GetSize();
	}

	if (s.size == s.inherited) { return; }

	uint32_t offset = s.inherited;
	static auto generatePadding = [](std::vector<Member>& members, uint32_t offset, uint32_t size) {
		Member padding;
		padding.name = fmt::format("char UnknownData{:#04x}[{:#04x}]", offset, size);
		padding.offset = offset;
		padding.size = size;
		members.push_back(padding);
	};
	for (auto prop = object.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
	{
		Member m;

		m.name = prop.GetType() + " " + prop.GetName();
		m.offset = prop.GetOffset();
		m.size = prop.GetSize();

		if (m.offset != offset)
		{
			if (m.size != 1)
			{
				generatePadding(s.members, offset, m.offset - offset);
			}
			
			offset = m.offset;
		}
		offset += m.size;
		s.members.push_back(m);
	}

	if (offset != s.size)
	{
		generatePadding(s.members, offset, s.size - offset);
	}

	arr.push_back(s);

}

bool UE_UPackage::Save(const fs::path& dir)
{
	if (!(classes.size() || structures.size()))
	{ 
		return false;
	}
	auto packageName = UE_UObject(package->first).GetName();
	static auto SaveStruct = [](std::vector<Struct>& v, File file)
	{
		for (auto& c : v)
		{
			fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{", c.fullname, c.size, c.inherited, c.header);
			for (auto& p : c.members)
			{
				fmt::print(file, "\n\t{}; // {:#04x}({:#04x})", p.name, p.offset, p.size);
			}
			fmt::print(file, "\n}}\n\n");
		}
	};
	
	
	
	if (classes.size())
	{
		File file(dir / (packageName + "_classes.h"));
		if (!file) { return false; }
		SaveStruct(classes, file);
	}
	
	
	if (structures.size())
	{
		File file(dir / (packageName + "_struct.h"));
		if (!file) { return false; }
		SaveStruct(structures, file);
	}
	
	
	return true;
}

UE_FField UE_FField::GetNext() const
{
	return UE_FField(Read<FField*>(reinterpret_cast<char*>(object) + offsetof(FField, FField::Next)));
};

std::string UE_FField::GetName() const
{
	FName fname = Read<FName>(reinterpret_cast<char*>(object) + offsetof(FField, FField::Name));
	auto entry = Read<FNameEntry>(NamePoolData.GetEntry(fname.GetIndex()));
	auto name = entry.GetString();

	if (!name.size()) { return "None"; }
	auto number = fname.GetNumber();
	if (number) { name += "_" + std::to_string(number); }
	return name;
}

UE_UClass UE_UClass::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Class");
	return UE_UClass(obj);
};

std::string UE_FStructProperty::GetType()
{
	auto obj = Read<UE_UObject>(reinterpret_cast<char*>(object) + offsetof(FStructProperty, FStructProperty::Struct) + offsets.addition);
	return obj.GetNameCPP();
}

std::string UE_FObjectPropertyBase::GetType() const
{
	auto obj = Read<UE_UObject>(reinterpret_cast<char*>(object) + offsetof(FStructProperty, FStructProperty::Struct) + offsets.addition);
	return obj.GetNameCPP() + "*";
}

std::string UE_FArrayProperty::GetType() const
{
	auto obj = UE_FProperty(Read<FProperty*>(reinterpret_cast<char*>(object) + offsetof(FArrayProperty, FArrayProperty::Inner) + offsets.addition));
	return "TArray<" + obj.GetType() + ">";
}

uint8_t UE_FBoolProperty::GetFieldSize() const
{
	return Read<uint8_t>(reinterpret_cast<char*>(object) + offsetof(FBoolProperty, FBoolProperty::FieldSize) + offsets.addition);
}

uint8_t UE_FBoolProperty::GetByteOffset() const
{
	return Read<uint8_t>(reinterpret_cast<char*>(object) + offsetof(FBoolProperty, FBoolProperty::ByteOffset) + offsets.addition);
}

uint8_t UE_FBoolProperty::GetFieldMask() const
{
	return Read<uint8_t>(reinterpret_cast<char*>(object) + offsetof(FBoolProperty, FBoolProperty::FieldMask) + offsets.addition);
}

std::string UE_FBoolProperty::GetType() const
{
	if (GetFieldMask() == 0xFF) { return "bool"; };
	return "char";
}
