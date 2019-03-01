//spencer jackson
//jackinternal.c

//main entry point for the internal jack client version of osc2midi
//most of the actual jack functionality is in jackmidi.c 

#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<signal.h>
#include<ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <sysexits.h>
#include <errno.h>

//may need pthread
#include"jackprvt.h"
#include"pair.h"
#include"oscserver.h"
#include"converter.h"
#include"midiseq.h"
#include"ht_stuff.h"

#ifndef PREFIX
#define PREFIX "/usr/local"
#endif

typedef struct _combothingo
{
    lo_address loaddr;
    lo_server_thread st;
    CONVERTER conv;
}COMBOTHINGO;

//converts cli string into argc,argv format
//note: clobbers the string s
int str2args(char* s, char*** argv)
{
    int i,l = strlen(s);
    int argc = 1;

    for(i=0;i<l;i++)
        if(s[i] == ' ')
            argc++;
        
    if(!argc)
        return 0;
        
    *argv = (char**)malloc(argc*sizeof(char*)); //not called in RT

    argc = 0;
    *argv[argc++] = &s[0];
    for(i=0;i<l;i++)
    {
        if(s[i] == ' ')
        {
            s[i] = 0;
            *argv[argc++] = &s[i+1];
        }
    }
    return argc;
}


//jack process function
int inframes(jack_nframes_t nframes, void* arg)
{
    return process_callback(nframes, &((COMBOTHINGO*)arg)->conv.seq);
}

int init_internal_midi_seq(COMBOTHINGO* handle, bool verbose, jack_client_t* client)
{
    int err;
    MIDI_SEQ* mseq;
    JACK_SEQ* seq;

    mseq = &handle->conv.seq;
    mseq->nnotes = 0;
    mseq->old_filter = 0;
    seq = (JACK_SEQ*)malloc(sizeof(JACK_SEQ));
    mseq->driver = seq;
    if(verbose)printf("opening client...\n");

    seq->jack_client = client;

    if (seq->jack_client == NULL)
    {
        printf("Could not connect to the JACK server; run jackd first?\n");
        free(seq);
        return 0;
    }

    if(verbose)printf("assigning process callback...\n");
    //pass the whole thing in so we can free everything when needed
    err = jack_set_process_callback(seq->jack_client, inframes, (void*)handle);
    if (err)
    {
        printf("Could not register JACK process callback.\n");
        free(seq);
        return 0;
    }

    if(!init_ports(mseq,verbose))
    {
        free(seq);
        return 0;
    }
    return 1;
}

//main entry point
int __attribute__((visibility("default"))) jack_initialize (jack_client_t *client, const char *load_init)
{
    char file[200], port[200], addr[200], clientname[200];
    char arglist[400];
    int i, argc;
    COMBOTHINGO* combo;
    char** argv;

    strcpy(arglist, load_init);
    argc = str2args(arglist,&argv); 

    combo = (COMBOTHINGO*)malloc(sizeof(COMBOTHINGO)); //not in RT

    if(process_cli_args(argc,argv,file,port,addr,clientname,&combo->conv))
    {
        return -1;
    }

    if(!combo->conv.mon_mode)
    {
        if(load_map(&combo->conv,file) == -1)
            return -1;
        if(combo->conv.dry_run)
        {
            printf("Found %d error(s), exiting\n", combo->conv.errors);
            return combo->conv.errors?-1:0;
        }
        else if(combo->conv.errors > 0)
        {
            printf("Found %d error(s)\n", combo->conv.errors);
        }
        if( (i = check_pair_set_for_filter(combo->conv.p, combo->conv.npairs)) )
        {
            combo->conv.seq.usefilter = true;
            combo->conv.seq.filter = &combo->conv.filter;
            printf("Found pair %i with filter functions, creating midi filter in/out pair.\n", i);
        }
    }
    else if(combo->conv.verbose)
        printf("Monitor mode, incoming OSC messages will only be printed.\n");

    //start the server
    if(combo->conv.convert > -1)
    {
        combo->st = start_osc_server(port,&combo->conv);
        combo->conv.seq.useout = true;
    }
    else
    {
        combo->conv.seq.useout = false;
    }
    if(combo->conv.convert < 1)
    {
        //get address ready to send osc messages to
        combo->conv.seq.usein = true;
        combo->loaddr = lo_address_new_from_url(addr);
        printf(" sending osc messages to address %s\n",addr);
    }
    else
    {
        combo->conv.seq.usein = false;
    }

    //start midi client
    if(!combo->conv.mon_mode)
    {
        if(!init_internal_midi_seq(combo,combo->conv.verbose,client))
        {
            printf("MIDI connection failed");
            return -1;
        }
    }

    if(combo->conv.verbose)
        printf("Ready.\n");
    fflush(stdout);

    return 0;
}

#if 0
void stuffthatneedsthreading()
{
    signal(SIGINT, quitter);
    while(!quit)
    {
        if(conv.convert < 1)
        {
            convert_midi_in(loaddr,&conv);
            usleep(1000);
        }
        else
            usleep(50000);
    }
}
#endif

void __attribute__((visibility("default"))) jack_finish(void* arg)
{
    if(!arg)
        return;

    COMBOTHINGO* combo = (COMBOTHINGO*)arg;
    CONVERTER* conv = &combo->conv;

    //stop everything
    printf("\nquitting...\n");
    if(!conv->mon_mode)
    {
        if(conv->verbose)
            printf(" closing midi ports\n");
        close_midi_seq(&conv->seq);
    }
    if(conv->convert > -1)
    {
        if(conv->verbose)
            printf(" closing osc server\n");
        stop_osc_server(combo->st);
    }
    if(conv->convert < 1)
    {
        lo_address_free(combo->loaddr);
    }
}
