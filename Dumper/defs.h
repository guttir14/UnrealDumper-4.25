#pragma once

#ifdef GetObject
#undef GetObject
#endif

enum class STATUS {
  SUCCESS,
  FAILED,
  WINDOW_NOT_FOUND,
  PROCESS_NOT_FOUND,
  READER_ERROR,
  CANNOT_GET_PROCNAME,
  MODULE_NOT_FOUND,
  ENGINE_NOT_FOUND,
  ENGINE_FAILED,
  CANNOT_READ,
  INVALID_IMAGE,
  NAMES_NOT_FOUND,
  OBJECTS_NOT_FOUND,
  FILE_NOT_OPEN,
  ZERO_PACKAGES
};

typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
