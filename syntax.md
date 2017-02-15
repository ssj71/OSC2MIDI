OSC2MIDI Map File Syntax
========================

OSC2MIDI map (.omm) files are simply collections of rules specifying the
mapping of OSC to MIDI messages. This document describes the format of the
mapping rules and explains their meaning, so that you can get up and running
quickly using osc2midi and start creating your own mappings.

To get the most out of the description, you should be familiar with the format
and meaning of both OSC and MIDI messages. Discussing OSC (Open Sound Control)
and MIDI (Musical Instrument Digital Interface) is beyond the scope of this
document, but there's a wealth of information available about these on the
web. In particular, you may want to refer to the official [OSC website][1] and
(the mirror of) Jeff Glatt's [MIDI website][2].

[1]: http://opensoundcontrol.org/
[2]: http://midi.teragonaudio.com/

Syntax
======

A map file is a collection of mapping rules, one rule per line. Empty lines
and lines containing nothing but whitespace and comments will be ignored.
Whitespace serves to delimit the different tokens in a rule, but will be
ignored otherwise, so you can indent and format rules as you like, as long as
each rule remains on a single line. Everything following the hash symbol `#`
on a line is a comment which will be ignored as well.

A mapping rule will have the form of:

OscPath ArgTypes, VarName1, VarName2, ..., VarNameN : MIDIFUNCTION(args);

To accommodate the widest possible range of OSC applications, the syntax is
deliberately lenient with the OSC path syntax and will accept any string not
containing whitespace. Therefore `path` and `argtypes` *must* be delimited by
whitespace (even if `argtypes` is empty).

In contrast, the osc2midi parser is fairly strict about the `argtypes` string
and only allows valid OSC type symbols there.

In order to provide good error-checking, the parser is also picky about the
end of the line (anything which comes after the mapping rule). Only
whitespace, any number of semicolons and a line-end comment (`# ...`) are
permitted there, anything else will be flagged as an error.

If you are familiar with BNF documentation of syntax there is a grammar table
in the appendix at the bottom of this document.

Rules
=====

Generally speaking, a mapping rule describes how OSC messages are to be
converted to corresponding MIDI messages and vice versa. Consequently, they
consist of two parts separated by a colon:

* the *left-hand side*, which is an *OSC message pattern* in a symbolic form
  which specifies type and content of the OSC message to be converted;

* the *right-hand side*, which is a *MIDI message pattern* in a symbolic form
  which specifies the type and content of the corresponding MIDI message.

Both parts may contain *variables*, placeholders for the actual values to be
converted. The variables may also be part of simple arithmetic expressions
involving the operators `+`, `-`, `*` and `/` which specify how a variable is
to be transformed by multiplying or dividing by a constant scale factor and
then adding or subtracting a constant offset, when converting between OSC
values and MIDI data bytes.

For instance, the following rule maps an OSC message `/fader1` with a single
`float` argument to a corresponding MIDI control change message (and vice
versa):

    /fader1 f, x: controlchange(0, 7, x*127)

In this example, the `x` argument in the OSC message is mapped to the
controller value `x*127`, scaling the value from the standard OSC argument
range 0-1 to the MIDI data byte range 0-127 (to produce a MIDI data byte,
osc2midi will also round the value to a 7 bit integer). The `0` in the
`controlchange` message indicates the first MIDI channel, while `7` denotes
the MIDI controller (the volume controller in this case). Conversely, when
converting from MIDI back to OSC, the same rule causes the 2nd data by of a
control change message for the volume controller on the first MIDI channel to
be converted to an OSC message for the OSC path `/fader1`, dividing the value
of the 2nd data byte by 127 to yield the `float` argument of the OSC message.

A semicolon and a line comment may optionally follow the rule, so you may also
write something like:

    /fader1 f, x: controlchange(0, 7, x*127); # map /fader1 to volume change

Anything following the `#` symbol on a line is interpreted as a comment which
is ignored by the parser.

