#include <Windows.h>
#include "memory.h"

HANDLE hProcess;
uint64 Base;

bool Read(void *address, void *buffer, uint64 size) {
  return ReadProcessMemory(hProcess, address, buffer, size, nullptr);
}

bool ReaderInit(uint32 pid) {
  hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  return hProcess != nullptr;
}