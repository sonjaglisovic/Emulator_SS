#include "TimerHandler.h"

TimerHandler::TimerHandler()
{
    prevInterruput = clock();
    hashMap.insert(pair<int, long>(0, 500));
    hashMap.insert(pair<int, long>(1, 1000));
    hashMap.insert(pair<int, long>(2, 1500));
    hashMap.insert(pair<int, long>(3, 2000));
    hashMap.insert(pair<int, long>(4, 5000));
    hashMap.insert(pair<int, long>(5, 10000));
    hashMap.insert(pair<int, long>(6, 30000));
    hashMap.insert(pair<int, long>(7, 60000));
}

bool TimerHandler::checkIfTime()
{
    long double newInterrupt = clock();
    long double timePassed = (double)(clock() - prevInterruput) / CLOCKS_PER_SEC;
    timePassed *= 1000;
    long double shouldPass;
    map<int, long>::iterator it = hashMap.find(memory[TIMER_CFG] & 7);
    if (it != hashMap.end())
    {
        shouldPass = it->second;
    }
    if (timePassed >= shouldPass)
    {
        prevInterruput = newInterrupt;
        return true;
    }
    return false;
}