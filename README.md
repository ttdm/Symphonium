# Synphonium v0.1 [alpha]

A tool to help when learning to play the piano.
Non technical user should go the [website](https://symphonium.net/). A windows executable is available.
I'm currently working on creating a package for Arch-linux.

1. Installation
2. First steps inside the soft
3. Future devlopments
4. Acknowledgments

-----------------------------------------------

### Installation

First you need QT Creator.
Then Synphonium.pro needs some editing :
- line 11, you get to choose the MIDI input/output library you will use across
 Linux (ALSA & JACK), Macintosh OS X (CoreMIDI & JACK), and Windows (Multimedia Library)
 using the following constant : WINDOWS_MM, MACOSX_CORE, LINUX_ALSA, UNIX_JACK
- then you need to link your chosen MIDI input/output library

Once those step are completed, hopefully, it should compile.
Right now, this has only been tested under windows.

-----------------------------------------------

### First steps inside the soft

1. Add MIDI file to the library by selecting file in the menu (file should be in ".mid")
 /!\ there is a known weird bug which make it impossible to select file in the menu when there
  is no file in the library ... So you will need to click on view and slide your mouse to file. /!\

2. Select a MIDI device using "Options" in the menu. If you don't have any MIDI device, it won't
 work and you will only be able to "watch the notes fall on the keyboard" in
 the "listen to the song" mode.

3. I believe (hopefully) the rest of the soft is self explanatory.

-----------------------------------------------

### Future devlopments

Important / Fixes :
- create package for easy use on linux.
- adding a metronom which isn't that straightforward since there are plenty of rythm
change in complex classical MIDI pieces.
- automatize the compilation for every platform and MIDI interface.

Minor / For later :
- resetSettings and resetLibrary options
- improve the library : MIDI file durations, note number, difficulty estimation (note/sec?)
- add in game points ?
- make it usable with any piano by analyzing the input sounds and not the MIDI inputs.

	-----------------------------------------------

### Acknowledgments

This soft has been inspired by Synthesia and PianoFromAbove but one is closed source and
 commercial and the other is focused on windows only and is starting to get quite old.
After taking advices from better devs than myself, it would appear that it was faster
 to create this soft from scratch than to build on PFA which explain why I created nearly a copy of PFA.

The following libraries are used in the soft : 
- Qt
- RTMIDI
- MIDIFile

I thank all the people that gave me some advices while I created Symphonium.
Now that an alpha version is available, I'm totally open to contributions !
