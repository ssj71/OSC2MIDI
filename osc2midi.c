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
p = alloc_pair("/midi/pitchbend{i} fif,bad ,b,a,c,ad:noteon(31-bad/2,-31,1 +a  +2)",&glob_chan);

print_pair(p);

}
