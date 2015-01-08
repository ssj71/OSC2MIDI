//spencer jackson
//oscserver.h
#ifndef OSCSERVER_H
#define OSCSERVER_H
#include"converter.h"
#include "lo/lo.h"

lo_server_thread start_osc_server(char* port,CONVERTER* data);
int stop_osc_server(lo_server_thread st);
void convert_midi_in(lo_address addr, CONVERTER* data);
#endif
