#pragma once

#include "imgui.h"
#include "os.h"
namespace mockos {
inline void MakeDebug(mockos::OS* this_os) {
    if (ImGui::Begin("MockOS Dashboard", &this_os->flags.show_debug)) {
        ImGui::TextWrapped("Welcome to MockOS!");
        ImGui::TextWrapped("By: Stephen Borja, Erin Chua, Zhean Ganituen, and Aaron Go");
        ImGui::TextWrapped("For: CSOPESY [S06] Semi-MO - Desktop-Style OS Mockup");
        ImGui::Separator();
        ImGui::TextWrapped("Application framerate: %.1f FPS", this_os->io->Framerate);
    }
    ImGui::End();
}
} // namespace mockos
