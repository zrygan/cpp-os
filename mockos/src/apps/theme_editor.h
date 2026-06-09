#pragma once
#include "context.h"
#include "imgui.h"

namespace mockos {

inline void MakeThemeEditor(mockos::Context *this_ctx) {
  if (ImGui::Begin("Theme Editor", &this_ctx->flags.show_theme_editor)) {
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui::TextWrapped("Window");
    ImGui::ColorEdit4(
        "Window Background", (float *)&style.Colors[ImGuiCol_WindowBg],
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);

    ImGui::Separator();
    ImGui::TextWrapped("Text");
    ImGui::ColorEdit4("Text", (float *)&style.Colors[ImGuiCol_Text],
                      ImGuiColorEditFlags_NoInputs |
                          ImGuiColorEditFlags_PickerHueWheel);

    ImGui::Separator();
    ImGui::TextWrapped("Buttons");
    ImGui::ColorEdit4("Button", (float *)&style.Colors[ImGuiCol_Button],
                      ImGuiColorEditFlags_NoInputs |
                          ImGuiColorEditFlags_PickerHueWheel);
    ImGui::ColorEdit4(
        "Button Hovered", (float *)&style.Colors[ImGuiCol_ButtonHovered],
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
    ImGui::ColorEdit4(
        "Button Active", (float *)&style.Colors[ImGuiCol_ButtonActive],
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);

    ImGui::Separator();
    ImGui::TextWrapped("Titlebar");
    ImGui::ColorEdit4(
        "Title Background", (float *)&style.Colors[ImGuiCol_TitleBg],
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
    ImGui::ColorEdit4("Title Background Active",
                      (float *)&style.Colors[ImGuiCol_TitleBgActive],
                      ImGuiColorEditFlags_NoInputs |
                          ImGuiColorEditFlags_PickerHueWheel);

    ImGui::Separator();
    ImGui::TextWrapped("Taskbar");
    ImGui::ColorEdit4(
        "Taskbar Background", (float *)&style.Colors[ImGuiCol_MenuBarBg],
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);

    ImGui::Separator();
    if (ImGui::Button("Reset to Default"))
      ImGui::StyleColorsDark();
  }
  ImGui::End();
}

} // namespace mockos