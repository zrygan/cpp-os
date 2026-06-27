#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
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

  if (input == "clear") {
    view.DisplayClear();
    view.DisplayMenu();
    return true;
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

  if (commands.cliCommand == CLI_COMMAND::CLI_SCREEN_S ||
      commands.cliCommand == CLI_COMMAND::CLI_SCREEN_R) {
    if (trimmed.size() >= 3) {
      commands.processName = trimmed[2];
    }
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

    case CLI_COMMAND::CLI_CLEAR:
      view.DisplayClear();
      view.DisplayCommands();
      break;

    case CLI_COMMAND::CLI_SCREEN_LS:
      this->scheduler->ShowScreenProcesses();
      // this->scheduler->PrintProcesses();
      break;

    case CLI_COMMAND::CLI_SCREEN_S: {
      if (command.processName.empty()) {
        std::cout << "screen -s: a process name is required.\n\n";
        break;
      }
      prosched::Process *p =
          this->scheduler->CreateNamedProcess(command.processName);
      this->scheduler->AddProcess(p);
      EnterProcessScreen(p);
      break;
    }

    case CLI_COMMAND::CLI_SCREEN_R: {
      if (command.processName.empty()) {
        std::cout << "screen -r: a process name is required.\n\n";
        break;
      }
      prosched::Process *p =
          this->scheduler->FindProcessByName(command.processName);
      if (p == nullptr) {
        std::cout << "Process " << command.processName << " not found.\n\n";
      } else {
        EnterProcessScreen(p);
      }
      break;
    }

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
      PrintReportUtil();
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

void Controller::EnterProcessScreen(prosched::Process *p) {
  std::cout << "\033[?1049h" << std::flush;
  std::string cmd;

  while (true) {
    std::cout << "root:\\> ";
    std::getline(std::cin, cmd);

    size_t start = cmd.find_first_not_of(" \t\r\n");
    size_t end = cmd.find_last_not_of(" \t\r\n");
    cmd = (start == std::string::npos) ? "" : cmd.substr(start, end - start + 1);

    if (cmd == "process-smi") {
      std::cout << "\nProcess Name: " << p->GetName() << "\n";
      std::cout << "ID: " << p->GetPID() << "\n\n";

      auto logs = p->GetLogs();
      for (const auto &log : logs) {
        std::cout << log << "\n";
      }

      if (p->IsFinished()) {
        std::cout << "\nFinished!\n";
      }
      std::cout << "\n";
    } else if (cmd == "exit") {
      std::cout << "\033[?1049l" << std::flush;
      break;
    } else if (!cmd.empty()) {
      std::cout << "Unknown command. Use 'process-smi' or 'exit'.\n\n";
    }
  }
}

void Controller::PrintReportUtil() {
  this->scheduler->PrintProcesses(std::cout);

  std::ofstream file("csopesy-log.txt");
  if (file.is_open()) {
    this->scheduler->PrintProcesses(file);
    file.close();
    std::cout << "Report saved to csopesy-log.txt\n\n";
  } else {
    std::cerr << "Warning: could not write csopesy-log.txt\n";
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
    } else if (command[1] == "-r") {
      return CLI_COMMAND::CLI_SCREEN_R;
    } else {
      return CLI_COMMAND::UNKNOWN;
    }
  } else if (command[0] == "exit") {
    return CLI_COMMAND::CLI_EXIT;
  } else if (command[0] == "clear") {
    return CLI_COMMAND::CLI_CLEAR;
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