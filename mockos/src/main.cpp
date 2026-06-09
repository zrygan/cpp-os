#include "apps/task_manager.h"
#include "game_loop.h"
#include "interface/taskbar.h"
#include "apps/info.h"
#include "apps/text_editor.h"
#include "os.h"
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
  mockos::OS *this_os = mockos::Init();

  // @OutForMilks add this to a taskmanager init function
  // so that we dont bloat up the main loop
  // if needed, use inline for that function
  TaskManager taskManager;
  taskManager.initialize();

  // ********************
  // GAME LOOP
  // ********************
  while (!glfwWindowShouldClose(this_os->window) && !this_os->flags.kill) {

    glfwPollEvents();
    mockos::NewFrame();

    // **********
    // SYSTEM CLOCK
    // **********
    mockos::GetTimeString(this_os);

    // **********
    // TASKMANAGER STUFF
    // **********
    taskManager.randomize();
    // **********
    // INTERFACE STUFF
    // **********
    mockos::MakeTaskbar(this_os);

    if (this_os->flags.show_info)
      mockos::MakeInfo(this_os);

    if (this_os->flags.show_text_editor)
      mockos::MakeTextEditor(this_os);

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(this_os->window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(this_os->window);
  }

  mockos::Cleanup(this_os);

  return 0;
}