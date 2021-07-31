//
// Jackie Gronotte and Jesus Lopez.
//

#ifndef ASSIGNMENT4_PAGETABLE_H
#define ASSIGNMENT4_PAGETABLE_H

#define POINTERS 256

using namespace std;

/* The countLevels function takes in the user-provided number of levels from the command line. */
int countLevels(int argc, char **argv);

/*
* The calcBitMask function takes in the levelCount, levelSizes and the bitMask as arguments to
* calculate the bitMask array for each level.
*/
void calcBitMask(int levelCount, unsigned int *levelSizes, unsigned int *bitMask);

/*
* The calcShiftArray takes in the levelCount, levelSizes and shiftArray as arguments to determine how
* much to shift.
*/
void calcShiftArray(int levelCount, unsigned int *levelSizes, unsigned int *shiftArray);

/* The calcEntryCount function calculates the number of possible pages for each level. */
void calcEntryCount(int levelCount, unsigned int *levelSizes, unsigned int *entries);

/* The storeLevels function contains the size of each level as defined by the user on the command line. */
void storeLevels(int levelCount, int argc, char **argv, unsigned int* levels);

/*
* The LogicalToPage function takes in the LogicalAddress, Mask and the Shift as arguments to return the virtual
* page number given the logical address of a page.
*/
unsigned int LogicalToPage(unsigned int LogicalAddress, unsigned int Mask, unsigned int Shift);

/* Forward declaration of the Level class */
class Level;

/*
* The Map struct contains the status of the which frame numbers are valid or invalid for input into the Map.
* Valid frames are set to "1" and invalid frames are set to "0". The Map struct also contains the
* frame numbers for each frame.
*/
struct Map {
    int valid;
    int frameNumber;
};

/*
* The PageTable class contains the rootNodePtr, LevelCount, current_depth, Levels, BitMaskArray, ShiftArray
* and EntryCount. We use a rootNodePtr of type Level that points to the top of the PageTable, the root. The
* LevelCount contains the number of levels in the page table. The current_depth indicates the current depth at
* each level in the PageTable. The Levels pointer contains the levels for the PageTable. The BitMaskArray contains the bit mask for each level. The ShiftArray contains
* the number of bits to shift for each level in the page. The EntryCount is an array of the number of possible
* pages for each level.
*/
class PageTable {
public:
    Level *rootNodePtr;
    int AddressProcessed;
    int LevelCount;
    int current_depth;
    unsigned int *Levels;
    unsigned int *BitMaskArray;
    unsigned int *ShiftArray;
    unsigned int *EntryCount;
    Map *mapPtr;
};

/*
* The Level class contains the depth, LevelCount, tablePtr, mapPtr, and nextLevelPtr.
* Depth refers to the depth at each level, starting at a depth equal to "0" for level 0. We also
* keep a LevelCount to track the number of levels we have in our PageTable. We have a tablePtr of
* type PageTable that points back to our PageTable from each level. When we are at our last level,
* we have a mapPtr to point to the map data structure. Lastly, we have a nextLevelPtr at each level, excluding the
* last level, that points to the next level below where we currently are.
*/
class Level {
public:
    int depth;
    int LevelCount;
    PageTable *tablePtr;
    Map *mapPtr;
    Level **nextLevelPtr;
};

/*
* The createNextLevel function takes in the the pageTable, the current_depth, and the levelCount
* as arguments in order to create the next level.
*/
Level *createNextLevel(PageTable *pageTable, int current_depth, int levelCount);

/*
* The createMapPtr function takes no arguments and creates a pointer to a new map.
*/
Map createMapPtr();

/*
* The PageLookup function takes in a pointer to the PageTable and the LogicalAddress as arguments to
* lookup if a page exists and return the page entry if it does.
*/
Map *PageLookup(PageTable *PageTable, unsigned int LogicalAddress);

/*
* The PageInsert function takes in a pointer to the PageTable, the LogicalAddress and the frame index as
* arguments to insert a new page into the PageTable.
*/
void PageInsert(PageTable *PageTable, unsigned int LogicalAddress, int frame);

/*
* This PageInsert function takes in a level pointer, the LogicalAddress of the page and the frame index
* as arguments to find the index into the current page level to insert.
*/
void PageInsert(Level *level, unsigned int LogicalAddress, int frame);

/*
 This function shows the logical to physical address translation for each memory reference
 */
void LogicalToPhysical(PageTable *pageTable, unsigned int LogicalAddress, int frame);
/*
 Function to convert a virtual page number to physical frame number.
 */
void PageToFrame(PageTable *pageTable, string fileName);

/*
 Function to report the number of bytes used by the page table.
 */
int memoryProcessed(PageTable *pageTable);
void memoryProcessed(Level*pageTable, int &size, int tempSize);

/*
Round off a floatig point value to two places
*/
float roundNum(float var);

#endif //ASSIGNMENT4_PAGETABLE_H
