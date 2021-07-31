#include <stdio.h>
#include <iostream>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <iomanip>
#include <fstream>
#include <bitset>
#include "pagetable.h"

using namespace std;

#define bitSize 32

//=========================================================================================

/*
* The countLevels function determines the number of levels for PageTable. The function reads the
* user-provided arguments from the end of the command line to the beginning and counts the number
* of levels given. Returns the number of levels that the user defined.
*/
int countLevels(int argc, char **argv) {
    //go from end, check if argument is a number if it is increment level count
    int numLevels = 0;
    for (int i = argc - 1; i > 1; i--) {
        if (!isdigit(argv[i][0])) {
            break;
        }
        numLevels++;
    }
    return numLevels;
}

//=========================================================================================

/*
* The storeLevels function takes in the levelCount, levels pointer and the command line arguments to retrieve the
* user-specified size for each level.
*/
void storeLevels(int levelCount, int argc, char **argv, unsigned int* levels) {
    int idx = levelCount - 1;
    for(int i = argc - 1; i >= argc - levelCount; i--) {
        levels[idx] = atoi(argv[i]);
        idx--;
    }
}

//=========================================================================================

/*
* The calcBitMask function takes in the levelCount, levelSizes and bitMask pointer to calculate the bitMask for each
* level in the PageTable.
*/
void calcBitMask(int levelCount, unsigned int *levelSizes, unsigned int *bitMask) {
    int idx = bitSize - 1;
    for(int i = 0; i < levelCount; i++) {
        bitset<bitSize> temp_bset;
        for(int j = idx; j > idx - (*(levelSizes + i)); j--) {
            temp_bset[j] = 1;
        }
        idx = idx - (*(levelSizes + i));
        bitMask[i] = (unsigned int) temp_bset.to_ulong();
    }
}

//=========================================================================================

/*
* The calcShiftArray function takes in the levelCount, levelSizes and the shiftArray to calculate how much is needed to
* shift for each level.
*/
void calcShiftArray(int levelCount, unsigned int *levelSizes, unsigned int *shiftArray) {
    int idx = 0;
    int temp = 32;
    for(int i=0; i < levelCount; i++) {
        temp = temp - *(levelSizes + i);
        shiftArray[idx] = temp;
        idx++;
    }
}

//=========================================================================================

/*
* The calcEntryCount function takes in the levelCount, levelSizes and entries to calculate the number of possible
* pages for each level.
*/
void calcEntryCount(int levelCount, unsigned int *levelSizes, unsigned int *entries) {
    int idx = 0;
    for(int i=0; i < levelCount; i++) {
        entries[idx] = pow(2, *levelSizes);
        idx++;
        levelSizes++;
    }
}

//=========================================================================================

/*
* The LogicalToPage function takes in the LogicalAddress, Mask and Shift for a page and returns the virtual page
* number.
*/
unsigned int LogicalToPage(unsigned int LogicalAddress, unsigned int Mask, unsigned int Shift) {
    unsigned int page = Mask & LogicalAddress;
    page = page >> Shift; //virtual page number
    return  page;
}

//=========================================================================================

/*
* The creatMapPtr function creates a new Map and points to it. Initializes each frame in the map to invalid (0).
*/
Map createMapPtr() {
    Map *mapPtr = new Map();
    mapPtr -> valid = 0;
    return *mapPtr;
}

//=========================================================================================

/*
* The createNextLevel function creates a new Level and points to it. Initializes each entry to NULL.
*/
Level *createNextLevel(PageTable *pageTable, int current_depth, int levelCount) {
    Level *level = new Level();
    level -> depth = 1 + current_depth;
    level -> LevelCount = levelCount;
    
    if(level->depth == levelCount-1 && pageTable->AddressProcessed == 0) { //everything in here only will run once
        level -> mapPtr = new Map[pageTable->EntryCount[level->depth]];
        for(int i = 0; i < pageTable->EntryCount[level->depth]; i++) {
            level -> mapPtr[i] = createMapPtr();
        }
        pageTable->mapPtr = level->mapPtr;
    }
    else if(level->depth != levelCount-1) {
        level -> nextLevelPtr = new Level*[pageTable->EntryCount[level->depth]];
        for(int i = 0; i < pageTable->EntryCount[level->depth]; i++) {
            level -> nextLevelPtr[i] = NULL;
        }
    }
    else {
        level -> mapPtr = pageTable->mapPtr;
    }
    level -> tablePtr = pageTable;
    return level;
}

//=========================================================================================

