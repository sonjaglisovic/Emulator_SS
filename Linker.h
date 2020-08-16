#ifndef LINKER_H
#define LINKER_H
#include <iostream>
#include "FileReader.h"
#include "Memory.h"

using namespace std;

#define ALLOWED_VALUE 60279
#define TIP_1 "R_16"
#define TIP_2 "R_PC16"

class Linker
{
public:
    unsigned short linkFiles(int argc, char *argv[]);

private:
    int countArguments(int argc, char *argv[]);
    bool checkIfTooBig();
    bool checkIfNoMain();
    bool checkForMultipleDefinition();
    bool checkForUndefinedSimbols();
    bool considerPlaceArgumentsAndLinkFiles(int argc, int start, char *argv[]);
    void placeSection(int place, string section);
    map<string, int> linkSections();
    void addStartSectionValueToAll(list<struct Entry *> AllEntries, int startValue, string sectionName);
    void updateRelocationRecords(struct Entry *entry, int size);
    void updateAllRecords(int place, string section);
    void solveUndefinedSimbols();
    void loadMemory();
    void solveRelocationRecords();
    unsigned short findMain();
    unsigned short findNthEntry(list<struct Entry *> AllEntries, int n);
    struct LinkerData *linkerData = new LinkerData();
    Memory memory;
    map<string, int> sectionSizes;
    map<string, int> sectionStartAddress;
    static FileReader *fileReader;
};

#endif