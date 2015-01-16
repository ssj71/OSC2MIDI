//Spencer Jackson
//pair.c

//This should be sufficient to take a config file (.omm) line and have everything
//necessary to generate a midi message from the OSC and visa versa.

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"pair.h"


typedef struct _PAIR
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

    //flags
    uint8_t use_glob_chan;//flag decides if using global channel (1) or if its specified by message (0)
    uint8_t set_channel;//flag if message is actually control message to change global channel
    uint8_t use_glob_vel;//flag decides if using global velocity (1) or if its specified by message (0)
    uint8_t set_velocity;//flag if message is actually control message to change global velocity
    uint8_t set_shift;//flag if message is actually control message to change filter shift value
    uint8_t raw_midi;//flag if message sends osc datatype of midi message
    uint8_t use[3];//flags for midi message (channel, data1, data2)

    //midi data
    uint8_t opcode;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;

}PAIR;


void print_pair(PAIRHANDLE ph)
{
    int i;
    PAIR* p = (PAIR*)ph;

    //path
    for(i=0;i<p->argc_in_path;i++)
    {
        p->path[i][p->perc[i]] = '{';
        printf("%s}",p->path[i]);
        p->path[i][p->perc[i]] = '%';
    }
        printf("%s",p->path[i]);
    
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
    
    //command
    printf("\b\b : %s",opcode2cmd(p->opcode, 0));
    if(p->opcode==0x80)
    {
        //note or note off, check if 4 arguments
        for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=3;i++);
        if(i==p->argc+p->argc_in_path)
            printf("off");
    }
    printf("(");

    //global channel
    if(p->use_glob_chan)
        printf(" channel");

    //midi arg 0 
    if(p->use[0])printf(" %i",p->channel);
    for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=0;i++);
    if(i<p->argc+p->argc_in_path)
        printf("%.2f*%c + %.2f",p->scale[0],'a'+i,p->offset[0]);
    
    
    //midi arg1 
    if(p->use[1])printf(", %i",p->data1);
    for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=1;i++);
    if(i<p->argc+p->argc_in_path)
        printf(", %.2f*%c + %.2f",p->scale[1],'a'+i,p->offset[1]);
    
    //midi arg2
    if(p->use_glob_vel)
        printf(", velocity");
    if(p->use[2])printf(", %i",p->data2);
    for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=2;i++);
    if(i<p->argc+p->argc_in_path)
        printf(", %.2f*%c + %.2f",p->scale[2],'a'+i,p->offset[2]);

    //midi arg3 (only for note on/off)
    for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=3;i++);
    if(i<p->argc+p->argc_in_path)
        printf(", %.2f*%c + %.2f",p->scale[3],'a'+i,p->offset[3]);

    printf(" )\n");

    //debugging view
#if(0)
    printf("  l: %i, map: ", p->argc+p->argc_in_path);
    for(i=0;i<p->argc+p->argc_in_path;i++)
        printf("%i ",p->map[i]);
    printf("\n");
    printf("       scale: ");
    for(i=0;i<p->argc+p->argc_in_path;i++)
        printf("%.2f ",p->scale[i]);
    printf("\n");
    printf("      offset: ");
    for(i=0;i<p->argc+p->argc_in_path;i++)
        printf("%.2f ",p->offset[i]);
    printf("\n");
#endif
}

