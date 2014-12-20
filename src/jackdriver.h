#ifndef JACKDRIVER_H
#define JACKDRIVER_H
#include<jack/jack.h>
#include<jack/ringbuffer.h>

typedef struct _jseq{
    jack_ringbuffer_t *ringbuffer;
    jack_client_t	*jack_client;
    jack_port_t	*output_port;
    uint8_t usein;
}JACK_SEQ;

int init_jack(JACK_SEQ* seq, uint8_t verbose);
void close_jack(JACK_SEQ* seq);
void queue_midi(void* seqq, uint8_t msg[]);

#endif
