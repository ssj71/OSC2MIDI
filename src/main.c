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
#include"ht_stuff.h"

#ifndef PREFIX
#define PREFIX "/usr/local"
#endif

uint8_t quit = 0;

void quitter(int sig)
{
    quit = 1;
}

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
void init_regs(float ***regs, int n)
{
    *regs = (float**)calloc(n, sizeof(float*));
}


int load_map(CONVERTER* conv, char* file)
{
    int i;
    char path[200],line[400],*home;
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
    init_regs(&conv->registers,i);
    conv->tab = init_table();
    int nkeys = 0;
    rewind(map);
    i=0;
    while(!feof(map))
    {
        if (!fgets(line,400,map)) break;
        if(!is_empty(line))
        {
            p[i] = alloc_pair(line, conv->tab, conv->registers, &nkeys);
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

static int missing_arg(const char *opt)
{
    printf("Missing argument! %s\n",opt);
    usage();
    return -1;
}

int main(int argc, char** argv)
{
    char file[200], port[200], addr[200];
    int i;
    lo_address loaddr;
    CONVERTER conv;
    JACK_SEQ seq;

    //defaults
    strcpy(file,"default.omm");
    strcpy(port,"57120");
    strcpy(addr,"osc.udp://localhost:8000");
    conv.verbose = 0;
    conv.dry_run = 0;
    conv.errors = 0;
    conv.mon_mode = 0;
    conv.multi_match = 1;
    conv.strict_match = 0;
    conv.glob_chan = 0;
    conv.glob_vel = 100;
    conv.filter = 0;
    conv.convert = 0;
    seq.useout = 1;
    seq.usein = 1;
    seq.usefilter = 0;
    if(argc>1)
    {
        for (i = 1; i<argc; i++)
        {
            if(strcmp(argv[i], "-single") ==0)
            {
                //single matches
                conv.multi_match = 0;
            }
            else if(strcmp(argv[i], "-multi") ==0)
            {
                //multiple matches
                conv.multi_match = 1;
            }
            else if(strcmp(argv[i], "-strict") ==0)
            {
                //strict matches
                conv.strict_match = 1;
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
                conv.mon_mode = 1;
                conv.convert = 1;
            }
            else if(strcmp(argv[i], "-m2o") ==0)
            {
                //monitor mode (osc messages)
                conv.convert = -1;
            }
            else if(strcmp(argv[i], "-o2m") ==0)
            {
                //monitor mode (osc messages)
                conv.convert = 1;
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
                conv.dry_run = 1;
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
                conv.glob_chan = atoi(argv[++i]);
                if(conv.glob_chan < 0) conv.glob_chan = 0;
                if(conv.glob_chan > 15) conv.glob_chan = 15;
            }
            else if(strcmp(argv[i], "-vel") ==0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
                // global velocity
                conv.glob_vel = atoi(argv[++i]);
                if(conv.glob_vel < 0) conv.glob_vel = 0;
                if(conv.glob_vel > 127) conv.glob_vel = 127;
            }
            else if(strcmp(argv[i], "-s") == 0)
            {
                if (!argv[i+1]) return missing_arg(argv[i]);
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
                usage();
                return -1;
            }
            else
            {
                printf("Unknown argument! %s\n",argv[i]);
                usage();
                return -1;
            }

        }
    }//get args


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
            seq.usefilter = 1;
            seq.filter = &conv.filter;
            printf("Found pair %i with filter functions, creating midi filter in/out pair.\n", i);
        }
    }
    else if(conv.verbose)
        printf("Monitor mode, incoming OSC messages will only be printed.\n");

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
        loaddr = lo_address_new_from_url(addr);
        printf(" sending osc messages to address %s\n",addr);
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
