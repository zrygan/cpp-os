#include "apps/task_manager.h"
#include "context.h"
#include "game_loop.h"
#include "term.h"
#include <iostream>

int main() {
  mockos::Context *this_ctx = new mockos::Context;
  mockos::RunStartUp(this_ctx->bios_version, this_ctx->bios_date,
                     this_ctx->system_clock);
  mockos::Init(this_ctx);

  // @OutForMilks move this into a TaskManager init function
  TaskManager taskManager;
  taskManager.initialize();

  mockos::RunLoop(this_ctx, taskManager);
  mockos::Cleanup(this_ctx);

  mockos::TermClear();

  return 0;
}