#include "displayarea.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>

DisplayArea::DisplayArea(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    QLinearGradient gradient(QPointF(50, -20), QPointF(80, 20));
    gradient.setColorAt(0.0, Qt::white);
    gradient.setColorAt(1.0, QColor(0xa6, 0xce, 0x39));

    background = QBrush(QColor(40, 40, 40));
    midiNoteColor = QColor(0x31, 0x8C, 0xE7);
    keyboardNoteColor = QColor(0x79, 0x1C, 0xF8);

    minNote = 21;
    maxNote = 108;
    whiteKeySize = 17;
    blackKeySize = 11;

    songTime = -2; //in sec
    songTimeShowed = 5; //in sec
    selectedTrack = 0;

    isIntervalOn = false;
    intInitTime = -1;
    intFinalTime = -1;
}

//compute drawing parameters for new file then call update which will call paintEvent.
bool DisplayArea::createInitialDisplay(smf::MidiFile *midiFile)
{
    this->midifile = midiFile;

    // compute (once) all parameters that depends only on the midifile
    //find highest and lowest notes of the piano
    //Note careful : works only for single track midi file
    int highestNote = 21; //lowest possible note on a keyboard : C2 = 21 in MIDI
    int lowestNote = 108; //highest possible note on a keyboard : C8 = 108 in MIDI
    for (int i = 0; i < (*midifile).getTrackCount();i++ )
    {
        for (int j = 0; j < (*midifile)[i].getEventCount();j++ )
        {
            if ((*midifile)[i][j].isNoteOn())
            {
                int currNote = (*midifile)[i][j].getP1();
                if (currNote > highestNote) highestNote = currNote;
                if (currNote < lowestNote ) lowestNote = currNote;
            }
        }
    }

    //minNote and maxNote are defined as the first white keys outside the range of play
    this->minNote = std::max(lowestNote - 1,21);
    if (isBlackKey(this->minNote)) this->minNote -=1;
    this->maxNote = std::min(highestNote +1,108);
    if (isBlackKey(this->maxNote)) this->maxNote +=1;

    this->setOneLoopDisplayParameters();
    this->update(); //update the painting.
    return true;
}

void DisplayArea::setOneLoopDisplayParameters()
{
    //reinitialize things that need to be reinitialized at each loop :
    midiCurrentNotes.clear(); // clear this vector that holds data from the previous midifile

    //Get the notes and keys sizes.
    int whiteKeyNumber = 0;
    for (int i = this->minNote; i <= this->maxNote; i++)
    {
        if (!isBlackKey(i)) whiteKeyNumber++;
    }
    int w = this->width();
    whiteKeySize = std::max(w/whiteKeyNumber,1);
    blackKeySize = whiteKeySize * 1.5/2.3; //size ratio from my keyboard

    return;
}

bool DisplayArea::isBlackKey(int keyId)
{
    int id = keyId%12;
    if(id == 10 || id == 1 || id == 3 || id == 6 || id == 8 ) return true;
    return false;
}

void DisplayArea::paintPiano(QPainter *painter)
{   
    int h = this->height();
    int w = this->width();
    int ph = getPianoHeight();
    int h0 = h - ph;

    //draw black background
    painter->fillRect(0,h0, w, ph, Qt::black);

    //draw red line above the piano
    int redLineHeight = std::max(2,ph/50);
    painter->fillRect(0,h0, w, redLineHeight, Qt::red);
    int h0rl = h0 + redLineHeight;

    int keyBorderWidth = std::min(1,whiteKeySize/10);

    // draw all the white keys
    for(int i = keyBorderWidth; i < w ; i += whiteKeySize )
    {
        painter->fillRect(i,h0rl, whiteKeySize-keyBorderWidth, h - h0rl, Qt::white);
    }

    //paint MIDIWhitekeyPressed if there are some.
    if ( midiCurrentNotes.size() != 0 )
    {
        for (int i = 0; i <midiCurrentNotes.size(); i++)
        {
            if (!isBlackKey(midiCurrentNotes.at(i)))
            {
                QColor keyColor = midiNoteColor;
                keyColor.setAlpha(170);
                painter->fillRect(getNoteX0(midiCurrentNotes.at(i)), h0rl, whiteKeySize, h - h0rl, QBrush(keyColor));
            }
        }
    }
    //paint PIANO WhitekeyPressed if there are some.
    std::set<int>::iterator iter;
    for(iter=currPressedNotes.begin(); iter!=currPressedNotes.end();++iter){
        if (!isBlackKey(*iter))
        {
            QColor keyColor = keyboardNoteColor;
            keyColor.setAlpha(220);
            painter->fillRect(getNoteX0(*iter), h0rl, whiteKeySize, h - h0rl, QBrush(keyColor));
        }
    }

    //Draw the black keys above the white ones
    for (int i = this->minNote; i <= this->maxNote; i++)
    {
        if (isBlackKey(i))
        {
            painter->fillRect(getNoteX0(i),h0rl, blackKeySize, (h - h0rl)*0.65, Qt::black);
        }
    }


    //paint the midiBlackKeyPressed if there are some.
    if ( midiCurrentNotes.size() != 0 )
    {
        for (int i = 0; i <midiCurrentNotes.size(); i++)
        {
            if (isBlackKey(midiCurrentNotes.at(i)))
            {
                QColor keyColor = midiNoteColor;
                keyColor.setAlpha(170);
                painter->fillRect(getNoteX0(midiCurrentNotes.at(i)),h0rl, blackKeySize, (h - h0rl)*0.65, QBrush(keyColor));
            }
        }
    }
    //and finally paint the keyboard back key pressed
    for(iter=currPressedNotes.begin(); iter!=currPressedNotes.end();++iter){
        if (isBlackKey(*iter))
        {
            QColor keyColor = keyboardNoteColor;
            keyColor.setAlpha(220);
            painter->fillRect(getNoteX0(*iter), h0rl, blackKeySize, (h - h0rl)*0.65, QBrush(keyColor));
        }
    }

    return;
}

