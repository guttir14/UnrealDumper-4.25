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
    PROCESS_NOT_FOUND,
    INVALID_HANDLE,
    MODULE_NOT_FOUND,
    CANNOT_READ,
    NAMES_NOT_FOUND,
    OBJECTS_NOT_FOUND,
    FILE_NOT_OPEN,
    ZERO_PACKAGES
};

class File {
private:
    FILE* file;
public:
    File(fs::path path){ fopen_s(&file, path.string().c_str(), "w"); }
    ~File() { fclose(file); }
    operator bool() { return file != nullptr; }
    operator FILE* () { return file; }
};

class Dumper
{
protected:
    wchar_t* processName = nullptr;
    BYTE* processBase = nullptr;
    DWORD processSize = 0ul;
    fs::path directory;
private:

    Dumper(wchar_t* _processName) { processName = _processName; }

    bool FindObjObjects(BYTE* data) {
        static std::vector<BYTE> sigv[] = { {0x8b, 0x05, 0xa5, 0xb7, 0x98, 0x07, 0x48, 0x8d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x39, 0x45, 0x6f, 0x7c, 0x17, 0x48, 0x8d, 0x45, 0x6f, 0x48, 0x89, 0x5d, 0x1f, 0x48, 0x8d, 0x4d, 0x17, 0x48, 0x89, 0x45, 0x17}, {0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0C, 0xC8, 0x48, 0x8D, 0x04, 0xD1, 0xEB}, {0x48 , 0x8b , 0x0d , 0x00 , 0x00 , 0x00 , 0x00 , 0x81 , 0x4c , 0xd1 , 0x08 , 0x00 , 0x00 , 0x00 , 0x40} };
        for (auto& sig : sigv)
        {
            auto address = FindPointer(data + 0x1000, data + processSize, sig.data(), sig.size());
            if (!address) continue;
            ObjObjects = *reinterpret_cast<decltype(ObjObjects)*>(address);
            return true;
        }
        return false;
    }
    bool FindNamePoolData(BYTE* data) {
        static std::vector<BYTE> sigv[] = { { 0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00, 0xeb, 0x16 } };
        for (auto& sig : sigv)
        {
            auto address = FindPointer(data + 0x1000, data + processSize, sig.data(), sig.size());
            if (!address) continue;
            NamePoolData = *reinterpret_cast<decltype(NamePoolData)*>(address);
            return true;
        }
        return false;
    }
public:

    static Dumper& GetInstance(wchar_t* _processName) {
        static Dumper dumper(_processName);
        return dumper;
    }

    int Init() {
        auto pid = GetProcessIdByName(processName);
        if (!pid) { return PROCESS_NOT_FOUND; };
        
        g_Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!g_Proc) { return INVALID_HANDLE; };
        
        MODULEENTRY32W processInfo;
        if (!GetProcessModule(pid, processName, processInfo)) { return MODULE_NOT_FOUND; };
        processSize = processInfo.modBaseSize;
        processBase = processInfo.modBaseAddr;
       
        wchar_t buf[MAX_PATH];
        GetModuleFileNameW(GetModuleHandleA(0), buf, MAX_PATH);
        directory = fs::path(buf).remove_filename() / "SDK" / fs::path(processName).filename().stem();
        fs::create_directories(directory);
        
        {
            std::vector<BYTE> image(processSize);
            if (!ReadProcessMemory(g_Proc, processBase, image.data(), processSize, nullptr)) { return CANNOT_READ; }
            if (!FindObjObjects(image.data())) { return OBJECTS_NOT_FOUND; }
            if (!FindNamePoolData(image.data())) { return NAMES_NOT_FOUND; }
        }
        

        auto obj = ObjObjects.GetObjectPtr(1);
        auto number = Read<int>((char*)obj + 0x1C);
        if (number != 0) 
        { 
            offsets.addition = 8; 
            offsets.header = 4;
            offsets.stride = 4;
        }
        
        return SUCCESS;
    }
    int Dump(bool full) {

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
                for (auto id = 0; id < ObjObjects.NumElements; id++)
                {
                    UE_UObject object = ObjObjects.GetObjectPtr(id);
                    if (!object) { continue; }
                    if (full && object.IsA<UE_UStruct>())
                    {
                        auto packageObj = object.GetPackageObject();
                        auto It = std::find_if(packages.begin(), packages.end(), [&packageObj](std::pair<UE_UObject, std::vector<UE_UObject>>& package) {return package.first == packageObj; });
                        if (It == packages.end())
                        {
                            packages.push_back({ packageObj , {object} });
                        }
                        else {
                            auto& package = *It;
                            package.second.push_back(object);
                        }
                    }
                    fmt::print(file, "[{:0>6}] <{}> {}\n", object.GetIndex(), object.GetAddress(), object.GetFullName());
                }
                fmt::print("Objects: {}\n", ObjObjects.NumElements);
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
                for (auto& package : packages)
                {
                    fmt::print("\rProcessing: {}/{}", i, max); i++;
                    
                    auto packageObject = package.first;
                    auto packageName = packageObject.GetName();

                    File file(path / (packageName + ".txt"));
                    if (!file) { return FILE_NOT_OPEN; }

                    auto objects = package.second;
                    for (auto object : objects)
                    {
                        
                        auto dumpClass = object.Cast<UE_UClass>();
                        auto name = dumpClass.GetFullName();
                        if (auto superClass = dumpClass.GetSuper())
                        {
                            name += " : " + superClass.GetName();
                        };
                        fmt::print(file, "{} {{\n", name);

                        for (auto field = dumpClass.GetChildren(); field; field = field.GetNext())
                        {
                            auto prop = field.Cast<UE_FProperty>();
                            fmt::print(file, "{} {} // {:#04x}({:#04x})\n", prop.GetType(), prop.GetName(), prop.GetOffset(), prop.GetSize());
                        }
                        fmt::print(file, "}} // {:#04x}\n\n", dumpClass.GetSize());
                    }
                    
                }

            }
        }
        return SUCCESS;
    }
};

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 2) { puts(".\\dumper.exe 'game.exe'"); return 1; }
    auto processName = argv[1];
    auto& dumper = Dumper::GetInstance(processName);

    bool full = true;
    for (auto i = 2; i < argc; i++) { auto arg = argv[i]; if (!wcscmp(arg, L"-p")) { full = false; } }
    
    switch (dumper.Init())
    {
    case PROCESS_NOT_FOUND: { fmt::print("Process not found\n"); return 1;}
    case INVALID_HANDLE: { puts("Can't open process"); return 1; }
    case MODULE_NOT_FOUND: { puts("Can't enumerate modules"); return 1; }
    case CANNOT_READ: { puts("Can't read process memory"); return 1; }
    case OBJECTS_NOT_FOUND: {puts("Can't find objects array"); return 1; }
    case NAMES_NOT_FOUND: {puts("Can't find names array"); return 1; }
    case SUCCESS: { break; };
    default: { return 1; }
    }

    switch (dumper.Dump(full))
    {
    case FILE_NOT_OPEN: { puts("Can't open file"); return 1; }
    case ZERO_PACKAGES: { puts("Size of packages is zero"); return 1; }
    case SUCCESS: { break; }
    default: { return 1; }
    }

    return 0;
}