#include <iostream>
#include <stdio.h>

#include "Config.h"
#include "Context.h"
#include "controller/Controller.h"

int main() {
  printf("hello, world!\n");

  Controller controller;
  controller.run();
  return 0;
}