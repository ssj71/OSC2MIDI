//Spencer Jackson
//pair.c

//This should be sufficient to take a config file (.omm) line and have everything
//necessary to generate a midi message from the OSC and visa versa.

#include<pair.h>

typedef struct _pair
{

    //osc data
    char**   path;
    int* perc;//point in path string with printf format %
    lo_arg** argv;//arguments
    lo_arg** argv_in_path;
    int argc;
    int argc_in_path;
    char* types;
    float* scale;
    float* offset;
    
    //conversion factors
    int* used;
    float** coeff;
    float** offset;
    uint8_t glob_chan_flag;
    uint8_t *glob_chan;

    //midi data
    //uint8_t midi[5];
    uint8_t opcode;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;

}pair;

int alloc_pair(pair* p, char* config, uint8_t *glob_chan)
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
    //figure out how many chunks it will be broken up into
    tmp = path;
    i = 1;
    while(tmp = strchr(tmp,'<'))i++;
    p->path = (char**)malloc(sizeof(char*)*i);
    p->perc = (int*)malloc(sizeof(int)*i);
    //now break it up into chunks
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
            free(p->path);
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
        p->perc[p->argc_in_path] = j;//mark where the format percent sign is
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
    while(p->argc_in_path>=0)
        free(p->path[p->argc_in_path--]);
    free(p->path);
    free(p->perc);
    free(p->arg_in_path);
    free(p->argv);
    free(p);
}

//returns 1 if match is successful
int try_match_osc(pair* p, char* path, char* types, lo_arg** argv, int argc)
{
    //check the easy things first
    if(argc < p->argc)
    {
       return 0;
    }
    if(strncmp(types,p->types,strlen(p->types)))
    {
       return 0;
    }


    //now start trying to get the data
    int i,v;
    char *tmp;
    for(i=0;i<p->argc_in_path;i--)
    {
  
        //does it match?
        p->path[p->perc[i]] = 0;
        tmp = strstr(path,p->path[i]);  
        p->path[p->perc[i]] = '%';
        if(!tmp)
        {
            return 0;
        }
        //get the argument
        if(!sscanf(tmp,p->path[i],&v))
        {
            return 0;
        }
        
        midi[p->map_in_path[i]] += (uint8_t)(p->scale_in_path[i]*v + p->offset_in_path);
        //add arg flag if arg then get index
        
    }
}



int set_channel(pair* p, uint8_t channel);
int try_match_midi(pair*, uint8_t msg[]);
uint8_t* get_midi(pair* p);
lo_message get_osc(pair* p);