One pitfall for beginners is that the OSC path and the OSC type string *must*
be separated by whitespace (even if the type string is empty), and that the
type string *must* be followed by a comma, even if no arguments follow. Thus,
if an OSC message has no arguments, it must be written like this:

    /start , : rawmidi(250, 0, 0)

Note the whitespace before the comma; the parser accepts any non-whitespace
character (including a comma) as part of the OSC path, so the whitespace is
really needed there. (The blanks around the colon and the commas on the
right-hand side are optional, however.)

The left-hand side of a rule can be omitted if it's the same as for the
previous rule. This lets you write collections of MIDI conversions for the
same OSC message pattern without having to retype the same left-hand side for
each rule. For instance:

    /xy1 ff, x, y : controlchange(0, 1, x*127) # map x to controller #1
    	     	  : controlchange(0, 2, y*127) # and y to controller #2

This has exactly the same meaning as if you'd have written:

    /xy1 ff, x, y : controlchange(0, 1, x*127) # map x to controller #1
    /xy1 ff, x, y : controlchange(0, 2, y*127) # and y to controller #2

The syntax and meaning of the OSC and MIDI patterns are described in greater
detail below.

OSC Patterns
============

The OSC message pattern on the left-hand side of a rule consists of the
following elements:

* The *OSC path* denotes the *OSC address* of the message, which normally
  specifies a control element of your OSC device (such as a button or a
  slider). It takes the form of an arbitrary string not containing whitespace
  and not beginning with a colon. osc2midi allows the OSC path to contain one
  or more special placeholders of the form `{i}` which may stand for an
  arbitrary integer constant embedded in the path which can then be associated
  with a variable. This makes it possible to write generic rules mapping an
  entire collection of OSC control elements to a single type of MIDI message;
  we'll discuss this under "Conversions" below.

