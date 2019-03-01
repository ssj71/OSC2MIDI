//spencer jackson
//jackprivate.h

//shared functions for the external and internal client versions of jack
//not used elsewhere
#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>
#include "midiseq.h"

typedef struct _jackseq
{
    jack_ringbuffer_t *ringbuffer_out;
    jack_ringbuffer_t *ringbuffer_in;
    jack_client_t	*jack_client;
    jack_port_t	*output_port;
    jack_port_t	*input_port;
    jack_port_t	*filter_in_port;
    jack_port_t	*filter_out_port;
}JACK_SEQ;


int init_ports(MIDI_SEQ* mseq, bool verbose);
int process_callback(jack_nframes_t nframes, void *seqq);
