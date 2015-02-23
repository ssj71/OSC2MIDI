#ifndef JACKDRIVER_H
#define JACKDRIVER_H
#include<jack/jack.h>
#include<jack/ringbuffer.h>

typedef struct _jseq{
    jack_ringbuffer_t *ringbuffer_out;
    jack_ringbuffer_t *ringbuffer_in;
    jack_client_t	*jack_client;
    jack_port_t	*output_port;
    jack_port_t	*input_port;
    jack_port_t	*filter_in_port;
    jack_port_t	*filter_out_port;
    uint8_t usein;
    uint8_t useout;
    uint8_t usefilter;
    int8_t* filter;
    int8_t old_filter;
    //keep track off on notes to jump octaves mid note
    uint8_t notechan[127];
    uint8_t note[127];
    uint8_t notevel[127];
    uint8_t nnotes;
}JACK_SEQ;

int init_jack(JACK_SEQ* seq, uint8_t verbose);
void close_jack(JACK_SEQ* seq);
void queue_midi(void* seqq, uint8_t msg[]);
int pop_midi(void* seqq, uint8_t msg[]);

#endif
