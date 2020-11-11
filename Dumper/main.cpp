#include <filesystem>
#include <fmt/core.h>
#include "utils.h"
#include "Generic.h"
#include "wrappers.h"
#include "memory.h"
#include <algorithm>

namespace fs = std::filesystem;

enum {
    SUCCESS, 
    FAILED,
    PROCESS_NOT_FOUND,
    INVALID_HANDLE,
    MODULE_NOT_FOUND,
    CANNOT_READ,
    INVALID_IMAGE,
    NAMES_NOT_FOUND,
    OBJECTS_NOT_FOUND,
    FILE_NOT_OPEN,
    ZERO_PACKAGES
};



class Dumper
{
protected:
    wchar_t* processName;
    ModuleInfo processInfo;
    bool full;
    fs::path directory;
private:

    Dumper(wchar_t* processName, bool full) : processName(processName), full(full) {}

    bool FindObjObjects(byte* data) {
        static std::vector<byte> sigv[] = { {0x8b, 0x05, 0xa5, 0xb7, 0x98, 0x07, 0x48, 0x8d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x39, 0x45, 0x6f, 0x7c, 0x17, 0x48, 0x8d, 0x45, 0x6f, 0x48, 0x89, 0x5d, 0x1f, 0x48, 0x8d, 0x4d, 0x17, 0x48, 0x89, 0x45, 0x17}, {0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0C, 0xC8, 0x48, 0x8D, 0x04, 0xD1, 0xEB}, {0x48 , 0x8b , 0x0d , 0x00 , 0x00 , 0x00 , 0x00 , 0x81 , 0x4c , 0xd1 , 0x08 , 0x00 , 0x00 , 0x00 , 0x40} };
        for (auto& sig : sigv)
        {
            auto address = FindPointer(data, data + processInfo.size, sig.data(), sig.size());
            if (!address) continue;
            ObjObjects = *reinterpret_cast<decltype(ObjObjects)*>(address);
            return true;
        }
        return false;
    }
    bool FindNamePoolData(byte* data) {
        static std::vector<byte> sigv[] = { { 0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00, 0xeb, 0x16 } };
        for (auto& sig : sigv)
        {
            auto address = FindPointer(data, data + processInfo.size, sig.data(), sig.size());
            if (!address) continue;
            NamePoolData = *reinterpret_cast<decltype(NamePoolData)*>(address);
            return true;
        }
        return false;
    }
public:

    static Dumper* GetInstance(wchar_t* processName, bool full) {
        static Dumper dumper(processName, full);
        return &dumper;
    }

    int Init() {
        uint32_t pid = GetProcessIdByName(processName);
        if (!pid) { return PROCESS_NOT_FOUND; };
        
        g_Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!g_Proc) { return INVALID_HANDLE; };
        
        
        if (!GetProcessModule(pid, processName, processInfo)) { return MODULE_NOT_FOUND; };
       
        {
            wchar_t buf[MAX_PATH];
            GetModuleFileNameW(GetModuleHandleA(0), buf, MAX_PATH);
            directory = fs::path(buf).remove_filename() / "SDK" / fs::path(processName).filename().stem();
            fs::create_directories(directory);
        }
        
        
        {
            std::vector<byte> image(processInfo.size);
            if (!ReadProcessMemory(g_Proc, processInfo.base, image.data(), processInfo.size, nullptr)) { return CANNOT_READ; }
            auto ex = GetExSection(image.data());
            if (!ex) { return INVALID_IMAGE; }
            if (!FindObjObjects(ex)) { return OBJECTS_NOT_FOUND; }
            if (!FindNamePoolData(ex)) { return NAMES_NOT_FOUND; }
        }
        

        byte* address = reinterpret_cast<byte*>(ObjObjects.GetObjectPtr(1));
        int32_t number = Read<int32_t>(address + 0x1C);
        if (number != 0) { offsets.addition = 8; offsets.header = 4; offsets.stride = 4; }
        
