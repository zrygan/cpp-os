#pragma once
#include "imgui.h"
#include "os.h"

namespace mockos {

const ImGuiInputTextFlags kTextEditorFlags = ImGuiInputTextFlags_WordWrap;

inline void MakeTextEditor(mockos::OS *this_os) {
    if (ImGui::Begin("Notepad.exe", &this_os->flags.show_text_editor,
                     ImGuiWindowFlags_MenuBar)) {
        static char text_buffer[4096] =
            "Welcome to the MockOS Text Editor!\n"
            "You can click here and start typing freely.\n"
            "It supports multiline wrapping, selections, and standard hotkeys.";

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Ctrl+N")) {
                    text_buffer[0] = '\0';
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    this_os->flags.show_text_editor = false;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::InputTextMultiline("##TextEditorArea", text_buffer,
                                  IM_ARRAYSIZE(text_buffer),
                                  ImGui::GetContentRegionAvail(),
                                  kTextEditorFlags);
    }
    ImGui::End();
}

} // namespace mockos