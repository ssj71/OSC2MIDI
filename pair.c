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
    int argc;
    int argc_in_path;
    char* types;
    
    //conversion factors
    int8_t* map;//which byte in the midi message by index of var in OSC message (including in path args)
    float scale[4];//scale factor for each argument going into the midi message
    float offset[4];//linear offset for each arg going to midi message
    uint8_t *glob_chan;//pointer to global channel
    uint8_t use_glob_chan;//flag decides if using global channel (1) or if its specified by message (0)
    uint8_t set_channel;//flag if message is actually control message to change global channel
    uint8_t raw_midi;//flag if message sends osc datatype of midi message

    //midi data
    uint8_t opcode;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;

}pair;

void print_pair(pair* p)
{
    int i;

    //path
    for(i=0;i<p->argc_in_path;i++)
    {
        printf("%s",p->path[i]);
    }
    
    //types
    printf(" %s, ",p->types);
    
    //arg names
    for(i=0;i<p->argc_in_path;i++)
    {
        printf("%c, ",'a'+i);
    }
    for(;i<p->argc+p->argc_in_path;i++)
    {
        printf("%c, ",'a'+i);
    }
    
    //command global channel
    printf(": rawmidi(");
    if(p->use_glob_chan)
        printf(" glob_chan +");

    //midi arg 0
    printf(" %i + %i + ",p->opcode,p->channel);
    for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=0;i++);
    if(i<p->argc+p->argc_in_path)
        printf("%.2f*%c + %.2f",p->scale[0],'a'+i,p->offset[0]);
    
    //midi arg3 (only for note on/off)
    for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=3;i++);
    if(i<p->argc+p->argc_in_path)
        printf(" + %.2f*%c + %.2f",p->scale[3],'a'+i,p->offset[3]);
    
    //midi arg1
    printf(", %i + ",p->data1);
    for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=1;i++);
    if(i<p->argc+p->argc_in_path)
        printf("%.2f*%c + %.2f",p->scale[1],'a'+i,p->offset[1]);
    
    //midi arg2
    printf(", %i + ",p->data2);
    for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=2;i++);
    if(i<p->argc+p->argc_in_path)
        printf("%.2f*%c + %.2f",p->scale[2],'a'+i,p->offset[2]);

    printf(")\n");
}

