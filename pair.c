//Spencer Jackson
//pair.c

//This should be sufficient to take a config file (.omm) line and have everything
//necessary to generate a midi message from the OSC and visa versa.

#include<pair.h>

typedef struct _pair
{

    //osc data
    char   path[200];
    lo_arg** argv;//argument formats
    int argc;
    
    char**   informat;
    char**   outformat;
    int argc_in_path;
    
    //conversion factors
    float** coeff;
    float** offset;

    //midi data
    uint8_t midi[3];
    uint8_t opcode;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;

}pair;

int alloc_pair(pair* p, char* config)
{
    //path argtypes, arg1, arg2, ... argn : midicommand(arg1, arg3, 2*arg4);
    char path[200], argtypes[50], argnames[200], argname[100], midicommand[100], midiargs[200], work[50];
    char * tmp, *prev;
    unsigned short i;

    p = (pair*)malloc(sizeof(pair));

    i = sscanf(config,"%s %s,%s:%s(%s)",path,argtypes,argnames,argname,midicommand,midiargs);
    if(i==0)
        printf("ERROR in config line: %s, could not get OSC path!\n",config);
    else if(i==1)
        printf("ERROR in config line: %s, could not get OSC types!\n",config);
    else if(i==2)
        printf("ERROR in config line: %s, could not get OSC variable names!\n",config);
    else if(i==3)
        printf("ERROR in config line: %s, could not get MIDI command type!\n",config);
    else if(i==4)
        printf("ERROR in config line: %s, could not get MIDI command arguments!\n",config);
    if(i<5)
    {
        //abort this pair
        free(p);
        p=0;
        return -1;
    }
     
    //decide if path has some arguments in it
    prev = path;
    while(tmp = strchr(path,'<'))
    {
        prev = tmp;
        i = tmp-path;
        strncpy(p->path,path,i);
        p->path[i] = 0;
        strcat(p>path,"%i");
    }
    if(strchr(config,'<') && !p->path[0])
    {
        printf("ERROR in config line: %s, could not get variable from OSC path use \'<i>\'!\n",config);
    }
}

int free_pair(pair* p){

    free(p);
}

int try_match_osc(pair* p, char* oscpath, lo_arg** argv, int argc);
int set_channel(pair* p, uint8_t channel);
int try_match_midi(pair*, uint8_t msg[]);
uint8_t* get_midi(pair* p);
lo_message get_osc(pair* p);

