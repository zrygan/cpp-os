namespace mockos {
#pragma once
#include "context.h"
#include "imgui.h"
#include <cmath>

inline void MakeThemeEditor(mockos::Context *this_ctx) {
  if (ImGui::Begin("Background Gradient Editor",
                   &this_ctx->flags.show_theme_editor)) {

    ImGui::TextWrapped("Gradient Colors");
    ImGui::ColorEdit4(
        this_ctx->gradient_horizontal ? "Left Color" : "Top Color",
        (float *)&this_ctx->gradient_color1,
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);

    ImGui::ColorEdit4(
        this_ctx->gradient_horizontal ? "Right Color" : "Bottom Color",
        (float *)&this_ctx->gradient_color2,
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);

    ImGui::Separator();

    ImGui::TextWrapped("Direction");
    if (ImGui::RadioButton("Vertical", !this_ctx->gradient_horizontal)) {
      this_ctx->gradient_horizontal = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Horizontal", this_ctx->gradient_horizontal)) {
      this_ctx->gradient_horizontal = true;
    }

    ImGui::Separator();

    ImGui::TextWrapped("Effects");
    ImGui::Checkbox("Animate Color Shifting", &this_ctx->animate_gradient);

    if (this_ctx->animate_gradient) {
      ImGui::SliderFloat("Speed", &this_ctx->animation_speed, 0.1f, 5.0f,
                         "%.1fx");
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                         "Animation active: Pulsing hues smoothly.");
    }

    ImGui::Separator();

    if (ImGui::Button("Reset to Default OS Colors")) {
      this_ctx->gradient_color1 = ImVec4(0.15f, 0.15f, 0.25f, 1.0f);
      this_ctx->gradient_color2 = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
      this_ctx->gradient_horizontal = false;
      this_ctx->animate_gradient = false;
      this_ctx->animation_speed = 1.0f;
    }
  }
  ImGui::End();
}

} // namespace mockos