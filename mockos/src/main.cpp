#include "apps/task_manager.h"
#include "context.h"
#include "game_loop.h"
#include <iostream>

int main() {
  mockos::Context *ctx = mockos::Init();

  // @OutForMilks move this into a TaskManager init function
  TaskManager taskManager;
  taskManager.initialize();

  mockos::RunLoop(ctx, taskManager);
  mockos::Cleanup(ctx);
  return 0;
}