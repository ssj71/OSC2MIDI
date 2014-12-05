//spencer jackson
//oscserver.h
#ifndef OSCSERVER_H
#define OSCSERVER_H
#include"converter.h"

lo_server_thread start_osc_server(char* port,CONVERTER* data);
int stop_osc_server(lo_server_thread st);
#endif