* The *OSC type string* tells osc2midi about the number and types of the
  argument values of the message. Each argument type is denoted by a
  corresponding letter, as detailed below. As already mentioned, the OSC path
  and the type string *must* be separated by whitespace (even if the message
  doesn't have any arguments and thus the type string is empty), and the type
  string (even if it is empty) *must* be followed by a comma to separate it
  from the list of arguments (see below).

* The last element of the message pattern is a comma-delimited *argument list*
  for each of the `{i}` placeholders in the path, and each of the message
  arguments specified in the type string. If an argument is not needed for the
  conversion, the corresponding spot may be left empty (but must still be
  followed by a `,` or `:` delimiter). A non-empty argument may be a constant,
  a range or a variable, as detailed below. You can also omit arguments at
  the end of the list if they aren't needed.

In the OSC type string, osc2midi readily supports OSC arguments of the
following numeric types: `i` (integer), `h` (long integer), `f` (single
precision floating point numbers), `d` (double precision floating point
numbers), `c` (character code). It also recognizes the following special
constant designations and maps them to the corresponding values: `T` (true,
1), `F` (false, 0), `N` (nil, 0), `I` (impulse, 1). Internally, these are all
converted to double precision floating point numbers.

osc2midi also understands the `m` (MIDI message) type which allows short MIDI
messages to be passed between OSC and MIDI devices in a direct fashion, using
the special `midimessage` operation. Other customary types of message
arguments, like `S` (symbol), `s` (string), `b` (blob) and `t` (time tag) are
recognized, but are *not* supported by osc2midi at present; messages
containing such data will be silently ignored.

Arguments
=========

Argument lists are used to denote the content of an OSC or MIDI message.
Arguments are delimited by commas, and if an argument is not needed it can
also be omitted by leaving the corresponding spot empty. Valid arguments are:

* *Constants:* A numeric constant (integer or floating point value) will be
  output as is. On input, it must be matched literally in the input to enable
  a rule. Examples: `0`, `1`, `-1.5`.

* *Ranges:* A pair of numeric constants separated by `-` denotes a range of
  eligible values for the corresponding argument. On input, any of the values
  in the given range matches. On output, the minimum value of the range will
  be used. Examples: `0-1`, `0-0.5`, `0-127`.

* *Variables:* These let you refer to the variable parts of an OSC or MIDI
  message. The variable name may be any non-empty string of non-whitespace
  characters which doesn't look like a number and doesn't contain any of the
  special symbols delimiters `,`, `:` and `)`, or any of the operator symbols
  `+`, `-`, `*` and `/` used in conditioning (discussed below). Examples:
  `foo`, `bar99`, `baz'`.

Optionally, a variable `x` may be *conditioned* using a constant scale factor
`a` and a constant offset `b` as follows, to transform its range:

* `x*a+b`: scale `x` by `a`, offset the resulting value by `b`

* `b+a*x`: same as above, written in prefix form (prefix and postfix form can
  also be mixed, as in `b+x*a`)

Thus, `x*a+b` maps the range [0,1] to [b,a+b]. This transformation will be
applied automatically when outputting a message. On input, the *reverse*
transformation will be applied to the actual message argument (subtracting
`b`, then dividing the result by `a`) to give the value of the variable `x`.

Both the scale factor `a` and the offset `b` can be omitted, in which case
they default to 1 and 0, respectively. If the scale factor is given, it *must*
be nonzero, otherwise the argument is considered to be a constant (the offset
value), and a warning message will be printed. Thus, e.g., `0*x+b` denotes
just `b`.

There are also some alternative forms which are all equivalent to one of the
above:

* `-x`: negates `x` (same as `-1*x`)

* `x-b`: offsets `x` by `-b` (same as `-b+x`)

* `x/a`: scales `x` by `1/a`

The conditioning of OSC arguments is useful if the range of an integer or
floating point argument deviates from the standard [0,1] range. In such a case
it is often convenient to map the variable back to a normalized range such as
[0,1] or [-1,1]. For instance, if the OSC value ranges from -10 to 10, then
`x*10` transforms `x` to the [-1,1] range, and `x*20-10` to the [0,1] range.

Occasionally it may also be useful to scale an OSC argument in the default
range to some larger range; e.g., a `x/127` on the left-hand side scales the
`x` variable to the [0,127] range of a MIDI data byte. However, more commonly
this kind of conditioning is done on the right-hand side of a rule, where
`x*127` is used to achieve the same effect.

In any case, conditioning on the left-hand and the right-hand side of a rule
can be mixed freely as needed. When constructing the output message, first any
conditioning of a variable in the source pattern is applied in reverse, before
its conditioning in the target pattern is applied. This gives you some leeway
in formulating a rule. For instance, the following rule takes advantage of the
automatic rounding of MIDI arguments to evenly distribute note messages among
eight different MIDI channels:

    /xy ff, x/127, y/127 : noteon( x/16, x, y )

Here we applied conditionings on the left-hand side of the rule to transform
the `x` and `y` variables to the [0,127] range. The `x/16` expression on the
right-hand side then yields the MIDI channel of each note, scaling the note
number to a value between 0 and 7.

Note that it's always possible to achieve the same effect with conditioning on
just one side of the rule; after all, the net effect is always just an affine
transformation (scaling and offsetting a value). E.g., the above rule is in
fact equivalent to the following:

    /xy ff, x, y : noteon( 127*x/16, x*127, y*127 )

However, as the example shows, separating different transformations by
applying the appropriate conditionings to both sides of a rule may sometimes
help to simplify rules or at least make it easier to think about them.

MIDI Patterns
=============

The right-hand side of a mapping rule (everything which follows the colon)
takes the form of a function call, consisting of a *function name* denoting
the type (status byte) of a MIDI message, and an *argument list* enclosed in
`( )` denoting the arguments (MIDI channel and data bytes) of the message.

All the usual MIDI voice messages are readily supported by osc2midi. They take
the MIDI channel as the first argument and the requisite data bytes in the
remaining arguments. One important thing to note here is that osc2midi
consistently uses *zero-based* numbering throughout (just like in the actual
binary encoding of MIDI messages). This affects, in particular, the MIDI
channel numbers, so the first MIDI channel is denoted 0, not 1, and MIDI
channel #10 (commonly used as the percussion channel on General MIDI
instruments) has the number 9. Likewise, the MIDI program numbers used in
program change messages are denoted 0-127, not 1-128 as in most printed MIDI
bank charts, so the first program (the Grand Piano in General MIDI) is denoted
0, not 1.

