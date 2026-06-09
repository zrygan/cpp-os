#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "helpers.h"
#include "apps/task_manager.h"
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
  mockos::OS* this_os = mockos::init();
    
  // @OutForMilks add this to a taskmanager init function
  // so that we dont bloat up the main loop
  // if needed, use inline for that function
  TaskManager taskManager;
  taskManager.initialize({
      {"System",       80, 60, 50, 30, 20},
      {"Explorer",     45, 70, 30, 10,  5},
      {"Chrome",       12, 40, 15, 55, 80},
      {"Idle",          3, 90,  0,  2, 12},
      {"antivirus.exe",60, 55, 40, 20, 35},
  });

  while (!glfwWindowShouldClose(this_os->window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    taskManager.randomize();
    taskManager.display();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(this_os->window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(this_os->window);
  }

  mockos::cleanup(this_os);

  return 0;
}
