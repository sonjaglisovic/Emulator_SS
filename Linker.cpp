#include "Linker.h"
#include "CantLink.h"
#include "UndefinedSymbols.h"
#include "MultipleDefinition.h"

FileReader *Linker::fileReader = new FileReader();
int Linker::countArguments(int argc, char *argv[])
{
    int count = 0;
    for (int i = 1; i < argc; i++)
    {
        regex placeDetector("-place=.*");
        if (regex_match(argv[i], placeDetector))
        {
            break;
        }
        count++;
    }
    return count;
}

unsigned short Linker::linkFiles(int argc, char *argv[])
{
    int numOfArguments = countArguments(argc, argv);
    if (numOfArguments == 0)
    {
        throw new CantLinkException();
    }
    fileReader->readInputFiles(argv, numOfArguments, linkerData);
    if (checkIfTooBig())
        throw new CantLinkException();
    if (checkIfNoMain())
        throw new UndefinedSymbolsException();
    if (checkForMultipleDefinition())
        throw new MultipleDefinitionException();
    if (checkForUndefinedSimbols())
        throw new UndefinedSymbolsException();
    if (!considerPlaceArgumentsAndLinkFiles(argc, numOfArguments + 1, argv))
        throw new CantLinkException();

    solveUndefinedSimbols();
    unsigned short main;

    loadMemory();
    solveRelocationRecords();
    main = findMain();
    return main;
}
unsigned short Linker::findMain()
{
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    bool mainFound = false;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            if ((*data)->name.compare("main") == 0)
            {

                return (*data)->value;
            }
        }
    }
}

bool Linker::checkIfTooBig()
{
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    int size = 0;
    bool mainFound = false;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            size += (*data)->size;
        }
    }
    return size > (ALLOWED_VALUE - KERNEL_SPACE_BOUND);
}

bool Linker::checkIfNoMain()
{
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    bool mainFound = false;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            if ((*data)->name.compare("main") == 0)
            {
                return false;
            }
        }
    }
    return true;
}
bool Linker::checkForMultipleDefinition()
{
    map<string, string> hashMap;
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            map<string, string>::iterator it = hashMap.find((*data)->name);
            if ((*data)->section.compare("UND") != 0 && (*data)->size == 0)
            {
                if (it == hashMap.end())
                    hashMap.insert(pair<string, string>((*data)->name, "there"));
                else
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Linker::checkForUndefinedSimbols()
{
    map<string, string> undefined;
    map<string, string> defined;
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            if ((*data)->section.compare("UND") == 0)
            {
                map<string, string>::iterator it = defined.find((*data)->name);
                if (it != defined.end())
                    continue;
                else
                {
                    undefined.insert(pair<string, string>((*data)->name, "und"));
                }
            }
            else
            {
                undefined.erase((*data)->name);
                defined.insert(pair<string, string>((*data)->name, "defined"));
            }
        }
    }
    if (!undefined.empty())
    {
        return true;
    }
    return false;
}

bool Linker::considerPlaceArgumentsAndLinkFiles(int argc, int start, char *argv[])
{
    int highestAddress = 0;
    map<string, int> hashMap = linkSections();
    map<int, int> locations;
    for (int i = start; i < argc; i++)
    {
        string argument(argv[i]);
        regex findPlace("-place.*");
        if (regex_match(argument, findPlace))
        {
            argument.erase(0, 7);
            int position = argument.find("@");
            string section = argument.substr(0, position);
            argument.erase(0, position + 1);
            int place = stoi(argument, 0, 16);
            map<string, int>::iterator it = hashMap.find(section);
            if (it == hashMap.end())
            {
                throw "Linker error: Wrong place argument, specified section doesnt exist";
            }
            map<int, int>::iterator it1 = locations.begin();
            while (it1 != locations.end())
            {
                if ((it1->first + it1->second) >= place && it1->first < (place + it->second))
                    return false;
                it1++;
            }
            if (it->second + place > ALLOWED_VALUE)
            {
                return false;
            }
            locations.insert(pair<int, int>(place, it->second));
            if (it->second + place > highestAddress)
                highestAddress = it->second + place;
            hashMap.erase(section);
            placeSection(place, section);
            sectionStartAddress.insert(pair<string, int>(section, place));
            updateAllRecords(place, section);
        }
    }
    map<string, int>::iterator it = hashMap.begin();
    while (it != hashMap.end())
    {
        if (it->second + highestAddress > ALLOWED_VALUE)
        {
            return false;
        }
        placeSection(highestAddress, it->first);
        updateAllRecords(highestAddress, it->first);
        highestAddress += it->second;
        it++;
    }
    return true;
}

map<string, int> Linker::linkSections()
{
    map<string, int> hashMap;
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    bool mainFound = false;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            if ((*data)->size > 0 && (*data)->name.compare("UND") != 0 && (*data)->name.compare("EQU") != 0)
            {
                map<string, int>::iterator it = hashMap.find((*data)->name);
                if (it != hashMap.end())
                {
                    int currentSectionSize = it->second;
                    addStartSectionValueToAll((*files)->allEntries, currentSectionSize, (*data)->name);
                    updateRelocationRecords((*data), currentSectionSize);
                    it->second += (*data)->size;
                }
                else
                {
                    hashMap.insert(pair<string, int>((*data)->name, (*data)->size));
                }
            }
        }
    }
    map<string, int>::iterator it = hashMap.begin();
    while (it != hashMap.end())
    {
        sectionSizes.insert(pair<string, int>(it->first, it->second));
        it++;
    }
    return hashMap;
}

