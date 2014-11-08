//Spencer Jackson
//pair.c

//This should be sufficient to take a config file (.omm) line and have everything
//necessary to generate a midi message from the OSC and visa versa.

#include<pair.h>

typedef struct _pair
{

    //osc data
    char**   path;
    lo_arg** argv;//arguments
    int argc;
    int argc_in_path;
    char* types;
    
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
    unsigned short i,j,n;

    p = (pair*)malloc(sizeof(pair));

    //set defaults
    p->argc_in_path = 0;
    p->argc = 0;

    //break config into separate parts
    i = sscanf(config,"%s %[^,],%[^:]:%[^(](%[^)])",path,argtypes,argnames,argname,midicommand,midiargs);
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
    j = 0;
    char var[100];
    while(tmp = strchr(prev,'<'))
    {
        //get size of this part of path and allocate a string
        n = tmp - prev;
        i = 0;
        tmp = prev;
        while(tmp = strchr(tmp,'%')) i++;
        p->path[p->argc_in_path] = (char*)malloc(sizeof(char)*n+i+3);
        //double check the variable is good
        i = sscanf(prev,"%*[^<]<%[^>]",var);
        if(i < 1 || !strchr(var,'i'))
        {
            printf("ERROR in config line: %s, could not get variable from OSC path, use \'<i>\'!\n",config);
            while(p->argc_in_path >=0)
            {
                free(p->path[p->argc_in_path--]);
            }
            free(p)
            return -1;
        }
        //copy over path segment, delimit any % characters 
        j=0;
        for(i=0;i<n;i++)
        {
            if(prev[i] != '%')
                p->path[p->argc_in_path][j++] = '%';
            p->path[p->argc_in_path][j++] = prev[i];
        }
        p->path[p->argc_in_path][j] = 0;//null terminate to be safe
        strcat(p->path[p->argc_in_path++],"%i")
        prev++ = strchr(prev,'>');
    }
    //allocate space for end of path
    p->path[p->argc_in_path] = (char*)malloc(sizeof(char)*strlen(prev));
    strcpy(p->path[p->argc_in_path],prev);


    //now get the argument types
    j = 0;
    p->types = (char*)malloc(sizeof(char)*strlen(argtypes));
    for(i=0;i<strlen(argtypes);i++)
    {
        switch(argtypes[i])
        {
            case 'F':
            case 'f':
                //allocate an argument
                p->types[j] = argtypes[i];


            
        }
    }
}

int free_pair(pair* p)
{
    while(p->argc_in_path)
        free(p->path[p->argc_in_path--]);
    free(p->path[p->argc_in_path]);
    free(p);
}

int try_match_osc(pair* p, char* oscpath, lo_arg** argv, int argc);
int set_channel(pair* p, uint8_t channel);
int try_match_midi(pair*, uint8_t msg[]);
uint8_t* get_midi(pair* p);
lo_message get_osc(pair* p);

