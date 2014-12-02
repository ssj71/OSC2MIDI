//spencer jackson

//pair.h
#ifndef PAIR_H
#define PAIR_H

#include<lo/lo.h>
#include<stdint.h>

typedef void* PAIRHANDLE;

PAIRHANDLE alloc_pair(char* config, uint8_t *glob_chan);
void print_pair(PAIRHANDLE ph);
int free_pair(PAIRHANDLE ph);
int try_match_osc(PAIRHANDLE ph, char* path, char* types, lo_arg** argv, int argc, uint8_t msg[]);
int try_match_midi(PAIRHANDLE ph, uint8_t msg[], lo_message *oscmsg);

#endif
