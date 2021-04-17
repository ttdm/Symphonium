#include "gameoptions.h"
#include <QCoreApplication>

GameOptions::GameOptions()
{
    //setup default options for every game options.
    //later overrided by the config file is there is one
    //TO ADD : backGroundColor, NoteColor, keyboardWoMIDIsize ...
    isConfigModified = false;
    isConfigModifiedSinceLastDisplay = false;
    MIDIinPortName = "";
    MIDIoutPortName = "";
    timeB4Restart = 5.0;
    settingsFileName = "settings.ini";

    keyboardNoteColor = QColor(0x79, 0x1C, 0xF8);
    backgroundColor = QColor(40, 40, 40);
    for (int i = 0; i < 9 ; i++ ) MIDINoteColors << QColor(0x31, 0x8C, 0xE7) ;
}

bool GameOptions::saveConfig()
{
    QSettings settings(QString::fromStdString(settingsFileName),QSettings::IniFormat);
    //save DirList :
    if(dirLibrary.size() != 0)
    {
        settings.beginWriteArray("dirLibrary");
        for (int i = 0; i < dirLibrary.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("dir", dirLibrary.at(i).path());
        }
        settings.endArray();
    }

    //save fileList :
    if(fileLibrary.size() != 0)
    {
        settings.beginWriteArray("fileLibrary");
        for (int i = 0; i < fileLibrary.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("file", fileLibrary.at(i));
        }
        settings.endArray();
    }

    //save port config
    settings.setValue("MIDIinPortName", MIDIinPortName);
    settings.setValue("MIDIoutPortName", MIDIoutPortName);

    //save time B4 restart
    settings.setValue("timeB4Restart", timeB4Restart);


    //save colors ;
    settings.setValue("backgroundColor", backgroundColor.rgba());
    settings.setValue("keyboardNoteColor", keyboardNoteColor.rgba());

    settings.beginWriteArray("MIDINoteColors");
    for (int i = 0; i < MIDINoteColors.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("MIDINoteColor", MIDINoteColors.at(i).rgba());
    }
    settings.endArray();



    return true;
}

bool GameOptions::loadConfig()
{
    //set needed info for settings and load settings
    QCoreApplication::setOrganizationName("TTDMandHelpingFriends");
    QCoreApplication::setOrganizationDomain("https://github.com/ttdm/symphonium");
    QCoreApplication::setApplicationName("Synphonium");

    QSettings settings(QString::fromStdString(settingsFileName),QSettings::IniFormat);

    //read DirList
    int size = settings.beginReadArray("dirLibrary");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        dirLibrary.push_back(QDir(settings.value("dir").toString()));
    }
    settings.endArray();

    //read fileList
    size = settings.beginReadArray("fileLibrary");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        fileLibrary.push_back(settings.value("file").toString());
    }
    settings.endArray();

    if (settings.contains("MIDIinPortName")) MIDIinPortName = settings.value("MIDIinPortName").toString();
    if (settings.contains("MIDIoutPortName")) MIDIoutPortName = settings.value("MIDIoutPortName").toString();
    if (settings.contains("timeB4Restart")) timeB4Restart = settings.value("timeB4Restart").toDouble();

    if (settings.contains("backgroundColor")) backgroundColor = settings.value("backgroundColor").toUInt();
    if (settings.contains("keyboardNoteColor")) keyboardNoteColor = settings.value("keyboardNoteColor").toUInt();

    size = settings.beginReadArray("MIDINoteColors");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        MIDINoteColors[i] = settings.value("MIDINoteColor").toUInt();
    }
    settings.endArray();

    return true;
}
