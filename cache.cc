/*******************************************************
                          cache.cc
********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "cache.h"
using namespace std;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0;
   writeMisses = writeBacks = currentCycle = 0;

   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));
   log2Blk    = (ulong)(log2(b));

   //*******************//
   //initialize your counters here//
   countMemTransfer = 0;
   countIntervention = 0;
   countInvalidate = 0;
   countFlush = 0;
   countBusRdX = 0;
   countBusUpd = 0;
   //*******************//

   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
        tagMask <<= 1;
        tagMask |= 1;
   }

   /**create a two dimentional cache, sized as cache[sets][assoc]**/
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++)
      {
           cache[i][j].invalidate();
      }
   }

}

/**you might add other parameters to Access()
since this function is an entry point
to the memory hierarchy (i.e. caches)**/
ulong Cache::Access(ulong addr, uchar op)
{
   currentCycle++;
   ulong busSignal = NONE;
   cacheLine * line = findLine(addr);
   flagReadMiss = 0;     // Reset the flag
   flagWriteMiss = 0;    // Reset the flag
   flagWriteHit = 0;     // Reset the flag
   
   if(op == 'w'){
	   writes++;
	   if (line == NULL){ // WRITE MISS
		   cacheLine * lineReplace = fillLine(addr);
		   writeMisses++;
		   countMemTransfer++;
		   lineReplace->setFlags(M);
		   if(protocol == 0){ // MSI
				busSignal = BusRdX;
				countBusRdX++;
			}
			else{ // Dragon
				flagWriteMiss = 1;
				busSignal = BusRd;
			}
		   //busSignal = writeMiss(lineReplace);
	   }
	   else{
			updateLRU(line);
			if(protocol == 0){ // MSI
				if(line->getFlags() == S){
					line->setFlags(M);
					//countMemTransfer++; 
				}
				else{
					busSignal = NONE;
				}
			}
			else{ // Dragon
				flagWriteHit = 1;
				if(line->getFlags() == Sc){
					busSignal = BusUpd;    
					countBusUpd++;
				}
				else if(line->getFlags() ==  Sm){
					busSignal = BusUpd; 
					countBusUpd++;
				}
				else if(line->getFlags() == E){
					line->setFlags(M);  
				}
				else{ 
					busSignal = NONE;
				}
			}
			//busSignal = writeHit(line);
		}
   }
   else{
	   reads++;
	   if (line == NULL) {
		   cacheLine * lineReplace = fillLine(addr);
		   readMisses++;
		   busSignal = BusRd;
           countMemTransfer++;
			if(protocol == 0){ // MSI
				lineReplace->setFlags(S);
			}
			else{ // Dragon
				flagReadMiss = 1;
				lineReplace->setFlags(Sc);
			}
		   //busSignal = readMiss(lineReplace);
	   }
	   else {
		   updateLRU(line);
	   }
   }
   return busSignal;
}


/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
	ulong i, j, tag, pos;
	
	pos = assoc;
	tag = calcTag(addr);
	i   = calcIndex(addr);
	
	for(j=0; j<assoc; j++){
		if(cache[i][j].isValid()){
			if(cache[i][j].getTag() == tag){
				pos = j;
				break;
			}
		}
	}
	
	if(pos == assoc) return NULL;
	else return &(cache[i][pos]);
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);

   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);
   }
   for(j=0;j<assoc;j++)
   {
         if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   }
   assert(victim != assoc);

   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);

   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{
        ulong tag;

        cacheLine *victim = findLineToReplace(addr);
        assert(victim != 0);
        if((victim->getFlags() == M) || (victim->getFlags() == Sm)){
                writeBack(addr);
                countMemTransfer++;
        }
        tag = calcTag(addr);
        victim->setTag(tag);

        return victim;
}

