#include "utils.h"
#include <Psapi.h>

uint32_t GetProcessIdByName(const wchar_t* name)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return false;
	PROCESSENTRY32W entry = { sizeof(entry) };
    uint32_t pid = 0;
	while (Process32NextW(snapshot, &entry)) { if (wcscmp(entry.szExeFile, name) == 0) { pid = entry.th32ProcessID; break; } }
	CloseHandle(snapshot);
	return pid;
}

uint32_t GetProcessModules(uint32_t pid, uint32_t count, const wchar_t* names[], MODULEENTRY32W mods[])
{
    uint32_t found = 0u;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snapshot == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32W modEntry = { sizeof(modEntry) };
    while (Module32NextW(snapshot, &modEntry)) {
        if (found == count) { break; }
        for (auto i = 0u; i < count; i++)
        {
            if (wcscmp(modEntry.szModule, names[i]) == 0) {
                mods[i] = modEntry;
                found++;
                break;
            }
        }
        
    }
    CloseHandle(snapshot);
    return found;
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

bool GetExSections(byte* data, std::vector<std::pair<byte*, byte*>>& sections)
{
    PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(data);
    PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(data + dos->e_lfanew);
    uint32_t numSec = nt->FileHeader.NumberOfSections;
    auto section = IMAGE_FIRST_SECTION(nt);
    for (auto i = 0u; i < numSec; i++, section++) {
        if (section->Characteristics & IMAGE_SCN_CNT_CODE)
        {
            auto start = data + section->PointerToRawData;
            auto end = start + section->SizeOfRawData;
            sections.push_back({ start, end });
        }
    }
    return sections.size() != 0;
}

uint32_t GetProccessPath(uint32_t pid, wchar_t* processName, uint32_t size)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid);
    if (!QueryFullProcessImageNameW(hProcess, 0, processName, reinterpret_cast<DWORD*>(&size))) { size = 0; };
    CloseHandle(hProcess);
    return size;
}
