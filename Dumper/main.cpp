#include <fmt/core.h>
#include "memory.h"
#include "utils.h" 
#include "wrappers.h"
#include "engine.h"

namespace fs = std::filesystem;

class Dumper
{
protected:
    bool Full = true;
    bool Wait = false;
    fs::path Directory;
private:
    Dumper() {};
public:
    static Dumper* GetInstance() 
    {
        static Dumper dumper;
        return &dumper;
    }
    STATUS Init(int argc, char* argv[]) 
    {
        for (auto i = 1; i < argc; i++)
        {
            auto arg = argv[i];
            if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) { printf("'-p' - dump only names and objects\n'-w' - wait for input (it gives me time to inject mods)"); return STATUS::FAILED; }
            else if (!strcmp(arg, "-p")) { Full = false; }
            else if (!strcmp(arg, "-w")) { Wait = true; }
        }

        if (Wait) { system("pause"); }

        uint32_t pid = 0;

        {
            HWND hWnd = FindWindowA("UnrealWindow", nullptr);
            if (!hWnd) { return STATUS::WINDOW_NOT_FOUND; };
            GetWindowThreadProcessId(hWnd, reinterpret_cast<DWORD*>(&pid));
            if (!pid) { return STATUS::PROCESS_NOT_FOUND; };
        }
        
        if (!ReaderInit(pid)) { return STATUS::READER_ERROR; };

        fs::path processName;

        {
            wchar_t processPath[MAX_PATH]{};
            if (!GetProccessPath(pid, processPath, MAX_PATH)) { return STATUS::CANNOT_GET_PROCNAME; };
            processName = fs::path(processPath).filename();
            printf("Found UE4 game: %ls\n", processName.c_str());
        }

        {
            auto root = fs::path(argv[0]); root.remove_filename();
            auto game = processName.stem();
            Directory = root / "Games" / game;
            fs::create_directories(Directory);

            auto [base, size] = GetModuleInfo(pid, processName);
            if (!(base && size)) { return STATUS::MODULE_NOT_FOUND; }
            std::vector<byte> image(size);
            if (!Read(base, image.data(), size)) { return STATUS::CANNOT_READ; }
            auto sections = GetExSections(image.data());
            if (!sections.size()) { return STATUS::INVALID_IMAGE; }
            Base = reinterpret_cast<uint64_t>(base);
            return EngineInit(game.string(), &sections);
        }
    }
    STATUS Dump() 
    {
        /*
        * Names dumping.
        * We go through each block, except last, that is not fully filled.
        * In each block we calculate next entry depending on previous entry size.
        */
        {
            File file(Directory / "NamesDump.txt", "w");
            if (!file) { return STATUS::FILE_NOT_OPEN; }
            size_t size = 0;
            NamePoolData.Dump([&file, &size](std::string_view name, uint32_t id) { fmt::print(file, "[{:0>6}] {}\n", id, name); size++; });
            fmt::print("Names: {}\n", size);
        }
        {
            // Why we need to iterate all objects twice? We dumping objects and filling packages simultaneously.
            std::unordered_map<byte*, std::vector<UE_UObject>> packages;
            {
                File file(Directory / "ObjectsDump.txt", "w");
                if (!file) { return STATUS::FILE_NOT_OPEN; }
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

            if (!Full) { return STATUS::SUCCESS; }

            {
                // Clearing all packages with small amount of objects (comment this if you need all packages to be dumped)
                size_t size = packages.size();
                size_t erased = std::erase_if(packages, [](std::pair<byte* const, std::vector<UE_UObject>>& package) { return package.second.size() < 2; });

                fmt::print("Wiped {} out of {}\n", erased, size);
            }

            // Checking if we have any package after clearing.
            if (!packages.size()) { return STATUS::ZERO_PACKAGES; }

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
        return STATUS::SUCCESS;
    }
};

int main(int argc, char* argv[])
{
    auto dumper = Dumper::GetInstance();

    switch (dumper->Init(argc, argv))
    {
    case STATUS::WINDOW_NOT_FOUND: { puts("Can't find UE4 window"); return 1; }
    case STATUS::PROCESS_NOT_FOUND: { puts("Can't find process"); return 1; }
    case STATUS::READER_ERROR: { puts("Can't init reader"); return 1; }
    case STATUS::CANNOT_GET_PROCNAME: { puts("Can't get process name"); return 1; }
    case STATUS::ENGINE_NOT_FOUND: { puts("Can't find offsets for this game"); return 1; }
    case STATUS::ENGINE_FAILED: { puts("Can't init engine for this game"); return 1; }
    case STATUS::MODULE_NOT_FOUND: { puts("Can't enumerate modules (protected process?)"); return 1; }
    case STATUS::CANNOT_READ: { puts("Can't read process memory"); return 1; }
    case STATUS::INVALID_IMAGE: { puts("Can't get executable sections"); return 1; }
    case STATUS::OBJECTS_NOT_FOUND: { puts("Can't find objects array"); return 1; }
    case STATUS::NAMES_NOT_FOUND: { puts("Can't find names array"); return 1; }
    case STATUS::SUCCESS: { break; };
    default: { return 1; }
    }

    switch (dumper->Dump())
    {
    case STATUS::FILE_NOT_OPEN: { puts("Can't open file"); return 1; }
    case STATUS::ZERO_PACKAGES: { puts("Size of packages is zero"); return 1; }
    case STATUS::SUCCESS: { break; }
    default: { return 1; }
    }

    return 0;
}