void Cache::printStats(uint id)
{
        printf("============ Simulation results (Cache %d) ============\n", id);
		printf("01. number of reads:                            %lu\n", reads);
        printf("02. number of read misses:                      %lu\n", readMisses);
        printf("03. number of writes:                           %lu\n", writes);
        printf("04. number of write misses:                     %lu\n", writeMisses);
        printf("05. total miss rate:                            %4.2f%%\n", ((double)(readMisses + writeMisses)/(double)(reads + writes))*100 );
        printf("06. number of writebacks:                       %lu\n", writeBacks);
        printf("07. number of memory transactions:              %lu\n", countMemTransfer);
        if (protocol == 0){
			printf("08. number of invalidations:                    %lu\n", countInvalidate);
			printf("09. number of flushes:                          %lu\n", countFlush);
			printf("10. number of BusRdX:                           %lu\n", countBusRdX);
        }
        else{
			printf("08. number of interventions:                    %lu\n", countIntervention);
			printf("09. number of flushes:                          %lu\n", countFlush);
			printf("10. number of Bus Transactions(BusUpd):         %lu\n", countBusUpd);
        }
}

ulong Cache::snoopBus(ulong busSignal, ulong addr){
        cacheLine * line = findLine(addr);
        if(protocol == 0) return snoopMSI(busSignal, line);
		else return snoopDragon(busSignal, line);
}

ulong Cache::snoopMSI(ulong busSignal, cacheLine * line){
    ulong status;
    ulong snoopReaction = NONE;
    if(line != NULL){ 
        status = line->getFlags();
        if(status == M){
			line->invalidate();
            writeBacks++;  
            countInvalidate++;
            snoopReaction = Flush;
        }
		else if(status == S){
			line->invalidate();
            countInvalidate++;
        }
		else{
            return snoopReaction;
        }
    }
    return snoopReaction;
}

ulong Cache::snoopDragon(ulong busSignal, cacheLine * line){
    ulong status;
    ulong snoopReaction = NONE;
    if(line != NULL){
        status = line->getFlags();
        if(busSignal == BusRd){
            copyExist = 1;
            if(status == E){
                    line->setFlags(Sc);
                    countIntervention++;
            }
			else if(status == M){
                    line->setFlags(Sm);
                    countIntervention++;
                    snoopReaction = Flush;
                    writeBacks++;
                    countMemTransfer++;
            }
			else if(status == Sm){
                    snoopReaction = Flush;
                    writeBacks++;
                    countMemTransfer++; 
            }
        }
		else if(busSignal == BusUpd){
            copyExist = 1;
            if(status == Sm){
                line->setFlags(Sc);
            }
        }
    }
    return snoopReaction;
}

void Cache::snoopReaction(ulong busReaction){
    if(busReaction == Flush){
		countFlush++;
		if(protocol == 0) countMemTransfer++;
	}
}

ulong Cache::snoopProcessor(ulong addr){ // Only for the dragon protocol
    ulong post = NONE;
    if (protocol == 1){ // Dragon
        if(flagWriteMiss == 1){
            cacheLine * line = findLine(addr);
            if(line != NULL){
                if(copyExist == 1){
                    line->setFlags(Sm);
                    post = BusUpd; 
                    countBusUpd++;
                }
                else
				{
                    line->setFlags(M); 
                }
            }
        }
        else if(flagReadMiss == 1){
            cacheLine * line = findLine(addr);
            if(line != NULL){ 
                if(copyExist == 1) line->setFlags(Sc);
                else line->setFlags(E);
            }
        }
		else if(flagWriteHit == 1){
            cacheLine * line = findLine(addr);
            if(line != NULL){ 
                if(line->getFlags() == E){
                        line->setFlags(M);
                }
				else if(line->getFlags() == Sc){
                    if(copyExist == 1) line->setFlags(Sm);
					else line->setFlags(M);
                }
				else if(line->getFlags() == Sm){
                    if(copyExist == 0) line->setFlags(M);
                } 
            }
        }
    }
    return post; 
}









