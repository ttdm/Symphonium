#include "gamemanager.h"

#include "QMessageBox"
#include <QDebug>

GameManager::GameManager()
{
    isMIDIPlaying = false;
    gameMode = 0;
    handsInplay = 2;
    RTFactor = 1.0;
    isIntervalOn = false;
    isMidiSelectedFileValid = false;
    qtime.start();
    selectedTrack = 0;
    isMIDIDeviceReady = false;
    midiInPort = -1;
    midiOutPort = -1;
    soundLevel = 1.;
    restreamMIDIIn = false;
}

GameManager::~GameManager()
{
    //Clean up RTMIDI
    delete midiin;
    delete midiout;
}

void GameManager::connectRTMIDIatStart()
{
    unsigned int nInPorts = midiin->getPortCount();
    int inId = -1;
    for (  int i=0; i<nInPorts; i++ ) {
          try {
              std::string portName = midiin->getPortName(i);
              if (!QString::compare(options.MIDIinPortName, QString::fromStdString(portName), Qt::CaseInsensitive ))
              {
                  inId = i;
                  break;
              }

        }
              catch ( RtMidiError &error ) {
              }
    }
    unsigned int nOutPorts = midiout->getPortCount();
    int outId = -1;
    for ( unsigned int i=0; i<nOutPorts; i++ )
    {
        try {
            std::string portName = midiout->getPortName(i);
            if (!QString::compare(options.MIDIoutPortName, QString::fromStdString(portName), Qt::CaseInsensitive ))
            {
                outId = i;
                break;
            }
        }
        catch ( RtMidiError &error ) {
        }
    }

    //exit if no correspoding port has been found :
    if (inId == -1 | outId == -1) return;

    //connect the RTmidi objects to the channel stored in the options; exit if there is a problem
    if (!connectRTMIDIobjects2ports(inId,outId)) return;

    isMIDIDeviceReady = true;
}

bool GameManager::connectRTMIDIobjects2ports(uint inId, uint outId)
{
    bool openFail = false;
    try {
        midiin->openPort(inId);
    }
    catch ( RtMidiError &error ) {
      error.printMessage();
      openFail = true;
    }
    try {
        midiout->openPort(outId);
    }
    catch ( RtMidiError &error ) {
      error.printMessage();
      openFail = true;
    }
    if (openFail) return false; //don't update the config and display error msg

    // update config if needed
    if ( QString::compare(options.MIDIinPortName, QString::fromStdString(midiin->getPortName(inId)), Qt::CaseInsensitive) )
    {
        options.MIDIinPortName = QString::fromStdString(midiin->getPortName(inId));
        options.isConfigModified = true;
    }
    if ( QString::compare(options.MIDIoutPortName, QString::fromStdString(midiout->getPortName(outId)), Qt::CaseInsensitive) )
    {
        options.MIDIoutPortName = QString::fromStdString(midiout->getPortName(outId));
        options.isConfigModified = true;
    }

    return true;
}

bool GameManager::createRTMIDIobjects()
{
    // RtMidiIn constructor
    try {
      midiin = new RtMidiIn();
    }
    catch ( RtMidiError &error ) {
      error.printMessage();
      return false;
    }
    // RtMidiOut constructor
    try {
      midiout = new RtMidiOut();
    }
    catch ( RtMidiError &error ) {
      error.printMessage();
      return false;
    }

    return true;
}

void GameManager::selectMIDIFile(std::string path)
{
    //read selected MIDI file and deal with error
    midifile.read(path);
    if ( !midifile.status() )
    {
        QMessageBox Msgbox;
          Msgbox.setText("Error while reading the selected MIDI file.");
            Msgbox.exec();
            return;
    }

    //change the selected file to VALID since it is read by the midifile reader :
    isMidiSelectedFileValid = true;

    //sorting all the MIDI tracks (no real interest!, maybe needed for doTimeAnalysis and linkNotePair?)
    for (int i = 0; i < midifile.getTrackCount(); i ++)
    {
        midifile.sortTrack(i);
    }
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();

    double totalTime = 0.0;
    for (int i = 0; i < midifile.getTrackCount() ; i++ )
    {
        for (int j = 0; j <midifile[i].getEventCount() ; j++ )
        {
            if (!midifile[i][j].isNoteOn() && !midifile[i][j].isNoteOff() )continue;
            if (midifile.getTimeInSeconds(i,j) > totalTime)  totalTime = midifile.getTimeInSeconds(i,j);

        }
        int lastIndex = midifile[i].getEventCount() -1;
    }
    songDuration = totalTime; //note : this is way longer than just checking for the last event of each track;
    // but quite often this last event is irrelevant and the midi file is way longer than the last note
    songTime = - options.timeB4Restart/2.0;
    selectedTrack = 0;

    //reset game variables
    delayTime = 0;
    successNoteCount=0;
    errorNoteCount=0;
    missedNoteCount=0;
    currPressedNotes.clear();
}

