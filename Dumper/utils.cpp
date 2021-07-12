#include <windows.h>
#include <TlHelp32.h>
#include <psapi.h>
#include "utils.h"

uint32 GetProcessId(std::wstring name) {
  uint32 pid = 0;
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot != INVALID_HANDLE_VALUE) {
    PROCESSENTRY32W entry = {sizeof(entry)};
    while (Process32NextW(snapshot, &entry)) {
      if (name == entry.szExeFile) {
        pid = entry.th32ProcessID;
        break;
      }
    }
    CloseHandle(snapshot);
  }
  return pid;
}

std::pair<uint8*, uint32> GetModuleInfo(uint32 pid, std::wstring name) {
  std::pair<uint8*, uint32> info;
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
  if (snapshot != INVALID_HANDLE_VALUE) {
    MODULEENTRY32W modEntry = {sizeof(modEntry)};
    while (Module32NextW(snapshot, &modEntry)) {
      if (name == modEntry.szModule) {
        info = {modEntry.modBaseAddr, modEntry.modBaseSize};
        break;
      }
    }
  }
  return info;
}

bool Compare(uint8* data, uint8 *sig, uint32 size) {
  for (uint32 i = 0; i < size; i++) {
    if (data[i] != sig[i] && sig[i] != 0x00) {
      return false;
    }
  }
  return true;
}

uint8* FindSignature(uint8* start, uint8* end, const char* sig, uint32 size) {
  for (uint8* it = start; it < end - size; it++) {
    if (Compare(it, (uint8*)sig, size)) {
      return it;
    };
  }
  return nullptr;
}

void* FindPointer(uint8* start, uint8* end, const char* sig, uint32 size, int32 addition) {
  uint8* address = FindSignature(start, end, sig, size);
  if (!address) return nullptr;
  int32 k = 0;
  for (; sig[k]; k++);
  int32 offset = *(int32*)(address + k);
  return address + k + 4 + offset + addition;
}

std::vector<std::pair<uint8*, uint8*>> GetExSections(void* data) {
  std::vector<std::pair<uint8*, uint8*>> sections;
  auto dos = (PIMAGE_DOS_HEADER)data;
  auto nt = (PIMAGE_NT_HEADERS)((uint8*)data + dos->e_lfanew);
  auto s = IMAGE_FIRST_SECTION(nt);
  for (auto i = 0; i < nt->FileHeader.NumberOfSections; i++, s++) {
    if (s->Characteristics & IMAGE_SCN_CNT_CODE) {
      auto start = (uint8*)data + s->PointerToRawData;
      auto end = start + s->SizeOfRawData;
      sections.push_back({start, end});
    }
  }
  return sections;
}

uint32 GetProccessPath(uint32 pid, wchar_t* processName, uint32 size) {
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid);
  if (!QueryFullProcessImageNameW(hProcess, 0, processName, (DWORD*)(&size))) {
    size = 0;
  };
  CloseHandle(hProcess);
  return size;
}

extern "C" NTSTATUS NtQuerySystemTime(uint64* SystemTime);

uint64 GetTime() {
  uint64 ret;
  NtQuerySystemTime(&ret);
  return ret;
}
