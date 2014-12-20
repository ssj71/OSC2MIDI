//spencer jackson

//pair.h
#ifndef PAIR_H
#define PAIR_H

#include<lo/lo.h>
#include<stdint.h>

typedef void* PAIRHANDLE;

PAIRHANDLE alloc_pair(char* config);
void print_pair(PAIRHANDLE ph);
void free_pair(PAIRHANDLE ph);
int try_match_osc(PAIRHANDLE ph, char* path, char* types, lo_arg** argv, int argc, uint8_t* glob_chan, uint8_t* glob_vel, uint8_t msg[]);
int try_match_midi(PAIRHANDLE ph, uint8_t msg[], uint8_t* glob_chan, char* path, lo_message oscm);
char * opcode2cmd(uint8_t opcode, uint8_t noteoff);

#endif
