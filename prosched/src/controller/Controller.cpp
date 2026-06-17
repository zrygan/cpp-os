#include <stdio.h>
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

#include "Controller.h"
#include "config.h"
#include "context.h"
#include "src/scheduler/Scheduler.h"

using namespace std;

void Controller::initialize() { 
  ConfigStruct *cs = fromFile();

  if (cs == nullptr) {
    std::cout << "Failed to find config file\n";
    return;
  }

  this->ctx = AlgoContext::buildConfig(cs);
  this->scheduler = new Scheduler(this->ctx);
  this->isInitialized= true;

  std::cout << "Initialized successfully.\n\n";
  std::cout << "List of accessible commands:\n"
            << "- exit\n"
            << "- screen -s <process name>\n"
            << "- screen -ls\n"
            << "- scheduler-start\n"
            << "- scheduler-stop\n"
            << "- report-util\n\n";
}

void Controller::run() {
  this->view = View(); 
  view.DisplayMenu();
  std::string input;

    // while command not exit keep i/o open
    // wow this is messy lols
    while(input != "exit"){
      std::cout << "Enter Input: ";
      std::cin >> input;

      if (!isInitialized) {
        if (input == "initialize"){
          
            initialize();
            
        } else {
          std::cout << "\nType \"initialize\" to access commands\n";
        }
      } else {
        if (input == "scheduler-start") {
          this->scheduler->Start();
        } else if (input == "scheduler-stop") {
          if (this->scheduler->IsRunning() == true) {
            this->scheduler->Stop();
          } else {
            std::cout << "\nScheduler has not started, run \"scheduler-start\" to start\n\n";
          }
        } else if (input == "screen -ls") {
          this->scheduler->PrintProcesses();
        } else if (input == "screen -s") { // append process name

        } else if (input == "report-util") {

        } else {
          std::cout << "\nNot a valid command\n";
        }
      }
    }
}