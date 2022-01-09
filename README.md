# Synphonium v1.0

A tool to help when learning to play the piano.

To download the latest versions on all platforms, see [this post](https://github.com/ttdm/Symphonium/issues/11).
You can also find around 40 MIDI files to download on the website. 

The following ReadMe is focused on technical users that would want to build the project from its sources.
Non technical users will probably get more informations from reading [website](https://symphonium.net/).


1. How to compile
2. First steps inside the soft
3. Future devlopments
4. Acknowledgments

-----------------------------------------------

### How to compile

It's pretty simple : It's a QT Creator project so you will QT Creator for your plaform.
And that's all. All the others libraries are shipped (as sources or .Lib) with the project. 

So you only need to open the project file (.pro) with QT Creator and then you can build and run with Ctrl+R


Compilation options, directly by editing the Symphonium.pro file : 
- line 20-28, you get to choose the MIDI input/output library you will use and to link it. 
- By default, it will select ALSA for linux (and link it); and WindowsMM for windows. You can also, if you want, switch to JACK when using linux. 
- Here is the list of all possibilities and the constants to define : Linux (ALSA & JACK), Macintosh OS X (CoreMIDI & JACK), and Windows (Multimedia Library) using the following constant : WINDOWS_MM, MACOSX_CORE, LINUX_ALSA, UNIX_JACK. 

It does compile using the 3 default options, I never tested it with JACK.  

-----------------------------------------------

### First steps inside the soft

1. Add MIDI file to the library by selecting file in the menu (file should be in ".mid")

2. Select a MIDI device using "Options" in the menu. If you don't have any MIDI device, the soft capabilities will be limited; you will only be able to "watch the notes fall on the keyboard" in
 the "listen to the song" mode.

3. I believe (hopefully) the rest of the soft is self explanatory.

-----------------------------------------------

### Future devlopments

Important / Fixes :
- adding a metronom which isn't that straightforward since there are plenty of rythm
change in complex classical MIDI pieces.

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

Now that an alpha version is available, I'm totally open to contributions !
Thanks to the ones that already took some time to improve Symphonium : 
- zemser
- probonopd
