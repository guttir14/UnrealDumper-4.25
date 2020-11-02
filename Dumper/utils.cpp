#include "utils.h"


DWORD GetProcessIdByName(char* name)
{
	PVOID snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return false;
	PROCESSENTRY32 entry = { sizeof(entry) };
	DWORD pid = 0;
	while (Process32Next(snapshot, &entry)) { if (strcmp(entry.szExeFile, name) == 0) { pid = entry.th32ProcessID; break; } }
	CloseHandle(snapshot);
	return pid;
}

bool GetProcessModule(DWORD pid, char* modName, MODULEENTRY32& mod)
{
    bool status = false;
    PVOID snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snapshot == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32 modEntry = { sizeof(MODULEENTRY32) };
    while (Module32Next(snapshot, &modEntry)) {
        if (strcmp(modEntry.szModule, modName) == 0) {
            mod = modEntry;
            status = true;
            break;
        }
    }
    CloseHandle(snapshot);
    return status;
}

FORCEINLINE bool CompareByteArray(BYTE* data, BYTE* sig, SIZE_T size) { for (SIZE_T i = 0; i < size; i++) { if (data[i] != sig[i]) { if (sig[i] == 0x00) continue;  return false; } } return true; }

FORCEINLINE BYTE* FindSignature(BYTE* start, BYTE* end, BYTE* sig, SIZE_T size) { for (BYTE* it = start; it < end - size; it++) { if (CompareByteArray(it, sig, size)) { return it; }; } return 0; }

void* FindPointer(BYTE* start, BYTE* end, BYTE* sig, SIZE_T size, int addition)
{
    auto address = FindSignature(start, end, sig, size);
    if (!address) return nullptr;
    auto k = 0;
    for (; sig[k]; k++);
    auto offset = *reinterpret_cast<UINT32*>(address + k);
    return address + k + 4 + offset + addition;
}