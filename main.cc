/*******************************************************
                          main.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"

int copyExist;
int protocol;

int main(int argc, char *argv[])
{

    ifstream fin;
    FILE * pFile;

    if(argv[1] == NULL){
         printf("input format: ");
         printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
         exit(0);
        }

    ulong cache_size     = atoi(argv[1]);
    ulong cache_assoc    = atoi(argv[2]);
    ulong blk_size       = atoi(argv[3]);
    ulong num_processors = atoi(argv[4]);
    protocol       		 = atoi(argv[5]);
    char *fname          = (char *) malloc(20);
    fname                = argv[6];

//    printf("===== Simulator configuration =====\n");
    // print out simulator configuration here
    printf("===== 506 Personal information =====\n");
    printf("Padmanabha Nikhil\n");
    printf("pbhimav\n");
    printf("ECE406 Students? NO\n");
    printf("===== 506 SMP Simulator configuration =====\n");
    printf("L1_SIZE: %ld\n", cache_size);
    printf("L1_ASSOC: %ld\n", cache_assoc);
    printf("L1_BLOCKSIZE: %ld\n", blk_size);
    printf("NUMBER OF PROCESSORS: %ld\n", num_processors);
    if (protocol == 0) printf("COHERENCE PROTOCOL: MSI\n");
    else printf("COHERENCE PROTOCOL: Dragon\n");
    printf("TRACE FILE: %.27s\n", &fname[3]);

    Cache *cacheArray[num_processors];
        for(int i = 0 ; i < (int)num_processors ; i++){
                cacheArray[i] = new Cache(cache_size, cache_assoc, blk_size);
        }

    pFile = fopen (fname,"r");
    if(pFile == 0)
    {
        printf("Trace file problem\n");
        exit(0);
    }

    ulong proc;
    char op;
    ulong addr;

    unsigned long busSignal; 
    unsigned long busReaction; 
    unsigned long busSignalProcessor; 

    int line = 1;
    while(fscanf(pFile, "%lu %c %lx", &proc, &op, &addr) != EOF){
	copyExist = 0;

        busSignal = cacheArray[proc]->Access(addr, op);
        if(busSignal != NONE){ 
			for(int i = 0 ; i < (int)num_processors ; i++){
				if(i != (int)proc){
					busReaction = cacheArray[i]->snoopBus(busSignal, addr);
						cacheArray[i]->snoopReaction(busReaction); // such as flush
				}
			}
			/************ For Dragon Protocol ************/
			busSignalProcessor = cacheArray[proc]->snoopProcessor(addr);
			if(busSignalProcessor == BusUpd){
				for(int i = 0 ; i < (int)num_processors ; i++){
					if(i != (int)proc){
						cacheArray[i]->snoopBus(busSignalProcessor, addr); // the second bus snooping
					}
				}
			}
        }
        line++;
    }

    fclose(pFile);
	
    for(int i = 0 ; i < (int)num_processors ; i++){
		cacheArray[i]->printStats(i);
    }

}

