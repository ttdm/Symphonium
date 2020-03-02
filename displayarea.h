// This class only draw on the "displayArea" (where the "notes are drawn")
// it receive info from pianoLearning and gameManager.

#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H

#include <QWidget>
#include <QColor>
#include <QImage>
#include <QPoint>
#include <QPen>
#include "MIDIFile/include/MidiFile.h"

#include "gamemanager.h"


class DisplayArea : public QWidget
{
    Q_OBJECT

public:
    DisplayArea(QWidget *parent = 0);
    virtual ~DisplayArea() {};
    bool createInitialDisplay(smf::MidiFile *midifile);
    void getParamsFromManager(GameManager *manager);
    double songTimeShowed;

public slots:
    void animate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void paintNotes(QPainter *painter);
    void paintPiano(QPainter *painter);
    int getPianoHeight();
    int getNoteAreaHeight();
    int getNoteX0(int note);
    int getHeightForTime(float t);
    bool isBlackKey(int keyId);
    void setOneLoopDisplayParameters();

    bool isMidiSelectedFileValid;

    QBrush background;
    QBrush circleBrush;
    QFont textFont;
    QPen circlePen;
    QPen textPen;
    int elapsed;

    //display parameters :
    int minNote;
    int maxNote;

    smf::MidiFile *midifile;
    int whiteKeySize, blackKeySize;
    QVector<int> midiCurrentNotes;
    QColor midiNoteColor;
    std::set<int> currPressedNotes;
    QColor keyboardNoteColor;

    double songTime;
    int selectedTrack;

    //interval
    bool isIntervalOn;
    double intInitTime, intFinalTime;



};

#endif // DISPLAYAREA_H
