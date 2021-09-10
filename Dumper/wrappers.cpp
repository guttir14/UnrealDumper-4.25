#include <Windows.h>
#include <algorithm>
#include <fmt/core.h>
#include <hash/hash.h>
#include "engine.h"
#include "memory.h"
#include "wrappers.h"

std::pair<bool, uint16> UE_FNameEntry::Info() const {
  auto info = Read<uint16>(object + offsets.FNameEntry.Info);
  auto len = info >> offsets.FNameEntry.LenBit;
  bool wide = (info >> offsets.FNameEntry.WideBit) & 1;
  return {wide, len};
}

std::string UE_FNameEntry::String(bool wide, uint16 len) const {
  std::string name("\x0", len);
  String(name.data(), wide, len);
  return name;
}

void UE_FNameEntry::String(char *buf, bool wide, uint16 len) const {
  if (wide) {
    wchar_t wbuf[1024]{};
    Read(object + offsets.FNameEntry.HeaderSize, wbuf, len * 2ull);
    /*if (Decrypt_WIDE) { Decrypt_WIDE(wbuf, len); }*/
    auto copied = WideCharToMultiByte(CP_UTF8, 0, wbuf, len, buf, len, 0, 0);
    if (copied == 0) {
      buf[0] = '\x0';
    }
  } else {
    Read(object + offsets.FNameEntry.HeaderSize, buf, len);
    if (Decrypt_ANSI) {
      Decrypt_ANSI(buf, len);
    }
  }
}

std::string UE_FNameEntry::String() const {
  auto [wide, len] = this->Info();
  return this->String(wide, len);
}

uint16 UE_FNameEntry::Size(bool wide, uint16 len) {
  uint16 bytes = offsets.FNameEntry.HeaderSize + len * (wide ? 2 : 1);
  return (bytes + offsets.Stride - 1u) & ~(offsets.Stride - 1u);
}

std::string UE_FName::GetName() const {
  uint32 index = Read<uint32>(object);
  auto entry = UE_FNameEntry(NamePoolData.GetEntry(index));
  if (!entry) return std::string();
  auto [wide, len] = entry.Info();
  auto name = entry.String(wide, len);
  uint32 number = Read<uint32>(object + offsets.FName.Number);
  if (number > 0) {
    name += '_' + std::to_string(number);
  }
  auto pos = name.rfind('/');
  if (pos != std::string::npos) {
    name = name.substr(pos + 1);
  }
  return name;
}

uint32 UE_UObject::GetIndex() const {
  return Read<uint32>(object + offsets.UObject.Index);
};

UE_UClass UE_UObject::GetClass() const {
  return Read<UE_UClass>(object + offsets.UObject.Class);
}

UE_UObject UE_UObject::GetOuter() const {
  return Read<UE_UObject>(object + offsets.UObject.Outer);
}

UE_UObject UE_UObject::GetPackageObject() const {
  UE_UObject package(nullptr);
  for (auto outer = GetOuter(); outer; outer = outer.GetOuter()) {
    package = outer;
  }
  return package;
}

std::string UE_UObject::GetName() const {
  auto fname = UE_FName(object + offsets.UObject.Name);
  return fname.GetName();
}

std::string UE_UObject::GetFullName() const {
  std::string temp;
  for (auto outer = GetOuter(); outer; outer = outer.GetOuter()) {
    temp = outer.GetName() + "." + temp;
  }
  UE_UClass objectClass = GetClass();
  std::string name = objectClass.GetName() + " " + temp + GetName();
  return name;
}

std::string UE_UObject::GetCppName() const {
  std::string name;
  if (IsA<UE_UClass>()) {
    for (auto c = Cast<UE_UStruct>(); c; c = c.GetSuper()) {
      if (c == UE_AActor::StaticClass()) {
        name = "A";
        break;
      } else if (c == UE_UObject::StaticClass()) {
        name = "U";
        break;
      }
    }
  } else {
    name = "F";
  }

  name += GetName();
  return name;
}

bool UE_UObject::IsA(UE_UClass cmp) const {
  for (auto super = GetClass(); super; super = super.GetSuper().Cast<UE_UClass>()) {
    if (super == cmp) {
      return true;
    }
  }

  return false;
}

UE_UClass UE_UObject::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Object"));
  return obj;
};

UE_UClass UE_AActor::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class Engine.Actor"));
  return obj;
}

UE_UField UE_UField::GetNext() const {
  return Read<UE_UField>(object + offsets.UField.Next);
}

UE_UClass UE_UField::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Field"));
  return obj;
};

std::string IUProperty::GetName() const {
  return ((UE_UProperty*)(this->prop))->GetName();
}

int32 IUProperty::GetArrayDim() const {
  return ((UE_UProperty*)(this->prop))->GetArrayDim();
}

int32 IUProperty::GetSize() const {
  return ((UE_UProperty*)(this->prop))->GetSize();
}

int32 IUProperty::GetOffset() const {
  return ((UE_UProperty*)(this->prop))->GetOffset();
}

uint64 IUProperty::GetPropertyFlags() const {
  return ((UE_UProperty*)(this->prop))->GetPropertyFlags();
}

std::pair<PropertyType, std::string> IUProperty::GetType() const {
  return ((UE_UProperty*)(this->prop))->GetType();
}

uint8 IUProperty::GetFieldMask() const {
  return ((UE_UBoolProperty *)(this->prop))->GetFieldMask();
}

