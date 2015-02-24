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
#include"jackdriver.h"

uint8_t quit = 0;

void quitter(int sig)
{
    quit = 1;
}

int is_empty(const char *s) 
{
    while (*s != '\0') 
    {
        if (!isspace(*s))
            return 0;
        s++;
    }
    return 1;
}

int load_map(CONVERTER* conv, char* path, char* file)
{
    int i;
    char file2[200],line[400];
    FILE* map;
    PAIRHANDLE *p;

    //try to load the file:
    //if(file[0] == '/')
    {
        //absolute path
        map = fopen(file,"r");
        if(!map)
        {
            strcpy(file2,file);
            map = fopen(strcat(file2,".omm"),"r");
        }
        if(map && conv->verbose)
            printf("Using map file %s\n",file);
    }
    if(!map)
    {
        map = fopen(strcat(path,file),"r");
        if(!map)
        {
            map = fopen(strcat(path,".omm"),"r");
        }
        if(!map)
        {
            printf("Error opening map file! %s",path);
            fflush(stdout);
            printf("\b\b\b\n");
            return -1;
        }
        if(conv->verbose)
            printf("Using map file %s\n",path);
    }

    //count how many lines there are
    i=0;
    while(!feof(map))
    {
        fgets(line,400,map);
        if(!sscanf(line,"%[ \t#]",file))
            i++;//line is not commented out
    }

    p = (PAIRHANDLE*)malloc(sizeof(PAIRHANDLE)*i);
    rewind(map);
    i=0;
    while(!feof(map))
    {
        fgets(line,400,map);
        if(!sscanf(line,"%[ \t#]",file) && !is_empty(line))
        {
            p[i] = alloc_pair(line);
            if(p[i++])
            {
                if(conv->verbose)
                {
                    printf("pair created: ");
                    print_pair(p[i-1]);
                }
            }
            else
                i--;//error message will be printed by alloc_pair
        }
    }
    if(conv->verbose)
    {
        printf("%i pairs created.\n ",i);
    }
    conv->npairs = i;
    conv->p = p;
    return i;
} 

void useage()
{
    printf("osc2midi - a linux OSC to MIDI bridge\n");
    printf("\n");
    printf("\n");
    printf("USEAGE:\n");
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
    printf("    -mon           only print OSC messages that come into the port\n");
    printf("    -o2m           only convert OSC messages to MIDI\n");
    printf("    -m2o           only convert MIDI messages to OSC\n");
    printf("    -h             show this message\n");
    printf("\n");
    printf("NOTES:\n");
    printf("    By default it looks for mapping files relative to ~/.osc2midi/. for information\n");
    printf("     on how to create your own mapping see default.omm in that directory\n");
    printf("    Multi match mode is heavier: It checks all potential matches, but its useful\n");
    printf("     if OSC messages contain more data than can be sent in a single MIDI message\n");
    printf("     by default multi mode is on. Pass -single to disable\n");
    printf("\n");

    return;
}

int main(int argc, char** argv)
{
    char path[200],file[200], port[200], addr[200], aport[50];
    int i;
    lo_address loaddr;
    CONVERTER conv;
    JACK_SEQ seq;

    //defaults
    strcpy(file,"default.omm");
    strcpy(path,getenv("HOME"));
    strcat(path,"/.osc2midi/");
    strcpy(port,"57120");
    strcpy(addr,"");
    strcpy(aport,"8000");
    conv.verbose = 0;
    conv.mon_mode = 0;
    conv.multi_match = 1;
    conv.glob_chan = 0;
    conv.glob_vel = 100;
    conv.filter = 0;
    conv.convert = 0;
    seq.useout = 1;
    seq.usein = 1;
    seq.usefilter = 0;
    if(argc>1)
    {
    for (i = 1;i<argc;i++)
    {
        if(strcmp(argv[i], "-single") ==0)
        {
            //monitor mode (osc messages)
            conv.multi_match = 0;
        }
        else if(strcmp(argv[i], "-multi") ==0)
        {
            //monitor mode (osc messages)
            conv.multi_match = 1;
        }
        else if (strcmp(argv[i], "-map") == 0) 
        {
            //load map file
            strcpy(file,argv[++i]);
        }
        else if(strcmp(argv[i], "-mon") ==0)
        {
            //monitor mode (osc messages)
            conv.mon_mode = 1;
            conv.convert = 1;
        }
        else if(strcmp(argv[i], "-m2o") ==0)
        {
            //monitor mode (osc messages)
            conv.convert = 1;
        }
        else if(strcmp(argv[i], "-o2m") ==0)
        {
            //monitor mode (osc messages)
            conv.convert = -1;
        }
        else if (strcmp(argv[i], "-m") == 0) 
        {
            //load map file
            strcpy(file,argv[++i]);
        }
        else if(strcmp(argv[i], "-p") ==0)
        {
            //set osc server port
            strcpy(port,argv[++i]);
        }
        else if(strcmp(argv[i], "-a") ==0)
        {
            //osc client address to send return osc messaged so
            //strcpy(addr,argv[++i]);
            if(!sscanf(argv[++i],"%[^:]:%[^:]",addr,aport))
            {
                sscanf(argv[i],":%[^:]",aport);
            }
        }
        else if(strcmp(argv[i], "-c") ==0)
        {
            // global channel
            conv.glob_chan = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-vel") ==0)
        {
            // global velocity
            conv.glob_vel = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-s") == 0)
        {
            // filter shift
            conv.filter = atoi(argv[++i]);
            seq.usefilter = 1;
            seq.filter = &conv.filter;
        }
        else if (strcmp(argv[i], "-v") == 0) 
        {
             conv.verbose = 1;
        }
        else if (strcmp(argv[i], "-h") == 0) 
        {
            //help
            useage();
        }
        else
        {
            printf("Unknown argument! %s\n",argv[i]);
            useage();
        }

    }
    }//get args


    if(!conv.mon_mode)
    {
        if(load_map(&conv,path,file) == -1)
            return -1;
        if( (i = check_pair_set_for_filter(conv.p,conv.npairs)) )
        {
            seq.usefilter = 1;
            seq.filter = &conv.filter;
                printf("Found pair %i with filter functions, creating midi filter in/out pair.\n", i); 
        }
    }
    else if(conv.verbose)
        printf("Monitor mode, incoming OSC or MIDI messages will only be printed.\n");

    //start the server
    lo_server_thread st;
    if(conv.convert > -1)
    {
        st = start_osc_server(port,&conv);
        seq.useout = 1;
    }
    else
    {
        seq.useout = 0;
    }
    if(conv.convert < 1)
    {
        //get address ready to send osc messages to
        seq.usein = 1;
        loaddr = lo_address_new(addr,aport);
        printf(" sending osc messages to address %s:%s\n",addr,aport);
    }
    else
    {
        seq.usein = 0;
    }

    //start JACK client
    if(!conv.mon_mode)
    {
        if(!init_jack(&seq,conv.verbose))
        {
            printf("JACK connection failed");
            return -1;
        }
        conv.seq = (void*)&seq;
    }

    if(conv.verbose)
        printf("Ready.\n");

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
            printf(" closing jack ports\n");
        close_jack(&seq);
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
