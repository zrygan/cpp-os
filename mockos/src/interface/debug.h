#pragma once

#include "imgui.h"
#include "os.h"
namespace mockos {
inline void MakeDebug(mockos::OS *this_os) {
  ImGui::Begin("MockOS Dashboard");

  ImGui::Text("MockOS!");
  ImGui::Text("By: Stephen Borja, Erin Chua, Zhean Ganituen, Aaron Go");
  ImGui::Separator();
  ImGui::Text("Application framerate: %.1f FPS", this_os->io->Framerate);

  ImGui::End();
}
} // namespace mockos
