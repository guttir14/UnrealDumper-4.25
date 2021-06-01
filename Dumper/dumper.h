#pragma once
#include "defs.h"
#include <filesystem>

namespace fs = std::filesystem;

class Dumper {
protected:
  bool Full = true;
  bool Wait = false;
  bool Spacing = false;
  fs::path Directory;
  void* Image = nullptr;
 

private:
  Dumper(){};

public:
  static Dumper* GetInstance() {
    static Dumper dumper;
    return &dumper;
  }
  ~Dumper();
  STATUS Init(int argc, char *argv[]);
  STATUS Dump();
};
