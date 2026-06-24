#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>

#include "Config.h"
#include "Context.h"
#include "Controller.h"
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

  this->view.DisplayCommands();

  return this->ctx;
}

void Controller::HandleView() {
  this->view = View();
  view.DisplayMenu();
}

bool Controller::HandlePreInit(const std::string &input) {
  if (input == "initialize") {
    initialize();
    if (!isInitialized) {
      std::cout << "Initialization failed. Check config.txt.\n";
    }
    return true;
  }

  if (input == "exit") {
    ExitOS();
  }

  std::cout << "Type \"initialize\" to access commands.\n";
  return false;
}

void Controller::HandlePostInit(const std::string &input) {
  if (input.empty())
    return;

  Command command = GetParsedInput(input);
  ExecuteCommand(command);
}

void Controller::run() {
  HandleView();
  std::string input;

  while (GetParsedInput(input).cliCommand != CLI_COMMAND::CLI_EXIT) {
    std::cout << "Enter Input: ";
    std::getline(std::cin, input);

    if (!isInitialized) {
      HandlePreInit(input);
    } else {
      HandlePostInit(input);
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

void Controller::ExitOS() {
  std::cout << "Stopping prosched...\n";
  std::_Exit(EXIT_SUCCESS);
}

Command Controller::GetParsedInput(const std::string &input) {
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

  if (commands.cliCommand == CLI_COMMAND::CLI_SCREEN_S) {
    commands.processName = trimmed[2];
    return commands;
  }

  return commands;
}

void Controller::ExecuteCommand(const Command &command) {
  try {
    switch (command.cliCommand) {
    case CLI_COMMAND::CLI_EXIT:
      ExitOS();
      break;

    case CLI_COMMAND::CLI_SCREEN_LS:
      if (this->scheduler->IsRunning()) {
        this->scheduler->PrintProcesses();
      } else {
        std::cout << "Start the scheduler before viewing processes, run "
                     "\"scheduler-start\"\n\n";
      }
      break;

    case CLI_COMMAND::CLI_SCREEN_S:
      // none for now
      break;

    case CLI_COMMAND::CLI_SCHEDULER_START:
      if (!this->scheduler->IsRunning()) {
        this->scheduler->Start();
      } else {
        this->scheduler->ResumeGenerating();
        std::cout << "Resumed process generation.\n\n";
      }
      break;

    case CLI_COMMAND::CLI_SCHEDULER_STOP:
      if (this->scheduler->IsRunning()) {
        this->scheduler->StopGenerating();
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

CLI_COMMAND
Controller::IdentifyCommand(const std::vector<std::string> &command) {
  if (command[0] == "screen") {
    if (command.size() < 2)
      return CLI_COMMAND::UNKNOWN;
    if (command[1] == "-ls") {
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