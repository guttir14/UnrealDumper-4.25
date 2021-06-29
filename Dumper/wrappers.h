#pragma once
#include "generic.h"
#include <filesystem>
#undef GetObject

namespace fs = std::filesystem;

// Wrapper for 'FILE*' that closes the file handle when it goes out of scope
class File {
private:
  FILE* file;

public:
  File(fs::path path, const char *mode) {
    fopen_s(&file, path.string().c_str(), mode);
  }
  ~File() {
    if (file) {
      fclose(file);
    }
  }
  operator bool() const { return file != nullptr; }
  operator FILE *() { return file; }
};

// Wrapper for array unit in global names array
class UE_FNameEntry {
protected:
  uint8 *object;

public:
  UE_FNameEntry(uint8 *object) : object(object) {}
  UE_FNameEntry() : object(nullptr) {}
  operator bool() { return object != nullptr; }
  // Gets info about contained string (bool wide, uint16 len) depending on
  // 'offsets.FNameEntry' info
  std::pair<bool, uint16> Info() const;
  // Gets string out of array unit
  std::string String(bool wide, uint16 len) const;
  // Gets string out of array unit
  void String(char *buf, bool wide, uint16 len) const;
  std::string String() const;
  // Calculates the unit size depending on 'offsets.FNameEntry' and information
  // about string
  static uint16 Size(bool wide, uint16 len);
};

class UE_FName {
protected:
  uint8* object;

public:
  UE_FName(uint8 *object) : object(object) {}
  UE_FName() : object(nullptr) {}
  std::string GetName() const;
};

class UE_UClass;
class UE_FField;

enum class PropertyType {
  Unknown,
  StructProperty,
  ObjectProperty,
  SoftObjectProperty,
  FloatProperty,
  ByteProperty,
  BoolProperty,
  IntProperty,
  Int8Property,
  Int16Property,
  Int64Property,
  UInt16Property,
  UInt32Property,
  UInt64Property,
  NameProperty,
  DelegateProperty,
  SetProperty,
  ArrayProperty,
  WeakObjectProperty,
  StrProperty,
  TextProperty,
  MulticastSparseDelegateProperty,
  EnumProperty,
  DoubleProperty,
  MulticastDelegateProperty,
  ClassProperty,
  MulticastInlineDelegateProperty,
  MapProperty,
  InterfaceProperty,
  FieldPathProperty,
  SoftClassProperty
};

class UE_UObject {
protected:
  uint8* object;

public:
  UE_UObject(void* object) : object((uint8*)object) {}
  UE_UObject() : object(nullptr) {}
  bool operator==(const UE_UObject obj) const { return obj.object == object; };
  bool operator!=(const UE_UObject obj) const { return obj.object != object; };
  uint32 GetIndex() const;
  UE_UClass GetClass() const;
  UE_UObject GetOuter() const;
  UE_UObject GetPackageObject() const;
  std::string GetName() const;
  std::string GetFullName() const;
  std::string GetCppName() const;
  void* GetAddress() const { return object; }
  operator uint8*() const { return object; };
  operator bool() const { return object != nullptr; }

  template <typename Base> Base Cast() const { return Base(object); }

  template <typename T> bool IsA() const;

  bool IsA(UE_UClass cmp) const;

  static UE_UClass StaticClass();
};

class UE_AActor : public UE_UObject {
public:
  static UE_UClass StaticClass();
};

class UE_UField : public UE_UObject {
public:
  using UE_UObject::UE_UObject;
  UE_UField GetNext() const;
  static UE_UClass StaticClass();
};

typedef std::pair<PropertyType, std::string> type;

class IProperty {
protected:
  const void* prop;

public:
  IProperty(const void *object) : prop(object) {}
  virtual std::string GetName() const = 0;
  virtual int32 GetArrayDim() const = 0;
  virtual int32 GetSize() const = 0;
  virtual int32 GetOffset() const = 0;
  virtual uint64 GetPropertyFlags() const = 0;
  virtual type GetType() const = 0;
  virtual uint8 GetFieldMask() const = 0;
};

class IUProperty : public IProperty {
public:
  using IProperty::IProperty;
  IUProperty(const class UE_UProperty *object) : IProperty(object) {}
  virtual std::string GetName() const;
  virtual int32 GetArrayDim() const;
  virtual int32 GetSize() const;
  virtual int32 GetOffset() const;
  virtual uint64 GetPropertyFlags() const;
  virtual type GetType() const;
  virtual uint8 GetFieldMask() const;
};

class UE_UProperty : public UE_UField {
public:
  using UE_UField::UE_UField;
  int32 GetArrayDim() const;
  int32 GetSize() const;
  int32 GetOffset() const;
  uint64 GetPropertyFlags() const;
  type GetType() const;

  IUProperty GetInterface() const;
  static UE_UClass StaticClass();
};

class UE_UStruct : public UE_UField {
public:
  using UE_UField::UE_UField;
  UE_UStruct GetSuper() const;
  UE_FField GetChildProperties() const;
  UE_UField GetChildren() const;
  int32 GetSize() const;
  static UE_UClass StaticClass();
};