int check_pair_set_for_filter(PAIRHANDLE* pa, int npairs)
{
    PAIR* p;
    int i;

    for(i=0;i<npairs;i++)
    {
        p = (PAIR*)pa[i];
        if(p->set_shift)
            return i+1;
    }
    return 0;
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

int get_pair_path(char* config, PAIR* p)
{
    char path[200];
    char* tmp,*prev;
    int n,i,j = 0;
    char var[100];
    if(!sscanf(config,"%s %*[^,],%*[^:]:%*[^(](%*[^)])",path))
    {
        printf("\nERROR in config line:\n%s -could not get OSC path!\n\n",config);
        return -1;
    }

    //decide if path has some arguments in it
    //figure out how many chunks it will be broken up into
    prev = path;
    tmp = path;
    i = 1;
    while( (tmp = strchr(tmp,'{')) )
    {
        i++;
        tmp++;
    }
    p->path = (char**)malloc(sizeof(char*)*i);
    p->perc = (int*)malloc(sizeof(int)*(i-1));
    //now break it up into chunks
    while( (tmp = strchr(prev,'{')) )
    {
        //get size of this part of path and allocate a string
        n = tmp - prev;
        i = 0;
        tmp = prev;
        while( (tmp = strchr(tmp,'%')) ) i++;
        p->path[p->argc_in_path] = (char*)malloc(sizeof(char)*n+i+3);
        //double check the variable is good
        i = sscanf(prev,"%*[^{]{%[^}]}",var);
        if(i < 1 || !strchr(var,'i'))
        {
            printf("\nERROR in config line:\n%s -could not get variable in OSC path, use \'{i}\'!\n\n",config);
            return -1;
        }
        //copy over path segment, delimit any % characters 
        j=0;
        for(i=0;i<n;i++)
        {
            if(prev[i] == '%')
                p->path[p->argc_in_path][j++] = '%';
            p->path[p->argc_in_path][j++] = prev[i];
        }
        p->path[p->argc_in_path][j] = 0;//null terminate to be safe
        p->perc[p->argc_in_path] = j;//mark where the format percent sign is
        strcat(p->path[p->argc_in_path++],"%i");
        prev = strchr(prev,'}');
        prev++;
    }
    //allocate space for end of path and copy
    p->path[p->argc_in_path] = (char*)malloc(sizeof(char)*strlen(prev));
    strcpy(p->path[p->argc_in_path],prev);
    return 0;
}

int get_pair_argtypes(char* config, PAIR* p)
{
    char argtypes[100];
    int i,j = 0;
    if(!sscanf(config,"%*s %[^,],%*[^:]:%*[^(](%*[^)])",argtypes))
    {
        printf("\nERROR in config line:\n%s -could not get OSC data types!\n\n",config);
        return -1;
    }

    //now get the argument types
    p->types = (char*)malloc( sizeof(char) * (strlen(argtypes)+1) );
    p->map = (signed char*)malloc( sizeof(char) * (p->argc_in_path+strlen(argtypes)+1) );
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
                printf("\nERROR in config line:\n%s -argument type '%c' not supported!\n\n",config,argtypes[i]);
                return -1;
                break;
        }
    }
    p->types[j] = 0;//null terminate. Its good practice
    for(i=0;i<strlen(argtypes)+p->argc_in_path;i++)
        p->map[j] = -1;//initialize mapping for in-path args
    return 0;
}

int get_pair_midicommand(char* config, PAIR* p)
{
    char midicommand[100];
    int n;
    if(!sscanf(config,"%*s %*[^,],%*[^:]:%[^(](%*[^)])",midicommand))
    {
        printf("\nERROR in config line:\n%s -could not get MIDI command!\n\n",config);
        return -1;
    }

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
        p->raw_midi = 1;
    }
    else if(strstr(midicommand,"midimessage"))
    {
        p->opcode = 0x01;
        p->channel = 0xFF;
        n = 1;
    }
    //non-midi commands
    else if(strstr(midicommand,"setchannel"))
    {
        p->opcode = 0x02;
        p->channel = 0xFE;
        n = 1;
        p->set_channel = 1;
    }
    else if(strstr(midicommand,"setvelocity"))
    {
        p->opcode = 0x03;
        p->channel = 0xFD;
        n = 1;
        p->set_velocity = 1;
    }
    else if(strstr(midicommand,"setshift"))
    {
        p->opcode = 0x04;
        p->channel = 0xFC;
        n = 1;
        p->set_shift = 1;
    }
    else
    {
        printf("\nERROR in config line:\n%s -midi command %s unknown!\n\n",config,midicommand);
        return -1;
    }
    return n;
}


