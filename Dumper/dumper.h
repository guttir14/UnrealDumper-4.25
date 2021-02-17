#pragma once
#include <filesystem>
#include "defs.h"

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
    STATUS Init(int argc, char* argv[]);
    STATUS Dump();
};