#include <algorithm>
#include <fmt/core.h>
#include "memory.h"
#include "engine.h"
#include "wrappers.h"

std::pair<bool, uint16> UE_FNameEntry::Info() const
{
	auto info = Read<uint16>(object + offsets.FNameEntry.Info);
	auto len = info >> offsets.FNameEntry.LenBit;
	bool wide = (info >> offsets.FNameEntry.WideBit) & 1;
	return {wide, len};
}

std::string UE_FNameEntry::String(bool wide, uint16 len) const
{
	std::string name("\x0", len);
	String(name.data(), wide, len);
	return name;
}

void UE_FNameEntry::String(char* buf, bool wide, uint16 len) const
{
	if (wide)
	{
		wchar_t wbuf[1024]{};
		Read(object + offsets.FNameEntry.HeaderSize, wbuf, len * 2ull);
		if (Decrypt_WIDE && !Decrypt_WIDE(wbuf, len)) { buf[0] = '\x0'; return; }
		auto copied = WideCharToMultiByte(CP_UTF8, 0, wbuf, len, buf, len, 0, 0);
		if (copied == 0) { buf[0] = '\x0'; }
	}
	else
	{
		Read(object + offsets.FNameEntry.HeaderSize, buf, len);
		if (Decrypt_ANSI && Decrypt_ANSI(buf, len)) { buf[0] = '\x0'; }
	}
}

uint16 UE_FNameEntry::Size(bool wide, uint16 len)
{
	uint16 bytes = offsets.FNameEntry.HeaderSize + len * (wide ? sizeof(wchar_t) : sizeof(char));
	return (bytes + offsets.Stride - 1u) & ~(offsets.Stride - 1u);
}

std::string UE_FName::GetName() const
{
	uint32_t index = Read<uint32_t>(object);
	auto entry = UE_FNameEntry(NamePoolData.GetEntry(index));
	if (!entry) return std::string();
	auto [wide, len] = entry.Info();
	auto name = entry.String(wide, len);
	uint32_t number = Read<uint32_t>(object + offsets.FName.Number);
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
	return Read<uint32_t>(object + offsets.UObject.Index);
};

UE_UClass UE_UObject::GetClass() const
{
	return Read<UE_UClass>(object + offsets.UObject.Class);
}

