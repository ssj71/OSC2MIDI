#ifndef MIDI_SEQ_H
#define MIDI_SEQ_H
#include<stdint.h>
#include<stdbool.h>

//general midi sequencer data
typedef struct _mseq
{
    void* driver;
    bool usein;
    bool useout;
    bool usefilter;
    int8_t* filter;
    int8_t old_filter;
    //keep track of on notes to jump octaves mid note
    uint8_t notechan[127];
    uint8_t note[127];
    uint8_t notevel[127];
    uint8_t nnotes;
} MIDI_SEQ;

int init_midi_seq(MIDI_SEQ* seq, uint8_t verbose, const char* clientname);
void close_midi_seq(MIDI_SEQ* seq);
void queue_midi(MIDI_SEQ* seqq, uint8_t msg[]);
int pop_midi(MIDI_SEQ* seqq, uint8_t msg[]);

#endif
