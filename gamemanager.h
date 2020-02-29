// Main, non related to UI and display, class.
// It handles reading MIDI real time and MIDI file; and define what the output display will be
// depending on the MIDI inputs and the user defined options.

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "MIDIFile/include/MidiFile.h"
#include "RTMIDI/RtMidi.h"

#include "gameoptions.h"

#include <QElapsedTimer>
#include <set>

class GameManager
{
public:
    GameManager();
    ~GameManager();

    void update();
    void connectRTMIDIatStart();
    bool createRTMIDIobjects();
    bool connectRTMIDIobjects2ports(uint inId, uint outId);

    smf::MidiFile midifile;// the midifile that's currently being displayed (and proably played!)
    //RT MIDI
    RtMidiIn  *midiin;
    RtMidiOut *midiout;
    bool isMIDIDeviceReady;
    int midiInPort, midiOutPort;

    GameOptions options; //all game options that can be set up from config file.

    //Game live and moving variables
    double songTime, prevsongTime, songDuration;

    //Game Live Parameters
    bool isMidiSelectedFileValid;
    bool isPlaying;
    int gameMode; //0 : list to song, 1 : pratice song ; 2 : play song
    int selectedTrack; // which track will be taken into account when checking inputs vs midifile
    int handsInplay; //0 : left hand, 1 : right hand, 2 both
    double RTFactor; //factor that is used to

    //interval handling :
    bool isIntervalOn; //true if we play only a specific interval
    double intInitTime, intFinalTime; //interval initial and final time

    void selectMIDIFile(std::string path);
    QElapsedTimer qtime;

    //Asses Playing
    double delayTime;
    int successNoteCount, errorNoteCount, missedNoteCount;
    std::set<int> currPressedNotes, midifileCurrNotes;
};

#endif // GAMEMANAGER_H
