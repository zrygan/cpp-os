#pragma once

#include "imgui.h"
#include "os.h"

namespace mockos {

inline void MakeTextEditor(mockos::OS *this_os) {
  if (ImGui::Begin("Notepad.exe", &this_os->flags.show_text_editor,
                   ImGuiWindowFlags_MenuBar)) {

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", "Ctrl+N")) {
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) {
          this_os->flags.show_text_editor = false;
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Cut", "Ctrl+X")) {
        }
        if (ImGui::MenuItem("Copy", "Ctrl+C")) {
        }
        if (ImGui::MenuItem("Paste", "Ctrl+V")) {
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    static char text_buffer[4096] =
        "Welcome to the MockOS Text Editor!\n"
        "You can click here and start typing freely.\n"
        "It supports multiline wrapping, selections, and standard hotkeys.";

    ImVec2 input_size = ImVec2(-1.0f, -1.0f);

    ImGui::InputTextMultiline("##TextEditorArea", text_buffer,
                              IM_ARRAYSIZE(text_buffer), input_size);
  }
  ImGui::End();
}

} // namespace mockos