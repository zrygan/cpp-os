#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <string>
namespace mockos {

struct Flags {
  bool show_info = false;
  bool show_taskbar = false;
  bool show_text_editor = false;
  bool kill = false;
};

struct Context {
  GLFWwindow *window;
  ImGuiIO *io;
  Flags flags;
  std::string system_clock;
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
inline Context *Init() {
  if (!glfwInit())
    throw std::runtime_error("Failed to initialize GLFW.");

  Context *this_ctx = new Context();

  // ************
  // (TUI) print the system info (like bios?)
  // ************

  // ************
  // (TUI) then start up screen
  // ************

  // ************
  // GUI init
  // ************

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

  // ImGui::StyleColorsDark();

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

/**
 * This gets the system clock and formats it. This puts the time
 * inplace at the Context parameter.
 */
inline void GetTimeString(Context *this_ctx) {
  auto now = std::chrono::system_clock::now();
  std::time_t time_now = std::chrono::system_clock::to_time_t(now);
  std::tm *local_time = std::localtime(&time_now);
  char buf[16];
  std::strftime(buf, sizeof(buf), "%I:%M %p", local_time);
  this_ctx->system_clock = buf;
}
} // namespace mockos