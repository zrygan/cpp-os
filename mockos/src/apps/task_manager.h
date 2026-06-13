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

  std::vector<ProcessDetail> pds;

public:
  struct PerformanceMetrics {
    float cpu = 0;
    float memory = 0;
    float gpu = 0;
    float disk = 0;
    float network = 0;
  };

  PerformanceMetrics performance;

public:
  void randomize();
  void initialize();
  void updateMetrics();
};

#endif

#pragma once
#include "context.h"
#include "imgui.h"

namespace mockos {
inline void MakeTaskManager(mockos::Context *this_ctx,
                            TaskManager &taskManager) {
  if (ImGui::Begin("Task Manager", &this_ctx->flags.show_task_manager)) {
    ImGui::TextWrapped("Processes");
    if (ImGui::BeginTable("taskManagerTable", 6,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg)) {

      ImGui::TableSetupColumn("Process");
      ImGui::TableSetupColumn("CPU");
      ImGui::TableSetupColumn("Memory");
      ImGui::TableSetupColumn("GPU");
      ImGui::TableSetupColumn("Disk");
      ImGui::TableSetupColumn("Network");
      ImGui::TableHeadersRow();

      for (auto &pd : taskManager.pds) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", pd.processName.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d%%", pd.cpu);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%d%%", pd.memory);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%d%%", pd.gpu);
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%d%%", pd.disk);
        ImGui::TableSetColumnIndex(5);
        ImGui::Text("%d%%", pd.network);
      }

      ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::TextWrapped("Performance");
    auto &m = taskManager.performance;
    struct {
      const char *label;
      float val;
    } bars[] = {
        {"CPU", m.cpu},   {"Memory", m.memory},   {"GPU", m.gpu},
        {"Disk", m.disk}, {"Network", m.network},
    };

    for (auto &b : bars) {
      ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                            ImVec4(0.0f, 1.0f, 0.0f, 0.5f));

      char overlay[16];
      snprintf(overlay, sizeof(overlay), "%.1f%%", b.val * 100);
      ImGui::Text("%-8s", b.label);
      ImGui::SameLine();
      ImGui::ProgressBar(b.val, ImVec2(-1, 0), overlay);

      ImGui::PopStyleColor(1);
    }
  }
  ImGui::End();
}
} // namespace mockos