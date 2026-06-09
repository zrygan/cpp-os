#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
namespace mockos {

struct OS {
  GLFWwindow *window;
  ImGuiIO *io;
};

/**
 * Initializes the environment for the OS, and returns a struct of
 * the important OS variables.
 */
inline OS *Init() {
  if (!glfwInit())
    throw std::runtime_error("Failed to initialize GLFW.");

  OS *this_os = new OS();

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

  this_os->window = glfwCreateWindow(1280, 720, "mockos", nullptr, nullptr);
  if (!this_os->window) {
    glfwTerminate();
    delete this_os;
    throw std::runtime_error("GLFW Error: Window cannot be created.");
  }

  glfwMakeContextCurrent(this_os->window);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  this_os->io = &ImGui::GetIO();

  // ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(this_os->window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  return this_os;
}

inline void Cleanup(OS *this_os) {
  if (!this_os)
    return;

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  if (this_os->window) {
    glfwDestroyWindow(this_os->window);
  }
  glfwTerminate();

  delete this_os;
}
} // namespace mockos