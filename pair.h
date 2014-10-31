//spencer jackson

//pair.h

typedef struct _pair
{

    //osc data
    char*   path;
    /*   handled by lo_arg
    int32_t i[];//int
    float   f[];//float
    char*   s[];//strings
    int32_t t[];//timestamp
    int32_t b[];//blob
    uint8_t* m[3];//midi
    */
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

int alloc_pair(pair* p, char* config);
int set_channel(pair* p, uint8_t channel);
int free_pair(pair* p);
int try_match_osc(pair* p, char* oscpath, lo_arg** argv, int argc);
int try_match_midi(pair*, uint8_t msg[]);
uint8_t* get_midi(pair* p);
lo_message get_osc(pair* p);

