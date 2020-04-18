QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The folling define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS = symphonium.ico

DEFINES += __WINDOWS_MM__ #TODO choose your MIDI intput/output library
#TODO LIBS and INCLUDEPATH must be edited to link your chosen MIDI intput/output library
win32: LIBS += -L$$PWD/'libs/winMM' -lWinMM-x64
win32: PRE_TARGETDEPS += $$PWD/'libs/winMM/WinMM-x64.lib'
unix: LIBS += -lm -lasound -pthread

INCLUDEPATH += libs/

SOURCES += \
    libs/MIDIFile/src/Binasc.cpp \
    libs/MIDIFile/src/MidiEvent.cpp \
    libs/MIDIFile/src/MidiEventList.cpp \
    libs/MIDIFile/src/MidiFile.cpp \
    libs/MIDIFile/src/MidiMessage.cpp \
    libs/RTMIDI/RtMidi.cpp \
    displayarea.cpp \
    gamemanager.cpp \
    gameoptions.cpp \
    main.cpp \
    symphonium.cpp

HEADERS += \
    libs/MIDIFile/include/Binasc.h \
    libs/MIDIFile/include/MidiEvent.h \
    libs/MIDIFile/include/MidiEventList.h \
    libs/MIDIFile/include/MidiFile.h \
    libs/MIDIFile/include/MidiMessage.h \
    libs/RTMIDI/RtMidi.h \
    displayarea.h \
    gamemanager.h \
    gameoptions.h \
    symphonium.h

FORMS += \
    symphonium.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
