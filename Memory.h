#ifndef MEMORY_H
#define MEMORY_H
#include <iostream>

using namespace std;
#define MEMORY_SIZE 65536
#define KERNEL_SPACE_BOUND 4096
#define ALLOWED_FOR_USERS_ROUTINES 8
#define NOT_ALLOWED_FOR_USERS 15
#define ALLOWED_VALUE 60279
#define DATA_OUT 65280

class Memory
{
public:
    unsigned char operator[](unsigned short address);
    void write(unsigned char value, unsigned short address);
    void copy(unsigned short address, unsigned char toCopy[], int size);
    void systemWrite(unsigned short address, char value);
    static bool dataOutFull;
    static bool kernelTime;

private:
    static unsigned char memory[MEMORY_SIZE];
};
#endif
