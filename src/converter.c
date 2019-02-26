//spencer jackson
//converter.c

//osc2midi converter utility functions

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

int is_empty(const char *s)
// check whether a config line is empty or just a comment
{
    while (*s != '\0')
    {
        if (*s == '#')
            return 1;
        else if (!isspace(*s))
            return 0;
        s++;
    }
    return 1;
}

// This must be called  with n == the
// number of config lines in the map, to allocate enough storage to hold all
// the register pointers that some pairs might share (note that there is <=
// one entry per configuration pair in the table).
void init_registers(float ***regs, int n)
{
    *regs = (float**)calloc(n, sizeof(float*));
}


int load_map(CONVERTER* conv, char* file)
{
    int i;
    char path[200],line[400],prefix[400],*home;
    FILE* map = NULL;
    FILE* tmp = NULL;
    PAIRHANDLE *p;
    int use_stdin = strcmp(file, "-") == 0;

    //try to load the file:
    //from stdin
    if(use_stdin)
    {
        strcpy(path, "<stdin>");
        map = stdin;
    }
    //absolute path
    if(!map)
    {
        map = fopen(strcpy(path,file),"r");
    }
    if(!map)
    {
        map = fopen(strcat(path,".omm"),"r");
    }
    if(!map)
    {
        //check at home
        home = getenv("XDG_CONFIG_HOME");
        if(!home)
        {
            home = getenv("HOME");
            strcpy(path,home);
            strcat(path,"/.config");
        }
        else
        {
            strcpy(path,home);
        }
        strcat(path,"/osc2midi/");
        map = fopen(strcat(path,file),"r");
    }
    if(!map)
    {
        //try with extra .omm
        map = fopen(strcat(path,".omm"),"r");
    }
    if(!map)
    {
        //try etc location
        strcpy(path,"/etc/osc2midi/");
        map = fopen(strcat(path,file),"r");
    }
    if(!map)
    {
        //try with extra .omm
        map = fopen(strcat(path,".omm"),"r");
    }
    if(!map)
    {
        //try default install location
        strcpy(path,PREFIX "/share/osc2midi/");
        map = fopen(strcat(path,file),"r");
    }
    if(!map)
    {
        //try with extra .omm
        map = fopen(strcat(path,".omm"),"r");
    }
    if(!map)
    {
        path[strlen(path)-4]=0;
        printf("Error opening map file! %s",path);
        fflush(stdout);
        printf("\b\b\b\n");
        return -1;
    }
    if(conv->verbose)
        printf("Using map file %s\n",path);

    //copy to temp file if reading from stdin
    if(use_stdin && !(tmp = tmpfile()))
    {
        printf("Error opening temporary file!\n");
        return -1;
    }

    //count how many lines there are
    i=0;
    while(!feof(map))
    {
        if (!fgets(line,400,map)) break;
        if (use_stdin) fputs(line,tmp);
        if(!is_empty(line))
            i++;//line is not commented out and not empty
    }
    if (use_stdin) map = tmp;

    p = (PAIRHANDLE*)malloc(sizeof(PAIRHANDLE)*i);
    //initialize the register table (cf. pair.c) -ag
    init_registers(&conv->registers,i);
    conv->tab = init_table();
    int nkeys = 0;
    rewind(map);
    i=0;
    *prefix = 0;
    while(!feof(map))
    {
        char rule[800];
        if (!fgets(line,400,map)) break;
        if(!is_empty(line))
        {
            // This provides a quick and dirty way to left-factor rules, where
            // the same osc message is mapped to different midi messages. -ag
            char *s = line;
            while (*s && isspace(*s)) ++s;
            if (*s == ':')
            {
                // Line starts with ':' delimiter, use prefix from previous
                // rule.
                strcat(strcpy(rule, prefix), s);
            }
            else
            {
                // Skip over the OSC path.
                while (*s && !isspace(*s)) ++s;
                // Skip over the rest of the lhs of the rule.
                while (*s && *s != ':') ++s;
                if (*s == ':')
                {
                    // Complete rule, store prefix for subsequent rules.
                    int n = s-line;
                    strncpy(prefix, line, n);
                    prefix[n] = 0;
                }
                strcpy(rule, line);
            }
            p[i] = alloc_pair(rule, conv->tab, conv->registers, &nkeys);
            if(p[i++])
            {
                if(conv->verbose)
                {
                    printf("pair created: ");
                    print_pair(p[i-1]);
                }
            }
            else
            {
                conv->errors++;
                i--;//error message will be printed by alloc_pair
            }
        }
    }
    free_table(conv->tab);
    if(conv->verbose)
    {
        printf("%i pairs created.\n",i);
    }
    conv->npairs = i;
    conv->p = p;
    return i;
}

