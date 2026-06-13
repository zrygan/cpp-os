#include <imgui.h>

namespace mockos
{
  
/**
 * Renders a vertical gradient background filling the entire window.
 */
inline void RenderBackground() {
    ImDrawList* background_draw_list = ImGui::GetBackgroundDrawList();
    ImVec2 window_pos = ImGui::GetMainViewport()->Pos;
    ImVec2 window_size = ImGui::GetMainViewport()->Size;
    
    ImVec2 window_end = ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);

    ImU32 color_top    = ImColor(0.15f, 0.15f, 0.25f, 1.0f); 
    ImU32 color_bottom = ImColor(0.05f, 0.05f, 0.05f, 1.0f);

    background_draw_list->AddRectFilledMultiColor(
        window_pos, 
        window_end, 
        color_top,    
        color_top,    
        color_bottom, 
        color_bottom  
    );
}
} // namespace mockos