int get_pair_mapping(char* config, PAIR* p, int n)
{
    char argnames[200],midiargs[200],
        arg0[70], arg1[70], arg2[70], arg3[70],
        pre[50], var[50], post[50];
    char *tmp, *marg[4];
    float f,arg[4] = {0,0,0,0};
    int i,j,k,use[4] = {0,0,0,0};

    arg0[0]=0;
    arg1[0]=0;
    arg2[0]=0;
    arg3[0]=0;
    if(2 < sscanf(config,"%*s %*[^,],%[^:]:%*[^(](%[^)])",argnames,midiargs))
    {
        if(!sscanf(config,"%*s %*[^,],%*[^:]:%*[^(](%[^)])",midiargs))
        {
            printf("\nERROR in config line:\n%s -could not get MIDI command arguments!\n\n",config);
            return -1;
        }
        argnames[0] = 0;//all constants in midi command
    }

    //lets get those arguments
    rm_whitespace(argnames);
    strcat(argnames,",");//add trailing comma
    i = sscanf(midiargs,"%[^,],%[^,],%[^,],%[^,]",arg0,arg1,arg2,arg3);
    if(n != i)
    {
        printf("\nERROR in config line:\n%s -incorrect number of args in midi command!\n\n",config);
        return -1;
    }

    //and the most difficult part: the mapping
    marg[0] = arg0;
    marg[1] = arg1;
    marg[2] = arg2;
    marg[3] = arg3;
    for(i=0;i<n;i++)//go through each argument
    {
        p->scale[i] = 1;
        p->offset[i] = 0;
        pre[0] = 0;
        var[0] = 0;
        post[0] = 0;

        tmp = marg[i];
        if( !(j = sscanf(tmp,"%[.1234567890*/+- ]%[^*/+- ]%[.1234567890*/+- ]",pre,var,post)) )
        {
            j = sscanf(tmp,"%[^*/+- ]%[.1234567890*/+- ]",var,post);
        }
        if(!j)
        {
            printf("\nERROR in config line:\n%s -could not understand arg %i in midi command\n\n",config,i);
            return -1;
        }
        if (strlen(var)==0)
        {
            //must be a constant
            if(!sscanf(pre,"%f",&f))
            {
                printf("\nERROR in config line:\n%s -could not get constant arg %i in midi command\n\n",config,i);
                return -1;
            }
            arg[i] = (uint8_t)f;
            use[i] = 1;
        }
        else//not a constant
        {
            //check if its the global channel keyword
            if(!strncmp(var,"channel",7))
            {
                p->use_glob_chan = 1;
                p->scale[i] = 1;
                p->offset[i] = 0;
                arg[i] = 0;
                use[i] = 0;
            }
            else if(!strncmp(var,"velocity",8))
            {
                p->use_glob_vel = 1;
                p->scale[i] = 1;
                p->offset[i] = 0;
                arg[i] = 0;
                use[i] = 0;
            }
            else
            {
                //find where it is in the OSC message
                k = strlen(var);
                tmp = argnames;
                for(j=0;(j<p->argc_in_path+p->argc) && tmp;j++)
                {
                    if(!strncmp(var,tmp,k) && *(tmp+k) == ',')
                    {
                        //match
                        p->map[j] = i;                    
                        tmp = 0;//exit loop
                        j--;
                    }
                    else
                    {
                        //next arg name
                        tmp = strchr(tmp,',');
                        tmp++;//go to char after ','
                    }
                }
                if(j==p->argc_in_path+p->argc)
                {
                                printf("\nERROR in config line:\n%s -variable %s not identified in OSC message!\n\n",config,var);
                                return -1;
                }
                //get conditioning, should be pre=b+a* and/or post=*a+b
                if(strlen(pre))
                {
                    char s1[20],s2[20];
                    float a,b;
                    switch(sscanf(pre,"%f%[-+* ]%f%[+-* ]",&b,s1,&a,s2))
                    {
                        case 4:
                            if(strchr(s2,'*'))//only multiply makes sense here
                            {
                                p->scale[i] *= a;
                            }
                            else
                            {
                                printf("\nERROR in config line:\n%s -could not get conditioning in MIDI arg %i!\n\n",config,i);
                                return -1;
                            }
                        case 3:
                            rm_whitespace(s2);
                            if(strlen(s2))
                            {
                                printf("\nERROR in config line:\n%s -could not get conditioning in MIDI arg %i!\n\n",config,i);
                                return -1;
                            }
                        case 2:
                            if(strchr(s1,'*'))
                            {
                                p->scale[i] *= b;
                            }
                            else if(strchr(s1,'+'))
                            {
                                p->offset[i] += b;
                            }
                            else if(strchr(s1,'-'))
                            {
                                p->offset[i] += b;
                                p->scale[i] *= -1;
                            }
                            break;
                        default:
                            if(strchr(pre,'-'))
                            {
                                p->scale[i] *= -1;
                            }
                            else
                            {
                               // printf("\nERROR in config line: %s, could not get conditioning in MIDI arg %i!\n",config,i);
                                //return -1; 
                                //just ignore it
                            }
                            break;
                    }//switch
                }//if pre conditions
                if(strlen(post))
                {
                    char s1[20],s2[20];
                    float a,b;
                    switch(sscanf(post,"%[-+*/ ]%f%[+- ]%f",s1,&a,s2,&b))
                    {
                        case 4:
                            if(strchr(s2,'+'))//only add/subtract makes sense here
                            {
                                p->offset[i] += b;
                            }
                            else if(strchr(s2,'-'))
                            {
                                p->offset[i] -= b;
                            }
                            else
                            {
                                printf("\nERROR in config line:\n%s -could not get conditioning in MIDI arg %i!\n\n",config,i);
                                return -1; 
                            }
                        case 3:
                            rm_whitespace(s2);
                            if(strlen(s2))
                            {
                                printf("\nERROR in config line:\n%s -could not get conditioning in MIDI arg %i!\n\n",config,i);
                                return -1;
                            }
                        case 2:
                            if(strchr(s1,'*'))
                            {
                                p->scale[i] *= a;
                            }
                            else if(strchr(s1,'/'))
                            {
                               p->scale[i] /= a; 
                            }
                            else if(strchr(s1,'+'))
                            {
                                p->offset[i] += a;
                            }
                            else if(strchr(s1,'-'))
                            {
                                p->offset[i] -= a;
                            }
                            break;
                        default:
                            //printf("\nERROR in config line: %s, could not get conditioning in MIDI arg %i!\n",config,i);
                            //return -1; 
                            //ignore it
                            break;
                    }//switch
                    
                }//if post conditioning
            }//not global channel
        }//not constant
    }//for each arg
    p->channel = arg[0];
    p->data1 = arg[1];
    p->data2 = arg[2];
    p->opcode += arg[3];
    p->use[0] = use[0];
    p->use[1] = use[1];
    p->use[2] = use[2];
    
    return 0;
}

