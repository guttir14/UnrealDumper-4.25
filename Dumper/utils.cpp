#include "utils.h"

uint32_t GetProcessIdByName(wchar_t* name)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return false;
	PROCESSENTRY32W entry = { sizeof(entry) };
    uint32_t pid = 0;
	while (Process32NextW(snapshot, &entry)) { if (wcscmp(entry.szExeFile, name) == 0) { pid = entry.th32ProcessID; break; } }
	CloseHandle(snapshot);
	return pid;
}

bool GetProcessModule(uint32_t pid, wchar_t* modName, MODULEENTRY32W& mod)
{
    bool status = false;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
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

bool CompareByteArray(byte* data, byte* sig, size_t size) 
{ 
    for (size_t i = 0; i < size; i++) 
    { 
        if (data[i] != sig[i]) 
        { 
            if (sig[i] == 0x00) { continue; }
            return false;
        }
    } 
    return true; 
}

byte* FindSignature(byte* start, byte* end, byte* sig, size_t size) 
{ 
    for (byte* it = start; it < end - size; it++) 
    { 
        if (CompareByteArray(it, sig, size)) 
        { 
            return it; 
        }; 
    } 
    return 0;
}

void* FindPointer(byte* start, byte* end, byte* sig, size_t size, int32_t addition)
{
    byte* address = FindSignature(start, end, sig, size);
    if (!address) return nullptr;
    int32_t k = 0;
    for (; sig[k]; k++);
    int32_t offset = *reinterpret_cast<int32_t*>(address + k);
    return address + k + 4 + offset + addition;
}

byte* GetExSection(byte* data)
{
    PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(data);
    PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(data + dos->e_lfanew);
    uint32_t numSec = nt->FileHeader.NumberOfSections;
    auto section = IMAGE_FIRST_SECTION(nt);
    for (auto i = 0u; i < numSec; i++, section++) { if (section->Characteristics & IMAGE_SCN_CNT_CODE) { return data + section->PointerToRawData; } }
    return nullptr;
}
