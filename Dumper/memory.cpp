#include <Windows.h>
#include <winternl.h>
#include "memory.h"

HANDLE hProcess;
uint64 Base;

bool Read(void* address, void* buffer, uint64 size) {
	uint64 read;
	return ReadProcessMemory(hProcess, address, buffer, size, &read) && read == size;
}

bool ReaderInit(uint32 pid) {
	PROCESS_BASIC_INFORMATION pbi;
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
	if (!hProcess) return false;
	if (0 > NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), 0)) goto failed;
	Base = Read<uint64>((uint8*)pbi.PebBaseAddress + 0x10);
	if (!Base) goto failed;
	return true;
failed:
	CloseHandle(hProcess);
	return false;
}

uint64 GetImageSize() {
	char buffer[0x400];
	if (!Read((void*)Base, buffer, 0x400)) return 0;
	auto nt = (PIMAGE_NT_HEADERS)(buffer + ((PIMAGE_DOS_HEADER)buffer)->e_lfanew);
	return nt->OptionalHeader.SizeOfImage;
}