void Linker::addStartSectionValueToAll(list<struct Entry *> AllEntries, int startValue, string sectionName)
{
    list<struct Entry *>::iterator entries;
    for (entries = AllEntries.begin(); entries != AllEntries.end(); ++entries)
    {
        if ((*entries)->section.compare(sectionName) == 0)
        {
            (*entries)->value += startValue;
        }
    }
}

void Linker::updateRelocationRecords(struct Entry *entry, int size)
{
    list<struct RelocationRecord *>::iterator record;
    for (record = entry->relocationRecords.begin(); record != entry->relocationRecords.end(); ++record)
    {
        (*record)->address += size;
    }
}

void Linker::placeSection(int place, string section)
{
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            if ((*data)->section.compare(section) == 0)
            {
                (*data)->value += place;
            }
        }
    }
}

void Linker::updateAllRecords(int place, string section)
{
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            if ((*data)->name.compare(section) == 0)
            {
                list<struct RelocationRecord *>::iterator record;
                for (record = (*data)->relocationRecords.begin(); record != (*data)->relocationRecords.end(); record++)
                {
                    (*record)->address += place;
                }
            }
        }
    }
}
//solve equ
void Linker::solveUndefinedSimbols()
{
    map<string, int> defined;
    map<string, struct Entry *> undefined;
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    regex matchEqu("EQU(.*)");
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            bool equSimbol = false;

            string equDependencies;
            if (regex_match((*data)->section, matchEqu))
            {
                equSimbol = true;
                equDependencies = (*data)->section;
                equDependencies.erase(0, 4);
                int pos = equDependencies.find(")");
                equDependencies.erase(pos);
            }

            if ((*data)->section.compare("UND") == 0 || equSimbol)
            {
                if (!equSimbol)
                {
                    map<string, int>::iterator it = defined.find((*data)->name);
                    if (it != defined.end())
                    {
                        (*data)->value = it->second;
                    }
                    else
                    {
                        undefined.insert(pair<string, struct Entry *>((*data)->name, (*data)));
                    }
                }
                else
                {
                    map<string, int>::iterator it = defined.find(equDependencies);
                    if (it != defined.end())
                    {
                        (*data)->value += it->second;
                    }
                    else
                    {
                        undefined.insert(pair<string, struct Entry *>(equDependencies, (*data)));
                    }
                }
            }
            else
            {
                map<string, struct Entry *>::iterator it1 = undefined.find((*data)->name);
                while (it1 != undefined.end())
                {
                    if (regex_match(it1->second->section, matchEqu))
                        it1->second->value += (*data)->value;
                    else
                        it1->second->value = (*data)->value;
                    if (it1->second->name.compare((*data)->name) != 0)
                    {
                        defined.insert(pair<string, int>(it1->second->name, it1->second->value));
                    }
                    it1++;
                }
                undefined.erase((*data)->name);
                defined.insert(pair<string, int>((*data)->name, (*data)->value));
            }
        }
    }
    int number = undefined.size();
    while (number > 0 && !undefined.empty())
    {
        map<string, struct Entry *>::iterator it = undefined.begin();
        while (it != undefined.end())
        {
            map<string, int>::iterator it1 = defined.find(it->first);
            if (it1 != defined.end())
            {
                if (regex_match(it->second->section, matchEqu))
                    it->second->value += it1->second;
                else
                {
                    it->second->value = it1->second;
                }
                if (it->first.compare(it->second->name) != 0)
                {
                    defined.insert(pair<string, int>(it->second->name, it->second->value));
                }
                undefined.erase(it->first);
            }
            it++;
        }
        number--;
    }
    if (!undefined.empty())
    {
        throw new CantLinkException();
    }
}

void Linker::loadMemory()
{
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            if ((*data)->size > 0)
            {
                memory.copy((*data)->value, (*data)->code, (*data)->size);
            }
        }
    }
}

void Linker::solveRelocationRecords()
{
    list<struct FileInfo *>::iterator files;
    list<struct Entry *>::iterator data;
    for (files = linkerData->linkerData.begin(); files != linkerData->linkerData.end(); files++)
    {
        for (data = (*files)->allEntries.begin(); data != (*files)->allEntries.end(); data++)
        {
            if ((*data)->size >= 0)
            {
                list<struct RelocationRecord *>::iterator record;
                for (record = (*data)->relocationRecords.begin(); record != (*data)->relocationRecords.end(); record++)
                {
                    unsigned short address = (*record)->address;
                    unsigned short number = findNthEntry((*files)->allEntries, (*record)->entryNumber);
                    unsigned short finalValue;
                    if ((*record)->type.compare(TIP_1) == 0)
                    {
                        finalValue = number;
                    }
                    else
                    {
                        finalValue = number - address;
                    }
                    short prevValue = 0;
                    prevValue |= memory[address];

                    prevValue |= (memory[address + 1] << 8);
                    finalValue += prevValue;
                    memory.write(finalValue & 255, address);
                    memory.write((finalValue >> 8) & 255, address + 1);
                }
            }
        }
    }
}

unsigned short Linker::findNthEntry(list<struct Entry *> AllEntries, int n)
{
    list<struct Entry *>::iterator data;
    int counter = 0;
    for (data = AllEntries.begin(); data != AllEntries.end(); data++)
    {
        if (++counter == n)
        {
            return (*data)->value;
        }
    }
}