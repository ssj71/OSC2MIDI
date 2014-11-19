//spencer jackson

//pair.h

int alloc_pair(pair* p, char* config, uint8_t *glob_chan);
int free_pair(pair* p);
int try_match_osc(pair* p, char* oscpath, lo_arg** argv, int argc);
int try_match_midi(pair*, uint8_t msg[]);

