#pragma once
#include "context.h"
#include "imgui.h"
#include "sys.h"

namespace mockos {

inline void MakeInfo(mockos::Context *this_ctx) {
  if (ImGui::Begin("MockOS Dashboard", &this_ctx->flags.show_info)) {
    ImGui::TextWrapped("Welcome to MockOS!");
    ImGui::TextWrapped(
        "By: Stephen Borja, Erin Chua, Zhean Ganituen, & Aaron Go");
    ImGui::TextWrapped("For: CSOPESY [S06] Semi-MO - Desktop-Style OS Mockup");
    ImGui::Separator();

    static std::string cpu_model = mockos::GetCPUModel();
    static long total_ram_mb = mockos::GetTotalRAMMB();

    ImGui::TextWrapped("Main Processor: %s", cpu_model.c_str());
    if (total_ram_mb >= 0)
      ImGui::TextWrapped("Memory Available: %ld MB", total_ram_mb);
    else
      ImGui::TextWrapped("Memory Available: Unknown");

    ImGui::Separator();

    // Hardcoded strings — fill these in
    ImGui::TextWrapped("BIOS Version: %s", this_ctx->bios_version.c_str());
    ImGui::TextWrapped("Release Date: %s", this_ctx->bios_date.c_str());

    ImGui::Separator();
    ImGui::TextWrapped("Application framerate: %.1f FPS",
                       this_ctx->io->Framerate);
  }
  ImGui::End();
}

} // namespace mockos