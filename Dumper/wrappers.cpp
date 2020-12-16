#include "wrappers.h"
#include <algorithm>
#include <fmt/core.h>
#include "memory.h"

std::pair<bool, uint16_t> UE_FNameEntry::Info() const
{
	auto info = Read<uint16_t>(object + defs.FNameEntry.InfoOffset);
	auto len = info >> defs.FNameEntry.LenBitOffset;
	bool wide = (info >> defs.FNameEntry.WideBitOffset) & 1;
	return {wide, len};
}

std::string UE_FNameEntry::String(bool wide, uint16_t len) const
{
	std::string name("\x0", len);
	String(name.data(), wide, len);
	return name;
}

void UE_FNameEntry::String(char* buf, bool wide, uint16_t len) const
{
	if (wide)
	{
		wchar_t wbuf[1024]{};
		Read(object + defs.FNameEntry.HeaderSize, wbuf, len * 2ull);
		auto copied = WideCharToMultiByte(CP_UTF8, 0, wbuf, len, buf, len, 0, 0);
		if (copied == 0) { buf[0] = '\x0'; }
	}
	else
	{
		Read(object + defs.FNameEntry.HeaderSize, buf, len);
	}
}

uint16_t UE_FNameEntry::Size(bool wide, uint16_t len)
{
	uint16_t bytes = defs.FNameEntry.HeaderSize + len * (wide ? sizeof(wchar_t) : sizeof(char));
	return (bytes + defs.Stride - 1u) & ~(defs.Stride - 1u);
}

std::string UE_FName::GetName() const
{
	uint32_t index = Read<uint32_t>(object + defs.FName.ComparisonIndex);
	auto entry = UE_FNameEntry(NamePoolData.GetEntry(index));
	auto [wide, len] = entry.Info();
	auto name = entry.String(wide, len);
	uint32_t number = Read<uint32_t>(object + defs.FName.Number);
	if (number > 0)
	{
		name += '_' + std::to_string(number);
	}
	auto pos = name.rfind('/');
	if (pos != std::string::npos)
	{
		name = name.substr(pos + 1);
	}
	return name;
}

uint32_t UE_UObject::GetIndex() const
{
	return Read<uint32_t>(object + defs.UObject.Index);
};

UE_UClass UE_UObject::GetClass() const
{
	return Read<UE_UClass>(object + defs.UObject.Class);
}

UE_UObject UE_UObject::GetOuter() const
{
	return Read<UE_UObject>(object + defs.UObject.Outer);
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
	auto fname = UE_FName(object + defs.UObject.Name);
	return fname.GetName();
}

std::string UE_UObject::GetFullName() const
{
	std::string temp;
	for (auto outer = GetOuter(); outer; outer = outer.GetOuter())
	{
		temp = outer.GetName() + "." + temp;
	}

	UE_UClass objectClass = GetClass();
	std::string name = objectClass.GetName() + " " + temp + GetName();

	return name;
}

std::string UE_UObject::GetCppName() const
{
	static auto ActorClass = ObjObjects.FindObject("Class Engine.Actor");
	std::string name;
	if (this->IsA<UE_UClass>())
	{
		for (auto c = this->Cast<UE_UStruct>(); c; c = c.GetSuper())
		{
			if (c == ActorClass)
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
	return obj;
};

UE_UField UE_UField::GetNext() const
{
	return Read<UE_UField>(object + defs.UField.Next);
}

UE_UClass UE_UField::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Field");
	return obj;
};

UE_UClass UE_UProperty::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Property");
	return obj;
}

UE_UStruct UE_UStruct::GetSuper() const
{
	return Read<UE_UStruct>(object + defs.UStruct.SuperStruct);
}

UE_FField UE_UStruct::GetChildProperties() const
{
	return Read<UE_FField>(object + defs.UStruct.ChildProperties);
}

UE_UField UE_UStruct::GetChildren() const
{
	return Read<UE_UField>(object + defs.UStruct.Children);
}

int32_t UE_UStruct::GetSize() const
{
	return Read<int32_t>(object + defs.UStruct.PropertiesSize);
};

UE_UClass UE_UStruct::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Struct");
	return obj;
};

UE_UClass UE_UFunction::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Function");
	return obj;
}

UE_UClass UE_UScriptStruct::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.ScriptStruct");
	return obj;
};

UE_UClass UE_UClass::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Class");
	return obj;
};

TArray UE_UEnum::GetNames() const
{
	return Read<TArray>(object + defs.UEnum.Names);
}

UE_UClass UE_UEnum::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Enum");
	return obj;
}

