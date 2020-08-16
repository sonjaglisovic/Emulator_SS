#ifndef SYSTEM_H
#define SYSTEM_H
#include "Linker.h"
#include "Processor.h"
#include <fstream>
#include <unistd.h>
#include <termios.h>

class System
{

public:
    void linkFiles(int argc, char *argv[])
    {
        main = linker.linkFiles(argc, argv);
    }
    void executeProgram()
    {
        processor = new Processor(main);
        processor->executeProgram();
    }
    void initializeSystem()
    {
        initializeTerminal();
        std::cout.setf(std::ios::unitbuf);
    }
    ~System()
    {
        recoverTerminal();
    }

private:
    struct termios old;
    void initializeTerminal()
    {
        old = {0};
        fflush(stdout);
        if (tcgetattr(0, &old) < 0)
            perror("ERROR");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
            perror("Error");
    }
    void recoverTerminal()
    {
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
            perror("ERROR");
    }

    Processor *processor;
    static Linker linker;
    Memory memory;
    unsigned short main;
};
Linker System::linker;
#endif