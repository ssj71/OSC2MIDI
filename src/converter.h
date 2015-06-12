//spencer jackson
//converter.h

//this is a data struct used to pass data from the OSC server thread to the midi thread
#ifndef CONVERTER_H
#define CONVERTER_H
#include<stdint.h>
#include"pair.h"

typedef struct _CONVERTER
{
    uint8_t glob_chan;
    uint8_t glob_vel;
    int8_t  filter;
    uint8_t verbose;
    uint8_t mon_mode;
    uint8_t multi_match;
    int8_t  convert; //0 = both, 1 = o2m, -1 = m2o
    uint8_t dry_run;
    int errors;

    uint16_t npairs;
    PAIRHANDLE* p;

    table tab;
    float** registers;

    void * seq;
} CONVERTER;
#endif