PAIRHANDLE abort_pair_alloc(int step, PAIR* p)
{
    switch(step)
    {
        case 3:
            free(p->types);
            free(p->map);
        case 2:
            while(p->argc_in_path >=0)
            {
                free(p->path[p->argc_in_path--]);
            }
            free(p->perc);
            free(p->path);
        case 1:
            free(p);
        default:
            break;
    }
    return 0;
}

PAIRHANDLE alloc_pair(char* config)
{
    //path argtypes, arg1, arg2, ... argn : midicommand(arg1+4, arg3, 2*arg4);
    PAIR* p;
    int n;

    p = (PAIR*)malloc(sizeof(PAIR));

    //set defaults
    p->argc_in_path = 0;
    p->argc = 0;
    p->raw_midi = 0;
    p->opcode = 0;
    p->channel = 0;
    p->data1 = 0;
    p->data2 = 0;

    //config into separate parts
    if(-1 == get_pair_path(config,p))
        return abort_pair_alloc(2,p);


    if(-1 == get_pair_argtypes(config,p))
        return abort_pair_alloc(3,p);


    n = get_pair_midicommand(config,p);
    if(-1 == n)
        return abort_pair_alloc(3,p);


    if(-1 == get_pair_mapping(config,p,n))
        return abort_pair_alloc(3,p);

    return p;//success
}

void free_pair(PAIRHANDLE ph)
{
    PAIR* p = (PAIR*)ph;
    free(p->types);
    free(p->map);
    while(p->argc_in_path >=0)
    {
        free(p->path[p->argc_in_path--]);
    }
    free(p->perc);
    free(p->path);
    free(p);
}

