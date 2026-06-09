#pragma once
#include "imgui.h"
namespace mockos {
void inline MakeVerticalSeparator() {
  ImVec2 p = ImGui::GetCursorScreenPos();
  ImGui::GetWindowDrawList()->AddRectFilled(
      p, ImVec2(p.x + 1.0f, p.y + 30.0f),
      ImGui::GetColorU32(ImGuiCol_Separator));
  ImGui::Dummy(ImVec2(1.0f, 30.0f));
}
} // namespace mockos
