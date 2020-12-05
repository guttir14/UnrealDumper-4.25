#include "wrappers.h"
#include <algorithm>
#include <fmt/core.h>
#include "memory.h"
#include <mutex>

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
	return UE_UClass(Read<byte*>(object + defs.UObject.Class));
}

UE_UObject UE_UObject::GetOuter() const
{
	return UE_UObject(Read<byte*>(object + defs.UObject.Outer));
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

std::string UE_UObject::GetNameCPP() const
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

UE_UStruct UE_UStruct::GetSuper() const
{
	return UE_UStruct(Read<byte*>(object + defs.UStruct.SuperStruct));
}

UE_FField UE_UStruct::GetChildProperties() const
{
	return UE_FField(Read<byte*>(object + defs.UStruct.ChildProperties));
}

UE_UField UE_UStruct::GetChildren() const
{
	return UE_UField(Read<byte*>(object + defs.UStruct.Children));
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

TArray UE_UEnum::GetNames() const
{
	return Read<TArray>(object + defs.UEnum.Names);
}

UE_UClass UE_UEnum::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Enum");
	return obj;
}

UE_UField UE_UField::GetNext() const
{
	return UE_UField(Read<byte*>(object + defs.UField.Next));
}

UE_UClass UE_UField::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Field");
	return obj;
};

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

std::pair<std::string, bool> UE_FProperty::GetInfo() const
{
	auto objectClass = UE_FFieldClass(Read<byte*>(object + defs.FField.Class));
	auto str = objectClass.GetName();
	std::pair<std::string, bool> info = { str, true };


	static std::unordered_map<std::string, std::function<void(decltype(this), std::pair<std::string, bool>&)>> types;
	static std::once_flag called;
	std::call_once(called, []() 
		{
			types["StructProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				auto obj = prop->Cast<UE_FStructProperty>();
				info = { obj.GetType(), true };
			};

			types["ObjectProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {

				auto obj = prop->Cast<UE_FObjectPropertyBase>();
				info = { obj.GetType(), false };
			};

			types["SoftObjectProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {

				auto obj = prop->Cast<UE_FObjectPropertyBase>();
				info = { "struct TSoftObjectPtr<struct " + obj.GetPropertyClass().GetNameCPP() + ">", false };
			};

			types["FloatProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "float", false };
			};

			types["ByteProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "char", false };
			};

			types["BoolProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				auto obj = prop->Cast<UE_FBoolProperty>();
				info = { obj.GetType(), false };
			};

			types["IntProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "int32_t", false };
			};

			types["Int8Property"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "int8_t", false };
			};

			types["Int16Property"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "int16_t", false };
			};

			types["Int64Property"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "int64_t", false };
			};

			types["UInt16Property"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "uint16_t", false };
			};

			types["UInt32Property"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "uint32_t", false };
			};

			types["UInt64Property"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "uint64_t", false };
			};

			types["NameProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "struct FName", true };
			};

			types["DelegateProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "struct FDelegate", false };
			};

			types["SetProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				auto obj = prop->Cast<UE_FSetProperty>();
				info = { obj.GetType(), false };
			};

			types["ArrayProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				auto obj = prop->Cast<UE_FArrayProperty>();
				info = { obj.GetType(), false };
			};

			types["WeakObjectProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				auto obj = prop->Cast<UE_FStructProperty>();
				info = { "struct FWeakObjectPtr<" + obj.GetType() + ">", false };
			};

			types["StrProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "struct FString", true };
			};

			types["TextProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "struct FText", true };
			};

			types["MulticastSparseDelegateProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "struct FMulticastSparseDelegate", false };
			};

			types["EnumProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				auto obj = prop->Cast<UE_FEnumProperty>();
				info = { obj.GetType(), false };
			};

			types["DoubleProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "double", false };
			};

			types["MulticastDelegateProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "FMulticastDelegate", true };
			};


			types["ClassProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				auto obj = prop->Cast<UE_FClassProperty>();
				info = { obj.GetType(), false };
			};

			types["MulticastInlineDelegateProperty"] = [](decltype(this) prop, std::pair<std::string, bool>& info) {
				info = { "struct FMulticastInlineDelegate", false };
			};
	
		}
	);

	auto& fn = types[str];

	if (fn) { fn(this, info); }

	return info;
}

std::string UE_FProperty::GetType() const
{
	auto info = this->GetInfo();
	return info.first;
}

UE_UClass UE_UScriptStruct::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.ScriptStruct");
	return obj;
};