std::string UE_FFieldClass::GetName() const
{
	auto name = UE_FName(object + defs.FFieldClass.Name);
	return name.GetName();
}

UE_FField UE_FField::GetNext() const
{
	return Read<UE_FField>(object + defs.FField.Next);
};

std::string UE_FField::GetName() const
{
	auto name = UE_FName(object + defs.FField.Name);
	return name.GetName();
}

int32_t UE_FProperty::GetArrayDim() const
{
	return Read<int32_t>(object + defs.FProperty.ArrayDim);
}

int32_t UE_FProperty::GetSize() const
{
	return Read<int32_t>(object + defs.FProperty.ElementSize);
}

int32_t UE_FProperty::GetOffset() const
{
	return Read<int32_t>(object + defs.FProperty.Offset);
}

uint64_t UE_FProperty::GetPropertyFlags() const
{
	return Read<uint64_t>(object + defs.FProperty.PropertyFlags);
}

std::string UE_FProperty::GetType() const
{
	auto objectClass = Read<UE_FFieldClass>(object + defs.FField.Class);
	std::string str = objectClass.GetName();

	static std::unordered_map<std::string, std::function<void(decltype(this), std::string&)>> types =
	{
		{
			"StructProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FStructProperty>();
				str = obj.GetType();
			}
		},
		{
			"ObjectProperty",
			[](decltype(this) prop, std::string& str) {

				auto obj = prop->Cast<UE_FObjectPropertyBase>();
				str = obj.GetType();
			}
		},
		{
			"SoftObjectProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FObjectPropertyBase>();
				str = "struct TSoftObjectPtr<struct " + obj.GetPropertyClass().GetCppName() + ">";
			}
		},
		{
			"FloatProperty",
			[](decltype(this) prop, std::string& str) {
				str = "float";
			}
		},
		{
			"ByteProperty",
			[](decltype(this) prop, std::string& info) {
				info = "char";
			}
		},
		{
			"BoolProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FBoolProperty>();
				str = obj.GetType();
			}
		},
		{
			"IntProperty",
			[](decltype(this) prop, std::string& str) {
				str = "int32_t";
			}
		},
		{
			"Int8Property",
			[](decltype(this) prop, std::string& str) {
				str = "int8_t";
			}
		},
		{
			"Int16Property",
			[](decltype(this) prop, std::string& str) {
				str = "int16_t";
			}
		},
		{
			"Int64Property",
			[](decltype(this) prop, std::string& str) {
				str = "int64_t";
			}
		},
		{
			"UInt16Property",
			[](decltype(this) prop, std::string& str) {
				str = "uint16_t";
			}
		},
		{
			"UInt32Property",
			[](decltype(this) prop, std::string& str) {
				str = "uint32_t";
			}
		},
		{
			"UInt64Property",
			[](decltype(this) prop, std::string& str) {
				str = "uint64_t";
			}
		},
		{
			"NameProperty",
			[](decltype(this) prop, std::string& str) {
				str = "struct FName";
			}
		},
		{
			"DelegateProperty",
			[](decltype(this) prop, std::string& str) {
				str = "struct FDelegate";
			}
		},
		{
			"SetProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FSetProperty>();
				str = obj.GetType();
			}
		},
		{
			"ArrayProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FArrayProperty>();
				str = obj.GetType();
			}
		},
		{
			"WeakObjectProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FStructProperty>();
				str = "struct FWeakObjectPtr<" + obj.GetType() + ">";
			}
		},
		{
			"StrProperty",
			[](decltype(this) prop, std::string& str) {
				str = "struct FString";
			}
		},
		{
			"TextProperty",
			[](decltype(this) prop, std::string& str) {
				str = "struct FText";
			}
		},
		{
			"MulticastSparseDelegateProperty",
			[](decltype(this) prop, std::string& str) {
				str = "struct FMulticastSparseDelegate";
			}
		},
		{
			"EnumProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FEnumProperty>();
				str = obj.GetType();
			}
		},
		{
			"DoubleProperty",
			[](decltype(this) prop, std::string& str) {
				str = "double";
			}
		},
		{
			"MulticastDelegateProperty",
			[](decltype(this) prop, std::string& str) {
				str = "FMulticastDelegate";
			}
		},
		{
			"ClassProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FClassProperty>();
				str = obj.GetType();
			}
		},
		{
			"MulticastInlineDelegateProperty",
			[](decltype(this) prop, std::string& str) {
				str = "struct FMulticastInlineDelegate";
			}
		},
		{
			"MapProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FMapProperty>();
				str = obj.GetType();
			}
		},

		{
			"InterfaceProperty",
			[](decltype(this) prop, std::string& str) {
				auto obj = prop->Cast<UE_FInterfaceProperty>();
				str = obj.GetType();
			}
		}
	};

	auto& fn = types[str];

	if (fn) { fn(this, str); }

	return str;
}