UE_UObject UE_UObject::GetOuter() const
{
	return Read<UE_UObject>(object + offsets.UObject.Outer);
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
	auto fname = UE_FName(object + offsets.UObject.Name);
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
	std::string name;
	if (IsA<UE_UClass>())
	{
		for (auto c = Cast<UE_UStruct>(); c; c = c.GetSuper())
		{
			if (c == UE_AActor::StaticClass())
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

bool UE_UObject::IsA(UE_UClass cmp) const
{
	for (auto super = GetClass(); super; super = super.GetSuper().Cast<UE_UClass>())
	{
		if (super == cmp)
		{
			return true;
		}
	}

	return false;
}

UE_UClass UE_UObject::StaticClass()
{
	static auto obj = static_cast<UE_UClass>(ObjObjects.FindObject("Class CoreUObject.Object"));
	return obj;
};

UE_UClass UE_AActor::StaticClass()
{
	static auto obj = static_cast<UE_UClass>(ObjObjects.FindObject("Class Engine.Actor"));
	return obj;
}

UE_UField UE_UField::GetNext() const
{
	return Read<UE_UField>(object + offsets.UField.Next);
}

UE_UClass UE_UField::StaticClass()
{
	static auto obj = static_cast<UE_UClass>(ObjObjects.FindObject("Class CoreUObject.Field"));
	return obj;
};

UE_UClass UE_UProperty::StaticClass()
{
	static auto obj = static_cast<UE_UClass>(ObjObjects.FindObject("Class CoreUObject.Property"));
	return obj;
}

UE_UStruct UE_UStruct::GetSuper() const
{
	return Read<UE_UStruct>(object + offsets.UStruct.SuperStruct);
}

UE_FField UE_UStruct::GetChildProperties() const
{
	return Read<UE_FField>(object + offsets.UStruct.ChildProperties);
}

UE_UField UE_UStruct::GetChildren() const
{
	return Read<UE_UField>(object + offsets.UStruct.Children);
}

int32_t UE_UStruct::GetSize() const
{
	return Read<int32_t>(object + offsets.UStruct.PropertiesSize);
};

UE_UClass UE_UStruct::StaticClass()
{
	static auto obj = static_cast<UE_UClass>(ObjObjects.FindObject("Class CoreUObject.Struct"));
	return obj;
};

uint64_t UE_UFunction::GetFunc() const
{
	return Read<uint64_t>(object + offsets.UFunction.Func);
}

std::string UE_UFunction::GetFunctionFlags() const
{
	auto flags = Read<uint32_t>(object + offsets.UFunction.FunctionFlags);
	std::string result;
	if (flags && FUNC_None) { result = "None"; }
	else
	{
		if (flags & FUNC_Final) { result += "Final|"; }
		if (flags & FUNC_RequiredAPI) { result += "RequiredAPI|"; }
		if (flags & FUNC_BlueprintAuthorityOnly) { result += "BlueprintAuthorityOnly|"; }
		if (flags & FUNC_BlueprintCosmetic) { result += "BlueprintCosmetic|"; }
		if (flags & FUNC_Net) { result += "Net|"; }
		if (flags & FUNC_NetReliable) { result += "NetReliable"; }
		if (flags & FUNC_NetRequest) { result += "NetRequest|"; }
		if (flags & FUNC_Exec) { result += "Exec|"; }
		if (flags & FUNC_Native) { result += "Native|"; }
		if (flags & FUNC_Event) { result += "Event|"; }
		if (flags & FUNC_NetResponse) { result += "NetResponse|"; }
		if (flags & FUNC_Static) { result += "Static|"; }
		if (flags & FUNC_NetMulticast) { result += "NetMulticast|"; }
		if (flags & FUNC_UbergraphFunction) { result += "UbergraphFunction|"; }
		if (flags & FUNC_MulticastDelegate) { result += "MulticastDelegate|"; }
		if (flags & FUNC_Public) { result += "Public|"; }
		if (flags & FUNC_Private) { result += "Private|"; }
		if (flags & FUNC_Protected) { result += "Protected|"; }
		if (flags & FUNC_Delegate) { result += "Delegate|"; }
		if (flags & FUNC_NetServer) { result += "NetServer|"; }
		if (flags & FUNC_HasOutParms) { result += "HasOutParms|"; }
		if (flags & FUNC_HasDefaults) { result += "HasDefaults|"; }
		if (flags & FUNC_NetClient) { result += "NetClient|"; }
		if (flags & FUNC_DLLImport) { result += "DLLImport|"; }
		if (flags & FUNC_BlueprintCallable) { result += "BlueprintCallable|"; }
		if (flags & FUNC_BlueprintEvent) { result += "BlueprintEvent|"; }
		if (flags & FUNC_BlueprintPure) { result += "BlueprintPure|"; }
		if (flags & FUNC_EditorOnly) { result += "EditorOnly|"; }
		if (flags & FUNC_Const) { result += "Const|"; }
		if (flags & FUNC_NetValidate) { result += "NetValidate|"; }
		if (result.size()) { result.erase(result.size() - 1); }
	}
	return result;
}

UE_UClass UE_UFunction::StaticClass()
{
	static auto obj = static_cast<UE_UClass>(ObjObjects.FindObject("Class CoreUObject.Function"));
	return obj;
}

UE_UClass UE_UScriptStruct::StaticClass()
{
	static UE_UClass obj = static_cast<UE_UClass>(ObjObjects.FindObject("Class CoreUObject.ScriptStruct"));
	return obj;
};

UE_UClass UE_UClass::StaticClass()
{
	static UE_UClass obj = static_cast<UE_UClass>(ObjObjects.FindObject("Class CoreUObject.Class"));
	return obj;
};

TArray UE_UEnum::GetNames() const
{
	return Read<TArray>(object + offsets.UEnum.Names);
}

UE_UClass UE_UEnum::StaticClass()
{
	static UE_UClass obj = static_cast<UE_UClass>(ObjObjects.FindObject("Class CoreUObject.Enum"));
	return obj;
}

std::string UE_FFieldClass::GetName() const
{
	auto name = UE_FName(object);
	return name.GetName();
}

UE_FField UE_FField::GetNext() const
{
	return Read<UE_FField>(object + offsets.FField.Next);
};

std::string UE_FField::GetName() const
{
	auto name = UE_FName(object + offsets.FField.Name);
	return name.GetName();
}

int32_t UE_FProperty::GetArrayDim() const
{
	return Read<int32_t>(object + offsets.FProperty.ArrayDim);
}

int32_t UE_FProperty::GetSize() const
{
	return Read<int32_t>(object + offsets.FProperty.ElementSize);
}

int32_t UE_FProperty::GetOffset() const
{
	return Read<int32_t>(object + offsets.FProperty.Offset);
}

uint64_t UE_FProperty::GetPropertyFlags() const
{
	return Read<uint64_t>(object + offsets.FProperty.PropertyFlags);
}

std::unordered_map<std::string, std::function<void(const UE_FProperty*, std::pair<PropertyType, std::string>&)>> types =
{

	{
		"StructProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FStructProperty>();
			type = {PropertyType::StructProperty, obj.GetType()};
		}
	},
	{
		"ObjectProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FObjectPropertyBase>();
			type = {PropertyType::ObjectProperty, obj.GetType()};
		}
	},
	{
		"SoftObjectProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FObjectPropertyBase>();
			type = {PropertyType::SoftObjectProperty, "struct TSoftObjectPtr<struct " + obj.GetPropertyClass().GetCppName() + ">"};
		}
	},
	{
		"FloatProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::FloatProperty, "float"};
		}
	},
	{
		"ByteProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FByteProperty>();
			type = {PropertyType::ByteProperty, obj.GetType()};
		}
	},
	{
		"BoolProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FBoolProperty>();
			type = {PropertyType::BoolProperty, obj.GetType()};
		}
	},
	{
		"IntProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::IntProperty, "int32_t"};
		}
	},
	{
		"Int8Property",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::Int8Property, "int8_t"};
		}
	},
	{
		"Int16Property",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::Int16Property, "int16_t"};
		}
	},
	{
		"Int64Property",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::Int64Property, "int64_t"};
		}
	},
	{
		"UInt16Property",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::UInt16Property, "uint16"};
		}
	},
	{
		"UInt32Property",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::UInt32Property, "uint32_t"};
		}
	},
	{
		"UInt64Property",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::UInt64Property, "uint64_t"};
		}
	},
	{
		"NameProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::NameProperty, "struct FName"};
		}
	},
	{
		"DelegateProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::DelegateProperty, "struct FDelegate"};
		}
	},
	{
		"SetProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FSetProperty>();
			type = {PropertyType::SetProperty, obj.GetType()};
		}
	},
	{
		"ArrayProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FArrayProperty>();
			type = {PropertyType::ArrayProperty, obj.GetType()};
		}
	},
	{
		"WeakObjectProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FStructProperty>();
			type = {PropertyType::WeakObjectProperty, "struct FWeakObjectPtr<" + obj.GetType() + ">"};
		}
	},
	{
		"StrProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::StrProperty, "struct FString"};
		}
	},
	{
		"TextProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::TextProperty, "struct FText"};
		}
	},
	{
		"MulticastSparseDelegateProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::MulticastSparseDelegateProperty, "struct FMulticastSparseDelegate"};
		}
	},
	{
		"EnumProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FEnumProperty>();
			type = {PropertyType::EnumProperty, obj.GetType()};
		}
	},
	{
		"DoubleProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::DoubleProperty, "double"};
		}
	},
	{
		"MulticastDelegateProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::MulticastDelegateProperty, "FMulticastDelegate"};
		}
	},
	{
		"ClassProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FClassProperty>();
			type = {PropertyType::ClassProperty, obj.GetType()};
		}
	},
	{
		"MulticastInlineDelegateProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			type = {PropertyType::MulticastDelegateProperty, "struct FMulticastInlineDelegate"};
		}
	},
	{
		"MapProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FMapProperty>();
			type = {PropertyType::MapProperty, obj.GetType()};
		}
	},

	{
		"InterfaceProperty",
		[](const UE_FProperty* prop, std::pair<PropertyType, std::string>& type)
		{
			auto obj = prop->Cast<UE_FInterfaceProperty>();
			type = {PropertyType::InterfaceProperty, obj.GetType()};
		}
	}
};