void DisplayArea::paintNotes(QPainter *painter)
{
    // update the curent keypressed
    midiCurrentNotes.clear();

    // paint the keys.
    float maxTime = songTime + songTimeShowed;

    for (int i = 0; i < midifile->getTrackCount(); i++ )
    {
        for (int j = 0; j < (*midifile)[i].getEventCount(); j++ )
        {
            if ((*midifile)[i][j].isNoteOn())
            {
                int currNote = (*midifile)[i][j].getP1();
                float startNoteTime = (*midifile)[i][j].seconds;
                float endNoteTime = (*midifile)[i][j].seconds + (*midifile)[i][j].getDurationInSeconds();

                if (startNoteTime > maxTime + 0.05) continue;
                if (endNoteTime < songTime - 0.05 ) continue;

                int noteSize = isBlackKey(currNote) ? blackKeySize : whiteKeySize;
                QColor thismidiNoteColor = isBlackKey(currNote) ? this->midiNoteColor.darker() : this->midiNoteColor;
                if (selectedTrack != 0 && i != selectedTrack - 1) thismidiNoteColor = Qt::gray;
                if (isIntervalOn && (startNoteTime < intInitTime || startNoteTime > intFinalTime)) thismidiNoteColor = Qt::gray;

                QPainterPath path;
                // -.5 below improve the quality of the borders, see https://stackoverflow.com/questions/29196610/qt-drawing-a-filled-rounded-rectangle-with-border#29196812
                int visualOffset = -2; // With a visual small offset, the sound and the visual seems to be better synchronized.
                path.addRoundedRect(QRectF(getNoteX0(currNote)-0.5, getHeightForTime(endNoteTime)-0.5+visualOffset, noteSize-1, getHeightForTime(startNoteTime)-getHeightForTime(endNoteTime)-1+visualOffset), 4, 4);
                QPen pen(Qt::black, 2);
                painter->setPen(pen);
                painter->fillPath(path, thismidiNoteColor);
                painter->drawPath(path);

                //painter->fillRect(getNoteX0(currNote),getHeightForTime(endNoteTime), noteSize-1, getHeightForTime(startNoteTime)-getHeightForTime(endNoteTime) -1 , midiNoteColor);

                if (startNoteTime < songTime && endNoteTime > songTime ) midiCurrentNotes.push_back(currNote);
            }
        }
    }
}

int DisplayArea::getHeightForTime(float t)
{
    float h = getNoteAreaHeight();
    float timeAtH = songTime;
    float timeAt0 = songTime + songTimeShowed;
    return (timeAt0 - t) * h / (timeAt0 - timeAtH);
}

int DisplayArea::getNoteX0(int note)
{
    int numberOfWhiteNotesB4Note = 0; //not counting current note.
    for (int i = minNote; i < note; i++)
    {
        if (!isBlackKey(i)) numberOfWhiteNotesB4Note ++;
    }
    if (!isBlackKey(note))
    {
        return numberOfWhiteNotesB4Note*whiteKeySize;
    }
    else if (note%12 == 8) //centered black key between 2 white
    {
        return numberOfWhiteNotesB4Note*whiteKeySize-blackKeySize/2;
    }
    else if (note%12 == 1 || note%12 == 6) //black keys more on the left white key
    {
        return numberOfWhiteNotesB4Note*whiteKeySize-blackKeySize/4*3;
    }
    else //black keys more on the right white key
    {
        return numberOfWhiteNotesB4Note*whiteKeySize-blackKeySize/4;
    }
    return -1;
}

int DisplayArea::getNoteAreaHeight()
{
    return this->height() - getPianoHeight();
}

int DisplayArea::getPianoHeight()
{
    return std::max(100,this->height()/4);
}

void DisplayArea::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    //qDebug() << "why does a Qdebug here make it work ???? ";
    painter.fillRect(event->rect(), background); //create the uniform colored background
    this->setOneLoopDisplayParameters(); //(mainly) update key sizes depending on window width
    if (isMidiSelectedFileValid) this->paintNotes(&painter); //add all the notes if there are some
    this->paintPiano(&painter); //add the keyboard above the notes (to have proper boarders, the notes goes "under" the keyboard)

    painter.end();
}

void DisplayArea::getParamsFromManager(GameManager *manager){
    isMidiSelectedFileValid = manager->isMidiSelectedFileValid;
    songTime = manager->songTime;
    selectedTrack = manager->selectedTrack;
    isIntervalOn = manager->isIntervalOn;
    intInitTime = manager->intInitTime;
    intFinalTime = manager->intFinalTime;
    currPressedNotes = manager->currPressedNotes;

    if (manager->isMidiSelectedFileValid) createInitialDisplay(&manager->midifile);
}

void DisplayArea::animate()
{
   update(); //QT created slot that call paintEvent.
}

