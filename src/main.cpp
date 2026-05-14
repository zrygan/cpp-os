#include <stdio.h>

#include "config.h"

using namespace std;

int main()
{
    printf("hello, world!\n");

    ConfigStruct *cs = fromFile();

    std::cout << cs->batch_process_freq;
    return 0;
}