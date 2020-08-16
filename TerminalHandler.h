#ifndef TERMINAL_H
#define TERMINAL_H
#include "Memory.h"
#include <iostream>

using namespace std;
#define DATA_IN 65282

class TerminalHandler
{
public:
    bool checkIf();
    bool getch();

private:
    Memory memory;
};
#endif