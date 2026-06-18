#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <cstdlib>

#include "Controller.h"
#include "config.h"
#include "context.h"
#include "src/scheduler/Scheduler.h"

AlgoContext Controller::initialize() {
  ConfigStruct *cs = fromFile();

  /*
    <RV @zrygan> ===========
    Since this function will inherently fail if cs if not found.
    This shouldn't just return void. Make it return AlgoContext

    and add test for it

    @stephen
    <RV @zrygan> ===========
  */
  if (cs == nullptr) {
    std::cout << "Failed to find config file\n";
    this->ctx = {};
    return this->ctx;
  }

  this->ctx = AlgoContext::buildConfig(cs);
  this->scheduler = new prosched::Scheduler(this->ctx);
  this->isInitialized = true;

  /* <RV @zrygan> ===========
  Shouldn't this be outside this function?

  Also the function view.DisplayMenu() kind of doesn not display
  the menu but instead shows a start up page. I suggest some changes
  implemented in the naming and organization of stuff since we are
  using MVC.

  @stephen @erin
  <RV @zrygan> =========== */
  std::cout << "Initialized successfully.\n\n";
  std::cout << "List of accessible commands:\n"
            << "- exit\n"
            << "- screen -s <process name>\n"
            << "- screen -ls\n"
            << "- scheduler-start\n"
            << "- scheduler-stop\n"
            << "- report-util\n\n";
  
  return this->ctx;         
}

void Controller::run() {
  this->view = View();
  view.DisplayMenu();
  std::string input;

  /* <RV @zrygan> ===========
  There must be an easier way to parse these commands especially
  since some commands have parameters (see screen -s <proc>)

  To add: Also introduce separation of concerns here.
  This does multiple things: DisplayMenu, get input, parse input,
  execute input.

  I suggest running the input parsing (with parameters) as a separate
  function. See what happens in (Interpreter.hpp). And I think
  a match case on command types would also be better.

  suggestion:
  enum CLI_COMMAND {
    CLI_EXIT,
    CLI_SCREEN_S
    CLI_SCREEN_LS
    // ...
  };

  parsed_input = get_cli_command(input);
  // this takes the input line and returns the enum type and the
  // parameters of that cli-type.
  // the type of parsed_input is a CLI_COMMAND

  match parsed_input {
    case CLI_EXIT { };
    case CLI_SCREEN_S { };
    case_CLI_SCREEN_LS { };
    // ...
  }

  <RV @zrygan> =========== */
  while (GetParsedInput(input).cliCommand != CLI_COMMAND::CLI_EXIT) {
    std::cout << "Enter Input: ";
    std::getline(std::cin, input);

    if (!isInitialized) {
      /* <RV @zrygan> ==========
      There must be an easier way to always check if it's initialized
      perhaps move this outside the code. Everything below SHOUD NOT RUN
      if it isn't initialized. (Maybe move this to the main loop/outside this
      function). <RV @zrygan> ========== */
      if (input == "initialize") {

        initialize();

      } else {
        std::cout << "\nType \"initialize\" to access commands\n";
      }
    } else {
      Command command = GetParsedInput(input);
      ExecuteCommand(command);
    }
  }
}

std::vector<std::string> trim(const std::string &s, char separator) {
  std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);

    while (std::getline(tokenStream, token, separator)) {
        tokens.push_back(token);
    }
    return tokens;
}

void Controller::ExitOS(){
  std::cout << "Stopping prosched...\n";
  std::_Exit(EXIT_SUCCESS);
}

Command Controller::GetParsedInput(const std::string& input) {
  Command commands;

  if (input.empty()) {
        commands.cliCommand = CLI_COMMAND::UNKNOWN;
        return commands;
  }

  std::vector<std::string> trimmed = trim(input, ' ');

  if (trimmed.empty()) {
        commands.cliCommand = CLI_COMMAND::UNKNOWN;
        return commands;
  }

  commands.cliCommand = IdentifyCommand(trimmed);
  
  if (commands.cliCommand == CLI_COMMAND::UNKNOWN) {
    commands.cliCommand = CLI_COMMAND::UNKNOWN;
    return commands;
  }

  if(commands.cliCommand == CLI_COMMAND::CLI_SCREEN_S){
    commands.processName = trimmed[2];
    return commands;
  }

  return commands;
}

void Controller::ExecuteCommand(const Command& command){
  try {
    switch(command.cliCommand) {
      case CLI_COMMAND::CLI_EXIT:
        ExitOS();
      break;

      case CLI_COMMAND::CLI_SCREEN_LS:
        this->scheduler->PrintProcesses();
      break;

      case CLI_COMMAND::CLI_SCREEN_S:
        // none for now
      break;

      case CLI_COMMAND::CLI_SCHEDULER_START:
        if (!this->scheduler->IsRunning() == true) {
          this->scheduler->Start();
        } else {
          std::cout << "Scheduler is still running...\n\n";
        }
      break;

      case CLI_COMMAND::CLI_SCHEDULER_STOP:
        if (this->scheduler->IsRunning() == true) {
          this->scheduler->Stop();
        } else {
          std::cout << "Scheduler has not started, run \"scheduler-start\" "
                       "to start\n\n";
        }
      break;

      case CLI_COMMAND::CLI_REPORT_UTIL:
      // none for now
      break;

      case CLI_COMMAND::UNKNOWN:
        std::cout << "Command unknown. Try again. \n";
      break;
    }

  } catch (const std::exception &e) {
    std::cerr << "Controller: Could not execute command" << e.what();
    return;
  }
}

CLI_COMMAND Controller::IdentifyCommand(const std::vector<std::string> &command) {
  if(command[0] == "screen") {
    if (command.size() < 2) return CLI_COMMAND::UNKNOWN;
    if(command[1] == "-ls") {
      return CLI_COMMAND::CLI_SCREEN_LS;
    } else if (command[1] == "-s") {
      return CLI_COMMAND::CLI_SCREEN_S;
    } else {
      return CLI_COMMAND::UNKNOWN;
    }
  } else if (command[0] == "exit") {
    return CLI_COMMAND::CLI_EXIT;
  } else if (command[0] == "scheduler-start") {
    return CLI_COMMAND::CLI_SCHEDULER_START;
  } else if (command[0] == "scheduler-stop") {
    return CLI_COMMAND::CLI_SCHEDULER_STOP;
  } else if (command[0] == "report-util") {
    return CLI_COMMAND::CLI_REPORT_UTIL;
  } else {
    return CLI_COMMAND::UNKNOWN;
  }
}