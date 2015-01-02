//spencer jackson
//oscserver.c
//modified from the example code from the liblo documentation
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lo/lo.h"
#include "pair.h"
#include "oscserver.h"
#include "converter.h"
#include "jackdriver.h"

int done = 0;

void error(int num, const char *m, const char *path);

int mon_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);
int msg_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);


lo_server_thread start_osc_server(char* port, CONVERTER* data)
{
    /* start a new server on port 7770 */
    lo_server_thread st = lo_server_thread_new(port, error);

    /* add method that will match any path and args */
    if(data->mon_mode)
        lo_server_thread_add_method(st, NULL, NULL, mon_handler, NULL);
    else
        lo_server_thread_add_method(st, NULL, NULL, msg_handler, data);

    lo_server_thread_start(st);
    if(data->verbose)
    {
        printf("starting osc server on port %s\n",port);
    }
    return st;
}


int stop_osc_server(lo_server_thread st)
{
    lo_server_thread_free(st);

    return 0;
}

void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
int mon_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
    int i;

    printf("%s ", path);
    for (i = 0; i < argc; i++) {
        printf("%c", types[i]);
    }
    for (i = 0; i < argc; i++) {
        printf(", ");
        lo_arg_pp((lo_type)types[i], argv[i]);
    }
    printf("\n\n");
    fflush(stdout);

    return 0;
}

//this handles the osc to midi conversions
int msg_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
    int i,j,n;
    uint8_t first = 1;
    uint8_t midi[3];
    CONVERTER* conv = (CONVERTER*)user_data;

    for(j=0;j<conv->npairs;j++)
    {
        if( (n = try_match_osc(conv->p[j],(char *)path,(char *)types,argv,argc,&(conv->glob_chan),&(conv->glob_vel),midi)) )
        {
            if(!conv->multi_match)
                j = conv->npairs;
            if(conv->verbose)
            {
                if(first)
                    printf("matches found:\n");
                first = 0;
                printf("  %s ", path);
                for (i = 0; i < argc; i++) {
                    printf("%c", types[i]);
                }
                for (i = 0; i < argc; i++) {
                    printf(", ");
                    lo_arg_pp((lo_type)types[i], argv[i]);
                }
                printf(" : %s ( %i, %i, %i )\n", opcode2cmd(midi[0]&0xF0,1), midi[0]&0x0F, midi[1], midi[2]);
                fflush(stdout);
            }

            //push message onto ringbuffer (with timestamp)
            if(n>0)
                queue_midi(conv->seq,midi);
        }
    }
    if(conv->verbose && !first)
        printf("\n");
    return 0;
}