void UE_UPackage::Process()
{
	auto& objects = package->second;
	for (auto& object : objects)
	{
		if (object.IsA<UE_UClass>())
		{
			GenerateStruct(object.Cast<UE_UStruct>(), classes);
		}
		else if (object.IsA<UE_UScriptStruct>())
		{
			GenerateStruct(object.Cast<UE_UStruct>(), structures);
		}
		else if (object.IsA<UE_UEnum>())
		{
			GenerateEnum(object.Cast<UE_UEnum>(), enums);
		}
	}
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
	s.Fullname = object.GetFullName();
	s.NameCppFull = "struct " + object.GetNameCPP();

	auto super = object.GetSuper();
	if (super)
	{
		s.NameCppFull += " : public " + super.GetNameCPP();
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

			for (auto prop = fn.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
			{

				auto flags = prop.GetPropertyFlags();
				
				auto [type, ref] = prop.GetInfo();
				if (flags & 0x400) // if property has 'ReturnParm' flag
				{
					f.CppName = type + " " + fn.GetName();
					continue;
				}
				else if (flags & 0x80) // if property has 'Parm' flag
				{
					if (prop.GetArrayDim() > 1)
					{
						f.Params += fmt::format("{}* {}, ", type, prop.GetName());
					}
					else if (ref)
					{
						f.Params += fmt::format("{}& {}, ", type, prop.GetName());
					}
					else
					{
						f.Params += fmt::format("{} {}, ", type, prop.GetName());
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
	auto enumName = object.GetName();
	e.FullName = object.GetFullName();
	e.NameCppFull = fmt::format("enum class {} : uint8_t", enumName); ;
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
		fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{", s.Fullname, s.Size, s.Inherited, s.NameCppFull);
		for (auto& m : s.Members)
		{
			fmt::print(file, "\n\t{}; // {:#04x}({:#04x})", m.Name, m.Offset, m.Size);
		}
		if (s.Functions.size())
		{
			fwrite("\n", 1, 1, file);
			for (auto& f : s.Functions)
			{
				fmt::print(file, "\n\t{}({});", f.CppName, f.Params);
			}
		}
		
		fmt::print(file, "\n}}\n\n");
	}
}

bool UE_UPackage::Save(const fs::path& dir)
{
	if (!(classes.size() || structures.size() || enums.size()))
	{ 
		return false;
	}

	std::string packageName = this->GetName();

	if (classes.size())
	{
		File file(dir / (packageName + "_classes.h"), "w");
		if (!file) { return false; }
		UE_UPackage::SaveStruct(classes, file);
	}
	
	{
		File file(dir / (packageName + "_struct.h"), "w");
		if (!file) { return false; }

		if (enums.size())
		{
			for (auto& e : enums)
			{
				fmt::print(file, "// {}\n{} {{", e.FullName, e.NameCppFull);
				for (auto& m : e.Members)
				{
					fmt::print(file, "\n\t{},", m);
				}
				fmt::print(file, "\n}}\n\n");
			}
		}

		if (structures.size())
		{
			UE_UPackage::SaveStruct(structures, file);
		}
	}

	return true;
}

std::string UE_UPackage::GetName()
{
	return UE_UObject(package->first).GetName();
}

UE_FField UE_FField::GetNext() const
{
	return UE_FField(Read<byte*>(object + defs.FField.Next));
};

std::string UE_FField::GetName() const
{
	auto name = UE_FName(object + defs.FField.Name);
	auto str = name.GetName();
	return str;
}


UE_UClass UE_UProperty::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Property");
	return obj;
}


uint32_t UE_UFunction::GetFunctionFlags() const
{
	return uint32_t();
}

UE_UClass UE_UFunction::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Function");
	return obj;
}

UE_UClass UE_UClass::StaticClass()
{
	static auto obj = ObjObjects.FindObject("Class CoreUObject.Class");
	return obj;
};

UE_UStruct UE_FStructProperty::GetStruct() const
{
	return Read<UE_UStruct>(object + defs.FStructProperty.Struct);
}

std::string UE_FStructProperty::GetType() const
{
	return "struct " +  GetStruct().GetNameCPP();
}

UE_UClass UE_FObjectPropertyBase::GetPropertyClass() const
{
	return Read<UE_UClass>(object + defs.FObjectPropertyBase.PropertyClass);
}

std::string UE_FObjectPropertyBase::GetType() const
{
	return "struct " + GetPropertyClass().GetNameCPP() + "*";
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
	return GetEnum().GetName();
}

UE_UClass UE_FClassProperty::GetMetaClass() const
{
	return Read<UE_UClass>(object + defs.FClassProperty.MetaClass);
}

std::string UE_FClassProperty::GetType() const
{
	return "struct " + GetMetaClass().GetNameCPP() + "*";
}

UE_FProperty UE_FSetProperty::GetElementProp() const
{
	return Read<UE_FProperty>(object + defs.FSetProperty.ElementProp);
}

std::string UE_FSetProperty::GetType() const
{
	return "struct TSet<" + GetElementProp().GetType() + ">";	
}

std::string UE_FFieldClass::GetName() const
{
	auto name = UE_FName(object + defs.FFieldClass.Name);
	return name.GetName();
}
