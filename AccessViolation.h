#ifndef ACCESS_H
#define ACCESS_H
#include <iostream>

using namespace std;

class AccessViolationException : public exception
{
public:
    const char *what() const throw() override
    {
        return "Access violation";
    }
};
#endif