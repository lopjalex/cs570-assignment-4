//
// main.cpp
// Jacqueline Gronotte and Jesus Lopez
// CS 570 Assignment 4
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  /* define bool */
#include <unistd.h>
#include <iostream>
#include <math.h>
#include "byutr.h"
#include "pagetable.h"

using namespace std;

#define OOPS 5

int main(int argc, char **argv) {
    // An element of argv that starts with '-' (and is not exactly "-" or "--") is an option element.
    bool t = false; //set to true if -t flag is set
    int n = 0; //set to N references to process
    bool p = false;
    string filename; //set to filename to output when -p is set
    int Option;
    
    int hits = 0;
    int misses = 0;
    
    while ( (Option = getopt(argc, argv, "n:p:t")) != -1) {
        switch (Option) {
            case 'n': /* Number of addresses to process */
                    // optarg will contain the string following -n
                    // Process appropriately (e.g. convert to integer atoi(optarg))
                n = atoi(optarg);
                break;
            case 'p': /* produce map of pages */
                // optarg contains name of page file…
                p = true;
                filename = optarg;
                break;
            case 't': /* Show address translation */
                // No argument this time, just set a flag
                t = true;
                break;
            default:
                // print something about the usage and die…
                //exit(BADFLAG); // BADFLAG is an error # defined in a header
                abort();
        }
    }
    
    PageTable* pagetable = new PageTable();
    pagetable->AddressProcessed = 0;
    pagetable->LevelCount = countLevels(argc, argv);

    unsigned int levels[pagetable->LevelCount];
    storeLevels(pagetable->LevelCount, argc, argv, levels);
    pagetable->Levels = levels;

    unsigned int entries[pagetable->LevelCount];
    calcEntryCount(pagetable->LevelCount, pagetable->Levels, entries);
    pagetable->EntryCount = entries;

    unsigned int shiftArray[pagetable->LevelCount];
    calcShiftArray(pagetable->LevelCount, pagetable->Levels, shiftArray);
    pagetable->ShiftArray= shiftArray;

    unsigned int bitMask[pagetable->LevelCount];
    calcBitMask(pagetable->LevelCount, pagetable->Levels, bitMask);
    pagetable->BitMaskArray = bitMask;
    
    pagetable->rootNodePtr = createNextLevel(pagetable, -1, pagetable->LevelCount);
    
    FILE *fp = fopen("trace.sample.tr", "r");
    if (fp == NULL) {
        //couldn't open, bail gracefully
        printf("uh oh\n");
        exit(OOPS);
        
    }
    int i =0;
    int addressesProcessed = 0;
    //Start reading addresses
    p2AddrTr trace_item;  //Structure with trace information
    bool done = false;
    while (! done) {
        //Grab the next address
        int bytesread = NextAddress(fp, &trace_item);
        // Check if we actually have something
        done = bytesread == 0;
        if (! done) {
            Map *temp = PageLookup(pagetable, trace_item.addr);
            if(temp == nullptr) {
                misses++;
                PageInsert(pagetable, trace_item.addr, i);
                i++;
            }
            else {
                hits++;
            }
            if(t == true) {
                LogicalToPhysical(pagetable, trace_item.addr, i-1);
            }
            addressesProcessed++;
            if(n == addressesProcessed)
                break;
        }
    }
    if(p == true) {
        PageToFrame(pagetable, filename);
    }
    
    int pageSize = 0;
    for(int i = 0; i < pagetable->LevelCount; i++) {
        pageSize += pagetable->Levels[i];
    }
    
    float hitpercent = (float)hits/(float)addressesProcessed*100;
    float misspercent = (float)misses/(float)addressesProcessed*100;

    cout << dec << "Page Size " << pow(2, 32-pageSize) << "\n";
    cout << "Hits: " << hits << " (" << roundNum(hitpercent) << "%), " << "Misses: " << misses << " (" << roundNum(misspercent) << "%) ";
    cout << dec << "# Addresses " << addressesProcessed << "\n";
    //Total bytes used in creating the page table
    cout << dec << "Bytes used: " << memoryProcessed(pagetable) << "\n";
    
    return 0;
}
