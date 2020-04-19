# Synphonium v0.1 [alpha]

A tool to help when learning to play the piano.

Non technical users should go the [website](https://symphonium.net/). A windows executable is available there. Non technical linux user should use the appImage available in the [release directory](https://github.com/ttdm/Symphonium/releases). Once downloaded, you just need to set it as executable and execute it. Some MIDI files are available in the windows download on the website. 

*Warning: The windows version should be more or less bug-free but it isn't the case for the linux one. Feel free to open issues for any bug you find.*

1. How to compile
2. First steps inside the soft
3. Future devlopments
4. Acknowledgments

-----------------------------------------------

### How to compile

It's a QT Creator project so you will QT creator for your plaform. 

Then Synphonium.pro might needs some editing. :
- line 20-28, you get to choose the MIDI input/output library you will use and to link it. 
- By default, it will select ALSA for linux (and link it); and WindowsMM for windows. The mac setup doesnt link to any library since i'm no mac user and i don't know what to link. You can also, if you want, switch to JACK when using linux. 
- Here is the list of all possibilities and the constants to define : Linux (ALSA & JACK), Macintosh OS X (CoreMIDI & JACK), and Windows (Multimedia Library) using the following constant : WINDOWS_MM, MACOSX_CORE, LINUX_ALSA, UNIX_JACK. 

It does compile using the 2 default options, hopefully, it should also compile when switching for the other 2 and proprely linking the libs. 

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
- mac support would be nice but I can't do it. 

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
