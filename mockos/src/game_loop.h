#pragma once
#include "apps/info.h"
#include "apps/task_manager.h"
#include "apps/text_editor.h"
#include "apps/theme_editor.h"
#include "context.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "interface/taskbar.h"
#include <GLFW/glfw3.h>

namespace mockos {

inline void NewFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

inline void Update(mockos::Context *ctx, TaskManager &taskManager) {
  mockos::GetTimeString(ctx);
  taskManager.randomize();

  int h;
  glfwGetWindowSize(ctx->window, nullptr, &h);
  ctx->io->FontGlobalScale = (float)h / 720.0f;
}

inline void DrawApps(mockos::Context *ctx) {
  mockos::MakeTaskbar(ctx);
  if (ctx->flags.show_info)
    mockos::MakeInfo(ctx);
  if (ctx->flags.show_text_editor)
    mockos::MakeTextEditor(ctx);
  if (ctx->flags.show_theme_editor)
    mockos::MakeThemeEditor(ctx);
}

inline void Render(mockos::Context *ctx) {
  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(ctx->window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(ctx->window);
}

inline void RunLoop(mockos::Context *ctx, TaskManager &taskManager) {
  while (!glfwWindowShouldClose(ctx->window) && !ctx->flags.kill) {
    glfwPollEvents();
    NewFrame();
    Update(ctx, taskManager);
    DrawApps(ctx);
    Render(ctx);
  }
}

} // namespace mockos