enum EFunctionFlags : uint32
{
	// Function flags.
	FUNC_None = 0x00000000,
	FUNC_Final = 0x00000001,	// Function is final (prebindable, non-overridable function).
	FUNC_RequiredAPI = 0x00000002,	// Indicates this function is DLL exported/imported.
	FUNC_BlueprintAuthorityOnly = 0x00000004,   // Function will only run if the object has network authority
	FUNC_BlueprintCosmetic = 0x00000008,   // Function is cosmetic in nature and should not be invoked on dedicated servers
	// FUNC_				= 0x00000010,   // unused.
	// FUNC_				= 0x00000020,   // unused.
	FUNC_Net = 0x00000040,   // Function is network-replicated.
	FUNC_NetReliable = 0x00000080,   // Function should be sent reliably on the network.
	FUNC_NetRequest = 0x00000100,	// Function is sent to a net service
	FUNC_Exec = 0x00000200,	// Executable from command line.
	FUNC_Native = 0x00000400,	// Native function.
	FUNC_Event = 0x00000800,   // Event function.
	FUNC_NetResponse = 0x00001000,   // Function response from a net service
	FUNC_Static = 0x00002000,   // Static function.
	FUNC_NetMulticast = 0x00004000,	// Function is networked multicast Server -> All Clients
	FUNC_UbergraphFunction = 0x00008000,   // Function is used as the merge 'ubergraph' for a blueprint, only assigned when using the persistent 'ubergraph' frame
	FUNC_MulticastDelegate = 0x00010000,	// Function is a multi-cast delegate signature (also requires FUNC_Delegate to be set!)
	FUNC_Public = 0x00020000,	// Function is accessible in all classes (if overridden, parameters must remain unchanged).
	FUNC_Private = 0x00040000,	// Function is accessible only in the class it is defined in (cannot be overridden, but function name may be reused in subclasses.  IOW: if overridden, parameters don't need to match, and Super.Func() cannot be accessed since it's private.)
	FUNC_Protected = 0x00080000,	// Function is accessible only in the class it is defined in and subclasses (if overridden, parameters much remain unchanged).
	FUNC_Delegate = 0x00100000,	// Function is delegate signature (either single-cast or multi-cast, depending on whether FUNC_MulticastDelegate is set.)
	FUNC_NetServer = 0x00200000,	// Function is executed on servers (set by replication code if passes check)
	FUNC_HasOutParms = 0x00400000,	// function has out (pass by reference) parameters
	FUNC_HasDefaults = 0x00800000,	// function has structs that contain defaults
	FUNC_NetClient = 0x01000000,	// function is executed on clients
	FUNC_DLLImport = 0x02000000,	// function is imported from a DLL
	FUNC_BlueprintCallable = 0x04000000,	// function can be called from blueprint code
	FUNC_BlueprintEvent = 0x08000000,	// function can be overridden/implemented from a blueprint
	FUNC_BlueprintPure = 0x10000000,	// function can be called from blueprint code, and is also pure (produces no side effects). If you set this, you should set FUNC_BlueprintCallable as well.
	FUNC_EditorOnly = 0x20000000,	// function can only be called from an editor scrippt.
	FUNC_Const = 0x40000000,	// function can be called from blueprint code, and only reads state (never writes state)
	FUNC_NetValidate = 0x80000000,	// function must supply a _Validate implementation
	FUNC_AllFlags = 0xFFFFFFFF,
};

class UE_UFunction : public UE_UStruct {
public:
  using UE_UStruct::UE_UStruct;
  uint64 GetFunc() const;
  std::string GetFunctionFlags() const;
  static UE_UClass StaticClass();
};

class UE_UScriptStruct : public UE_UStruct {
public:
  using UE_UStruct::UE_UStruct;
  static UE_UClass StaticClass();
};

class UE_UClass : public UE_UStruct {
public:
  using UE_UStruct::UE_UStruct;
  static UE_UClass StaticClass();
};

class UE_UEnum : public UE_UField {
public:
  using UE_UField::UE_UField;
  TArray GetNames() const;
  static UE_UClass StaticClass();
};

class UE_UDoubleProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UFloatProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UIntProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UInt16Property : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UInt64Property : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UInt8Property : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UUInt16Property : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UUInt32Property : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UUInt64Property : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UTextProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UStrProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UStructProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  UE_UStruct GetStruct() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UNameProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UObjectPropertyBase : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  UE_UClass GetPropertyClass() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UArrayProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  UE_UProperty GetInner() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UByteProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  UE_UEnum GetEnum() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UBoolProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  uint8 GetFieldMask() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UEnumProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  UE_UClass GetEnum() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UClassProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  UE_UClass GetMetaClass() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_USetProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  UE_UProperty GetElementProp() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UMapProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  UE_UProperty GetKeyProp() const;
  UE_UProperty GetValueProp() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UInterfaceProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  UE_UProperty GetInterfaceClass() const;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UMulticastDelegateProperty : public UE_UProperty {
public:
  using UE_UProperty::UE_UProperty;
  std::string GetTypeStr() const;
  static UE_UClass StaticClass();
};

