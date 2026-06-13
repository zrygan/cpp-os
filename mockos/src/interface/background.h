#include <imgui.h>

namespace mockos {

/**
 * Renders a vertical gradient background filling the entire window.
 * This is designated the task of drawing the gradient background
 * and animating it if it is toggled in the context struct.
 */
inline void RenderBackground(mockos::Context *ctx) {
  ImDrawList *background_draw_list = ImGui::GetBackgroundDrawList();
  ImVec2 window_pos = ImGui::GetMainViewport()->Pos;
  ImVec2 window_size = ImGui::GetMainViewport()->Size;
  ImVec2 window_end =
      ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);

  ImVec4 c1 = ctx->gradient_color1;
  ImVec4 c2 = ctx->gradient_color2;

  if (ctx->animate_gradient) {
    float time = (float)ImGui::GetTime() * ctx->animation_speed;

    float wave = (std::sin(time) + 1.0f) * 0.2f;

    c1.x = std::fmod(c1.x + wave, 1.0f);
    c1.z = std::fmod(c1.z + wave * 0.5f, 1.0f);
    c2.y = std::fmod(c2.y + wave, 1.0f);
  }

  ImU32 col_tl, col_tr, col_br, col_bl;

  if (ctx->gradient_horizontal) {
    col_tl = ImColor(c1);
    col_bl = ImColor(c1);
    col_tr = ImColor(c2);
    col_br = ImColor(c2);
  } else {
    col_tl = ImColor(c1);
    col_tr = ImColor(c1);
    col_br = ImColor(c2);
    col_bl = ImColor(c2);
  }

  background_draw_list->AddRectFilledMultiColor(window_pos, window_end, col_tl,
                                                col_tr, col_br, col_bl);
}
} // namespace mockos
