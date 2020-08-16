#ifndef CANTLINK_H
#define CANTLINK_H
#include <iostream>

using namespace std;

class CantLinkException : public exception
{
public:
    const char *what() const throw() override
    {
        return "Linker error: cant link files";
    }
};
#endif