class UE_UWeakObjectProperty : public UE_UProperty {
  public:
    using UE_UProperty::UE_UProperty;
    std::string GetTypeStr() const;
    static UE_UClass StaticClass();
};

class UE_FFieldClass {
protected:
  uint8 *object;

public:
  UE_FFieldClass(uint8 *object) : object(object){};
  UE_FFieldClass() : object(nullptr){};
  std::string GetName() const;
};

class UE_FField {
protected:
  uint8 *object;

public:
  UE_FField(uint8 *object) : object(object) {}
  UE_FField() : object(nullptr) {}
  operator bool() const { return object != nullptr; }
  UE_FField GetNext() const;
  std::string GetName() const;

  template <typename Base> Base Cast() const { return Base(object); }
};

class IFProperty : public IProperty {
public:
  IFProperty(const class UE_FProperty *object) : IProperty(object) {}
  virtual std::string GetName() const;
  virtual int32 GetArrayDim() const;
  virtual int32 GetSize() const;
  virtual int32 GetOffset() const;
  virtual uint64 GetPropertyFlags() const;
  virtual type GetType() const;
  virtual uint8 GetFieldMask() const;
};



class UE_FProperty : public UE_FField {
public:
  using UE_FField::UE_FField;
  int32 GetArrayDim() const;
  int32 GetSize() const;
  int32 GetOffset() const;
  uint64 GetPropertyFlags() const;
  type GetType() const;
  IFProperty GetInterface() const;
};

class UE_FStructProperty : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  UE_UStruct GetStruct() const;
  std::string GetTypeStr() const;
};

class UE_FObjectPropertyBase : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  UE_UClass GetPropertyClass() const;
  std::string GetTypeStr() const;
};

class UE_FArrayProperty : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  UE_FProperty GetInner() const;
  std::string GetTypeStr() const;
};

class UE_FByteProperty : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  UE_UEnum GetEnum() const;
  std::string GetTypeStr() const;
};

class UE_FBoolProperty : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  uint8 GetFieldMask() const;
  std::string GetTypeStr() const;
};

class UE_FEnumProperty : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  UE_UClass GetEnum() const;
  std::string GetTypeStr() const;
};

class UE_FClassProperty : public UE_FObjectPropertyBase {
public:
  using UE_FObjectPropertyBase::UE_FObjectPropertyBase;
  UE_UClass GetMetaClass() const;
  std::string GetTypeStr() const;
};

class UE_FSetProperty : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  UE_FProperty GetElementProp() const;
  std::string GetTypeStr() const;
};

class UE_FMapProperty : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  UE_FProperty GetKeyProp() const;
  UE_FProperty GetValueProp() const;
  std::string GetTypeStr() const;
};

class UE_FInterfaceProperty : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  UE_UClass GetInterfaceClass() const;
  std::string GetTypeStr() const;
};

class UE_FFieldPathProperty : public UE_FProperty {
public:
  using UE_FProperty::UE_FProperty;
  UE_FName GetPropertyName() const;
  std::string GetTypeStr() const;
};

template <typename T> bool UE_UObject::IsA() const {
  auto cmp = T::StaticClass();
  if (!cmp) {
    return false;
  }

  return IsA(cmp);
}

class UE_UPackage {
private:
  struct Member {
    std::string Type;
    std::string Name;
    uint32 Offset = 0;
    uint32 Size = 0;
  };
  struct Function {
    std::string FullName;
    std::string CppName;
    std::string Params;
    std::string Flags;
    uint64 Func = 0;
  };
  struct Struct {
    std::string FullName;
    std::string CppName;
    uint32 Inherited = 0;
    uint32 Size = 0;
    std::vector<Member> Members;
    std::vector<Function> Functions;
  };
  struct Enum {
    std::string FullName;
    std::string CppName;
    std::vector<std::string> Members;
  };

private:
  std::pair<uint8 *const, std::vector<UE_UObject>> *Package;
  std::vector<Struct> Classes;
  std::vector<Struct> Structures;
  std::vector<Enum> Enums;
public:
  bool FindPointers = false;


private:
  static void GenerateBitPadding(std::vector<Member>& members, uint32 offset, uint8 bitOffset, uint8 size);
  static void GeneratePadding(std::vector<Member>& members, uint32 offset, uint32 size);
  static void FillPadding(UE_UStruct object, std::vector<Member>& members, uint32& offset, uint8& bitOffset, uint32 end, bool findPointers);
  static void GenerateFunction(UE_UFunction fn, Function* out);
  static void GenerateStruct(UE_UStruct object, std::vector<Struct>& arr, bool findPointers);
  static void GenerateEnum(UE_UEnum object, std::vector<Enum>& arr);
  static void SaveStruct(std::vector<Struct> &arr, FILE *file);
  static void SaveStructSpacing(std::vector<Struct> &arr, FILE* file); // save struct with spacing to members applied
  static void SaveEnum(std::vector<Enum> &arr, FILE* file);
public:
  UE_UPackage(std::pair<uint8* const, std::vector<UE_UObject>>& package) : Package(&package){};
  void Process();
  bool Save(const fs::path& dir, bool spacing);
  UE_UObject GetObject() const;
};
