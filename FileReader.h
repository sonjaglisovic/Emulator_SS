#ifndef FILEREADER_H
#define FILEREADER_H
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <fstream>
#include "DataStructures.h"
#include "CantOpenFile.h"
using namespace std;

class FileReader
{
public:
    void readInputFiles(char *fileNames[], int bound, struct LinkerData *linkerData);

private:
    void readEntries(ifstream &file, struct FileInfo *fileInfo);
    void readRelocationRecords(ifstream &file, struct FileInfo *fileInfo);
    void removeSpaces(string &line);
    struct Entry *findMyEntry(struct FileInfo *fileInfo, string sectionName);
    void readCode(ifstream &file, struct FileInfo *fileInfo, string section);
    map<int, int> hashMap;
    struct LinkerData *linkerData;
};
#endif