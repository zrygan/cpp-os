#pragma once

#include "imgui.h"
#include "interface/util.h"
#include "os.h"
#include <array>
#include <functional>
#include <string>

namespace mockos {
const ImGuiWindowFlags kTaskbarFlags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

struct TaskbarButton {
  std::string label;
  std::function<void(mockos::OS *)> action_on_press;
};

const std::array<TaskbarButton, 5> kTaskbarButtons = {
    {{"PWR", [](mockos::OS *os) { os->flags.kill = true; }},
     {"Task Manager",
      [](mockos::OS *os) { os->flags.show_taskbar = !os->flags.show_taskbar; }},
     {"Info",
      [](mockos::OS *os) { os->flags.show_info = !os->flags.show_info; }},
     {"Text Editor",
      [](mockos::OS *os) {
        os->flags.show_text_editor = !os->flags.show_text_editor;
      }},
     {"PLACEHOLDER 2", [](mockos::OS *os) {}}}};

inline ImVec2 GetButtonSize(mockos::OS *this_os) {
  return ImVec2(100.0f, 30.0f);
}

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

  if (ImGui::Begin("##Taskbar", nullptr, kTaskbarFlags)) {
    for (size_t i = 0; i < kTaskbarButtons.size(); ++i) {
      if (i > 0)
        ImGui::SameLine();

      if (ImGui::Button(kTaskbarButtons[i].label.c_str(),
                        GetButtonSize(this_os))) {
        kTaskbarButtons[i].action_on_press(this_os);
      }
    }

    ImGui::SameLine();
    mockos::MakeVerticalSeparator();

    float system_clock_width = 110.0f;
    ImGui::SameLine(ImGui::GetWindowWidth() - system_clock_width);

    ImGui::Text("%s", this_os->system_clock.c_str());
  }
  ImGui::End();

  ImGui::PopStyleVar(2);
}
} // namespace mockos