int32 UE_UProperty::GetArrayDim() const {
  return Read<int32>(object + offsets.UProperty.ArrayDim);
}

int32 UE_UProperty::GetSize() const {
  return Read<int32>(object + offsets.UProperty.ElementSize);
}

int32 UE_UProperty::GetOffset() const {
  return Read<int32>(object + offsets.UProperty.Offset);
}

uint64 UE_UProperty::GetPropertyFlags() const {
  return Read<uint64>(object + offsets.UProperty.PropertyFlags);
}

std::pair<PropertyType, std::string> UE_UProperty::GetType() const {
  if (IsA<UE_UDoubleProperty>()) { return {PropertyType::DoubleProperty,Cast<UE_UDoubleProperty>().GetTypeStr()}; };
  if (IsA<UE_UFloatProperty>()) { return {PropertyType::FloatProperty, Cast<UE_UFloatProperty>().GetTypeStr()}; };
  if (IsA<UE_UIntProperty>()) { return {PropertyType::IntProperty, Cast<UE_UIntProperty>().GetTypeStr()}; };
  if (IsA<UE_UInt16Property>()) { return {PropertyType::Int16Property,Cast<UE_UInt16Property>().GetTypeStr()}; };
  if (IsA<UE_UInt64Property>()) { return {PropertyType::Int64Property, Cast<UE_UInt64Property>().GetTypeStr()}; };
  if (IsA<UE_UInt8Property>()) { return {PropertyType::Int8Property, Cast<UE_UInt8Property>().GetTypeStr()}; };
  if (IsA<UE_UUInt16Property>()) { return {PropertyType::UInt16Property, Cast<UE_UUInt16Property>().GetTypeStr()}; };
  if (IsA<UE_UUInt32Property>()) { return {PropertyType::UInt32Property, Cast<UE_UUInt32Property>().GetTypeStr()}; }
  if (IsA<UE_UUInt64Property>()) { return {PropertyType::UInt64Property, Cast<UE_UUInt64Property>().GetTypeStr()}; };
  if (IsA<UE_UTextProperty>()) { return {PropertyType::TextProperty, Cast<UE_UTextProperty>().GetTypeStr()}; }
  if (IsA<UE_UStrProperty>()) { return {PropertyType::TextProperty, Cast<UE_UStrProperty>().GetTypeStr()}; };
  if (IsA<UE_UClassProperty>()) { return {PropertyType::ClassProperty, Cast<UE_UClassProperty>().GetTypeStr()}; };
  if (IsA<UE_UStructProperty>()) { return {PropertyType::StructProperty, Cast<UE_UStructProperty>().GetTypeStr()}; };
  if (IsA<UE_UNameProperty>()) { return {PropertyType::NameProperty, Cast<UE_UNameProperty>().GetTypeStr()}; };
  if (IsA<UE_UBoolProperty>()) { return {PropertyType::BoolProperty, Cast<UE_UBoolProperty>().GetTypeStr()}; }
  if (IsA<UE_UByteProperty>()) { return {PropertyType::ByteProperty, Cast<UE_UByteProperty>().GetTypeStr()}; };
  if (IsA<UE_UArrayProperty>()) { return {PropertyType::ArrayProperty, Cast<UE_UArrayProperty>().GetTypeStr()}; };
  if (IsA<UE_UEnumProperty>()) { return {PropertyType::EnumProperty, Cast<UE_UEnumProperty>().GetTypeStr()}; };
  if (IsA<UE_USetProperty>()) { return {PropertyType::SetProperty, Cast<UE_USetProperty>().GetTypeStr()}; };
  if (IsA<UE_UMapProperty>()) { return {PropertyType::MapProperty, Cast<UE_UMapProperty>().GetTypeStr()}; };
  if (IsA<UE_UInterfaceProperty>()) { return {PropertyType::InterfaceProperty, Cast<UE_UInterfaceProperty>().GetTypeStr()}; };
  if (IsA<UE_UMulticastDelegateProperty>()) { return {PropertyType::MulticastDelegateProperty, Cast<UE_UMulticastDelegateProperty>().GetTypeStr()}; };
  if (IsA<UE_UWeakObjectProperty>()) { return { PropertyType::WeakObjectProperty, Cast<UE_UWeakObjectProperty>().GetTypeStr() }; };
  if (IsA<UE_UObjectPropertyBase>()) { return {PropertyType::ObjectProperty, Cast<UE_UObjectPropertyBase>().GetTypeStr()}; };
  return {PropertyType::Unknown, GetClass().GetName()};
}

IUProperty UE_UProperty::GetInterface() const { return IUProperty(this); }

UE_UClass UE_UProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Property"));
  return obj;
}

UE_UStruct UE_UStruct::GetSuper() const {
  return Read<UE_UStruct>(object + offsets.UStruct.SuperStruct);
}

UE_FField UE_UStruct::GetChildProperties() const {
  if (offsets.UStruct.ChildProperties) return Read<UE_FField>(object + offsets.UStruct.ChildProperties);
  else return nullptr;
}

UE_UField UE_UStruct::GetChildren() const {
  return Read<UE_UField>(object + offsets.UStruct.Children);
}

int32 UE_UStruct::GetSize() const {
  return Read<int32>(object + offsets.UStruct.PropertiesSize);
};

UE_UClass UE_UStruct::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Struct"));
  return obj;
};

uint64 UE_UFunction::GetFunc() const {
  return Read<uint64>(object + offsets.UFunction.Func);
}

