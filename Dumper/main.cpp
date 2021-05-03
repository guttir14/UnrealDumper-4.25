#include "dumper.h"

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
