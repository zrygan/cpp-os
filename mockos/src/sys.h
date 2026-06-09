#pragma once
#include <fstream>
#include <string>

namespace mockos {
inline std::string GetCPUModel() {
  std::ifstream f("/proc/cpuinfo");
  std::string line;
  while (std::getline(f, line)) {
    if (line.rfind("model name", 0) == 0) {
      auto pos = line.find(':');
      if (pos != std::string::npos)
        return line.substr(pos + 2);
    }
  }
  return "Unknown";
}

inline long GetTotalRAMMB() {
  std::ifstream f("/proc/meminfo");
  std::string line;
  while (std::getline(f, line)) {
    if (line.rfind("MemTotal", 0) == 0) {
      long kb = 0;
      sscanf(line.c_str(), "MemTotal: %ld kB", &kb);
      return kb / 1024;
    }
  }
  return -1;
}
} // namespace mockos
