//spencer jackson
//osc2midi.m

//main file for the osc2midi utility

#include<stdio.h>
#include<stdint.h>
#include"pair.h"


int main(int argc, char** argv)
{

if(argc>1)
{
}

PAIRHANDLE p;
uint8_t glob_chan = 1;
alloc_pair(p,"/midi/pitchbend f, a : pitchbend(channel,2*a)",&glob_chan);

print_pair(p);

}
