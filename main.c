//spencer jackson
//osc2midi.m

//main file for the osc2midi utility

#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include"pair.h"

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
    printf("    -mon           print all OSC messages that come into the port\n");
    printf("    -h             show this message\n");
    printf("\n");
    printf("NOTES:\n");
    printf("    By default it looks for mapping files relative to ~/.osc2midi/. for information\n");
    printf("     on how to create your own mapping see default.omm in that directory\n");
    printf("\n");

    return;
}

int main(int argc, char** argv)
{

    FILE* map;
    char line[400], path[200],s[200];
    int i;
    unsigned char glob_chan=0;
    PAIRHANDLE *p;

    //defaults
    strcpy(s,"default.omm");
    strcpy(path,getenv("HOME"));
    strcat(path,"/.osc2midi/");
    if(argc>1)
    {
    for (i = 1;i<argc;i++)
    {
        if (strcmp(argv[i], "-v") == 0) 
        {
             //verbose;
        }
        else if (strcmp(argv[i], "-map") == 0) 
        {
            //load map file
            strcpy(s,argv[++i]);
        }
        else if(strcmp(argv[i], "-mon") ==0)
        {
            //monitor mode (osc messages)
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
        //if(verbose)
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
        //if(verbose)
            printf("Using map file %s\n",path);
    }

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
            p[i] = alloc_pair(line,&glob_chan);
            if(p[i++])
            {
                //if(verbose)
                    printf("pair created: ");
                    print_pair(p[i-1]);
            }
            else
                i--;//error message will be printed by alloc_pair
        }
    }
    //if(verbose)
    {
        printf("%i pairs total.\n ",i-1);
    }


}
