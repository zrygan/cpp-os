#include <iostream>
#include <stdio.h>

#include "config.h"
#include "context.h"
#include "controller/Controller.h"



int main() {
  printf("hello, world!\n");

  Controller controller;
  controller.run();
  return 0;
}