        return SUCCESS;
    }
    int Dump() {

        /*
        * Names dumping.
        * We go through each block, except last, that is not fully filled.
        * In each block we calculate next entry depending on previous entry size.
        */
        {
            File file(directory / "NamesDump.txt");
            if (!file) { return FILE_NOT_OPEN; }
            size_t size = 0;
            NamePoolData.Dump([&file, &size](std::string_view name, int32_t id) { fmt::print(file, "[{:0>6}] {}\n", id, name); size++; });
            fmt::print("Names: {}\n", size);
        }

        
        {
            // Why we need to iterate all objects twice? We dumping objects and filling packages simultaneously.
            std::vector<std::pair<UE_UObject, std::vector<UE_UObject>>> packages;
            {
                File file(directory / "ObjectsDump.txt");
                if (!file) { return FILE_NOT_OPEN; }
                size_t size = 0;
                if (full)
                {
                    ObjObjects.Dump(
                        [&file, &size, &packages](UE_UObject object) 
                        {
                            fmt::print(file, "[{:0>6}] <{}> {}\n", object.GetIndex(), object.GetAddress(), object.GetFullName()); size++;
                            if (object.IsA<UE_UStruct>())
                            {
                                auto packageObj = object.GetPackageObject();
                                auto It = std::find_if(packages.begin(), packages.end(), [&packageObj](std::pair<UE_UObject, std::vector<UE_UObject>>& package) {return package.first == packageObj; });
                                if (It == packages.end()) { packages.push_back({ packageObj , {object} }); } else { It->second.push_back(object); }
                            }
                        }
                    );
                }
                else
                {
                    ObjObjects.Dump([&file, &size](UE_UObject object) { fmt::print(file, "[{:0>6}] <{}> {}\n", object.GetIndex(), object.GetAddress(), object.GetFullName()); size++; });
                }

                fmt::print("Objects: {}\n", size);
            }

            if (!full) { return SUCCESS; }

            // Clearing all empty packages
            packages.erase(std::remove_if(packages.begin(), packages.end(), [](std::pair<UE_UObject, std::vector<UE_UObject>>& package) { return package.second.size() < 2; }), packages.end());

            // Checking if we have any package after clearing.
            if (!packages.size()) { return ZERO_PACKAGES; }

            fmt::print("Packages: {}\n", packages.size());
            
            {
                auto path = directory / "DUMP";
                fs::create_directories(path);
                int i = 1;
                int max = packages.size();

                // array of all already 'processed' objects (it is needed to be sure we can freely inherit from it)
                std::vector<void*> processedObjects;
                for (UE_UPackage package : packages)
                {
                    fmt::print("\rProcessing: {}/{}", i, max); i++;

                    package.Process(processedObjects);
                    package.Save(path);
                }

            }
        }
        return SUCCESS;
    }
};

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 2) { puts(".\\Dumper.exe 'game.exe'"); return 1; }
    auto processName = argv[1];
    bool full = true;
    for (auto i = 2; i < argc; i++) { auto arg = argv[i]; if (!wcscmp(arg, L"-p")) { full = false; } }
    auto dumper = Dumper::GetInstance(processName, full);

    switch (dumper->Init())
    {
    case PROCESS_NOT_FOUND: { fmt::print("Process not found\n"); return FAILED;}
    case INVALID_HANDLE: { puts("Can't open process"); return FAILED; }
    case MODULE_NOT_FOUND: { puts("Can't enumerate modules"); return FAILED; }
    case CANNOT_READ: { puts("Can't read process memory"); return FAILED; }
    case INVALID_IMAGE: { puts("Can't get first executable section"); return FAILED; }
    case OBJECTS_NOT_FOUND: {puts("Can't find objects array"); return FAILED; }
    case NAMES_NOT_FOUND: {puts("Can't find names array"); return FAILED; }
    case SUCCESS: { break; };
    default: { return 1; }
    }

    switch (dumper->Dump())
    {
    case FILE_NOT_OPEN: { puts("Can't open file"); return FAILED; }
    case ZERO_PACKAGES: { puts("Size of packages is zero"); return FAILED; }
    case SUCCESS: { break; }
    default: { return FAILED; }
    }

    return SUCCESS;
}