static int missing_arg(const char *opt)
{
    printf("Missing argument! %s\n",opt);
    return -1;
}

int process_cli_args(int argc, char** argv, char* file, char* port, char* addr, char* clientname, CONVERTER* conv)
{
    int i;
    //defaults
    strcpy(file,"default.omm");
    strcpy(port,"57120");
    strcpy(addr,"osc.udp://localhost:8000");
    strcpy(clientname,"osc2midi");
    conv->verbose = 0;
    conv->dry_run = 0;
    conv->errors = 0;
    conv->mon_mode = 0;
    conv->multi_match = 1;
    conv->strict_match = 0;
    conv->glob_chan = 0;
    conv->glob_vel = 100;
    conv->filter = 0;
    conv->convert = 0;
    conv->seq.useout = 1;
    conv->seq.usein = 1;
    conv->seq.usefilter = 0;

    if(argc>1)
    {
        for (i = 1; i<argc; i++)
        {
            if(strcmp(argv[i], "-single") ==0)
            {
                //single matches
                conv->multi_match = 0;
            }
            else if(strcmp(argv[i], "-multi") ==0)
            {
                //multiple matches
                conv->multi_match = 1;
            }
            else if(strcmp(argv[i], "-strict") ==0)
            {
                //strict matches
                conv->strict_match = 1;
            }
            else if (strcmp(argv[i], "-map") == 0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
                //load map file
                strcpy(file,argv[++i]);
            }
            else if(strcmp(argv[i], "-mon") ==0)
            {
                //monitor mode (osc messages)
                conv->mon_mode = 1;
                conv->convert = 1;
            }
            else if(strcmp(argv[i], "-m2o") ==0)
            {
                //monitor mode (osc messages)
                conv->convert = -1;
            }
            else if(strcmp(argv[i], "-o2m") ==0)
            {
                //monitor mode (osc messages)
                conv->convert = 1;
            }
            else if (strcmp(argv[i], "-m") == 0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
                //load map file
                strcpy(file,argv[++i]);
            }
            else if (strcmp(argv[i], "-n") == 0)
            {
                //dry run (only check syntax and exit with error code)
                conv->dry_run = 1;
            }
            else if(strcmp(argv[i], "-p") ==0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
                //set osc server port
                strcpy(port,argv[++i]);
            }
            else if(strcmp(argv[i], "-a") ==0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
                //osc client address to send return osc messages to
                if(lo_url_get_protocol_id(argv[i+1]) < 0)
                    //protocol id missing, assume osc.udp
                    sprintf(addr, "osc.udp://%s", argv[++i]);
                else
                    strcpy(addr, argv[++i]);
            }
            else if(strcmp(argv[i], "-c") ==0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
                // global channel
                conv->glob_chan = atoi(argv[++i]);
                if(conv->glob_chan < 0) conv->glob_chan = 0;
                if(conv->glob_chan > 15) conv->glob_chan = 15;
            }
            else if(strcmp(argv[i], "-vel") ==0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
                // global velocity
                conv->glob_vel = atoi(argv[++i]);
                if(conv->glob_vel < 0) conv->glob_vel = 0;
                if(conv->glob_vel > 127) conv->glob_vel = 127;
            }
            else if(strcmp(argv[i], "-s") == 0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
                // filter shift
                conv->filter = atoi(argv[++i]);
                conv->seq.usefilter = 1;
                conv->seq.filter = &conv->filter;
            }
            else if (strcmp(argv[i], "-name") == 0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
                // JACK client name
                strcpy(clientname, argv[++i]);
            }
            else if (strcmp(argv[i], "-v") == 0)
            {
                conv->verbose = 1;
            }
            else if (strcmp(argv[i], "-h") == 0)
            {
                //help
                return -1;
            }
            else
            {
                printf("Unknown argument! %s\n",argv[i]);
                return -1;
            }

        }
    }//get args
    return 0;
}
