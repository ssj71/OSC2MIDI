OSC2MIDI
========

OSC2MIDI is a highly configurable OSC to jack MIDI (and back) bridge. It was designed especially for use on linux desktop 
and the open source Android app
called "Control (OSC+MIDI)" but was deliberately written to be flexible enough
to be used with any OSC controller or target. It has not been tested on windows, but has been reported to work on OSX.

To try it out download the source files, extract them, and open the resulting
directory.

This project uses cmake. The easiest and safest way to install is to do the
following commands:

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

If you are missing anything cmake will tell you. Mostly you will just need
cmake, liblo and jack. To get these on ubuntu just run

    sudo apt-get install cmake liblo-dev libjack-dev

Once installed you can simply run OSC2MIDI with the command

    osc2midi

For additional details on what options are available run

    osc2midi -h


OSC2MIDI allows you to change the mapping between OSC and MIDI messages using
an OSC to MIDI map file (.omm). This package has several mappings already
installed in the folder /usr/local/share/osc2midi/. The default mapping is
made to work with all the default interfaces that come with the Control
Android app. Other included mappings are:

* touchosc - this was made to work with the proprietary Android app of the
  same name, but I don't own it so it is untested. **NOTE:** There is a
  "to2omm" script available at <https://bitbucket.org/agraef/osc2midi-utils>
  which allows you to convert TouchOSC layout (.touchosc) files to OSC2MIDI
  map (.omm) files. The script extracts the MIDI mappings contained in the
  TouchOSC layout and creates a corresponding map file ready to be used with
  OSC2MIDI.

* androsc - this file works with the default widgets in the open source
  android app andrOSC

* generic - this was made to work with generic OSC clients that send MIDI like
  messages

* control - identical to the default map, made to work with the Control app

* gameOfLife - this mapping is a more detailed mapping for the game of life
  interface that comes default with Control. It is a bit more musical than the
  default mapping.

* sensors2osc - made to work with many of the sensors in the app of the same
  name available on the fdroid app store.

Mappings are in plain text and are completely human readable. If you wish to
make your own map file or tweak the existing ones there is extensive
documentation in [syntax.md](syntax.md) and also the default map file installed to 
/usr/local/share/osc2midi/default.omm or found in the maps/ directory of
this repository.

You can make your own mappings and store them in $XDG_CONFIG_HOME/osc2midi/
which is usually located at ~/.config/osc2midi/. Mappings in this directory
will be used first, followed by maps in /etc/osc2midi/ and finally in the
default install location of /usr/local/share/osc2midi.

If you want to try a different mapping just run with the `-m` argument, i.e.

    osc2midi -m gameOfLife

This will run with the mapping dictated by gameOfLife.omm in the first
directory it finds it in. The app will check the directories as outlined above 
if it cannot find the specified file as a relative or absolute path.

For creating your own mappings it might be useful to use monitor mode (`-mon`)
which only prints out the OSC messages that are received. While testing a new
mapping it is often useful to run with verbose mode on (`-v`).

If you develop a mapping that others might find useful please post it in our
as an issue in github or do a pull request so it can be included with the source.

Also included with this source code are some custom interfaces that can be
loaded into the Control app for Android and iOS. These can also be accessed
through the URL http://ssj71.github.io/OSC2MIDI/interfaces/[interface_name].js
for convenient loading into Control. For Example: 
http://ssj71.github.io/OSC2MIDI/interfaces/gen_synth_phone.js