//returns 1 if match is successful and msg has a message to be sent to the output
int try_match_osc(PAIRHANDLE ph, char* path, char* types, lo_arg** argv, int argc, uint8_t* glob_chan, uint8_t* glob_vel, int8_t *filter, uint8_t msg[])
{
    PAIR* p = (PAIR*)ph;
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
        msg[0] += *glob_chan;
    }
    if(p->use_glob_vel)
    {
        msg[2] += *glob_vel;
    }

    //now start trying to get the data
    int i,v,n;
    char *tmp;
    int place;
    for(i=0;i<p->argc_in_path;i++)
    {
        //does it match?
        p->path[i][p->perc[i]] = 0;
        tmp = strstr(path,p->path[i]);  
        n = strlen(p->path[i]);
        p->path[i][p->perc[i]] = '%';
        //if(!tmp || strncmp(tmp,p->path[i],n))
        if( !tmp )
        {
            return 0;
        }
        //get the argument
        if(!sscanf(tmp,p->path[i],&v))
        {
            return 0;
        }
        //put it in the message;
        place = p->map[i];
        if(place != -1)
        {
            msg[place] += ((uint8_t)(p->scale[place]*v + p->offset[place]))&0x7F; 
            if(p->opcode == 0xE0 && place == 1)//pitchbend is special case (14 bit number)
            {
                msg[place+1] += (uint8_t)((p->scale[place]*v + p->offset[place])/128.0); 
            }
        }
        path += n;
        //path = tmp;
    }
    //compare the end of the path
    n = strlen(p->path[i]);
    if(n)
    {
        tmp = strstr(path,p->path[i]);
        if( !tmp) //tmp !=path );
        {
            return 0;
        } 
        if(strcmp(tmp,p->path[i]))
        {
            return 0;
        }
    }
    //n = strlen(p->path[i]);
    //if(strncmp(tmp,p->path[i],n) )


    //now the actual osc args
    double val;
    for(i=0;i<p->argc;i++)
    {
        //put it in the message;
        place = p->map[i+p->argc_in_path];
        if(place != -1)
        {
            switch(types[i])
            {
                case 'i':
                    val = (double)argv[i]->i;
                    break;
                case 'h'://long
                    val = (double)argv[i]->h;
                    break;
                case 'f':
                    val = (double)argv[i]->f;
                    break;
                case 'd':
                    val = (double)argv[i]->d;
                    break;
                case 'c'://char
                    val = (double)argv[i]->c;
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
                        msg[0] = argv[i+1]->i;
                        msg[1] = argv[i+2]->i;
                        msg[2] = argv[i+3]->i;
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
                msg[place+1] = (uint8_t)(p->scale[place]*val + p->offset[place]);
                *glob_chan = msg[place+1];
                return -1;//not an error but don't need to send a midi message (ret 0 for error)
            }   
            else if(p->set_velocity)
            {
                msg[place+1] = (uint8_t)(p->scale[place]*val + p->offset[place]);
                *glob_vel = msg[place+1];
                return -1;
            }
            else if(p->set_shift)
            {
                msg[place+1] = (uint8_t)(p->scale[place]*val + p->offset[place]);
                *filter = msg[place+1];
                return -1;
            }
            if(place == 3)//only used for note on or off
            {
                msg[0] += ((uint8_t)(p->scale[place]*val + p->offset[place])>0)<<4;
            }
            else
            {
                msg[place] += ((uint8_t)(p->scale[place]*val + p->offset[place]))&0x7F; 
                if(p->opcode == 0xE0 && place == 1)//pitchbend is special case (14 bit number)
                {
                    msg[place+1] += (uint8_t)((p->scale[place]*val + p->offset[place])/128.0); 
                }
            }
        }//if arg is used
    }//for args
    return 1;
}

int load_osc_value(lo_message oscm, char type, float val)
{
    switch(type)
    {
        case 'i':
            lo_message_add_int32(oscm,(int)val);
            break;
        case 'h'://long
            lo_message_add_int64(oscm,(long)val);
            break;
        case 'f':
            lo_message_add_float(oscm,val);
            break;
        case 'd':
            lo_message_add_double(oscm,(double)val);
            break;
        case 'c'://char
            lo_message_add_char(oscm,(char)val);
            break;
        case 'T'://true
            lo_message_add_true(oscm);
            break;
        case 'F'://false
            lo_message_add_false(oscm);
            break;
        case 'N'://nil
            lo_message_add_nil(oscm);
            break;
        case 'I'://impulse
            lo_message_add_infinitum(oscm);
            break;
        //all the following just load with null values
        case 'm'://midi
        {
            uint8_t m[4] = {0,0,0,0};            
            lo_message_add_midi(oscm,m);
            break;
        }
        case 's'://string
            lo_message_add_string(oscm,"");//at some point may want to be able to interpret/send numbers as strings?
            break;
        case 'b'://blob
            lo_message_add_blob(oscm,0);
        case 'S'://symbol
            lo_message_add_symbol(oscm,"");
        case 't'://timetag
        {
            lo_timetag t;
            lo_timetag_now(&t);
            lo_message_add_timetag(oscm,t);
        }
        default:
            //this isn't supported, they shouldn't use it as an arg, return error
            return 0;

    }
    return 1;
}