std::string UE_UFunction::GetFunctionFlags() const {
  auto flags = Read<uint32>(object + offsets.UFunction.FunctionFlags);
  std::string result;
  if (flags == FUNC_None) {
    result = "None";
  } else {
    if (flags & FUNC_Final) {
      result += "Final|";
    }
    if (flags & FUNC_RequiredAPI) {
      result += "RequiredAPI|";
    }
    if (flags & FUNC_BlueprintAuthorityOnly) {
      result += "BlueprintAuthorityOnly|";
    }
    if (flags & FUNC_BlueprintCosmetic) {
      result += "BlueprintCosmetic|";
    }
    if (flags & FUNC_Net) {
      result += "Net|";
    }
    if (flags & FUNC_NetReliable) {
      result += "NetReliable";
    }
    if (flags & FUNC_NetRequest) {
      result += "NetRequest|";
    }
    if (flags & FUNC_Exec) {
      result += "Exec|";
    }
    if (flags & FUNC_Native) {
      result += "Native|";
    }
    if (flags & FUNC_Event) {
      result += "Event|";
    }
    if (flags & FUNC_NetResponse) {
      result += "NetResponse|";
    }
    if (flags & FUNC_Static) {
      result += "Static|";
    }
    if (flags & FUNC_NetMulticast) {
      result += "NetMulticast|";
    }
    if (flags & FUNC_UbergraphFunction) {
      result += "UbergraphFunction|";
    }
    if (flags & FUNC_MulticastDelegate) {
      result += "MulticastDelegate|";
    }
    if (flags & FUNC_Public) {
      result += "Public|";
    }
    if (flags & FUNC_Private) {
      result += "Private|";
    }
    if (flags & FUNC_Protected) {
      result += "Protected|";
    }
    if (flags & FUNC_Delegate) {
      result += "Delegate|";
    }
    if (flags & FUNC_NetServer) {
      result += "NetServer|";
    }
    if (flags & FUNC_HasOutParms) {
      result += "HasOutParms|";
    }
    if (flags & FUNC_HasDefaults) {
      result += "HasDefaults|";
    }
    if (flags & FUNC_NetClient) {
      result += "NetClient|";
    }
    if (flags & FUNC_DLLImport) {
      result += "DLLImport|";
    }
    if (flags & FUNC_BlueprintCallable) {
      result += "BlueprintCallable|";
    }
    if (flags & FUNC_BlueprintEvent) {
      result += "BlueprintEvent|";
    }
    if (flags & FUNC_BlueprintPure) {
      result += "BlueprintPure|";
    }
    if (flags & FUNC_EditorOnly) {
      result += "EditorOnly|";
    }
    if (flags & FUNC_Const) {
      result += "Const|";
    }
    if (flags & FUNC_NetValidate) {
      result += "NetValidate|";
    }
    if (result.size()) {
      result.erase(result.size() - 1);
    }
  }
  return result;
}

UE_UClass UE_UFunction::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Function"));
  return obj;
}

UE_UClass UE_UScriptStruct::StaticClass() {
  static UE_UClass obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.ScriptStruct"));
  return obj;
};

UE_UClass UE_UClass::StaticClass() {
  static UE_UClass obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Class"));
  return obj;
};

TArray UE_UEnum::GetNames() const {
  return Read<TArray>(object + offsets.UEnum.Names);
}

UE_UClass UE_UEnum::StaticClass() {
  static UE_UClass obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Enum"));
  return obj;
}

std::string UE_UDoubleProperty::GetTypeStr() const { return "double"; }

UE_UClass UE_UDoubleProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.DoubleProperty"));
  return obj;
}

UE_UStruct UE_UStructProperty::GetStruct() const {
  return Read<UE_UStruct>(object + offsets.UProperty.Size);
}

std::string UE_UStructProperty::GetTypeStr() const {
  return "struct " + GetStruct().GetCppName();
}

UE_UClass UE_UStructProperty::StaticClass() {
  static auto obj = (UE_UClass)( ObjObjects.FindObject("Class CoreUObject.StructProperty"));
  return obj;
}

std::string UE_UNameProperty::GetTypeStr() const { return "struct FName"; }

UE_UClass UE_UNameProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.NameProperty"));
  return obj;
}

UE_UClass UE_UObjectPropertyBase::GetPropertyClass() const {
  return Read<UE_UClass>(object + offsets.UProperty.Size);
}

std::string UE_UObjectPropertyBase::GetTypeStr() const {
  return "struct " + GetPropertyClass().GetCppName() + "*";
}

UE_UClass UE_UObjectPropertyBase::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.ObjectPropertyBase"));
  return obj;
}

UE_UProperty UE_UArrayProperty::GetInner() const {
  return Read<UE_UProperty>(object + offsets.UProperty.Size);
}

std::string UE_UArrayProperty::GetTypeStr() const {
  return "struct TArray<" + GetInner().GetType().second + ">";
}

UE_UClass UE_UArrayProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.ArrayProperty"));
  return obj;
}

UE_UEnum UE_UByteProperty::GetEnum() const {
  return Read<UE_UEnum>(object + offsets.UProperty.Size);
}

std::string UE_UByteProperty::GetTypeStr() const {
  auto e = GetEnum();
  if (e) return "enum class " + e.GetName();
  return "char";
}

UE_UClass UE_UByteProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.ByteProperty"));
  return obj;
}

uint8 UE_UBoolProperty::GetFieldMask() const {
  return Read<uint8>(object + offsets.UProperty.Size + 3);
}

