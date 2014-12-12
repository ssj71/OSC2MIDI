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
    uint8_t verbose;
    uint8_t mon_mode;
    uint8_t multi_match;

    uint16_t npairs;
    PAIRHANDLE* p;

    void * seq;
} CONVERTER;
#endif