std::pair<PropertyType, std::string> UE_FProperty::GetType() const
{
	using namespace std;

	auto objectClass = Read<UE_FFieldClass>(object + offsets.FField.Class);
	pair<PropertyType, string> type = {PropertyType::Unknown, objectClass.GetName()};

	auto fn = types.find(type.second);

	if (fn != types.end())
	{
		fn->second(this, type);
	}

	return type;
}

UE_UStruct UE_FStructProperty::GetStruct() const
{
	return Read<UE_UStruct>(object + offsets.FProperty.Size);
}

std::string UE_FStructProperty::GetType() const
{
	return "struct " + GetStruct().GetCppName();
}

UE_UClass UE_FObjectPropertyBase::GetPropertyClass() const
{
	return Read<UE_UClass>(object + offsets.FProperty.Size);
}

std::string UE_FObjectPropertyBase::GetType() const
{
	return "struct " + GetPropertyClass().GetCppName() + "*";
}

UE_FProperty UE_FArrayProperty::GetInner() const
{
	return Read<UE_FProperty>(object + offsets.FProperty.Size);
}

std::string UE_FArrayProperty::GetType() const
{
	return "struct TArray<" + GetInner().GetType().second + ">";
}

UE_UEnum UE_FByteProperty::GetEnum() const
{
	return Read<UE_UEnum>(object + offsets.FProperty.Size);
}

