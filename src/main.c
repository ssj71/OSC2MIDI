//spencer jackson
//osc2midi.m

//main file for the osc2midi utility

#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<signal.h>
#include<ctype.h>
#include<unistd.h>//only for usleep
#include"pair.h"
#include"oscserver.h"
#include"converter.h"
#include"midiseq.h"
#include"ht_stuff.h"

#ifndef PREFIX
#define PREFIX "/usr/local"
#endif

uint8_t quit = 0;

void quitter(int sig)
{
    quit = 1;
}

void usage()
{
    printf("osc2midi - a linux OSC to MIDI bridge\n");
    printf("\n");
    printf("\n");
    printf("USAGE:\n");
    printf("    osc2midi [-option <value>...]\n");
    printf("\n");
    printf("OPTIONS:\n");
    printf("    -v             verbose mode\n");
    printf("    -p <value>     set OSC server port\n");
    printf("    -m <value>     set mapping file by name or path\n");
    printf("    -a <value>     address of OSC client for midi->OSC, <ip:port>\n");
    printf("    -c <value>     set default MIDI channel\n");
    printf("    -vel <value>   set default MIDI note velocity\n");
    printf("    -s <value>     set default filter shift value\n");
    printf("    -multi         multi mode (check all mappings/send multiple messages)\n");
    printf("    -single        multi mode off (stop checks after first match)\n");
    printf("    -strict        strict matches (check multiple occurrences of variables)\n");
    printf("    -mon           only print OSC messages that come into the port\n");
    printf("    -o2m           only convert OSC messages to MIDI\n");
    printf("    -m2o           only convert MIDI messages to OSC\n");
    printf("    -n             dry run: check syntax of map file and exit\n");
    printf("    -name <value>  midi client name (default osc2midi)\n");
    printf("    -h             show this message\n");
    printf("\n");
    printf("NOTES:\n");
    printf("    By default osc2midi looks for mapping files in /usr/local/share/osc2midi/.\n");
    printf("    See default.omm in that directory on how to create your own mappings.\n");
    printf("\n");
    printf("    Multi mode is heavier: It finds all matches, which is useful if OSC\n");
    printf("    messages contain more data than can be sent in a single MIDI message.\n");
    printf("    By default multi mode is on. Pass -single to disable.\n");
    printf("\n");
    printf("    Strict matches make sure that multiple occurrences of a variable are all\n");
    printf("    matched to the same value when converting an OSC or MIDI message. This\n");
    printf("    incurs a small overhead and is disabled by default; -strict enables it.\n");
    printf("\n");

    return;
}

int main(int argc, char** argv)
{
    char file[200], port[200], addr[200], clientname[200];
    int i;
    lo_address loaddr;
    lo_server_thread st;
    CONVERTER conv;

    if(process_cli_args(argc,argv,file,port,addr,clientname,&conv))
    {
        usage();
        return -1;
    }

    if(!conv.mon_mode)
    {
        if(load_map(&conv,file) == -1)
            return -1;
        if(conv.dry_run)
        {
            printf("Found %d error(s), exiting\n", conv.errors);
            return conv.errors?-1:0;
        }
        else if(conv.errors > 0)
        {
            printf("Found %d error(s)\n", conv.errors);
        }
        if( (i = check_pair_set_for_filter(conv.p,conv.npairs)) )
        {
            conv.seq.usefilter = true;
            conv.seq.filter = &conv.filter;
            printf("Found pair %i with filter functions, creating midi filter in/out pair.\n", i);
        }
    }
    else if(conv.verbose)
        printf("Monitor mode, incoming OSC messages will only be printed.\n");

    //start the server
    if(conv.convert > -1)
    {
        st = start_osc_server(port,&conv);
        conv.seq.useout = true;
    }
    else
    {
        conv.seq.useout = false;
    }
    if(conv.convert < 1)
    {
        //get address ready to send osc messages to
        conv.seq.usein = true;
        loaddr = lo_address_new_from_url(addr);
        printf(" sending osc messages to address %s\n",addr);
    }
    else
    {
        conv.seq.usein = false;
    }

    //start midi client
    if(!conv.mon_mode)
    {
        if(!init_midi_seq(&conv.seq,conv.verbose,clientname))
        {
            printf("MIDI connection failed");
            return -1;
        }
    }

    if(conv.verbose)
        printf("Ready.\n");
    fflush(stdout);

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

    //stop everything
    printf("\nquitting...\n");
    if(!conv.mon_mode)
    {
        if(conv.verbose)
            printf(" closing midi ports\n");
        close_midi_seq(&conv.seq);
    }
    if(conv.convert > -1)
    {
        if(conv.verbose)
            printf(" closing osc server\n");
        stop_osc_server(st);
    }
    if(conv.convert < 1)
    {
        lo_address_free(loaddr);
    }
    return 0;
}