The supported MIDI messages are as follows:

* `noteoff( chan, note, vel )`: A note off message (`0x8n` status, where `n`
  denotes the MIDI channel) on the given MIDI channel `chan` (0-15 range), for
  the given note number `note` (0-127) and velocity `vel` (0-127).

* `noteon( chan, note, vel )`: A note on message (`0x9n` status). This will
  also match any note off message (`0x8n` status), interpreting it as a note
  on message with zero velocity, as mandated by the MIDI standard.

* `polyaftertouch( chan, note, val )`: A polyphonic aftertouch message (`0xAn`
  status). The second argument denotes the note number (0-127), the third
  `val` argument the amount of pressure (0-127).

* `controlchange( chan, num, val )`: A control change message (`0xBn`
  status). Here the second argument `num` denotes the controller number
  (0-127) and the third argument `val` the controller value (0-127).

* `programchange( chan, num )`: A program change message (`0xCn` status). This
  takes a single data byte denoting the program number (0-127) as its second
  `num` argument.

* `aftertouch( chan, val )`: A monophonic aftertouch a.k.a. channel pressure
  message (`0xDn` status). The `val` argument denotes the amount of pressure.

* `pitchbend( chan, val )`: A pitch bend message (`0xEn` status). The `val`
  argument is a 14 bit number (0-16383), with 8192 denoting the center value.
  On output, osc2midi automatically converts this to two separate data bytes,
  the LSB and MSB values of the pitch bend message.

Note on and off messages can also be denoted using the `note` function which
takes an extra argument:

* `note( chan, note, vel, state )`: This expands to a note off or on message,
  depending on the value of `state` (zero denotes off, nonzero on).

There are also two additional functions to denote raw MIDI messages, which are
useful to transfer message types which aren't readily supported by osc2midi:

* `rawmidi( status, byte1, byte2 )`: Denotes the MIDI message with the given
  status byte (128-255) and data bytes (0-127). Note that all three arguments
  must be specified, even if the message takes just one data byte or none at
  all. Just specify the extra arguments as zero, they will be ignored.

* `midimessage( msg )`: The argument must always be an (unconditioned)
  variable here, which matches any MIDI message. The corresponding OSC
  argument must be of type `m`.

The `midimessage` function lets you pass short MIDI messages between the OSC
and the MIDI side of the bridge, by writing a rule like:

    /msg m, msg: midimessage( msg )

To make this work, your OSC device must be able to handle such MIDI arguments.

The `rawmidi` function provides you with a way to emit short system realtime
messages, which aren't readily supported by the built-in OSC MIDI functions.
For instance:

    /start , : rawmidi( 250, 0, 0 )
    /cont  , : rawmidi( 251, 0, 0 )
    /stop  , : rawmidi( 252, 0, 0 )

OSC arguments can be passed as usual:

    /select f, num: rawmidi( 243, num*127, 0 ) # song select message

Unfortunately, sysex messages can't be created this way, since they are all
longer than three bytes and thus can't be handled by osc2midi in the current
implementation. This limitation also holds for the `midimessage` function.

Using the other types of MIDI messages is rather straightforward. Arguments of
MIDI messages take the same format as in OSC patterns, thus they may be
constants, ranges or variables. In the latter case, conditioning is often used
to map the variable values from the OSC to the MIDI range. For instance:

    /fader f, x: controlchange( 0, 7, x*127 )

