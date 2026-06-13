#pragma once

#include "clock.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "interface/startup.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>
namespace mockos {

struct Flags {
  bool show_info = false;
  bool show_taskbar = false;
  bool show_task_manager = false;
  bool show_text_editor = false;
  bool show_theme_editor = false;
  bool kill = false;
};

struct Context {
  GLFWwindow *window;
  ImGuiIO *io;
  Flags flags;
  std::string bios_version = "CSOPESY-MO2";
  std::string bios_date = "June 9, 2026";
  std::string GetSystemClock() const { return mockos::GetTimeString(true); }

  ImVec4 gradient_color1 = ImVec4(0.15f, 0.15f, 0.25f, 1.0f);
  ImVec4 gradient_color2 = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
  bool gradient_horizontal = false;
  bool animate_gradient = false;
  float animation_speed = 1.0f;
};

/**
 * The imgui.ini file will ALWAYS be generated. This will save
 * the state of the GUI after termination.
 *
 * This is very unsafe for debugging and formatting the GUI since the
 * cached settings may overwrite the updated code/behavior.
 *
 * By default this should always be FALSE.
 */
const bool kSaveIni = false;

/**
 * Initializes the environment for the Context, and returns a struct of
 * the important Context variables.
 */
inline void *Init(Context *this_ctx) {
  if (!glfwInit())
    throw std::runtime_error("Failed to initialize GLFW.");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  this_ctx->window = glfwCreateWindow(1280, 720, "mockos", nullptr, nullptr);
  if (!this_ctx->window) {
    glfwTerminate();
    delete this_ctx;
    throw std::runtime_error("GLFW Error: Window cannot be created.");
  }

  glfwMakeContextCurrent(this_ctx->window);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  this_ctx->io = &ImGui::GetIO();

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(this_ctx->window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  return this_ctx;
}

inline void Cleanup(Context *this_ctx) {
  if (!this_ctx)
    return;

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  if (this_ctx->window) {
    glfwDestroyWindow(this_ctx->window);
  }
  glfwTerminate();

  delete this_ctx;
}

} // namespace mockos