#pragma once
#include "defs.h"
#include <string>
#include <vector>

struct Offsets {
  uint16 Stride = 0; // alignof(FNameEntry)
  struct {
    uint16 Size = 0;
  } FUObjectItem;
  struct {
    uint16 Number = 0;
  } FName;
  struct {
    uint16 Info =
        0; // Offset to Memory filled with info about type and size of string
    uint16 WideBit =
        0; // Offset to bit which shows if string uses wide characters
    uint16 LenBit = 0; // Offset to bit which has lenght of string
    uint16 HeaderSize =
        0; // Size of FNameEntry header (offset where a string begins)
  } FNameEntry;
  struct {
    uint16 Index = 0; // Offset to index of this object in all objects array
    uint16 Class = 0; // Offset to UClass pointer (UClass* ClassPrivate)
    uint16 Name = 0;  // Offset to FName structure
    uint16 Outer = 0; // (UObject* OuterPrivate)
  } UObject;
  struct {
    uint16 Next = 0;
  } UField;
  struct {
    uint16 SuperStruct = 0;
    uint16 Children = 0;
    uint16 ChildProperties = 0;
    uint16 PropertiesSize = 0;
  } UStruct;
  struct {
    uint16 Names = 0;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0;
    uint16 Func = 0; // ue3-ue4, always +0x28 from flags location.
  } UFunction;
  struct {
    uint16 Class = 0;
    uint16 Next = 0;
    uint16 Name = 0;
  } FField;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(FProperty)
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
};

extern Offsets offsets;

typedef int64(_fastcall* ansi_fn)(char* a1, int a2); // buf, len
// typedef int64(_fastcall*wide_fn)(wchar_t* a1, int a2);

extern ansi_fn Decrypt_ANSI;
// extern wide_fn Decrypt_WIDE;

STATUS EngineInit(std::string game, void* image);