void rm_whitespace(char* str)
{
    int i,j;
    int n = strlen(str);
    for(i=j=0;j<n;i++)//remove whitespace
    {
        while(str[j] == ' ' || str[j] == '\t')
        {
            j++;
        }
        str[i] = str[j++];
    }
}
int get_pair_path(char* path, pair* p)
{
    //decide if path has some arguments in it
    char* tmp,*prev = path;
    int n,i,j = 0;
    char var[100];
    //figure out how many chunks it will be broken up into
    tmp = path;
    i = 1;
    while(tmp = strchr(tmp,'{'))i++;
    p->path = (char**)malloc(sizeof(char*)*i);
    p->perc = (int*)malloc(sizeof(int)*i);
    //now break it up into chunks
    while(tmp = strchr(prev,'{'))
    {
        //get size of this part of path and allocate a string
        n = tmp - prev;
        i = 0;
        tmp = prev;
        while(tmp = strchr(tmp,'%')) i++;
        p->path[p->argc_in_path] = (char*)malloc(sizeof(char)*n+i+3);
        //double check the variable is good
        i = sscanf(prev,"%*[^{]<%[^}]",var);
        if(i < 1 || !strchr(var,'i'))
        {
            printf("ERROR in config line: %s, could not get variable in OSC path, use \'<i>\'!\n",config);
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
        prev++ = strchr(prev,'}');
    }
    //allocate space for end of path and copy
    p->path[p->argc_in_path] = (char*)malloc(sizeof(char)*strlen(prev));
    strcpy(p->path[p->argc_in_path],prev);
}

int get_path_argtypes(char* argtypes, pair* p)
{
    //now get the argument types
    int i,j = 0;
    p->types = (char*)malloc( sizeof(char) * (strlen(argtypes)+1) );
    p->map = (signed char*)malloc( sizeof(char) * (strlen(argtypes)+1) );
    for(i=0;i<strlen(argtypes);i++)
    {
        switch(argtypes[i])
        {
            case 'i':
            case 'h'://long
            case 's'://string
            case 'b'://blob
            case 'f':
            case 'd':
            case 'S'://symbol
            case 't'://timetag
            case 'c'://char
            case 'm'://midi
            case 'T'://true
            case 'F'://false
            case 'N'://nil
            case 'I'://infinity
                p->map[j] = -1;//initialize mapping to be not used
                p->types[j++] = argtypes[i];
                p->argc++;
            case ' ':
                break;
            default:
                printf("ERROR in config line: %s, argument type '%c' not supported!\n",config,argtypes[i]);
                return -1;
                break;
        }
    }
    p->types[j] = 0;//null terminate. Its good practice
}

int get_pair_midicommand(char* midicommand, path* p)
{

    int n;
    //next the midi command
    /*
      noteon( channel, noteNumber, velocity );
      noteoff( channel, noteNumber, velocity );
      note( channel, noteNumber, velocity, state );  # state dictates note on (state != 0) or note off (state == 0)
      polyaftertouch( channel, noteNumber, pressure );
      controlchange( channel, controlNumber, value );
      programchange( channel, programNumber );
      aftertouch( channel, pressure );
      pitchbend( channel, value );
      rawmidi( byte0, byte1, byte2 );  # this sends whater midi message you compose with bytes 0-2
      midimessage( message );  # this sends a message using the OSC type m which is a pointer to a midi message

         non-Midi functions that operate other system functions are:
      setchannel( channelNumber );  # set the global channel
      setshift( noteOffset ); # set the midi note filter shift amout
  */
    if(strstr(midicommand,"noteon"))
    {
        p->opcode = 0x90;
        n = 3;
    }
    else if(strstr(midicommand,"noteoff"))
    {
        p->opcode = 0x80;
        n = 3;
    }
    else if(strstr(midicommand,"note"))
    {
        p->opcode = 0x80;
        n = 4;
    }
    else if(strstr(midicommand,"polyaftertouch"))
    {
        p->opcode = 0xA0;
        n = 3;
    }
    else if(strstr(midicommand,"controlchange"))
    {
        p->opcode = 0xB0;
        n = 3;
    }
    else if(strstr(midicommand,"programchange"))
    {
        p->opcode = 0xC0;
        n = 2;
    }
    else if(strstr(midicommand,"aftertouch"))
    {
        p->opcode = 0xD0;
        n = 2;
    }
    else if(strstr(midicommand,"pitchbend"))
    {
        p->opcode = 0xE0;
        n = 2;
    }
    else if(strstr(midicommand,"rawmidi"))
    {
        p->opcode = 0x00;
        n = 3;
        p->rawmidi = 1;
    }
    else if(strstr(midicommand,"midimessage"))
    {
        p->opcode = 0x00;
        n = 1;
    }
    //non-midi commands
    else if(strstr(midicommand,"setchannel"))
    {
        p->opcode = 0x00;
        n = 1;
        p->set_channel = 1;
    }
    else if(strstr(midicommand,"setshift"))
    {
        p->opcode = 0x00;
        n = 1;
    }
    else
    {
        printf("ERROR in config line: %s, midi command %s unknown!\n",config,midicommand);
        return -1;
    }
    return n;
}

int abort_pair_alloc(int step)
{
    switch(step)
    {
        case 3:
            //p->argc_in_path += p->argc;
            free(p->types);
            free(p->map);
        case 2:
            while(p->argc_in_path >=0)
            {
                free(p->path[p->argc_in_path--]);
            }
            free(p->perc)
            free(p->path);
        case 1:
            free(p);
        default:
            break;
    }
    return -1;
}

int alloc_pair(pair* p, char* config, uint8_t *glob_chan)
{
    //path argtypes, arg1, arg2, ... argn : midicommand(arg1+4, arg3, 2*arg4);
    char path[200], argtypes[50], argnames[200], midicommand[100], midiargs[200];
    char * tmp, *prev;
    char *marg[4];
    unsigned short i,j,n;
    float f;
    uint8_t arg[4];

    p = (pair*)malloc(sizeof(pair));

    //set defaults
    p->argc_in_path = 0;
    p->argc = 0;
    p->raw_midi = 0;
    p->opcode = 0;
    p->channel = 0;
    p->data1 = 0;
    p->data2 = 0;

    //break config into separate parts
    i = sscanf(config,"%s %[^,],%[^:]:%[^(](%[^)])",path,argtypes,argnames,midicommand,midiargs);
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
        return abort_pair_alloc(1);
    }
     
    if(-1 == get_pair_path(path,p))
        return abort_pair_alloc(2);


    if(-1 == get_pair_argtypes(argtypes,p))
        return abort_pair_alloc(3);


    n = get_pair_midicommand(midicommand,p);
    if(-1 == n)
        return abort_pair_alloc(3);

    
    char arg0[70], arg1[70], arg2[70], arg3[70], pre[50], var[50], post[50], work[50];
    //lets get those arguments
    i = sscanf(midiargs,"%[^,],%[^,],%[^,],%[^,]",arg0,arg1,arg2,arg3);
    if(n != i)
    {
        printf("ERROR in config line: %s, incorrect number of args in midi command!\n",config);
        while(p->argc_in_path >=0)
        {
            free(p->path[p->argc_in_path--]);
        }
        free(p->perc);
        free(p->types);
        free(p->map);
        free(p->path);
        free(p)
        return -1;
    }

    //and the most difficult part: the mapping
    marg[0] = arg0;
    marg[1] = arg1;
    marg[2] = arg2;
    marg[3] = arg3;
    pre[0] = 0;
    var[0] = 0;
    post[0] = 0;
    for(i=0;i<n;i++)//go through each argument
    {
        tmp = marg[i];
        if( !(j = sscanf(tmp,"%[.1234567890*/+- ]%[^*/+- ]%[.1234567890*/+- ]",pre,var,post)) )
        {
            j = sscanf(tmp,"%[^*/+-]%[.1234567890*/+-]",var,post);
        }
        if(!j)
        {
            printf("ERROR in config line: %s, could not understand arg %i in midi command\n",config,i);
            p->argc_in_path += p->argc;
            while(p->argc_in_path >=0)
            {
                free(p->path[p->argc_in_path--]);
            }
            free(p->types);
            free(p->map);
            free(p->path);
            free(p)
            return -1;
        }
        if (strlen(var)==0)
        {
            //must be a constant
            if(!sscanf(pre,"%f",&f))
            {
                printf("ERROR in config line: %s, could not get constant arg %i in midi command\n",config,i);
                p->argc_in_path += p->argc;
                while(p->argc_in_path >=0)
                {
                    free(p->path[p->argc_in_path--]);
                }
                free(p->types);
                free(p->map);
                free(p->path);
                free(p)
                return -1;
            }
            p->scale[i] = 0;
            p->offset[i] = 0;
            arg[i] = (uint8_t)f;
        }
        else//not a constant
        {
            //check if its the global channel keyword
            if(!strncmp(var,"channel",7))
            {
                p->use_glob_chan = 1;
                p->scale[i] = 0;
                p->offset[i] = 0;
                arg[i] = 0;
            }
            else
            {
                //find where it is in the OSC message
                rm_whitespace(argnames);
                n = strlen(var);//verify n and tmp aren't used in this scope!
                tmp = argnames;
                for(j=0;j<p->argc_in_path+p->argc;j++)
                {
                    if(!strncmp(var,tmp,n))
                    {
                        
                    }
                }
                //get conditioning
            }


        }//not constant
    }//for each arg
    

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

//returns 1 if match is successful and msg has a message to be sent to the output
int try_match_osc(pair* p, char* path, char* types, lo_arg** argv, int argc, uint8_t msg)
{
    //check the easy things first
    if(argc < p->argc)
    {
       return 0;
    }
    if(strncmp(types,p->types,strlen(p->types)))//this won't work if it switches between T and F
    {
       return 0;
    }


    //set defaults / static data
    msg[0] = p->opcode + p->channel;
    msg[1] = p->data1;
    msg[2] = p->data2;
    if(p->use_glob_chan)
    {
        msg[0] += *p->glob_chan;
    }

    //now start trying to get the data
    int i,v;
    char *tmp;
    for(i=0;i<p->argc_in_path;i++)
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
        //put it in the message;
        if(p->map[i] != -1)
        {
            msg[p->map[i]] += (uint8_t)(p->scale[i]*v + p->offset[i]); 
        }
    }
    //compare the end of the path
    if(!strstr(path,p->path[i]))
    {
        return 0;
    }


    //now the actual osc args
    int j = 0;
    double val;
    for(i=0;i<p->argc;i++)
    {
        //put it in the message;
        if(p->map[i+p->argc_in_path] != -1)
        {
            switch(types[i])
            {
                case 'i':
                    val = (double)argv[j++]->i;
                    break;
                case 'h'://long
                    val = (double)argv[j++]->h;
                    break;
                case 'f':
                    val = (double)argv[j++]->f;
                    break;
                case 'd':
                    val = (double)argv[j++]->d;
                    break;
                case 'c'://char
                    val = (double)argv[j++]->c;
                    break;
                case 'T'://true
                    val = 1.0;
                    break;
                case 'F'://false
                    val = 0.0;
                    break;
                case 'N'://nil
                    val = 0.0;
                    break;
                case 'I'://impulse
                    val = 1.0;
                    break;
                case 'm'://midi
                    //send full midi message and exit
                    if(p->raw_midi)
                    {
                        msg[0] = argv[j+1];
                        msg[1] = argv[j+2];
                        msg[2] = argv[j+3];
                        return 1;
                    }

                case 's'://string
                case 'b'://blob
                case 'S'://symbol
                case 't'://timetag
                default:
                    //this isn't supported, they shouldn't use it as an arg, return error
                    return 0;

            }
            //check if this is a message to set global channel
            if(p->set_channel)
            {
                *p->glob_chan = (uint8_t)val;
                return 0;//not an error but don't need to send a midi message
            }   
            if(p->map[i+p->argc_in_path] == 3)//only used for note on or off
            {
                msg[0] += ((uint8_t)(p->scale[i+p->argc_in_path]*val + p->offset[i+p->argc_in_path])>0)<<4;
            }
            else
            {
                msg[p->map[i+p->argc_in_path]] += (uint8_t)(p->scale[i+p->argc_in_path]*val + p->offset[i+p->argc_in_path]); 
            }
        }//if arg is used
    }//for args
}

int try_match_midi(pair*, uint8_t msg[], lo_message *osc);

