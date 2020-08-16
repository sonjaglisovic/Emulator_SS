#ifndef DS_H
#define DS_H
#include <iostream>
#include <list>
using namespace std;

#define PC_RELATIVE "R_PC16"
#define APSOLUTE "R_16"

struct RelocationRecord
{
    unsigned short address;
    string type;
    int entryNumber;
    RelocationRecord(unsigned short address, string type, int entryNumber) : address(address), type(type), entryNumber(entryNumber) {}
};

struct Entry
{
    string name;
    string section;
    int size;
    int value;
    unsigned char *code;
    list<struct RelocationRecord *> relocationRecords;
    Entry(string name, string section, int size, int value) : name(name), section(section), size(size), value(value)
    {
        if (size > 0)
        {
            code = new unsigned char[size];
        }
    }
};

struct FileInfo
{
    list<struct Entry *> allEntries;
};

struct LinkerData
{
    list<struct FileInfo *> linkerData;
};
#endif