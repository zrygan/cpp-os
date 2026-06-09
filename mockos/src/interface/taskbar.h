#pragma once

#include "context.h"
#include "imgui.h"
#include "interface/util.h"
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
  std::function<void(mockos::Context *)> action_on_press;
};

const std::array<TaskbarButton, 5> kTaskbarButtons = {
    {{"PWR", [](mockos::Context *os) { os->flags.kill = true; }},
     {"Task Manager",
      [](mockos::Context *os) {
        os->flags.show_taskbar = !os->flags.show_taskbar;
      }},
     {"Info",
      [](mockos::Context *os) { os->flags.show_info = !os->flags.show_info; }},
     {"Text Editor",
      [](mockos::Context *os) {
        os->flags.show_text_editor = !os->flags.show_text_editor;
      }},
     {"Theme Editor", [](mockos::Context *os) {
        os->flags.show_theme_editor = !os->flags.show_theme_editor;
      }}}};

inline ImVec2 GetButtonSize(mockos::Context *this_ctx) {
  int h;
  glfwGetWindowSize(this_ctx->window, nullptr, &h);
  float scale = (float)h / 720.0f;
  return ImVec2(100.0f * scale, 30.0f * scale);
}

/**
 * This is the taskbar. The taskbar has three buttons and the task
 * manager button.
 *
 * This handled flag logic using its Context parameter. That is,
 * if any button on the Taskbar is clicked. It will change the flags
 * in-place. Hence its return type is void while also being able to
 * change the system state.
 */
inline void MakeTaskbar(mockos::Context *this_ctx) {
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
                        GetButtonSize(this_ctx))) {
        kTaskbarButtons[i].action_on_press(this_ctx);
      }
    }

    ImGui::SameLine();
    mockos::MakeVerticalSeparator();

    float system_clock_width = 110.0f;
    ImGui::SameLine(ImGui::GetWindowWidth() - system_clock_width);

    ImGui::Text("%s", this_ctx->GetSystemClock().c_str());
  }
  ImGui::End();

  ImGui::PopStyleVar(2);
}
} // namespace mockos