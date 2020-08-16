#ifndef TIMER_H
#define TIMER_H
#include <ctime>
#include <iostream>
#include <map>
#include "Memory.h"
#define TIMER_CFG 65296

class TimerHandler
{
public:
    TimerHandler();
    bool checkIfTime();

private:
    long double prevInterruput;
    map<int, long> hashMap;
    Memory memory;
};
#endif