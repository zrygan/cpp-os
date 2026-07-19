#include "Context.h"
#include "src/scheduler/Scheduler.h"
#include "memory/PagingManager.h"
#include "view/View.h"

#ifndef CONTROLLER_H
#define CONTROLLER_H

enum class CLI_COMMAND {
  CLI_EXIT,
  CLI_CLEAR,
  CLI_SCREEN_LS,
  CLI_SCREEN_S,
  CLI_SCREEN_R,
  CLI_SCHEDULER_START,
  CLI_SCHEDULER_STOP,
  CLI_REPORT_UTIL,
  UNKNOWN,
};

struct Command {
  CLI_COMMAND cliCommand;

  // speicfically for screen -s
  std::string processName;
};

class Controller {

private:
  View view;
  AlgoContext ctx;
  prosched::Scheduler *scheduler = nullptr;
  prosched::PagingManager *pagingManager = nullptr;
  bool isInitialized;

public:
  ~Controller();
  AlgoContext initialize();
  void run();
  void HandleView();
  bool HandlePreInit(const std::string &input);
  void HandlePostInit(const std::string &input);

  Command GetParsedInput(const std::string &input);
  void ExecuteCommand(const Command &command);
  CLI_COMMAND IdentifyCommand(const std::vector<std::string> &command);
  void ExitOS();
  void EnterProcessScreen(prosched::Process *p);
  void PrintReportUtil();
};

#endif