This maps a fader movement (assuming the standard OSC value range `0-1`) to a
corresponding control change message on the first MIDI channel, using the
volume controller (controller #7). The third argument, the controller value,
maps the OSC value to a MIDI byte in the range `0-127`, by scaling the `x`
argument appropriately.

Rounding and Clamping
---------------------

osc2midi automatically rounds MIDI arguments to integer values, using rounding
towards zero (i.e., truncating floating point values by cutting off their
fractional parts). To illustrate this, let's have another look at the `/fader`
rule from above:

    /fader f, x: controlchange( 0, 7, x*127 )

An `x` value of 0, 0.5 and 1 then gets mapped to 0, 63 and 127, respectively.
Note the 63, which is due to the fact that for `x=0.5`, `127*x` is a little
less than 64 and thus gets rounded down to 63. This might be undesirable in
some situations where the value 64 is used to denote the center value. A
remedy for this will be discussed below.

osc2midi also makes sure that MIDI arguments will not overflow the appropriate
ranges. If an argument would cause overflow, it will be *clamped* to the
maximum possible value: 15 in the case of MIDI channels, 127 in the case of a
regular data byte, 255 in the case of a status byte (1st argument of
`rawmidi`), and 16383 in the case of a pitch bend value. Likewise, if a MIDI
argument becomes negative, causing underflow, it will be clamped to a zero
value. So variables can be conditioned generously without having to worry
about illegal MIDI values. For instance, you may write:

    /fader f, x: controlchange( 0, 7, x*128 )

This maps the OSC value 1 to 128 which isn't a legal MIDI data byte, but
osc2midi turns this into 127, so everything is all right. And the rule above
makes sure that an OSC value of 0.5 produces the MIDI data byte 64 which is
customarily used in MIDI to denote the center value. This solves the "center
value problem" mentioned above. (On the other hand, the MIDI controller value
127 will now produce a value a little less than 1 for the OSC argument `x`
when converting back from MIDI to OSC. Something has to give here, since the
MIDI data byte range 0-127 isn't symmetric about the 64 center value.)

Special Non-MIDI Functions
--------------------------

Last but not least, there are three other special convenience functions which
can occur on the right-hand side of a mapping rule. In contrast to the MIDI
functions discussed above, these three are used solely for the purpose of
setting various global parameters in osc2midi and don't generate or receive
any MIDI messages:

* `setchannel( chan )`: Sets the value of the global `channel` variable (0-15)
  which can be used in the first argument of voice messages the denote the
  MIDI channel. By default, `channel` is set to zero, unless the user
  specified a different default value on the command line, using osc2midi's
  `-c` option.

* `setvelocity( vel )`: Sets the value of the global `velocity` variable
  (0-127) which can be used in the third argument of note messages the denote
  the velocity value. By default, `velocity` is set to 100, unless the user
  specified a different default value on the command line, using osc2midi's
  `-vel` option.

* `setshift( shift )`: Sets a global pitch shift value for note messages. This
  value isn't directly accessible in rules, but can be used to implement
  automatic transposition of notes using osc2midi's "MIDI filter" feature
  described below. The default shift value is zero, this can be set on the
  command line using osc2midi's `-s` option.

Here's how `setchannel` and `setvelocity` are used to set the MIDI channel and
velocity values of generated MIDI notes:

    /fader1 f, x: setchannel( 15*x )
    /fader2 f, x: setvelocity( 127*x )
    # generate notes using the channels and velocities set above:
    /fader3 f, x: noteon( channel, x*127, velocity )

Note that the special `channel` and `velocity` variables can only be used in
the corresponding positions of a MIDI message. They can't be conditioned.

The `setshift` function works differently. If it is used, osc2midi creates an
extra pair of Jack MIDI input and output ports named `filter_in` and
`filter_out`. Any MIDI note (or polyphonic aftertouch) message received at
`filter_in` gets transposed by the amount of semitones denoted by the current
`setshift` value and output to `filter_out`. All other kinds of MIDI messages
are passed through unchanged.

You can then hook up osc2midi's `midi_out` port to `filter_in` and connect the
`filter_out` port (instead of `midi_out`) to your MIDI playback device, in
order to transpose generated note messages. This provides a quick way to
implement a simple transposition feature like on some MIDI keyboards. Note
that this functionality works independently of the other OSC MIDI conversion
facilities provided by osc2midi, so it will work just as well with any other
Jack MIDI source.

Conversions
===========

The proof of the pudding is in the eating, so let's finally discuss how
osc2midi uses our mapping rules to convert incoming OSC and MIDI messages. For
the most part, the conversion process is rather straightforward: match up
incoming messages with rules and construct the converted messages by
substituting variable values. But there are some interesting issues and corner
cases which will be discussed in the following.

When converting from OSC to MIDI and vice versa, osc2midi applies mapping
rules both ways. To these ends, it keeps monitoring its OSC and MIDI input
ports for incoming messages. OSC messages are matched against the *left-hand*
sides of rules, MIDI messages against the *right-hand* sides. We also call
this the *source* pattern, the other side of the rule the *target* pattern.
That is, for OSC messages the left-hand side is the source and the right-hand
side the target pattern. For MIDI messages, it is the other way round.

A rule *matches* if there is a binding of the variables in the source pattern
so that it matches the incoming message. The variable values are then
substituted into the target pattern to yield the converted message which is
sent to its OSC or MIDI output port, depending on the type of message. If none
of the rules in the map file match then the input message is ignored.

For instance, consider:

    /fader f, x : controlchange( 0, 7, 127*x )

If we receive the OSC message `/fader 0.5` then the left-hand side of the
above rule matches with `x=0.5`, so after substituting that value the
right-hand side becomes `controlchange(0, 7, 63)`, because `127*0.5=63` after
rounding down. This is the converted message which is sent to osc2midi's MIDI
output.

Conversely, if we receive a control change on the first MIDI channel for
controller 7 with value 64, then this matches the right-hand side of the rule
with `x=0.503937` (`=64/127`, rounded to 6 decimals here). Substituting this
into the left-hand side gives the outgoing OSC message `/fader 0.503937`.

All this happens in real time, i.e., osc2midi responds to incoming messages
immediately (ideally without much latency, but this depends on the size of
incoming messages, the number of mapping rules, cpu power and various other
system characteristics).

Multiple Matches
----------------

In general, there may well be several rules that match an incoming message.
osc2midi will go through all the rules of the loaded map file and output one
converted message for each matching rule, in the same order in which the rules
occur in the map file. Thus each input message can produce as many output
messages as needed.

This is osc2midi's default or "multi" mode of operation, which should be
appropriate for most use cases. You can also invoke osc2midi with the
`-single` option to have it emit a message only for the *first* rule that
matches, but this is rarely needed. In the following we generally assume that
osc2midi is running in "multi" mode.

For instance, consider:

    /xy ff, x, y : controlchange( 0, 12, x*127 )
    /xy ff, x, y : controlchange( 0, 13, y*127 )

This is a rather typical example of an OSC message containing multiple values
which should be mapped to separate MIDI messages. As already mentioned, you
can also abbreviate this as:

    /xy ff, x, y : controlchange( 0, 12, x*127 )
                 : controlchange( 0, 13, y*127 )

Now, if we receive the OSC message `/xy 0.5 0.2`, then both rules match and
osc2midi sends out two control change messages, `controlchange(0, 12, 63)`
from the first and `controlchange(0, 13, 25)` from the second rule, in that
order.

Unbound Variables
-----------------

Unfortunately, rules like the above also raise an issue with the reverse, MIDI
to OSC conversions. Clearly, the intention in the previous example is to map a
pair of values `x` and `y` in the OSC message (which might the coordinates of
some kind of x-y controller) to a corresponding pair of MIDI control changes.
The reverse conversion should keep this intact. That is, we'd like to be able
to convert a pair of control changes for the MIDI controllers 12 and 13 back
to a `/xy x y` OSC message.

However, when the first rule is applied in MIDI to OSC conversion, upon
receiving a message for controller 12 the current value of `y` isn't known
since it's not in the MIDI message, and the same holds for `x` in the second
rule. If we simply use zero default values for the unbound variables, then one
of the OSC message arguments would always be zero, which is not what we want
here.

To remedy this, osc2midi automatically keeps track of groups of *related*
rules (i.e., rules with the same OSC path and argument types) and records
previous values for all the arguments in these rules. The values get recorded
whenever an OSC message triggers a rule in the group, and also if a MIDI
message is received which carries a value for one of the OSC message
arguments.

Thus, if the message `controlchange(0, 12, x*127)` is received then the value
of `x` is remembered and gets output again when subsequently the message
`controlchange(0, 13, y*127)` is received, along with the `y` value in the
second message (which then gets remembered for subsequent messages as well).
So, if the controller messages arrive in pairs then we get the behavior that's
expected in most cases:

    controlchange( 0, 12, x1*127 )  -->  /xy x1 0.0  # y unknown, use default
    controlchange( 0, 13, y1*127 )  -->  /xy x1 y1   # use recorded x value
    controlchange( 0, 12, x2*127 )  -->  /xy x2 y1   # use recorded y value
    controlchange( 0, 13, y2*127 )  -->  /xy x2 y2   # use recorded x value

The same applies if you want to send a fixed control change message if an OSC
control changes, no matter what the current value is:

    /fader f, x : controlchange( 0, 80, 127 )

Here a control change for controller 80 with value 127 is sent, ignoring the
`x` value in the message. In such a case you can also omit the variable:

    /fader f, : controlchange( 0, 80, 127 )

If you leave the spot of an argument in the source pattern empty, then any
value matches there (and will be ignored). However, the value will still be
recorded for subsequent MIDI to OSC conversions:

    /fader 0.5  -->  controlchange( 0, 80, 127 )  # x value 0.5 is recorded
    controlchange( 0, 80, 127 )  -->  /fader 0.5  # use recorded x value

Multiply-Bound Variables
------------------------

Another special case arises if the source pattern contains multiple instances
of the same variable. For instance:

    /fader f, x/127 : noteon( x/64, x, 127 )

The above rule can be used to implement a simple kind of "split keyboard"
which emits MIDI notes in the lower range `0-63` on the first, and notes in
the upper range `64-127` on the second MIDI channel.

But what happens in MIDI to OSC conversions? The problem here is that the MIDI
pattern will usually give two different possible bindings for the `x`
variable. E.g., consider the MIDI message `noteon(0, 48, 127)`. Then `x`
equals `0` (`0*64`) in the first argument, and `48` in the second one. So
which of the two values are we going to use?

Right now osc2midi uses a very simple strategy to resolve such ambiguities:
simply pick one of the values and run with it. As it's implemented, on the
right-hand side of a rule this happens to be the *rightmost* occurrence of a
variable, which makes sense because the channel argument, being a 4 bit value,
usually has a much lower resolution than the second and third arguments. So
in our example osc2midi would go with the `x=48` binding.

Incidentally, this is also the mathematically correct solution, since `x/64`
then gives `0` after rounding down, so the right-hand side of the rule
actually matches with this binding. But note that osc2midi will not actually
verify this in its normal mode of operation. Thus, e.g., osc2midi will also
happily convert `noteon(1, 48, 127)` to `/fader 0.377953` even though this
message does *not* match the right-hand side of this rule.

As a remedy, osc2midi also offers a "strict" mode (which is enabled by
invoking it with the `-strict` option) in which it actually verifies the
consistency of variable bindings if the same variable occurs more than once in
the source pattern. It will then accept `noteon(0, 48, 127)` with `x=48`, but
reject `noteon(1, 48, 127)`, because the binding `x=48` is inconsistent with
the channel argument `1` in the first argument.

Multiple occurrences of variables in OSC patterns are handled in an analogous
fashion. For instance, consider:

    /xy ff, x, x : controlchange( 0, 12, x*127 )

In strict mode, this rule will only be matched if both arguments of the OSC
message are exactly the same. No rounding is performed in the OSC case. Also
note that on the left-hand side osc2midi always picks the *leftmost*
occurrence of a variable. Thus in non-strict mode `/xy 0 y` will always be
converted to `controlchange(0, 12, 0)`, no matter what the value of `y` is.

Constants and Ranges
--------------------

As mentioned previously, values in the input message can also be matched
against constants and ranges in the source pattern. Constants are typically
used in situations where a control only emits certain discrete values, such as
a button control which only has the values 0 and 1:

    /button f, 0 : controlchange( 0, 80, 0 )
    /button f, 1 : controlchange( 0, 80, 127 )

This maps the OSC values 0.0 and 1.0 of the `/button` control to a
corresponding MIDI control change (controller 80 in this example) with values
0 and 127, and vice versa. Note that in this case the given values have to be
matched *exactly* by the input message, any other input values will cause the
input message to be ignored.

The input value can also be matched against a *range* of values. For instance:

    /fader f, 0-0.5 : controlchange( 0, 80, 0 )
    /fader f, 0.5-1 : controlchange( 0, 80, 127 )

This realizes a kind of gate function where the controller value becomes 0 or
127, depending on whether the actual input value is below or above 0.5,
respectively. (Note that the message `/fader 0.5` will trigger *both* rules
here, since 0.5 belongs to both ranges.)

Ranges can be used on both sides of a rule, so you may write:

    /fader f, 0-0.5 : controlchange( 0, 80, 0-64 )
    /fader f, 0.5-1 : controlchange( 0, 80, 64-127 )

If such a rule is triggered, the lower bound of the range in the target
pattern is used as the target value. E.g.:

    /fader 1.0  -->  controlchange( 0, 80, 64 )
    controlchange( 0, 80, 0 )  -->  /fader 0.0

{i} Placeholders
----------------

Using the `{i}` placeholders makes it possible to extract integer values from
the OSC path of a message and bind them to a variable. This is often used with
groups of related controls. For instance, assume some faders `/fader/1`,
`/fader/2` etc. which should be mapped to corresponding MIDI controllers:

    /fader/1 f, x : controlchange( 0, 1, x*127 )
    /fader/2 f, x : controlchange( 0, 2, x*127 )

With a `{i}` placeholder it becomes possible to condense these rules into a
single generic rule:

    /fader/{i} f, k, x : controlchange( 0, k, x*127 )

Note the index variable `k` which is bound to the actual control number
represented by the `{i}` placeholder. The left-hand side can now be matched
against any OSC path of the form `/fader/k` with integer `k`:

    /fader/1 1.0  -->  controlchange( 0, 1, 127 )
    /fader/2 1.0  -->  controlchange( 0, 2, 127 )
    /fader/9 1.0  -->  controlchange( 0, 9, 127 )

Conversely, just as one might expect, values are also substituted into the
`{i}` placeholders in the OSC path in MIDI to OSC conversions:

    controlchange( 0, 1, 127 )  -->  /fader/1 1.0
    controlchange( 0, 2, 127 )  -->  /fader/2 1.0
    controlchange( 0, 9, 127 )  -->  /fader/9 1.0

We mention in passing that it's also possible to have multiple `{i}`
placeholders in a single rule, each bound to their own variable, but this is
used much less frequently.

Examples
========

Here are some examples illustrating the rule syntax and the concepts
discussed above. More examples can be found in default.omm and the other
sample maps included in the osc2midi distribution.

TBD: faders, knobs, buttons, encoders, xy controls, multi controls: multi
faders, radio buttons, button arrays, keyboards, split keyboard


Appendix
========

The syntax of rules is described by the following grammar in extended BNF.
We'll discuss the meaning of the various elements of a rule below.

    rule ::= [ osc-pattern ] ':' midi-pattern

    osc-pattern ::= path argtypes ',' [ arglist ]

    midi-pattern ::= command '(' arglist ')'

    path ::= OSC path string, must be non-empty

    argtypes ::= OSC type string, may be empty

    arglist ::= [ arg ] { ',' [ arg ] }

    arg ::= [ pre ] var [ post ] | number | number '-' number

    pre ::= '-' | [ number add-op ] [ number '*' ]

    post ::= [ mul-op number ] [ add-op number ]

    add-op ::= '+' | '-'

    mul-op ::= '*' | '/'

    var ::= may contain anything but operators, comma and ':' or ')'
            delimiter, must not be empty

