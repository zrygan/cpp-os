#include "Controller.h"
#include "view/View.h"

void Controller::initialize() {
    this->view = View();
}

void Controller::run() {
    while(true) {
        view.displayMenu()
    }
}