#include <iostream>
#include <stdio.h>

#include "config.h"
#include "context.h"
#include "controller/Controller.h"

using namespace std;

int main() {
  printf("hello, world!\n");

  ConfigStruct *cs = fromFile();
  // AlgoContext ctx = AlgoContext::buildConfig(cs);
  
  if (cs) {
    std::cout << cs->batch_process_freq;
  }

  Controller controller;
  controller.initialize();
  controller.run();
  return 0;
}