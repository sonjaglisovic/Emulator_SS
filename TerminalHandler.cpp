#include "TerminalHandler.h"

#include <unistd.h>
#include <termios.h>
#include <poll.h>

bool TerminalHandler::getch()
{
    char buf = 0;
    struct pollfd stdin_poll = {.fd = STDIN_FILENO, .events = POLLIN | POLLRDBAND | POLLRDNORM | POLLPRI};
    if (poll(&stdin_poll, 1, 0) == 1)
    {
        if (read(0, &buf, 1) <= 0)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    memory.write(buf, DATA_IN);
    return true;
}

bool TerminalHandler::checkIf()
{
    if (Memory::dataOutFull)
    {
        cout << (char)memory[DATA_OUT] << " ";
        Memory::dataOutFull = false;
    }
    if (getch())
        return true;
    return false;
}