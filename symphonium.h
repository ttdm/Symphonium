#ifndef SYMPHONIUM_H
#define SYMPHONIUM_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QVector>
#include <QDir>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QTimeEdit>

#include "displayarea.h"
#include "gamemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Symphonium; }
QT_END_NAMESPACE

class DisplayArea;

class Symphonium : public QMainWindow
{
    Q_OBJECT

public:
    Symphonium(QWidget *parent = nullptr);
    ~Symphonium();

private slots:
    //main slot, executed every few ms.
    void update();

    // slots on library managment
    void on_actionAdd_File_To_Lib_triggered();
    void on_actionAdd_Folder_To_Lib_triggered();

    // slot to select and play one music with all options on it.
    void on_actionPlay_Pause_triggered();
    void on_libraryTableWidget_itemSelectionChanged();
    void on_actionStop_triggered();
    void on_actionSkip_Fwd_triggered();
    void on_actionSkip_Back_triggered();
    void on_actionLibrary_Panel_triggered();
    void on_actionFull_Screen_triggered();
    void on_actionAbout_triggered();
    void on_actionOnline_FAQ_triggered();
    void selectMode(int);
    void selectTrackMode(int);
    void setRTFactor(int);
    void setDisplayDuration(double);
    void setSoundLevel(int);
    void restreamMIDIin(int);

    //override of closeEvent to save config.
    void closeEvent(QCloseEvent *event);

    void on_slider_valueChanged(int value);

    void on_actionselect_MIDI_device_triggered();
    void finishMIDIdeviceSelection();
    void cancelMIDIdeviceSelection();
    void changeTimeB4Restart(double);
    void useIntervalSwitch(int);
    void intervalBeginTimeChanged(QTime);
    void intervalEndTimeChanged(QTime);

private:
    Ui::Symphonium *ui;
    DisplayArea *displayArea;
    GameManager manager;
    int timerCount;
    QAction *trackSelectionWidgetAddAction; // widget updated during execution; needs qaction ref to delete old irrelevant one
    QAction *trackSelectionFollowingWidgetAction; // needed to place corectly the previous widget
    QLabel *songDurationLabel;
    QComboBox *modeCombo;
    QDialog *midiDialog;
    QComboBox *midiinCombo, *midioutCombo;

    QTimeEdit *intervalBeginTime, *intervalEndTime;

    void displayMIDILibrary();
    bool addDirectoryToLib(QDir directory);
    bool addFileTolib(QString filePath);
    void setupToolbar();
};

#endif // SYMPHONIUM_H
