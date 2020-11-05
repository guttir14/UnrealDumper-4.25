#include "utils.h"


DWORD GetProcessIdByName(wchar_t* name)
{
	PVOID snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return false;
	PROCESSENTRY32W entry = { sizeof(entry) };
	DWORD pid = 0;
	while (Process32NextW(snapshot, &entry)) { if (wcscmp(entry.szExeFile, name) == 0) { pid = entry.th32ProcessID; break; } }
	CloseHandle(snapshot);
	return pid;
}

bool GetProcessModule(DWORD pid, wchar_t* modName, MODULEENTRY32W& mod)
{
    bool status = false;
    PVOID snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snapshot == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32W modEntry = { sizeof(MODULEENTRY32W) };
    while (Module32NextW(snapshot, &modEntry)) {
        if (wcscmp(modEntry.szModule, modName) == 0) {
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