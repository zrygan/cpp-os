#pragma once

#include "imgui.h"
#include "os.h"
#include "interface/util.h"

namespace mockos {
const ImGuiWindowFlags taskbar_flags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

/**
 * This is the taskbar. The taskbar has three buttons and the task
 * manager button.
 * 
 * This handled flag logic using its OS parameter. That is,
 * if any button on the Taskbar is clicked. It will change the flags
 * in-place. Hence its return type is void while also being able to
 * change the system state.
 */
inline void MakeTaskbar(mockos::OS *this_os) {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  const float taskbar_height = 45.0f;

  ImGui::SetNextWindowPos(ImVec2(
      viewport->Pos.x, viewport->Pos.y + viewport->Size.y - taskbar_height));
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, taskbar_height));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  if (ImGui::Begin("##Taskbar", nullptr, taskbar_flags)) {
    if (ImGui::Button("PWR", ImVec2(60, 30))) {
      this_os->flags.kill = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Task Manager", ImVec2(100, 30))) {
      // Code to open your taskManager window
    }
    

    ImGui::SameLine();
    mockos::MakeVerticalSeparator();

    // Align the system clock to the far right side
    float system_clock_width = 80.0f;
    ImGui::SameLine(ImGui::GetWindowWidth() - system_clock_width);

    // TODO: add system clock here
    ImGui::Text("16:20 PM");
  }
  ImGui::End();

  ImGui::PopStyleVar(2);
}
} // namespace mockos