UE_UStruct UE_FStructProperty::GetStruct() const
{
	return Read<UE_UStruct>(object + defs.FStructProperty.Struct);
}

std::string UE_FStructProperty::GetType() const
{
	return "struct " + GetStruct().GetCppName();
}

UE_UClass UE_FObjectPropertyBase::GetPropertyClass() const
{
	return Read<UE_UClass>(object + defs.FObjectPropertyBase.PropertyClass);
}

std::string UE_FObjectPropertyBase::GetType() const
{
	return "struct " + GetPropertyClass().GetCppName() + "*";
}

UE_FProperty UE_FArrayProperty::GetInner() const
{
	return Read<UE_FProperty>(object + defs.FArrayProperty.Inner);
}

std::string UE_FArrayProperty::GetType() const
{
	return "struct TArray<" + GetInner().GetType() + ">";
}

uint8_t UE_FBoolProperty::GetFieldSize() const
{
	return Read<uint8_t>(object + defs.FBoolProperty.FieldSize);
}

uint8_t UE_FBoolProperty::GetByteOffset() const
{
	return Read<uint8_t>(object + defs.FBoolProperty.ByteOffset);
}

uint8_t UE_FBoolProperty::GetFieldMask() const
{
	return Read<uint8_t>(object + defs.FBoolProperty.FieldMask);
}

std::string UE_FBoolProperty::GetType() const
{
	if (GetFieldMask() == 0xFF) { return "bool"; };
	return "char";
}

UE_UClass UE_FEnumProperty::GetEnum() const
{
	return Read<UE_UClass>(object + defs.FEnumProperty.Enum);
}

std::string UE_FEnumProperty::GetType() const
{
	return "enum class " + GetEnum().GetName();
}

UE_UClass UE_FClassProperty::GetMetaClass() const
{
	return Read<UE_UClass>(object + defs.FClassProperty.MetaClass);
}

std::string UE_FClassProperty::GetType() const
{
	return "struct " + GetMetaClass().GetCppName() + "*";
}

UE_FProperty UE_FSetProperty::GetElementProp() const
{
	return Read<UE_FProperty>(object + defs.FSetProperty.ElementProp);
}

std::string UE_FSetProperty::GetType() const
{
	return "struct TSet<" + GetElementProp().GetType() + ">";	
}

UE_FProperty UE_FMapProperty::GetKeyProp() const
{
	return Read<UE_FProperty>(object + defs.FMapProperty.KeyProp);
}

UE_FProperty UE_FMapProperty::GetValueProp() const
{
	return Read<UE_FProperty>(object + defs.FMapProperty.ValueProp);
}

std::string UE_FMapProperty::GetType() const
{
	return fmt::format("struct TMap<{}, {}>", GetKeyProp().GetType(), GetValueProp().GetType());
}

UE_FProperty UE_FInterfaceProperty::GetInterfaceClass() const
{
	return Read<UE_FProperty>(object + defs.FInterfaceProperty.InterfaceClass);
}

std::string UE_FInterfaceProperty::GetType() const
{
	return "struct TScriptInterface<" + GetInterfaceClass().GetType() + ">";
}

void UE_UPackage::GeneratePadding(std::vector<Member>& members, uint32_t offset, uint32_t size)
{
	Member padding;
	padding.Name = fmt::format("char UnknownData{:#04x}[{:#04x}]", offset, size);
	padding.Offset = offset;
	padding.Size = size;
	members.push_back(padding);
}

