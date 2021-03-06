//spencer jackson

//pair.h
#ifndef PAIR_H
#define PAIR_H

#include<lo/lo.h>
#include<stdint.h>
#include"hashtable.h"

typedef void* PAIRHANDLE;

PAIRHANDLE alloc_pair(char* config, table tab, float** regs, int* nkeys);
void free_pair(PAIRHANDLE ph);
int try_match_osc(PAIRHANDLE ph, char* path, char* types, lo_arg** argv, int argc,
                  uint8_t strict_match, uint8_t* glob_chan, uint8_t* glob_vel, int8_t* filter, uint8_t msg[]);
int try_match_midi(PAIRHANDLE ph, uint8_t msg[], uint8_t strict_match, uint8_t* glob_chan, char* path, lo_message oscm);
void print_pair(PAIRHANDLE ph);
int check_pair_set_for_filter(PAIRHANDLE* pa, int npair);
char * opcode2cmd(uint8_t opcode, uint8_t noteoff);
void print_midi(PAIRHANDLE ph, uint8_t msg[]);

#endif