std::string UE_UBoolProperty::GetTypeStr() const {
  if (GetFieldMask() == 0xFF) {
    return "bool";
  };
  return "char";
}

UE_UClass UE_UBoolProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.BoolProperty"));
  return obj;
}

std::string UE_UFloatProperty::GetTypeStr() const { return "float"; }

UE_UClass UE_UFloatProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.FloatProperty"));
  return obj;
}

std::string UE_UIntProperty::GetTypeStr() const { return "int"; }

UE_UClass UE_UIntProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.IntProperty"));
  return obj;
}

std::string UE_UInt16Property::GetTypeStr() const { return "int16"; }

UE_UClass UE_UInt16Property::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Int16Property"));
  return obj;
}

std::string UE_UInt64Property::GetTypeStr() const { return "int64"; }

UE_UClass UE_UInt64Property::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Int64Property"));
  return obj;
}

std::string UE_UInt8Property::GetTypeStr() const { return "uint8"; }

UE_UClass UE_UInt8Property::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.Int8Property"));
  return obj;
}

std::string UE_UUInt16Property::GetTypeStr() const { return "uint16"; }

UE_UClass UE_UUInt16Property::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.UInt16Property"));
  return obj;
}

std::string UE_UUInt32Property::GetTypeStr() const { return "uint32"; }

UE_UClass UE_UUInt32Property::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.UInt32Property"));
  return obj;
}

std::string UE_UUInt64Property::GetTypeStr() const { return "uint64"; }

UE_UClass UE_UUInt64Property::StaticClass() {
  static auto obj = (UE_UClass)( ObjObjects.FindObject("Class CoreUObject.UInt64Property"));
  return obj;
}

std::string UE_UTextProperty::GetTypeStr() const { return "struct FText"; }

UE_UClass UE_UTextProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.TextProperty"));
  return obj;
}

std::string UE_UStrProperty::GetTypeStr() const { return "struct FString"; }

UE_UClass UE_UStrProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.StrProperty"));
  return obj;
}

UE_UClass UE_UEnumProperty::GetEnum() const {
  return Read<UE_UClass>(object + offsets.UProperty.Size + 8);
}

std::string UE_UEnumProperty::GetTypeStr() const {
  return "enum class " + GetEnum().GetName();
}

UE_UClass UE_UEnumProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.EnumProperty"));
  return obj;
}

UE_UClass UE_UClassProperty::GetMetaClass() const {
  return Read<UE_UClass>(object + offsets.UProperty.Size + 8);
}

std::string UE_UClassProperty::GetTypeStr() const {
  return "struct " + GetMetaClass().GetCppName() + "*";
}

UE_UClass UE_UClassProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.ClassProperty"));
  return obj;
}

UE_UProperty UE_USetProperty::GetElementProp() const {
  return Read<UE_UProperty>(object + offsets.UProperty.Size);
}

std::string UE_USetProperty::GetTypeStr() const {
  return "struct TSet<" + GetElementProp().GetType().second + ">";
}

UE_UClass UE_USetProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.SetProperty"));
  return obj;
}

UE_UProperty UE_UMapProperty::GetKeyProp() const {
  return Read<UE_UProperty>(object + offsets.UProperty.Size);
}

UE_UProperty UE_UMapProperty::GetValueProp() const {
  return Read<UE_UProperty>(object + offsets.UProperty.Size + 8);
}

std::string UE_UMapProperty::GetTypeStr() const {
  return fmt::format("struct TMap<{}, {}>", GetKeyProp().GetType().second, GetValueProp().GetType().second);
}

UE_UClass UE_UMapProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.MapProperty"));
  return obj;
}

UE_UProperty UE_UInterfaceProperty::GetInterfaceClass() const {
  return Read<UE_UProperty>(object + offsets.UProperty.Size);
}

std::string UE_UInterfaceProperty::GetTypeStr() const {
  return "struct TScriptInterface<" + GetInterfaceClass().GetType().second + ">";
}

UE_UClass UE_UInterfaceProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.InterfaceProperty"));
  return obj;
}

std::string UE_UMulticastDelegateProperty::GetTypeStr() const {
  return "struct FScriptMulticastDelegate";
}

UE_UClass UE_UMulticastDelegateProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.MulticastDelegateProperty"));
  return obj;
}

std::string UE_UWeakObjectProperty::GetTypeStr() const {
  return "struct TWeakObjectPtr<" + this->Cast<UE_UStructProperty>().GetTypeStr() + ">";
}

UE_UClass UE_UWeakObjectProperty::StaticClass() {
  static auto obj = (UE_UClass)(ObjObjects.FindObject("Class CoreUObject.WeakObjectProperty"));
  return obj;
}

std::string UE_FFieldClass::GetName() const {
  auto name = UE_FName(object);
  return name.GetName();
}

UE_FField UE_FField::GetNext() const {
  return Read<UE_FField>(object + offsets.FField.Next);
};

std::string UE_FField::GetName() const {
  auto name = UE_FName(object + offsets.FField.Name);
  return name.GetName();
}

std::string IFProperty::GetName() const {
  return ((UE_FProperty *)prop)->GetName();
}

int32 IFProperty::GetArrayDim() const {
  return ((UE_FProperty*)prop)->GetArrayDim();
}

int32 IFProperty::GetSize() const { return ((UE_FProperty *)prop)->GetSize(); }

int32 IFProperty::GetOffset() const {
  return ((UE_FProperty*)prop)->GetOffset();
}

