// Handle all the options that can be setup by a user for multiple sessions throught QSettings.

#ifndef GAMEOPTIONS_H
#define GAMEOPTIONS_H

#include <QSettings>
#include <QVector>
#include <QDir>
#include <QString>
#include <QColor>

class GameOptions
{
public:
    GameOptions();
    //all stuff linked to the config file.
    bool isConfigModified; //true if any parameter in the config file has changed and a message box needs to be printed when closing the app.
    bool isConfigModifiedSinceLastDisplay; //true means that all the display options will be updated (and not just a few dynamic ones)
    std::string settingsFileName;

    //MIDI library handling
    QVector<QDir> dirLibrary;
    QVector<QString> fileLibrary;
    bool loadConfig();
    bool saveConfig();

    //midi port names
    QString MIDIinPortName, MIDIoutPortName;

    //game parameters saved over time
    double timeB4Restart;//time before song restart
    double positiveTimeErrorAllowed; //delta delay allowed between midiin and midifile before considering a note missed;
    double negativeTimeErrorAllowed; //delta delay allowed between midiin and midifile before considering a note missed;

    //colors
    QColor keyboardNoteColor, backgroundColor;
    QVector<QColor> MIDINoteColors;


};

#endif // GAMEOPTIONS_H
