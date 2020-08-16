#ifndef MULTIPLE_H
#define MULTIPLE_H
#include <iostream>

using namespace std;

class MultipleDefinitionException : public exception
{
public:
    const char *what() const throw() override
    {
        return "Linker error: Multiple definition";
    }
};
#endif