std::string UE_FByteProperty::GetType() const
{
	return "enum class " + GetEnum().GetName();
}

uint8_t UE_FBoolProperty::GetFieldMask() const
{
	return Read<uint8_t>(object + offsets.FProperty.Size + 3);
}

std::string UE_FBoolProperty::GetType() const
{
	if (GetFieldMask() == 0xFF) { return "bool"; };
	return "char";
}

UE_UClass UE_FEnumProperty::GetEnum() const
{
	return Read<UE_UClass>(object + offsets.FProperty.Size + 8);
}

std::string UE_FEnumProperty::GetType() const
{
	return "enum class " + GetEnum().GetName();
}

UE_UClass UE_FClassProperty::GetMetaClass() const
{
	return Read<UE_UClass>(object + offsets.FProperty.Size + 8);
}

std::string UE_FClassProperty::GetType() const
{
	return "struct " + GetMetaClass().GetCppName() + "*";
}

UE_FProperty UE_FSetProperty::GetElementProp() const
{
	return Read<UE_FProperty>(object + offsets.FProperty.Size);
}

std::string UE_FSetProperty::GetType() const
{
	return "struct TSet<" + GetElementProp().GetType().second + ">";
}

UE_FProperty UE_FMapProperty::GetKeyProp() const
{
	return Read<UE_FProperty>(object + offsets.FProperty.Size);
}

UE_FProperty UE_FMapProperty::GetValueProp() const
{
	return Read<UE_FProperty>(object + offsets.FProperty.Size + 8);
}

std::string UE_FMapProperty::GetType() const
{
	return fmt::format("struct TMap<{}, {}>", GetKeyProp().GetType().second, GetValueProp().GetType().second);
}

UE_FProperty UE_FInterfaceProperty::GetInterfaceClass() const
{
	return Read<UE_FProperty>(object + offsets.FProperty.Size);
}

std::string UE_FInterfaceProperty::GetType() const
{
	return "struct TScriptInterface<" + GetInterfaceClass().GetType().second + ">";
}

void UE_UPackage::GenerateBitPadding(std::vector<Member>& members, int32_t offset, int16_t bitOffset, int16_t size)
{
	Member padding;
	padding.Type = "char";
	padding.Name = fmt::format("UnknownData_{:0X}_{} : {}", offset, bitOffset, size);
	padding.Offset = offset;
	padding.Size = 1;
	members.push_back(padding);
}

