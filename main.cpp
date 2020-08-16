#include "System.h"

int main(int argc, char *argv[])
{
    try
    {
        System system;
        system.initializeSystem();
        system.linkFiles(argc, argv);
        system.executeProgram();
    }
    catch (char const *exc)
    {
        cout << exc << endl;
    }
    catch (exception *e)
    {
        cout << e->what() << endl;
    }
}