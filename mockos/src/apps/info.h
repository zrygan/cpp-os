#pragma once
#include "context.h"
#include "imgui.h"
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

inline void MakeInfo(mockos::Context *this_ctx) {
  if (ImGui::Begin("MockOS Dashboard", &this_ctx->flags.show_info)) {
    ImGui::TextWrapped("Welcome to MockOS!");
    ImGui::TextWrapped(
        "By: Stephen Borja, Erin Chua, Zhean Ganituen, & Aaron Go");
    ImGui::TextWrapped("For: CSOPESY [S06] Semi-MO - Desktop-Style OS Mockup");
    ImGui::Separator();

    static std::string cpu_model = GetCPUModel();
    static long total_ram_mb = GetTotalRAMMB();

    ImGui::TextWrapped("Main Processor: %s", cpu_model.c_str());
    if (total_ram_mb >= 0)
      ImGui::TextWrapped("Memory Available: %ld MB", total_ram_mb);
    else
      ImGui::TextWrapped("Memory Available: Unknown");

    ImGui::Separator();

    // Hardcoded strings — fill these in
    ImGui::TextWrapped("BIOS Version: 1.0.0");
    ImGui::TextWrapped("Release Date: January 1, 2025");

    ImGui::Separator();
    ImGui::TextWrapped("Application framerate: %.1f FPS",
                       this_ctx->io->Framerate);
  }
  ImGui::End();
}

} // namespace mockos