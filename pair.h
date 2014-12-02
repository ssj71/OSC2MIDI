//spencer jackson

//pair.h
#include<lo/lo.h>
#include<stdint.h>

typedef void* PAIRHANDLE;

int alloc_pair(PAIRHANDLE ph, char* config, uint8_t *glob_chan);
int free_pair(PAIRHANDLE ph);
int try_match_osc(PAIRHANDLE ph, char* path, char* types, lo_arg** argv, int argc, uint8_t msg[]);
int try_match_midi(PAIRHANDLE ph, uint8_t msg[], lo_message *oscmsg);