uint64 IFProperty::GetPropertyFlags() const {
  return ((UE_FProperty*)prop)->GetPropertyFlags();
}

std::pair<PropertyType, std::string> IFProperty::GetType() const {
  return ((UE_FProperty*)prop)->GetType();
}

uint8 IFProperty::GetFieldMask() const {
  return ((UE_FBoolProperty *)prop)->GetFieldMask();
}

int32 UE_FProperty::GetArrayDim() const {
  return Read<int32>(object + offsets.FProperty.ArrayDim);
}

int32 UE_FProperty::GetSize() const {
  return Read<int32>(object + offsets.FProperty.ElementSize);
}

int32 UE_FProperty::GetOffset() const {
  return Read<int32>(object + offsets.FProperty.Offset);
}

uint64 UE_FProperty::GetPropertyFlags() const {
  return Read<uint64>(object + offsets.FProperty.PropertyFlags);
}

type UE_FProperty::GetType() const {
  auto objectClass = Read<UE_FFieldClass>(object + offsets.FField.Class);
  type type = {PropertyType::Unknown, objectClass.GetName()};

  auto& str = type.second;
  auto hash = Hash(str.c_str(), str.size());
  switch (hash) {
  case HASH("StructProperty"): {
    auto obj = this->Cast<UE_FStructProperty>();
    type = { PropertyType::StructProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("ObjectProperty"): {
    auto obj = this->Cast<UE_FObjectPropertyBase>();
    type = { PropertyType::ObjectProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("SoftObjectProperty"): {
    auto obj = this->Cast<UE_FObjectPropertyBase>();
    type = { PropertyType::SoftObjectProperty, "struct TSoftObjectPtr<" + obj.GetPropertyClass().GetCppName() + ">" };
    break;
  }
  case HASH("FloatProperty"): {
    type = { PropertyType::FloatProperty, "float" };
    break;
  }
  case HASH("ByteProperty"): {
    auto obj = this->Cast<UE_FByteProperty>();
    type = { PropertyType::ByteProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("BoolProperty"): {
    auto obj = this->Cast<UE_FBoolProperty>();
    type = { PropertyType::BoolProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("IntProperty"): {
    type = { PropertyType::IntProperty, "int32_t" };
    break;
  }
  case HASH("Int8Property"): {
    type = { PropertyType::Int8Property, "int8_t" };
    break;
  }
  case HASH("Int16Property"): {
    type = { PropertyType::Int16Property, "int16_t" };
    break;
  }
  case HASH("Int64Property"): {
    type = { PropertyType::Int64Property, "int64_t" };
    break;
  }
  case HASH("UInt16Property"): {
    type = { PropertyType::UInt16Property, "uint16_t" };
    break;
  }
  case HASH("UInt32Property"): {
    type = { PropertyType::UInt32Property, "uint32_t" };
    break;
  }
  case HASH("UInt64Property"): {
    type = { PropertyType::UInt64Property, "uint64_t" };
    break;
  }
  case HASH("NameProperty"): {
    type = { PropertyType::NameProperty, "struct FName" };
    break;
  }
  case HASH("DelegateProperty"): {
    type = { PropertyType::DelegateProperty, "struct FDelegate" };
    break;
  }
  case HASH("SetProperty"): {
    auto obj = this->Cast<UE_FSetProperty>();
    type = { PropertyType::SetProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("ArrayProperty"): {
    auto obj = this->Cast<UE_FArrayProperty>();
    type = { PropertyType::ArrayProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("WeakObjectProperty"): {
    auto obj = this->Cast<UE_FStructProperty>();
    type = { PropertyType::WeakObjectProperty, "struct TWeakObjectPtr<" + obj.GetTypeStr() + ">" };

    break;
  }
  case HASH("StrProperty"): {
    type = { PropertyType::StrProperty, "struct FString" };
    break;
  }
  case HASH("TextProperty"): {
    type = { PropertyType::TextProperty, "struct FText" };
    break;
  }
  case HASH("MulticastSparseDelegateProperty"): {
    type = { PropertyType::MulticastSparseDelegateProperty, "struct FMulticastSparseDelegate" };
    break;
  }
  case HASH("EnumProperty"): {
    auto obj = this->Cast<UE_FEnumProperty>();
    type = { PropertyType::EnumProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("DoubleProperty"): {
    type = { PropertyType::DoubleProperty, "double" };
    break;
  }
  case HASH("MulticastDelegateProperty"): {
    type = { PropertyType::MulticastDelegateProperty, "FMulticastDelegate" };
    break;
  }
  case HASH("ClassProperty"): {
    auto obj = this->Cast<UE_FClassProperty>();
    type = { PropertyType::ClassProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("MulticastInlineDelegateProperty"): {
    type = { PropertyType::MulticastDelegateProperty, "struct FMulticastInlineDelegate" };
    break;
  }
  case HASH("MapProperty"): {
    auto obj = this->Cast<UE_FMapProperty>();
    type = { PropertyType::MapProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("InterfaceProperty"): {
    auto obj = this->Cast<UE_FInterfaceProperty>();
    type = { PropertyType::InterfaceProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("FieldPathProperty"): {
    auto obj = this->Cast<UE_FFieldPathProperty>();
    type = { PropertyType::FieldPathProperty, obj.GetTypeStr() };
    break;
  }
  case HASH("SoftClassProperty"): {
    type = { PropertyType::SoftClassProperty, "struct TSoftClassPtr<UObject>" };
    break;
  }
  }

  return type;
}

IFProperty UE_FProperty::GetInterface() const { return IFProperty(this); }

UE_UStruct UE_FStructProperty::GetStruct() const {
  return Read<UE_UStruct>(object + offsets.FProperty.Size);
}

std::string UE_FStructProperty::GetTypeStr() const {
  return "struct " + GetStruct().GetCppName();
}

UE_UClass UE_FObjectPropertyBase::GetPropertyClass() const {
  return Read<UE_UClass>(object + offsets.FProperty.Size);
}

std::string UE_FObjectPropertyBase::GetTypeStr() const {
  return "struct " + GetPropertyClass().GetCppName() + "*";
}

UE_FProperty UE_FArrayProperty::GetInner() const {
  return Read<UE_FProperty>(object + offsets.FProperty.Size);
}

std::string UE_FArrayProperty::GetTypeStr() const {
  return "struct TArray<" + GetInner().GetType().second + ">";
}

UE_UEnum UE_FByteProperty::GetEnum() const {
  return Read<UE_UEnum>(object + offsets.FProperty.Size);
}

std::string UE_FByteProperty::GetTypeStr() const {
  auto e = GetEnum();
  if (e) return "enum class " + e.GetName();
  return "char";
}

uint8 UE_FBoolProperty::GetFieldMask() const {
  return Read<uint8>(object + offsets.FProperty.Size + 3);
}

std::string UE_FBoolProperty::GetTypeStr() const {
  if (GetFieldMask() == 0xFF) {
    return "bool";
  };
  return "char";
}

UE_UClass UE_FEnumProperty::GetEnum() const {
  return Read<UE_UClass>(object + offsets.FProperty.Size + 8);
}

std::string UE_FEnumProperty::GetTypeStr() const {
  return "enum class " + GetEnum().GetName();
}

UE_UClass UE_FClassProperty::GetMetaClass() const {
  return Read<UE_UClass>(object + offsets.FProperty.Size + 8);
}

std::string UE_FClassProperty::GetTypeStr() const {
  return "struct " + GetMetaClass().GetCppName() + "*";
}

UE_FProperty UE_FSetProperty::GetElementProp() const {
  return Read<UE_FProperty>(object + offsets.FProperty.Size);
}

std::string UE_FSetProperty::GetTypeStr() const {
  return "struct TSet<" + GetElementProp().GetType().second + ">";
}

UE_FProperty UE_FMapProperty::GetKeyProp() const {
  return Read<UE_FProperty>(object + offsets.FProperty.Size);
}

UE_FProperty UE_FMapProperty::GetValueProp() const {
  return Read<UE_FProperty>(object + offsets.FProperty.Size + 8);
}

std::string UE_FMapProperty::GetTypeStr() const {
  return fmt::format("struct TMap<{}, {}>", GetKeyProp().GetType().second, GetValueProp().GetType().second);
}

UE_UClass UE_FInterfaceProperty::GetInterfaceClass() const {
  return Read<UE_UClass>(object + offsets.FProperty.Size);
}

std::string UE_FInterfaceProperty::GetTypeStr() const {
  return "struct TScriptInterface<I" + GetInterfaceClass().GetName() + ">";
}

UE_FName UE_FFieldPathProperty::GetPropertyName() const {
  return Read<UE_FName>(object + offsets.FProperty.Size);
}

std::string UE_FFieldPathProperty::GetTypeStr() const {
  return "struct TFieldPath<F" + GetPropertyName().GetName() + ">";
}

void UE_UPackage::GenerateBitPadding(std::vector<Member>& members, uint32 offset, uint8 bitOffset, uint8 size) {
  Member padding;
  padding.Type = "char";
  padding.Name = fmt::format("pad_{:0X}_{} : {}", offset, bitOffset, size);
  padding.Offset = offset;
  padding.Size = 1;
  members.push_back(padding);
}

void UE_UPackage::GeneratePadding(std::vector<Member>& members, uint32 offset, uint32 size) {
  Member padding;
  padding.Type = "char";
  padding.Name = fmt::format("pad_{:0X}[{:#0x}]", offset, size);
  padding.Offset = offset;
  padding.Size = size;
  members.push_back(padding);
}

void UE_UPackage::FillPadding(UE_UStruct object, std::vector<Member>& members, uint32& offset, uint8& bitOffset, uint32 end, bool findPointers) {
  if (bitOffset && bitOffset < 8) {
    UE_UPackage::GenerateBitPadding(members, offset, bitOffset, 8 - bitOffset);
    bitOffset = 0;
    offset++;
  }

  auto size = end - offset;
  if (findPointers && size >= 8) {

    auto normalizedOffset = (offset + 7) & ~7;

    if (normalizedOffset != offset) {
      auto diff = normalizedOffset - offset;
      GeneratePadding(members, offset, diff);
      offset += diff;
    }

    auto normalizedSize = size - size % 8;

    auto num = normalizedSize / 8;

    uint64* pointers = new uint64[num * 2]();
    uint64* buffer = pointers + num;

    uint32 found = 0;
    auto callback = [&](UE_UObject object) {

      auto address = (uint64*)((uint64)object.GetAddress() + offset);

      Read(address, buffer, normalizedSize);

      for (uint32 i = 0; i < num; i++) {

        if (pointers[i]) continue;

        auto ptr = buffer[i];
        if (!ptr) continue;

        uint64 vftable;
        if (Read((void*)ptr, &vftable, 8)) {
          pointers[i] = ptr;
        }
        else {
          pointers[i] = (uint64)-1;
        }

        found++;
      }

      if (found == num) return true;

      return false;

    };

    ObjObjects.ForEachObjectOfClass((UE_UClass)object, callback);

    auto start = offset;
    for (uint32 i = 0; i < num; i++) {
      auto ptr = pointers[i];
      if (ptr && ptr != (uint64)-1) {

        auto ptrObject = UE_UObject((void*)ptr);

        auto ptrOffset = start + i * 8;
        if (ptrOffset > offset) {
          GeneratePadding(members, offset, ptrOffset - offset);
          offset = ptrOffset;
        }

        Member m;
        m.Offset = offset;
        m.Size = 8;

        if (ptrObject.IsA<UE_UObject>()) {
          m.Type = "struct " + ptrObject.GetClass().GetCppName() + "*";
          m.Name = ptrObject.GetName();
        }
        else {
          m.Type = "void*";
          m.Name = fmt::format("ptr_{:x}", ptr);
        }


        members.push_back(m);

        offset += 8;
      }
    }
    delete[] pointers;

  }


  if (offset != end) {
    GeneratePadding(members, offset, end - offset);
    offset = end;
  }
}

void UE_UPackage::GenerateFunction(UE_UFunction fn, Function *out) {
  out->FullName = fn.GetFullName();
  out->Flags = fn.GetFunctionFlags();
  out->Func = fn.GetFunc();

  auto generateParam = [&](IProperty *prop) {
    auto flags = prop->GetPropertyFlags();
    // if property has 'ReturnParm' flag
    if (flags & 0x400) {
      out->CppName = prop->GetType().second + " " + fn.GetName();
    }
    // if property has 'Parm' flag
    else if (flags & 0x80) {
      if (prop->GetArrayDim() > 1) {
        out->Params += fmt::format("{}* {}, ", prop->GetType().second, prop->GetName());
      } else {
        if (flags & 0x100) {
          out->Params += fmt::format("{}& {}, ", prop->GetType().second, prop->GetName());
        }
        else {
          out->Params += fmt::format("{} {}, ", prop->GetType().second, prop->GetName());
        }
       
      }
    }
  };

  for (auto prop = fn.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>()) {
    auto propInterface = prop.GetInterface();
    generateParam(&propInterface);
  }
  for (auto prop = fn.GetChildren().Cast<UE_UProperty>(); prop; prop = prop.GetNext().Cast<UE_UProperty>()) {
    auto propInterface = prop.GetInterface();
    generateParam(&propInterface);
  }
  if (out->Params.size()) {
    out->Params.erase(out->Params.size() - 2);
  }

  if (out->CppName.size() == 0) {
    out->CppName = "void " + fn.GetName();
  }
}

void UE_UPackage::GenerateStruct(UE_UStruct object, std::vector<Struct>& arr, bool findPointers) {
  Struct s;
  s.Size = object.GetSize();
  if (s.Size == 0) {
    return;
  }
  s.Inherited = 0;
  s.FullName = object.GetFullName();
  s.CppName = "struct " + object.GetCppName();

  auto super = object.GetSuper();
  if (super) {
    s.CppName += " : " + super.GetCppName();
    s.Inherited = super.GetSize();
  }

  uint32 offset = s.Inherited;
  uint8 bitOffset = 0;

  auto generateMember = [&](IProperty *prop, Member *m) {
    auto arrDim = prop->GetArrayDim();
    m->Size = prop->GetSize() * arrDim;
    if (m->Size == 0) {
      return;
    } // this shouldn't be zero

    auto type = prop->GetType();
    m->Type = type.second;
    m->Name = prop->GetName();
    m->Offset = prop->GetOffset();

    if (m->Offset > offset) {
      UE_UPackage::FillPadding(object, s.Members, offset, bitOffset, m->Offset, findPointers);
    }
    if (type.first == PropertyType::BoolProperty && *(uint32*)type.second.data() != 'loob') {
      auto boolProp = prop;
      auto mask = boolProp->GetFieldMask();
      uint8 zeros = 0, ones = 0;
      while (mask & ~1) {
        mask >>= 1;
        zeros++;
      }
      while (mask & 1) {
        mask >>= 1;
        ones++;
      }
      if (zeros > bitOffset) {
        UE_UPackage::GenerateBitPadding(s.Members, offset, bitOffset, zeros - bitOffset);
        bitOffset = zeros;
      }
      m->Name += fmt::format(" : {}", ones);
      bitOffset += ones;

      if (bitOffset == 8) {
        offset++;
        bitOffset = 0;
      }

    } else {
      if (arrDim > 1) {
        m->Name += fmt::format("[{:#0x}]", arrDim);
      }

      offset += m->Size;
    }
  };

  for (auto prop = object.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>()) {
    Member m;
    auto propInterface = prop.GetInterface();
    generateMember(&propInterface, &m);
    s.Members.push_back(m);
  }

  for (auto child = object.GetChildren(); child; child = child.GetNext()) {
    if (child.IsA<UE_UFunction>()) {
      auto fn = child.Cast<UE_UFunction>();
      Function f;
      GenerateFunction(fn, &f);
      s.Functions.push_back(f);
    } else if (child.IsA<UE_UProperty>()) {
      auto prop = child.Cast<UE_UProperty>();
      Member m;
      auto propInterface = prop.GetInterface();
      generateMember(&propInterface, &m);
      s.Members.push_back(m);
    }
  }

  if (s.Size > offset) {
    UE_UPackage::FillPadding(object, s.Members, offset, bitOffset, s.Size, findPointers);
  }

  arr.push_back(s);
}

void UE_UPackage::GenerateEnum(UE_UEnum object, std::vector<Enum> &arr) {
  Enum e;
  e.FullName = object.GetFullName();
 
  auto names = object.GetNames();
  
  uint64 max = 0;
  uint64 nameSize = ((offsets.FName.Number + 4) + 7) & ~(7);
  uint64 pairSize = nameSize + 8;

  for (uint32 i = 0; i < names.Count; i++) {

    auto pair = names.Data + i * pairSize;
    auto name = UE_FName(pair);
    auto str = name.GetName();
    auto pos = str.find_last_of(':');
    if (pos != std::string::npos) {
      str = str.substr(pos + 1);
    }

    auto value = Read<int64>(pair + nameSize);
    if ((uint64)value > max) max = value;

    str.append(" = ").append(fmt::format("{}", value));
    e.Members.push_back(str);
  }

  const char* type = nullptr;

  // I didn't see int16 yet, so I assume the engine generates only int32 and uint8:
  if (max > 256) {
    type = " : int32"; // I assume if enum has a negative value it is int32
  }
  else {
    type = " : uint8";
  }

  e.CppName = "enum class " + object.GetName() + type;

  if (e.Members.size()) {
    arr.push_back(e);
  }
}

void UE_UPackage::SaveStruct(std::vector<Struct> &arr, FILE *file) {
  for (auto &s : arr) {
    fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{",  s.FullName, s.Size, s.Inherited, s.CppName);
    for (auto &m : s.Members) {
      fmt::print(file, "\n\t{} {}; // {:#04x}({:#04x})", m.Type, m.Name, m.Offset, m.Size);
    }
    if (s.Functions.size()) {
      fwrite("\n", 1, 1, file);
      for (auto &f : s.Functions) {
        fmt::print(file, "\n\t{}({}); // {} // ({}) // @ game+{:#08x}", f.CppName, f.Params, f.FullName, f.Flags, f.Func - Base);
      }
    }
    fmt::print(file, "\n}};\n\n");
  }
}

void UE_UPackage::SaveStructSpacing(std::vector<Struct> &arr, FILE *file) {
  for (auto &s : arr) {
    fmt::print(file, "// {}\n// Size: {:#04x} (Inherited: {:#04x})\n{} {{", s.FullName, s.Size, s.Inherited, s.CppName);
    for (auto &m : s.Members) {
      fmt::print(file, "\n\t{:69} {:60} // {:#04x}({:#04x})", m.Type, m.Name + ";", m.Offset, m.Size);
    }
    if (s.Functions.size()) {
      fwrite("\n", 1, 1, file);
      for (auto &f : s.Functions) {
        fmt::print(file, "\n\t{:130} // {} // ({}) // @ game+{:#08x}", fmt::format("{}({});", f.CppName, f.Params), f.FullName, f.Flags, f.Func - Base);
      }
    }

    fmt::print(file, "\n}};\n\n");
  }
}

void UE_UPackage::SaveEnum(std::vector<Enum> &arr, FILE *file) {
  for (auto &e : arr) {
    fmt::print(file, "// {}\n{} {{", e.FullName, e.CppName);

    auto lastIdx = e.Members.size() - 1;
    for (auto i = 0; i < lastIdx; i++) {
      auto& m = e.Members.at(i);
      fmt::print(file, "\n\t{},", m);
    }

    auto& m = e.Members.at(lastIdx);
    fmt::print(file, "\n\t{}", m);

    fmt::print(file, "\n}};\n\n");
  }
}

void UE_UPackage::Process() {
  auto &objects = Package->second;
  for (auto &object : objects) {
    if (object.IsA<UE_UClass>()) {
      GenerateStruct(object.Cast<UE_UStruct>(), Classes, FindPointers);
    } else if (object.IsA<UE_UScriptStruct>()) {
      GenerateStruct(object.Cast<UE_UStruct>(), Structures, false);
    } else if (object.IsA<UE_UEnum>()) {
      GenerateEnum(object.Cast<UE_UEnum>(), Enums);
    }
  }
}

bool UE_UPackage::Save(const fs::path &dir, bool spacing) {
  if (!(Classes.size() || Structures.size() || Enums.size())) {
    return false;
  }

  std::string packageName = GetObject().GetName();

  char chars[] = "/\\:*?\"<>|";
  for (auto c : chars) {
    auto pos = packageName.find(c);
    if (pos != std::string::npos) {
      packageName[pos] = '_';
    }
  }

  if (Classes.size()) {
    File file(dir / (packageName + "_classes.h"), "w");
    if (!file) {
      return false;
    }
    if (spacing) {
      UE_UPackage::SaveStructSpacing(Classes, file);
    } else {
      UE_UPackage::SaveStruct(Classes, file);
    }
  }

  if (Structures.size() || Enums.size()) {
    File file(dir / (packageName + "_struct.h"), "w");
    if (!file) {
      return false;
    }

    if (Enums.size()) {
      UE_UPackage::SaveEnum(Enums, file);
    }

    if (Structures.size()) {
      if (spacing) {
        UE_UPackage::SaveStructSpacing(Structures, file);
      } else {
        UE_UPackage::SaveStruct(Structures, file);
      }
    }
  }

  return true;
}

UE_UObject UE_UPackage::GetObject() const { return UE_UObject(Package->first); }


