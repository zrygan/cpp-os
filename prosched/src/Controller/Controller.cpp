#include "Controller.h"

void Controller::initialize() { this->view = View(); }

void Controller::run() {
  while (true) {
    view.displayMenu();
  }
}