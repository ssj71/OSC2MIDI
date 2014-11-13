//spencer jackson

//pair.h

int alloc_pair(pair* p, char* config);
int set_channel(pair* p, uint8_t channel);
int free_pair(pair* p);
int try_match_osc(pair* p, char* oscpath, lo_arg** argv, int argc);
int try_match_midi(pair*, uint8_t msg[]);
uint8_t* get_midi(pair* p);
lo_message get_osc(pair* p);

