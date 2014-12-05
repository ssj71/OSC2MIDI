//spencer jackson
//osc2midi.m

//main file for the osc2midi utility

#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include"pair.h"
#include"oscserver.h"
#include"converter.h"

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
    printf("    -a <value>     address of OSC client for midi->OSC\n");
    printf("    -c <value>     set default MIDI channel\n");
    printf("    -multi         multi mode (check all mappings/send multiple messages)\n");
    printf("    -single        multi mode off (stop checks after first match)\n");
    printf("    -mon           only print OSC messages that come into the port\n");
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

    FILE* map;
    char line[400], path[200],s[200], port[200], addr[200];
    int i;
    PAIRHANDLE *p;
    CONVERTER conv;

    //defaults
    strcpy(s,"default.omm");
    strcpy(path,getenv("HOME"));
    strcat(path,"/.osc2midi/");
    strcpy(port,"57120");
    strcpy(addr,"");
    conv.verbose = 0;
    conv.mon_mode = 0;
    conv.multi_match = 1;
    conv.glob_chan = 0;
    if(argc>1)
    {
    for (i = 1;i<argc;i++)
    {
        if (strcmp(argv[i], "-v") == 0) 
        {
             conv.verbose = 1;
        }
        else if(strcmp(argv[i], "-single") ==0)
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
            strcpy(s,argv[++i]);
        }
        else if(strcmp(argv[i], "-mon") ==0)
        {
            //monitor mode (osc messages)
            conv.mon_mode = 1;
        }
        else if (strcmp(argv[i], "-m") == 0) 
        {
            //load map file
            strcpy(s,argv[++i]);
        }
        else if(strcmp(argv[i], "-p") ==0)
        {
            strcpy(port,argv[++i]);
        }
        else if(strcmp(argv[i], "-a") ==0)
        {
            strcpy(addr,argv[++i]);
        }
        else if(strcmp(argv[i], "-c") ==0)
        {
            conv.glob_chan = atoi(argv[++i]);
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
        //try to load the file:
        if(s[0] == '/')
        {
            //absolute path
            map = fopen(s,"r");
            if(!map)
            {
                printf("Error opening map file! %s",s);
                fflush(stdout);
                printf("\b\b\b\n");
                return -1;
            }
            if(conv.verbose)
                printf("Using map file %s\n",s);
        }
        else 
        {
            map = fopen(strcat(path,s),"r");
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
            if(conv.verbose)
                printf("Using map file %s\n",path);
        }

        //count how many lines there are
        i=0;
        while(!feof(map))
        {
            fgets(line,400,map);
            if(!sscanf(line,"%[ \t#]",s))
                i++;//line is not commented out
        }

        p = (PAIRHANDLE)malloc(sizeof(PAIRHANDLE)*i);
        rewind(map);
        i=0;
        while(!feof(map))
        {
            fgets(line,400,map);
            if(!sscanf(line,"%[ \t#]",s) && !is_empty(line))
            {
                p[i] = alloc_pair(line);
                if(p[i++])
                {
                    if(conv.verbose)
                    {
                        printf("pair created: ");
                        print_pair(p[i-1]);
                    }
                }
                else
                    i--;//error message will be printed by alloc_pair
            }
        }
        if(conv.verbose)
        {
            printf("%i pairs created.\n ",i-1);
        }
        conv.npairs = i-1;
        conv.p = p;
    }
    else if(conv.verbose)
        printf("Monitor mode, OSC messages will only be printed.\n");

    //start the server
    lo_server_thread st;
    st = start_osc_server(port,&conv);

    while(1)
    {
        usleep(1000);
    }
    stop_osc_server(st);
}
