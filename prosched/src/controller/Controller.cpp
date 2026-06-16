#include <stdio.h>
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

#include "Controller.h"

using namespace std;

void Controller::initialize() { this->view = View(); }

void Controller::run() {
    view.DisplayMenu();
    std::string input;

    // while command not exit keep i/o open
    while(input != "exit"){
      std::transform(input.begin(), input.end(), input.begin(),
      [](unsigned char c){ return std::tolower(c); });

      std::cout << "Enter Input: ";
      std::cin >> input;

      // actions here
    }
}