int try_match_midi(PAIRHANDLE ph, uint8_t msg[], uint8_t* glob_chan, char* path, lo_message oscm)
{
    PAIR* p = (PAIR*)ph;
    uint8_t i,place,m[4] = {0,0,0,0}, noteon = 0;
    char chunk[100];

    if(!p->raw_midi)
    {
        //check the opcode
        if( (msg[0]&0xF0) != p->opcode )
        {
            if( p->opcode == 0x80 && (msg[0]&0xF0) == 0x90 )
            {//its a note on, see if pair has 3rd arg for a note() command
                for(i=0;i<p->argc+p->argc_in_path && p->map[i]!=3;i++);
                if(i == p->argc+p->argc_in_path)
                {
                    
                    return 0;
                }
                noteon = 1;
            }
            else
            {
                return 0;
            }
        }

        //check the channel
        if(p->use_glob_chan && (msg[0]&0x0F) != *glob_chan)
        {
            return 0;
        }
        else if(p->use[0] && (msg[0]&0x0F) != p->channel)
        {
            return 0;
        }
        
        if(p->use[1] && msg[1] != p->data1)
        {
            return 0;
        }

        if(p->use[2] && msg[2] != p->data2)
        {
            return 0;
        }

        //looks like a match, load values
        for(i=0;i<p->argc;i++)
        {
            place = p->map[i+p->argc_in_path];
            if(place == 3)
            {
                load_osc_value( oscm,p->types[i],(noteon - p->offset[place]) / p->scale[place] );
            }
            else if(place != -1)
            {
                load_osc_value( oscm,p->types[i],((float)msg[place] - p->offset[place]) / p->scale[place] );
            }
            else
            {
                load_osc_value( oscm,p->types[i],0.0 );//we have no idea what should be in these, so just load a 0
            }
        }
    }
    else
    {
        //anything matches a raw midi
        for(i=0;i<p->argc;i++)
        {
            if(p->types[i] == 'm' && p->map[i] != -1)
            {
                m[1] = msg[0];
                m[2] = msg[1];
                m[3] = msg[2];
                m[0] = 0;//port ID
                lo_message_add_midi(oscm,m);
            }
            else
            {
                load_osc_value( oscm,p->types[i],0.0 );//we have no idea what should be in these, so just load a 0
            }
        }
    }

    //now the path
    path[0] = 0;
    for(i=0;i<p->argc_in_path;i++)
    {
        place = p->map[i];
        if(place == 3)
        {
            sprintf(chunk,p->path[i], (int)((noteon - p->offset[place]) / p->scale[place]));
        }
        else if(place != -1)
        {
            sprintf(chunk, p->path[i], (int)((msg[place] - p->offset[place]) / p->scale[place]));
        }
        else
        {
            sprintf(chunk, p->path[i], 0);
        }
        strcat(path, chunk);
    }
    strcat(path, p->path[i]);
    
    return 1;
}

char * opcode2cmd(uint8_t opcode, uint8_t noteoff)
{
    switch(opcode)
    {
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
            return "noteon";
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
            if(noteoff) return "noteoff";
            return "note";
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA6:
        case 0xA7:
        case 0xA8:
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAE:
        case 0xAF:
            return "polyaftertouch";
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            return "controlchange";
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            return "programchange";
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            return "aftertouch";
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            return "pitchbend";
        case 0x00:
            return "rawmidi";
        case 0x01:
            return "midimessage";
        case 0x02:
            return "setchannel";
        case 0x03:
            return "setvelocity";
        case 0x04:
            return "setshift";
        default:
            return "unknown";
    }
}
