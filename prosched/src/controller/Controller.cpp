#include <stdio.h>
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

#include "Controller.h"
#include "config.h"
#include "context.h"
#include "src/scheduler/Scheduler.h"

void Controller::initialize() { 
  ConfigStruct *cs = fromFile();

  if (cs == nullptr) {
    std::cout << "Failed to find config file\n";
    return;
  }

  this->ctx = AlgoContext::buildConfig(cs);
  this->scheduler = new prosched::Scheduler(this->ctx);
  this->isInitialized= true;


    /* <RV @zrygan> ===========
    Shouldn't this be outside this function?

    Also the function view.DisplayMenu() kind of doesn not display
    the menu but instead shows a start up page. I suggest some changes
    implemented in the naming and organization of stuff since we are
    using MVC.

    @outformilks @chuae
    <RV @zrygan> =========== */
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
    while(input != "exit"){
      std::cout << "Enter Input: ";
      std::getline(std::cin, input);

      if (!isInitialized) {
        /* <RV @zrygan> ==========
        There must be an easier way to always check if it's initialized
        perhaps move this outside the code. Everything below SHOUD NOT RUN
        if it isn't initialized. (Maybe move this to the main loop/outside this function).
        <RV @zrygan> ========== */
        if (input == "initialize"){
          
            initialize();

        } else {
          std::cout << "\nType \"initialize\" to access commands\n";
        }
      } else {
        /* <RV @zrygan> ==========
        Refer to the review comment at the top.
        <RV @zrygan> ========== */
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