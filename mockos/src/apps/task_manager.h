#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <string>
#include <vector>

class TaskManager {
public:
  struct ProcessDetail {
    std::string processName;
    int cpu;
    int memory;
    int gpu;
    int disk;
    int network;
  };

public:
  std::vector<ProcessDetail> pds;

public:
  void randomize();
  void initialize();
};

#endif

#pragma once
#include "context.h"
#include "imgui.h"

namespace mockos {
  inline void MakeTaskManager(mockos::Context *this_ctx, TaskManager &taskManager) {
    if(ImGui::Begin("Task Manager", &this_ctx->flags.show_task_manager)) {
      if (ImGui::BeginTable("taskManagerTable", 6,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {

        ImGui::TableSetupColumn("Process");
        ImGui::TableSetupColumn("CPU");
        ImGui::TableSetupColumn("Memory");
        ImGui::TableSetupColumn("GPU");
        ImGui::TableSetupColumn("Disk");
        ImGui::TableSetupColumn("Network");
        ImGui::TableHeadersRow();

        for (auto &pd : taskManager.pds) {
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0); ImGui::Text("%s", pd.processName.c_str());
          ImGui::TableSetColumnIndex(1); ImGui::Text("%d%%", pd.cpu);
          ImGui::TableSetColumnIndex(2); ImGui::Text("%d%%", pd.memory);
          ImGui::TableSetColumnIndex(3); ImGui::Text("%d%%", pd.gpu);
          ImGui::TableSetColumnIndex(4); ImGui::Text("%d%%", pd.disk);
          ImGui::TableSetColumnIndex(5); ImGui::Text("%d%%", pd.network);
        }

        ImGui::EndTable();
      }
    }
    ImGui::End();
  }
}