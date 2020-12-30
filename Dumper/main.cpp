#include <filesystem>
#include <fmt/core.h>
#include "utils.h"
#include "wrappers.h"
#include "memory.h"
#include <algorithm>
#include <map>
#include <psapi.h>

namespace fs = std::filesystem;

enum {
    SUCCESS,
    FAILED,
    WINDOW_NOT_FOUND,
    PROCESS_NOT_FOUND,
    READER_ERROR,
    CANNOT_GET_PROCNAME,
    ENGINE_ERROR,
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
    struct 
    {
        byte* Base = nullptr;
        uint32_t Size = 0;
    } ProcessInfo;
    bool Full = true;
    bool Wait = false;
    fs::path Directory;
private:
    Dumper() {};
    static bool FindObjObjects(byte* start, byte* end) 
    {
        static std::vector<byte> sigv[] = { {0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0C, 0xC8, 0x48, 0x8D, 0x04, 0xD1, 0xEB}, {0x48 , 0x8b , 0x0d , 0x00 , 0x00 , 0x00 , 0x00 , 0x81 , 0x4c , 0xd1 , 0x08 , 0x00 , 0x00 , 0x00 , 0x40}, {0x48, 0x8d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x39, 0x44, 0x24, 0x68} };
        for (auto& sig : sigv)
        {
            auto address = FindPointer(start, end, sig.data(), sig.size());
            if (!address) continue;
            ObjObjects = *reinterpret_cast<decltype(ObjObjects)*>(address);
            return true;
        }
        return false;
    }
    static bool FindNamePoolData(byte* start, byte* end) 
    {
        static std::vector<byte> sigv[] = { { 0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00, 0xeb, 0x16 } };
        for (auto& sig : sigv)
        {
            auto address = FindPointer(start, end, sig.data(), sig.size());
            if (!address) continue;
            NamePoolData = *reinterpret_cast<decltype(NamePoolData)*>(address);
            return true;
        }
        return false;
    }
public:
    static Dumper* GetInstance() 
    {
        static Dumper dumper;
        return &dumper;
    }
    int Init(int argc, char* argv[]) 
    {

        for (auto i = 1; i < argc; i++)
        {
            auto arg = argv[i];
            if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) { printf("'-p' - dump only names and objects\n'-w' - wait for input (it gives me time to inject mods)"); return FAILED; }
            else if (!strcmp(arg, "-p")) { Full = false; }
            else if (!strcmp(arg, "-w")) { Wait = true; }
        }

        if (Wait) { system("pause"); }

        uint32_t pid = 0;

        {
            HWND hWnd = FindWindowA("UnrealWindow", nullptr);
            if (!hWnd) { return WINDOW_NOT_FOUND; };
            GetWindowThreadProcessId(hWnd, reinterpret_cast<DWORD*>(&pid));
            if (!pid) { return PROCESS_NOT_FOUND; };
            if (!ReaderInit(pid)) { return READER_ERROR; };
        }

        fs::path processName;

        {
            wchar_t processPath[MAX_PATH]{};
            if (!GetProccessPath(pid, processPath, MAX_PATH)) { return CANNOT_GET_PROCNAME; };
            processName = fs::path(processPath).filename();
            printf("Found UE4 game: %ls\n", processName.wstring().c_str());
        }

        {
            auto root = fs::path(argv[0]); root.remove_filename();
            auto game = processName.stem();
            Directory = root / "Games" / game;
            fs::create_directories(Directory);
            if (!EngineInit(game.string())) { return ENGINE_ERROR; };
        }

        {
            MODULEENTRY32W mod;
            auto str = processName.wstring();
            const wchar_t* arr[] = { str.c_str() };
            if (!GetProcessModules(pid, 1, arr, &mod)) { return MODULE_NOT_FOUND; };
            ProcessInfo = { mod.modBaseAddr, mod.modBaseSize };
        }

