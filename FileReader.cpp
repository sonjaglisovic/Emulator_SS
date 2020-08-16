#include "FileReader.h"

void FileReader::readInputFiles(char *argv[], int argc, struct LinkerData *linkerData)
{
    this->linkerData = linkerData;
    for (int i = 1; i < (argc + 1); i++)
    {
        ifstream file(argv[i]);
        if (!file.is_open())
        {
            throw new CantOpenFilesException();
        }
        struct FileInfo *fileInfo = new FileInfo();
        readEntries(file, fileInfo);
        if (file.eof())
        {
            file.close();
            continue;
        }
        readRelocationRecords(file, fileInfo);
        file.close();
        linkerData->linkerData.push_back(fileInfo);
    }
}

void FileReader::readEntries(ifstream &file, struct FileInfo *fileInfo)
{

    string line;
    getline(file, line);
    getline(file, line);

    while (line.size() == 0 && !file.eof())
    {
        getline(file, line);
    }
    int numOfEntries = 0;
    int numOfGlobal = 0;
    char firstLine = 1;
    while (line.size() != 0 && !file.eof())
    {
        if (!firstLine)
        {
            getline(file, line);
            if (line.size() == 0)
            {
                return;
            }
        }
        else
        {
            firstLine = 0;
        }

        numOfEntries++;
        removeSpaces(line);
        int position = line.find_first_of(' ');
        string name = line.substr(0, position);
        line.erase(0, position + 1);
        position = line.find_first_of(' ');
        string section = line.substr(0, position);
        line.erase(0, position + 1);
        position = line.find_first_of(' ');
        string sValue = line.substr(0, position);
        line.erase(0, position + 1);
        position = line.find_first_of(' ');
        string sSize = line.substr(0, position);
        line.erase(0, position + 1);
        string global = line.substr(0, 1);
        if (global.compare("g") != 0)
        {
            continue;
        }
        numOfGlobal++;
        hashMap.insert(pair<int, int>(numOfEntries, numOfGlobal));
        int size = stoi(sSize);
        int value = stoi(sValue, 0, 16);
        struct Entry *entry = new Entry(name, section, size, value);
        fileInfo->allEntries.push_back(entry);
    }
}
void FileReader::removeSpaces(string &line)
{
    std::string::iterator new_end =
        std::unique(line.begin(), line.end(),
                    [=](char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); });
    line.erase(new_end, line.end());
}

void FileReader::readRelocationRecords(ifstream &file, struct FileInfo *fileInfo)
{
    string line;
    getline(file, line);
    while (1)
    {
        while (line.size() == 0 && !file.eof())
        {
            getline(file, line);
        }
        if (file.eof())
            return;
        regex matchRecord("#rel_.*");
        regex matchCode("<.*>");
        string section;
        if (regex_match(line, matchRecord))
        {
            line.erase(0, 5);
            section = line;
        }
        else
        {
            if (regex_match(line, matchCode))
            {
                line.erase(0, 1);
                int position = line.find_first_of(">");
                line.erase(position);
                string section = line;
                readCode(file, fileInfo, section);
                return;
            }
            else
            {
                return;
            }
        }
        struct Entry *myEnry = findMyEntry(fileInfo, section);
        getline(file, line);
        getline(file, line);
        while (line.size() != 0 && !file.eof())
        {
            getline(file, line);
            if (line.size() == 0)
                break;
            removeSpaces(line);
            int position = line.find_first_of(' ');
            string sAddress = line.substr(0, position);
            line.erase(0, position + 1);
            int address = stoi(sAddress, 0, 16);
            position = line.find_first_of(' ');
            string type = line.substr(0, position);
            line.erase(0, position + 1);
            int symb = stoi(line);
            symb = hashMap.find(symb)->second;
            struct RelocationRecord *record = new RelocationRecord(address, type, symb);
            myEnry->relocationRecords.push_back(record);
        }
    }
}

void FileReader::readCode(ifstream &file, struct FileInfo *fileInfo, string section)
{
    string line;

    while (1)
    {
        getline(file, line);
        struct Entry *myEntry = findMyEntry(fileInfo, section);
        int position;
        int index = 0;
        while ((position = line.find(' ')) != std::string::npos)
        {
            string code = line.substr(0, position);
            line.erase(0, position + 1);
            unsigned char myCode = (char)stoi(code, 0, 16);
            myEntry->code[index++] = myCode;
        }
        getline(file, line);
        while (line.size() == 0 && !file.eof())
        {
            getline(file, line);
        }
        if (file.eof())
            return;
        line.erase(0, 1);
        int pos = line.find(">");
        line.erase(pos);
        section = line;
    }
}

struct Entry *FileReader::findMyEntry(struct FileInfo *fileInfo, string sectionName)
{
    list<struct Entry *>::iterator symbols;
    for (symbols = fileInfo->allEntries.begin(); symbols != fileInfo->allEntries.end(); symbols++)
    {
        if ((*symbols)->section.compare(sectionName) == 0)
            return (*symbols);
    }
}
