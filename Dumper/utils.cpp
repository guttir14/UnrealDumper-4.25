#include "utils.h"
#include <psapi.h>

uint32_t GetProcessId(std::wstring name)
{
    uint32_t pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) 
    {
        PROCESSENTRY32W entry = { sizeof(entry) };
        while (Process32NextW(snapshot, &entry)) { if (name == entry.szExeFile) { pid = entry.th32ProcessID; break; } }
        CloseHandle(snapshot);
    }
    return pid;
}

std::pair<byte*, uint32_t> GetModuleInfo(uint32_t pid, std::wstring name)
{
    std::pair<byte*, uint32_t> info;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32W modEntry = { sizeof(modEntry) };
        while (Module32NextW(snapshot, &modEntry)) 
        { 
            if (name == modEntry.szModule) { info = { modEntry.modBaseAddr, modEntry.modBaseSize }; break; }
        }
    }
    return info;
}

bool Compare(byte* data, byte* sig, size_t size) 
{ 
    for (size_t i = 0; i < size; i++) { if (data[i] != sig[i] && sig[i] != 0x00) { return false; } }
    return true; 
}

byte* FindSignature(byte* start, byte* end, byte* sig, size_t size) 
{ 
    for (byte* it = start; it < end - size; it++) { if (Compare(it, sig, size)) { return it; }; } 
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

std::vector<std::pair<byte*, byte*>> GetExSections(byte* data)
{
    std::vector<std::pair<byte*, byte*>> sections;
    PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(data);
    PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(data + dos->e_lfanew);
    auto s = IMAGE_FIRST_SECTION(nt);
    for (auto i = 0u; i < nt->FileHeader.NumberOfSections; i++, s++) {
        if (s->Characteristics & IMAGE_SCN_CNT_CODE)
        {
            auto start = data + s->PointerToRawData;
            auto end = start + s->SizeOfRawData;
            sections.push_back({ start, end });
        }
    }
    return sections;
}

uint32_t GetProccessPath(uint32_t pid, wchar_t* processName, uint32_t size)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid);
    if (!QueryFullProcessImageNameW(hProcess, 0, processName, reinterpret_cast<DWORD*>(&size))) { size = 0; };
    CloseHandle(hProcess);
    return size;
}