/*
* The PageLookup function takes in the PageTable and the LogicalAddress and returns the appropriate entry of the page
* table, or null if it is not found.
*/
Map *PageLookup(PageTable *PageTable, unsigned int LogicalAddress) {
    if(PageTable -> rootNodePtr == NULL)
        return NULL;
    Level *level = (PageTable -> rootNodePtr);
    for(int i = 0; i < PageTable -> LevelCount; i++) {
        if(i == (PageTable -> LevelCount) - 1) {//if we're at the last level look at the map
            int index = (int) LogicalToPage(LogicalAddress, *((PageTable->BitMaskArray) + i), *((PageTable->ShiftArray) + i));
            if(level->mapPtr[index].valid == 0)
                return NULL;
            else {
                Map *current_map = level->mapPtr;
                return current_map;
            }
        }
        else { //keep looking for the next level
            int index = (int) LogicalToPage(LogicalAddress, *((PageTable->BitMaskArray) + i), *((PageTable->ShiftArray) + i));
            if(level->nextLevelPtr[index] == NULL)
                return NULL;
            level = level->nextLevelPtr[index];
        }
    }
    return nullptr;
}

//=========================================================================================

/*
* The PageInsert function takes in the PageTable, LogicalAddress and the frame index and is used to add new entries to the page table when we discover that a page has not yet been
* allocated, or PageLookup returns NULL.
*/
void PageInsert(PageTable *PageTable, unsigned int LogicalAddress, int frame) {
    if(PageTable->rootNodePtr != NULL)
        PageInsert(PageTable->rootNodePtr, LogicalAddress, frame);
    else {
        PageTable->rootNodePtr = createNextLevel(PageTable, -1, PageTable->LevelCount);
        PageInsert(PageTable->rootNodePtr, LogicalAddress, frame);
    }
}

void PageInsert(Level *level, unsigned int LogicalAddress, int frame) {
    int index = (int) LogicalToPage(LogicalAddress, level->tablePtr->BitMaskArray[level->depth], level->tablePtr->ShiftArray[level->depth]);
    if(level->depth == level->LevelCount-1) { //base case
        if(level->mapPtr[index].valid == 0) { //invalid, no entries in it
//            cout << "Inserted, Map Updated with frame: " << hex << frame << "\n";
            level->mapPtr[index].frameNumber = frame;
            level->mapPtr[index].valid = 1;
            level->tablePtr->AddressProcessed++;
        }
    }
    else {
        if(level->nextLevelPtr[index] == NULL) {
            level->nextLevelPtr[index] = createNextLevel(level->tablePtr, level->depth, level->LevelCount);
        }
        PageInsert(level->nextLevelPtr[index], LogicalAddress, frame);
    }
}

//=========================================================================================

/*
 This function shows the logical to physical address translation for each memory reference.
 */
void LogicalToPhysical(PageTable *pageTable, unsigned int LogicalAddress, int frame) {
    unsigned int offset = bitSize;
    for (int i = 0; i < pageTable -> LevelCount; i++) {
        offset -= pageTable -> Levels[i];
    }
    int pageSize = pow(2, offset);
    bitset<bitSize> temp_bset;
    for(int j = offset - 1; j >= 0; j--) { //sets the offset bits to 1
        temp_bset[j] = 1;
    }
    unsigned int offsetAddress = LogicalAddress & ((unsigned int) temp_bset.to_ulong()); //& with logical address to get offset address
    unsigned int physicalAddress = 0;
    physicalAddress = (frame * pageSize) + offsetAddress;
    cout << hex << setw(8) << setfill('0') << LogicalAddress << " -> " << hex << setw(8) << setfill('0') << physicalAddress << "\n";
}

//=========================================================================================

/*
 Function to convert a virtual page number to physical frame number.
 */
void PageToFrame(PageTable *pageTable, string fileName) {
    ofstream outputFile(fileName);
    for (int i = 0; i< pageTable -> EntryCount[pageTable->LevelCount-1]; i++ ) {
        if (pageTable ->mapPtr[i].valid == 1) {
            outputFile <<  hex << setw(8) << setfill('0') << i << " -> " << hex << setw(8) << setfill('0') << pageTable -> mapPtr[i].frameNumber << "\n";
        }
    }
}

//=========================================================================================

/*
 Function to report the number of bytes used by the page table.
 */
int memoryProcessed(PageTable *pageTable) {
    int size = 0;
    int mapMemory = sizeof(pageTable->mapPtr);
    memoryProcessed(pageTable->rootNodePtr, size, 0);
    return mapMemory + size + sizeof(pageTable->rootNodePtr);
}

void memoryProcessed(Level *level, int &size, int tempSize) {
    if(level->nextLevelPtr == nullptr) //if we have a null in the nextlevelptr array
        return;
    for(int i=0; i < level->tablePtr->EntryCount[level->depth]; i++) {
        if(level->nextLevelPtr[i] == NULL)
            continue;
        else {
            memoryProcessed(level->nextLevelPtr[i], size, tempSize);
            tempSize = sizeof(level->nextLevelPtr[i]);
            size += tempSize;
        }
    }
}

//=========================================================================================

/*
 Round off a floatig point value to two places
 */
float roundNum(float var) {
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}
