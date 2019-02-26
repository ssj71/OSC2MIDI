//spencer jackson
//converter.h

//this is a data struct used to pass data from the OSC server thread to the midi thread
#ifndef CONVERTER_H
#define CONVERTER_H
#include<stdint.h>
#include<stdbool.h>
#include"pair.h"
#include"midiseq.h"
#include"hashtable.h"

typedef struct _CONVERTER
{
    uint8_t glob_chan;
    uint8_t glob_vel;
    int8_t  filter;
    bool verbose;
    bool mon_mode;
    bool multi_match;
    bool strict_match;
    int8_t  convert; //0 = both, 1 = o2m, -1 = m2o
    bool dry_run;
    int errors;

    uint16_t npairs;
    PAIRHANDLE* p;

    table tab;
    float** registers;

    MIDI_SEQ seq;
} CONVERTER;

int load_map(CONVERTER* conv, char* file);
int is_empty(const char *s);
void init_registers(float ***regs, int n);
int process_cli_args(int argc, char** argv, char* file, char* port, char* addr, char* clientname, CONVERTER* conv);
#endif
