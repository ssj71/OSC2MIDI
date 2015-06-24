//Spencer Jackson
//ht_stuff.c

// stuff for the hashtable to determine if an OSC message generates multiple
// MIDI messages. This way several pairs can share registers.
// the hashtable is used for a new pair to see a previous pair used the same OSC
// message. This way values are persistent for MIDI->OSC where all the OSC
// args cannot be contained in a single MIDI message

/*************************************************************************/

/* Register implementation for keeping track of OSC values of partial mappings
   Albert Graef <aggraef@gmail.com>, Computer Music Research Group, Johannes
   Gutenberg University Mainz, June 2015

   This stems from a discussion with Spencer on how to make partial
   assignments of OSC values work in reverse mappings (MIDI->OSC). This is
   needed for rules like the following which map a single, multi-argument OSC
   message to several different MIDI messages:

     /2/xy1 ff, val,  : controlchange( 1, 1, 127*val );
     /2/xy1 ff, , val : controlchange( 1, 2, 127*val );

   When receiving one of the MIDI messages on the right-hand side of such
   rules, a complete OSC message has to be constructed which supplies values
   to *all* arguments of the OSC message. To these ends, the present
   implementation stores previously received values for all arguments
   (obtained from previous OSC or MIDI messages) in a vector of "registers"
   which is accessible to all rules for the same type of OSC message.

   The (indexes of the) register vectors for each rule are kept in a hash
   table indexed by path-argtype pairs. (Note that indexing by just the OSC
   paths won't work, since the same path might be used with different argument
   combinations.) This means that each rule for a given path-argtype pair will
   get the same collection of registers. The hash table is constructed at
   startup time, while reading the map file. Accessing the register values in
   the real-time loop then just needs a constant-time lookup using a pointer
   stored in each configuration pair, and thus incurs only minimal overhead.

   After initialization, the necessary bookkeeping (storing received values in
   registers, and retrieving stored values in the MIDI->OSC mappings) is done
   in the try_match_osc() and try_match_midi() routines . */

#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include"ht_stuff.h"

/*************************************************************************/
// Not sure what's a good magic number for the hash table size here, just
// make it large enough so that the buckets don't grow too large.
#define TABLESZ 4711


// element type
struct elem
{
    char* key;
    int k;
};
typedef struct elem* elem;//for hashtable


// key comparison
bool ht_equal(ht_key s1, ht_key s2)
{
    return strcmp((char*)s1,(char*)s2) == 0;
}

// extracting keys from elements
ht_key ht_getkey(ht_elem e)
{
    return ((elem)e)->key;
}


// hash function using an inlined random number generator -- this comes from
// the sample code provided with Frank Pfenning's hash table implementation
int ht_hash(ht_key s, int m)
{
    unsigned int a = 1664525;
    unsigned int b = 1013904223;
    unsigned int r = 0xdeadbeef;	       /* initial seed */
    int len = strlen(s);
    int i;
    unsigned int h = 0;	       /* empty string maps to 0 */
    for (i = 0; i < len; i++)
    {
        h = r*h + ((char*)s)[i];	 /* mod 2^32 */
        r = r*a + b;	 /* mod 2^32, linear congruential random no */
    }
    h = h % m;			/* reduce to range */
    //@assert -m < (int)h && (int)h < m;
    int hx = (int)h;
    if (hx < 0) h += m;	/* make positive, if necessary */
    return hx;
}

void free_elem(ht_elem e)
{
    elem el = (elem)e;
    free(el->key);
    free(el);
}

//initialize hash table
table init_table()
{
    return table_new(TABLESZ, ht_getkey, ht_equal, ht_hash);
}

//uninitialize hash table
void free_table(table tab)
{
    table_free(tab,free_elem);
}

// Return a key (index into the regs table) which is unique for the given
// path, argtypes pair.
int strkey(table tab, char* path, char* argtypes, int* nkeys)
{
    // According to the OSC spec, path may not contain a comma, so we can use
    // that as a delimiter for the path,argtypes key value here.
    char *key = malloc(strlen(path)+strlen(argtypes)+2);
    sprintf(key, "%s,%s", path, argtypes);
    // Make sure that the hash table is initialized.
    ht_elem e = table_search(tab, key);
    if (!e)
    {
        // new key, add a new entry to the regs table
        elem el = (elem)malloc(sizeof(struct elem));
        el->key = key;
        el->k = (*nkeys)++;
        table_insert(tab, el);
        e = el;
    }
    else
    {
        free(key);
    }
    return ((elem)e)->k;
}
