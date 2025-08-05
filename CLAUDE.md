# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Development Commands

### Building the project
```bash
mkdir build
cd build
cmake ..
make
```

### Installing (requires sudo)
```bash
sudo make install
```

### Dependencies
- cmake
- liblo-dev (OSC library)
- libjack-dev (JACK MIDI library)

On Ubuntu: `sudo apt-get install cmake liblo-dev libjack-dev`

### Running
```bash
osc2midi                    # Run with default mapping
osc2midi -m touchosc        # Run with specific mapping file
osc2midi -v                 # Verbose mode
osc2midi -mon               # Monitor mode (print OSC messages only)
osc2midi -h                 # Show help
```

## Architecture Overview

OSC2MIDI is a bidirectional OSC ↔ JACK MIDI bridge written in C. The core architecture consists of:

### Core Components (src/)
- **main.c** - Entry point, command-line parsing, signal handling
- **oscserver.c/h** - OSC message receiving and sending using liblo
- **jackmidi.c** - JACK MIDI integration for MIDI I/O
- **converter.c/h** - Core mapping engine that processes OSC↔MIDI conversions
- **hashtable.c/h** - Hash table implementation for efficient lookups
- **ht_stuff.c/h** - Hash table utilities and variable binding management
- **pair.c/h** - Key-value pair utilities
- **midiseq.h** - MIDI sequencing definitions

### Mapping System
The application uses .omm (OSC-MIDI Mapping) files to define conversion rules:
- Maps are located in `/usr/local/share/osc2midi/` after installation
- Local maps can be stored in `~/.config/osc2midi/`
- Rules have format: `OscPath ArgTypes, Variables : MIDIFUNCTION(args)`
- Supports bidirectional conversion with variable conditioning (scaling/offsetting)

### Key Features
- **Bidirectional conversion**: OSC→MIDI and MIDI→OSC
- **Variable binding with conditioning**: `x*127+10` style transformations
- **Multi-match support**: One input can trigger multiple outputs
- **JACK integration**: Provides `osc_in`, `midi_in`, `midi_out` ports
- **Optional MIDI filter**: Transposition via `filter_in`/`filter_out` ports

### Mapping File Locations (search order)
1. `~/.config/osc2midi/` (user configs)
2. `/etc/osc2midi/` (system configs)  
3. `/usr/local/share/osc2midi/` (installation default)

### Interface Files
- `interfaces/` contains JavaScript files for Control app (Android/iOS)
- These can be loaded via URLs for convenient mobile setup

## Development Notes

The codebase follows a modular C architecture with clear separation of concerns. The converter is the heart of the system, using hash tables for efficient rule lookup and variable binding management for handling complex OSC↔MIDI transformations.