void UE_UPackage::GeneratePadding(std::vector<Member>& members, int32_t& minOffset, int32_t& bitOffset, int32_t maxOffset)
{
	if (bitOffset)
	{
		if (bitOffset < 7) { UE_UPackage::GenerateBitPadding(members, minOffset, bitOffset, 8 - bitOffset); }
		bitOffset = 0;
		minOffset++;
	}
	if (maxOffset > minOffset)
	{
		Member padding;
		auto size = maxOffset - minOffset;
		padding.Type = "char";
		padding.Name = fmt::format("UnknownData_{:0X}[{:#0x}]", minOffset, size);
		padding.Offset = minOffset;
		padding.Size = size;
		members.push_back(padding);
		minOffset = maxOffset;
	}
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
		s.CppName += " : " + super.GetCppName();
		s.Inherited = super.GetSize();
	}

	int32_t offset = s.Inherited;
	int32_t bitOffset = 0;
	for (auto prop = object.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
	{
		auto arrDim = prop.GetArrayDim();
		Member m;
		m.Size = prop.GetSize() * arrDim;
		if (m.Size == 0) { return; } // this shouldn't be zero

		auto type = prop.GetType();
		m.Type = type.second;
		m.Name = prop.GetName();
		m.Offset = prop.GetOffset();

		
		if (m.Offset > offset)
		{
			/*
			auto size = m.Offset - offset;
			if (size >= 8)
			{
				int32 endOffset = offset + (size / 8) * 8;
				auto& members = s.Members;

				ObjObjects.ForEachObjectOfClass(object.Cast<UE_UClass>(), [&members, &offset, &endOffset, &bitOffset](uint8* obj) {
					
					uint64* end = (uint64*)(obj + endOffset);
					int32 curOffset = offset;

					for (uint64* ptr = (uint64*)(obj + offset); ptr < end; ptr++, curOffset += 8)
					{
						auto address = Read<UE_UObject>(ptr);

						MEMORY_BASIC_INFORMATION memInfo;

						if (!address || !GetMemoryInfo(address, &memInfo) || !ObjObjects.IsObject(address)) continue;

						auto c = address.GetClass();

						auto cName = c.GetName();

						UE_UPackage::GeneratePadding(members, offset, bitOffset, curOffset);

						auto type = cName + "*";
						auto name = address.GetName();
						members.push_back({ type, name, curOffset, 8 });
					}
					});
			}
			*/
			
			
			UE_UPackage::GeneratePadding(s.Members, offset, bitOffset, m.Offset);

		}
		

		if (type.first == PropertyType::BoolProperty && *(uint32*)type.second.data() != *(uint32*)"bool")
		{
			auto boolProp = prop.Cast<UE_FBoolProperty>();
			auto mask = boolProp.GetFieldMask();
			int zeros = 0, ones = 0;
			while (mask & ~1)
			{
				mask >>= 1;
				zeros++;
			}
			while (mask & 1)
			{
				mask >>= 1;
				ones++;
			}
			if (zeros > bitOffset)
			{
				UE_UPackage::GenerateBitPadding(s.Members, offset, bitOffset, zeros - bitOffset);
				bitOffset = zeros;
			}
			m.Name += fmt::format(" : {}", ones);
			bitOffset += ones;
		}
		else
		{
			if (arrDim > 1)
			{
				m.Name += fmt::format("[{:#0x}]", arrDim);
			}

			offset += m.Size;
		}
		s.Members.push_back(m);
	}

	if (s.Size > offset)
	{
		UE_UPackage::GeneratePadding(s.Members, offset, bitOffset, s.Size);
	}

	for (auto fn = object.GetChildren().Cast<UE_UFunction>(); fn; fn = fn.GetNext().Cast<UE_UFunction>())
	{
		if (fn.IsA<UE_UFunction>())
		{
			Function f;
			f.FullName = fn.GetFullName();
			f.Flags = fn.GetFunctionFlags();
			f.Func = fn.GetFunc();

			for (auto prop = fn.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
			{
				auto flags = prop.GetPropertyFlags();
				if (flags & 0x400) // if property has 'ReturnParm' flag
				{
					f.CppName = prop.GetType().second + " " + fn.GetName();
				}
				else if (flags & 0x80) // if property has 'Parm' flag
				{
					if (prop.GetArrayDim() > 1)
					{
						f.Params += fmt::format("{}* {}, ", prop.GetType().second, prop.GetName());
					}
					else
					{
						f.Params += fmt::format("{} {}, ", prop.GetType().second, prop.GetName());
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
	e.CppName = "enum class " + object.GetName() + " : uint8";
	auto names = object.GetNames();
	for (auto i = 0ull; i < names.Count; i++)
	{
		auto size = (offsets.FName.Number + 4u + 8 + 7u) & ~(7u);
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

void UE_UPackage::SaveStruct(std::vector<Struct>& arr, FILE* file)
{
	for (auto& s : arr)
	{
		fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{", s.FullName, s.Size, s.Inherited, s.CppName);
		for (auto& m : s.Members)
		{
			fmt::print(file, "\n\t{} {}; // {:#04x}({:#04x})", m.Type, m.Name, m.Offset, m.Size);
		}
		if (s.Functions.size())
		{
			fwrite("\n", 1, 1, file);
			for (auto& f : s.Functions)
			{
				fmt::print(file, "\n\t{}({}); // {} // ({}) // @ game+{:#08x}", f.CppName, f.Params, f.FullName, f.Flags, f.Func - Base);
			}
		}

		fmt::print(file, "\n}};\n\n");
	}
}

void UE_UPackage::SaveStructSpacing(std::vector<Struct>& arr, FILE* file)
{
	for (auto& s : arr)
	{
		fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{", s.FullName, s.Size, s.Inherited, s.CppName);
		for (auto& m : s.Members)
		{
			fmt::print(file, "\n\t{:69} {:60} // {:#04x}({:#04x})", m.Type, m.Name + ";", m.Offset, m.Size);
		}
		if (s.Functions.size())
		{
			fwrite("\n", 1, 1, file);
			for (auto& f : s.Functions)
			{
				fmt::print(file, "\n\t{:130} // {} // ({}) // @ game+{:#08x}", fmt::format("{}({});", f.CppName, f.Params), f.FullName, f.Flags, f.Func - Base);
			}
		}

		fmt::print(file, "\n}};\n\n");
	}
}

void UE_UPackage::SaveEnum(std::vector<Enum>& arr, FILE* file)
{
	for (auto& e : arr)
	{
		fmt::print(file, "// {}\n{} {{", e.FullName, e.CppName);
		for (auto& m : e.Members)
		{
			fmt::print(file, "\n\t{},", m);
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

bool UE_UPackage::Save(const fs::path& dir, bool spacing)
{
	if (!(Classes.size() || Structures.size() || Enums.size()))
	{
		return false;
	}

	std::string packageName = GetObject().GetName();

	char chars[] = "/\\:*?\"<>|";
	for (auto c : chars)
	{
		auto pos = packageName.find(c);
		if (pos != std::string::npos) { packageName[pos] = '_'; }
	}

	if (Classes.size())
	{
		File file(dir / (packageName + "_classes.h"), "w");
		if (!file) { return false; }
		if (spacing) 
		{
			UE_UPackage::SaveStructSpacing(Classes, file);
		}
		else
		{
			UE_UPackage::SaveStruct(Classes, file);
		}
	}

	if (Structures.size() || Enums.size())
	{
		File file(dir / (packageName + "_struct.h"), "w");
		if (!file) { return false; }

		if (Enums.size())
		{
			UE_UPackage::SaveEnum(Enums, file);
		}

		if (Structures.size())
		{
			if (spacing)
			{
				UE_UPackage::SaveStructSpacing(Structures, file);
			}
			else
			{
				UE_UPackage::SaveStruct(Structures, file);
			}
		}
	}

	return true;
}

UE_UObject UE_UPackage::GetObject() const
{
	return UE_UObject(Package->first);
}
