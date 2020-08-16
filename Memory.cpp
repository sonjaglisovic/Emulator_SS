#include "Memory.h"
#include "AccessViolation.h"

unsigned char Memory::memory[] = {0};
bool Memory::dataOutFull = false;
bool Memory::kernelTime = false;

unsigned char Memory::operator[](unsigned short address)
{
    return memory[address];
}

void Memory::write(unsigned char value, unsigned short address)
{
    if (address == DATA_OUT)
        dataOutFull = true;
    memory[address] = value;
}

void Memory::copy(unsigned short address, unsigned char toCopy[], int size)
{
    int j = 0;
    for (int i = address; i < (address + size); i++)
    {
        memory[i] = toCopy[j];
        j++;
    }
}
void Memory::systemWrite(unsigned short address, char value)
{
    memory[address] = value;
}