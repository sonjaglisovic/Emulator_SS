#ifndef CANTOPEN_H
#define CANTOPEN_H
#include <iostream>

using namespace std;

class CantOpenFilesException : public exception
{
public:
    const char *what() const throw() override
    {
        return "Linker error: Can't open file";
    }
};
#endif