        {
            std::vector<byte> image(ProcessInfo.Size);
            std::vector<std::pair<byte*, byte*>> sections;
            {
               
                if (!Read(ProcessInfo.Base, image.data(), ProcessInfo.Size)) { return CANNOT_READ; }
                auto err = GetExSections(image.data(), sections);
                if (!err) { return INVALID_IMAGE; }
            }
            {
                bool err = false;
                for (auto& section : sections) { if (FindObjObjects(section.first, section.second)) { err = true; break; }; }
                if (!err) { return OBJECTS_NOT_FOUND; };
            }
            {
                bool err = false;
                for (auto& section : sections) { if (FindNamePoolData(section.first, section.second)) { err = true; break; }; }
                if (!err) { return NAMES_NOT_FOUND; };
            }
        }
        
        return SUCCESS;
    }
    int Dump() 
    {
        /*
        * Names dumping.
        * We go through each block, except last, that is not fully filled.
        * In each block we calculate next entry depending on previous entry size.
        */
        {
            File file(Directory / "NamesDump.txt", "w");
            if (!file) { return FILE_NOT_OPEN; }
            size_t size = 0;
            NamePoolData.Dump([&file, &size](std::string_view name, uint32_t id) { fmt::print(file, "[{:0>6}] {}\n", id, name); size++; });
            fmt::print("Names: {}\n", size);
        }
        {
            // Why we need to iterate all objects twice? We dumping objects and filling packages simultaneously.
            std::unordered_map<byte*, std::vector<UE_UObject>> packages;
            {
                File file(Directory / "ObjectsDump.txt", "w");
                if (!file) { return FILE_NOT_OPEN; }
                size_t size = 0;
                if (Full)
                {
                    ObjObjects.Dump(
                        [&file, &size, &packages](UE_UObject object)
                        {
                            fmt::print(file, "[{:0>6}] <{}> {}\n", object.GetIndex(), object.GetAddress(), object.GetFullName()); size++;
                            if (object.IsA<UE_UStruct>() || object.IsA<UE_UEnum>())
                            {
                                auto packageObj = object.GetPackageObject();
                                packages[packageObj].push_back(object);
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

            if (!Full) { return SUCCESS; }

            {
                // Clearing all empty packages
                size_t size = packages.size();
                size_t erased = std::erase_if(packages, [](std::pair<byte* const, std::vector<UE_UObject>>& package) { return package.second.size() < 2; });

                fmt::print("Wiped {} out of {}\n", erased, size);
            }

            // Checking if we have any package after clearing.
            if (!packages.size()) { return ZERO_PACKAGES; }

            fmt::print("Packages: {}\n", packages.size());

            {
                auto path = Directory / "DUMP";
                fs::create_directories(path);

                int i = 1; int saved = 0;
                std::string unsaved{};

                for (UE_UPackage package : packages)
                {
                    fmt::print("\rProcessing: {}/{}", i++, packages.size());

                    package.Process();
                    if (package.Save(path)) { saved++; }
                    else { unsaved += (package.GetObject().GetName() + ", "); };
                }

                
                fmt::print("\nSaved packages: {}", saved);

                if (unsaved.size())
                {
                    unsaved.erase(unsaved.size() - 2);
                    fmt::print("\nUnsaved packages (empty classes): [ {} ]", unsaved);
                }

            }
        }
        return SUCCESS;
    }
};

int main(int argc, char* argv[])
{
    auto dumper = Dumper::GetInstance();

    switch (dumper->Init(argc, argv))
    {
    case WINDOW_NOT_FOUND: { puts("Can't find UE4 window"); return FAILED; }
    case PROCESS_NOT_FOUND: { puts("Can't find process"); return FAILED; }
    case READER_ERROR: { puts("Can't init reader"); return FAILED; }
    case CANNOT_GET_PROCNAME: { puts("Can't get process name"); return FAILED; }
    case ENGINE_ERROR: {puts("Can't find offsets for this game"); return FAILED; }
    case MODULE_NOT_FOUND: { puts("Can't enumerate modules (protected process?)"); return FAILED; }
    case CANNOT_READ: { puts("Can't read process memory"); return FAILED; }
    case INVALID_IMAGE: { puts("Can't get executable sections"); return FAILED; }
    case OBJECTS_NOT_FOUND: { puts("Can't find objects array"); return FAILED; }
    case NAMES_NOT_FOUND: { puts("Can't find names array"); return FAILED; }
    case SUCCESS: { break; };
    default: { return FAILED; }
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