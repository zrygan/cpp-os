#include <iostream>
#include <stdio.h>

#include "Controller/Controller.h"
#include "prosched/config.h"

using namespace std;

int main() {
  printf("hello, world!\n");

  ConfigStruct *cs = fromFile();

  if (cs) {
    std::cout << cs->batch_process_freq;
  }

  Controller controller;
  controller.initialize();
  controller.run();
  return 0;
}