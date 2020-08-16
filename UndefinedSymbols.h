#ifndef UNDEFINED_H
#define UNDEFINED_H
#include <iostream>

using namespace std;

class UndefinedSymbolsException : public exception
{
public:
    const char *what() const throw() override
    {
        return "Linker error: Some symbols are declared but not defined";
    }
};
#endif