void GameManager::update()
{
    // qDebug() << "in manager update";

    //whatever the state, if possible, read MIDI IN, it does allow to listen to the what's played if MIDI IN is forfarwded.
    if (isMIDIDeviceReady)
    {
        std::vector<unsigned char> oneInput;
        midiin->getMessage(&oneInput);
        while (oneInput.size() != 0)  //which means that we have a MIDI message to decrypt
        {
            if (restreamMIDIIn) midiout->sendMessage(&oneInput); //XXX to watch, maybe better to only stream 144 and 128 event types (note in note off).
            if ( (int)oneInput.at(0) == 144 )
            {
                if ( (int)oneInput.at(2)>0)
                {
                    currPressedNotes.insert((int)oneInput.at(1));
                }
                else //weirdly, on my keyboard; when releasing a key, instead of a msg : 128 X X; i get a 144 key 0;
                    //implementing both behavior, in case this varies between keyboards and/or OSs
                {
                    std::set<int>::iterator it;
                    it = currPressedNotes.find((int)oneInput.at(1));
                    currPressedNotes.erase(it);
                }
            }
            if ( (int)oneInput.at(0) == 128 )
            {
                std::set<int>::iterator it;
                it = currPressedNotes.find((int)oneInput.at(1));
                currPressedNotes.erase(it);
            }
            midiin->getMessage(&oneInput);
        }
    }

    if (!isMIDIPlaying)//Don't to anything else if no MIDIfile is currently played
    {
        qtime.restart(); //but update the clock so that the first time step when playing will be correct.
        return;
    }
    int elapsed = qtime.restart();

    if (gameMode==0) // listen
    {
       prevsongTime = songTime;
       songTime += elapsed/1000.*RTFactor;
       for (int i = 0; i <midifile.getTrackCount(); i ++)
       {
           if ( selectedTrack != 0 && selectedTrack - 1 != i) continue;
            for (int j = 0; j < midifile[i].getEventCount(); j++ )
            {
                if (midifile[i][j].seconds > prevsongTime && midifile[i][j].seconds <= songTime )
                {
                    if (midifile[i][j].isNoteOn())
                    {
                        unsigned char currNote = midifile[i][j].getP1();
                        unsigned char velNote = std::min(int(midifile[i][j].getP2()*soundLevel),127);
                        std::vector<unsigned char> message(3);
                        // Note On: 144, note, vel
                        message[0] = 144;
                        message[1] = currNote;
                        message[2] = velNote;
                        try {
                            midiout->sendMessage( &message );
                        } catch (...) {
                            qDebug() << "error while sending 1 MIDI msg";
                        }
                    }
                    if (midifile[i][j].isNoteOff())
                    {
                        unsigned char currNote = midifile[i][j].getP1();
                        unsigned char smthNote = midifile[i][j].getP2();
                        std::vector<unsigned char> message(3);
                        // Note Off: 128, note, ??vel??
                        message[0] = 128;
                        message[1] = 64;
                        message[2] = 40;
                        try {
                            midiout->sendMessage( &message );
                        } catch (...) {
                            qDebug() << "error while sending 1 MIDI msg";
                        }
                    }
                }
            }
       }
   }

   //play MIDI OUT and compare MIDI IN to midifile.
   if (gameMode == 1) // Learn
   {
        //compute the expected time = future time if no mistakes
        double expectedSongTime = songTime + elapsed/1000.*RTFactor;
        bool increaseTime = true;

        //List all keys that should have been pressed during this time and update the midifile set
        for (int i = 0; i <midifile.getTrackCount(); i ++) //for all the midifile
        {
             for (int j = 0; j < midifile[i].getEventCount(); j++ )
             {
                 if ( selectedTrack == 0 | selectedTrack - 1 == i) //for the active track(s)
                 {
                     if (midifile[i][j].seconds > songTime && midifile[i][j].seconds <= expectedSongTime ) //at good time
                     {
                         if (midifile[i][j].isNoteOn())
                         {
                             unsigned char currNote = midifile[i][j].getP1();
                             if (currPressedNotes.find(currNote) == currPressedNotes.end()) increaseTime = false;
                         }
                     }
                 }
             }
        }

        //add a delay 50ms delay before deleting notes on the midifile set =>
        //check for false note by verifying that the union of both set is the same size of the midifile set.
        //check that all new notes are played by verifying that each of them are in the currPressedNotes set
        //we are at 250Hz, we arent at less than a ms; so increase time only if both tests are ok. If not,

        if (increaseTime)
        {
            prevsongTime = songTime;
            songTime += elapsed/1000.*RTFactor;
        }
    }
    else if (gameMode == 2) // Play
    {
      //update time, time allways increasing.
       prevsongTime = songTime;
       songTime += elapsed/1000.*RTFactor;

       for (int i = 0; i <midifile.getTrackCount(); i ++) //for all the midifile
       {
            for (int j = 0; j < midifile[i].getEventCount(); j++ )
            {
                if ( selectedTrack == 0 | selectedTrack - 1 == i) //for the active track(s)
                {
                    //simply check that newly pressed notes should have been pressed at songTime +- DeltaT;
                }
            }
       }
   }

   //play notes of other tracks in GameMode 1 and 2
   if ( gameMode == 1 | gameMode == 2)
   {
        for (int i = 0; i <midifile.getTrackCount(); i ++)
        {
            for (int j = 0; j < midifile[i].getEventCount(); j++ )
            {
                if ( selectedTrack != 0 && selectedTrack - 1 != i) //automatically play all notes of the non-active tracks
                {
                    if (midifile[i][j].seconds > prevsongTime && midifile[i][j].seconds <= songTime )
                    {
                        if (midifile[i][j].isNoteOn())
                        {
                            unsigned char currNote = midifile[i][j].getP1();
                            unsigned char velNote = midifile[i][j].getP2();
                            std::vector<unsigned char> message(3);
                            // Note On: 144, note, vel
                            message[0] = 144;
                            message[1] = currNote;
                            message[2] = velNote;
                            try {
                                midiout->sendMessage( &message );
                            } catch (...) {
                                qDebug() << "error while sending 1 MIDI msg";
                            }
                        }
                        if (midifile[i][j].isNoteOff())
                        {
                            unsigned char currNote = midifile[i][j].getP1();
                            unsigned char smthNote = midifile[i][j].getP2();
                            std::vector<unsigned char> message(3);
                            // Note Off: 128, note, ??vel??
                            message[0] = 128;
                            message[1] = 64;
                            message[2] = 40;
                            try {
                                midiout->sendMessage( &message );
                            } catch (...) {
                                qDebug() << "error while sending 1 MIDI msg";
                            }
                        }
                    }
                }
            }
        }
   }

   //same time limit for all modes; only time increase may change.
   if (songTime > songDuration + options.timeB4Restart/2.0)
   {
       songTime = - options.timeB4Restart/2.0;
       delayTime = 0;
       successNoteCount=0;
       errorNoteCount=0;
       missedNoteCount=0;
       currPressedNotes.clear();
   }
   if (isIntervalOn && songTime > intFinalTime + options.timeB4Restart/2.0) //when we finish the song :
   {
       songTime = intInitTime - options.timeB4Restart/2.0;
       delayTime = 0;
       successNoteCount=0;
       errorNoteCount=0;
       missedNoteCount=0;
       currPressedNotes.clear();
   }
   if (isIntervalOn && songTime > intFinalTime ) return; //don't play any note in that case
   if (isIntervalOn && songTime < intInitTime - options.timeB4Restart/2.0) songTime = intInitTime - options.timeB4Restart/2.0;
   if (isIntervalOn && songTime < intInitTime ) return; //don't play any note in that case
}