void UE_UPackage::GenerateStruct(UE_UStruct object, std::vector<Struct>& arr)
{
	Struct s;
	s.Size = object.GetSize();
	if (s.Size == 0) { return; }
	s.Inherited = 0;
	s.FullName = object.GetFullName();
	s.CppName = "struct " + object.GetCppName();

	auto super = object.GetSuper();
	if (super)
	{
		s.CppName += " : public " + super.GetCppName();
		s.Inherited = super.GetSize();
	}

	if (s.Size == s.Inherited) { return; }

	{
		int32_t offset = s.Inherited;
		for (auto prop = object.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
		{
			auto arrDim = prop.GetArrayDim();
			Member m;
			m.Size = prop.GetSize() * arrDim;
			if (m.Size == 0) { return; }
			m.Name = prop.GetType() + " " + prop.GetName();
			if (arrDim > 1)
			{
				m.Name += fmt::format("[{:#04x}]", arrDim);
			}

			m.Offset = prop.GetOffset();


			if (m.Offset > offset)
			{
				if (m.Size != 1)
				{
					UE_UPackage::GeneratePadding(s.Members, offset, m.Offset - offset);
				}

				offset = m.Offset;
			}


			offset += m.Size;
			s.Members.push_back(m);
		}

		if (offset != s.Size)
		{
			UE_UPackage::GeneratePadding(s.Members, offset, s.Size - offset);
		}
	}

	for (auto fn = object.GetChildren().Cast<UE_UFunction>(); fn; fn = fn.GetNext().Cast<UE_UFunction>())
	{
		if (fn.IsA<UE_UFunction>())
		{
			Function f;
			f.FullName = fn.GetFullName();
			for (auto prop = fn.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
			{
				auto flags = prop.GetPropertyFlags();
				if (flags & 0x400) // if property has 'ReturnParm' flag
				{
					f.CppName = prop.GetType() + " " + fn.GetName();
				}
				else if (flags & 0x80) // if property has 'Parm' flag
				{
					if (prop.GetArrayDim() > 1)
					{
						f.Params += fmt::format("{}* {}, ", prop.GetType(), prop.GetName());
					}
					else
					{
						f.Params += fmt::format("{} {}, ", prop.GetType(), prop.GetName());
					}
				}
			}

			if (f.Params.size())
			{
				f.Params.erase(f.Params.size() - 2);
			}


			if (f.CppName.size() == 0)
			{
				f.CppName = "void " + fn.GetName();
			}

			s.Functions.push_back(f);

		}
	}

	arr.push_back(s);

}

void UE_UPackage::GenerateEnum(UE_UEnum object, std::vector<Enum>& arr)
{
	Enum e;
	e.FullName = object.GetFullName();
	e.CppName = "enum class " + object.GetName() + " : uint8_t";
	auto names = object.GetNames();
	for (auto i = 0ull; i < names.Count; i++)
	{
		auto size = (defs.FName.Number + 4u + 8 + 7u) & ~(7u);
		auto name = UE_FName(names.Data + i * size);
		auto str = name.GetName();
		auto pos = str.find_last_of(':');
		if (pos != std::string::npos)
		{
			str = str.substr(pos + 1);
		}

		e.Members.push_back(str);
	}

	if (e.Members.size())
	{
		arr.push_back(e);
	}
}

void UE_UPackage::SaveStruct(std::vector<Struct>& arr, File file)
{
	for (auto& s : arr)
	{
		fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{", s.FullName, s.Size, s.Inherited, s.CppName);
		for (auto& m : s.Members)
		{
			fmt::print(file, "\n\t{}; // {:#04x}({:#04x})", m.Name, m.Offset, m.Size);
		}
		if (s.Functions.size())
		{
			fwrite("\n", 1, 1, file);
			for (auto& f : s.Functions)
			{
				fmt::print(file, "\n\t{}({}); // {}", f.CppName, f.Params, f.FullName);
			}
		}

		fmt::print(file, "\n}};\n\n");
	}
}

void UE_UPackage::Process()
{
	auto& objects = Package->second;
	for (auto& object : objects)
	{
		if (object.IsA<UE_UClass>())
		{
			GenerateStruct(object.Cast<UE_UStruct>(), Classes);
		}
		else if (object.IsA<UE_UScriptStruct>())
		{
			GenerateStruct(object.Cast<UE_UStruct>(), Structures);
		}
		else if (object.IsA<UE_UEnum>())
		{
			GenerateEnum(object.Cast<UE_UEnum>(), Enums);
		}
	}
}

bool UE_UPackage::Save(const fs::path& dir)
{
	if (!(Classes.size() || Structures.size() || Enums.size()))
	{
		return false;
	}

	std::string packageName = this->GetObject().GetName();

	if (Classes.size())
	{
		File file(dir / (packageName + "_classes.h"), "w");
		if (!file) { return false; }
		UE_UPackage::SaveStruct(Classes, file);
	}

	{
		File file(dir / (packageName + "_struct.h"), "w");
		if (!file) { return false; }

		if (Enums.size())
		{
			for (auto& e : Enums)
			{
				fmt::print(file, "// {}\n{} {{", e.FullName, e.CppName);
				for (auto& m : e.Members)
				{
					fmt::print(file, "\n\t{},", m);
				}
				fmt::print(file, "\n}};\n\n");
			}
		}

		if (Structures.size())
		{
			UE_UPackage::SaveStruct(Structures, file);
		}
	}

	return true;
}

UE_UObject UE_UPackage::GetObject() const
{
	return UE_UObject(Package->first);
}