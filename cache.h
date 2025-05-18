/*******************************************************
                          cache.h
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

/****add new states, based on the protocol****/
enum{
    I = 0,
    S,		// Only MSI
    E,		// Only Dragon
    M,		
    Sm,		// Only Dragon
    Sc		// Only Dragon
};
// define bus actions
enum{
   NONE = 0,
   BusRd,
   BusRdX,
   Flush,
   BusUpgr,
   BusUpd  //bus update for Dragon
};

extern int copyExist;
extern int protocol;

class cacheLine
{
protected:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty
   ulong seq;

public:
   cacheLine()            { tag = 0; Flags = 0; }
   ulong getTag()         { return tag; }
   ulong getFlags()                     { return Flags;}
   ulong getSeq()         { return seq; }
   void setSeq(ulong Seq)                       { seq = Seq;}
   void setFlags(ulong flags)                   {  Flags = flags;}
   void setTag(ulong a)   { tag = a; }
   void invalidate()      { tag = 0; Flags = I; }//useful function
   bool isValid()         { return ((Flags) != I); }
};

class Cache
{
protected:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;

   //******///
   //add coherence counters here///
   ulong countMemTransfer;              // memory transactions
   ulong countIntervention;     		// (E/M -> Sc/Sm state) only for Dragon Protocol
   ulong countInvalidate;       		// only for MSI Protocol
   ulong countFlush;                	// number of flush
   ulong countBusRdX;               	// number of BusRdx Signals - MSI Protocol
   ulong countBusUpd;					// number of BusUpdate Signals - Dragon Protocol
   //******///
   int flagReadMiss; 	// read  miss flag
   int flagWriteMiss;	// write miss flag
   int flagWriteHit; 	// write hit  flag

   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)  { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag)   { return (tag << (log2Blk));}

public:
    ulong currentCycle;

    Cache(int,int,int);
   ~Cache() { delete cache;}

   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);

   ulong getRM()     {return readMisses;}
   ulong getWM()     {return writeMisses;}
   ulong getReads()  {return reads;}
   ulong getWrites() {return writes;}
   ulong getWB()     {return writeBacks;}

   void writeBack(ulong)   {writeBacks++;}
   ulong Access(ulong,uchar);
   void printStats(uint id);
   void updateLRU(cacheLine *);

   //******///
   //add other functions to handle bus transactions///
   //******///
   ulong snoopBus(ulong, ulong);
   void snoopReaction(ulong);
   ulong snoopMSI(ulong, cacheLine * line);
   ulong snoopDragon(ulong, cacheLine * line);
   ulong snoopProcessor(ulong);
   int printCache(int, uchar, ulong);
};

#endif

