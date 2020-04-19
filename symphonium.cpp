#include "symphonium.h"
#include "ui_symphonium.h"
#include "MIDIFile/include/MidiFile.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QTableWidget>
#include <QTimer>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QDirIterator>

#include <cmath>

Symphonium::Symphonium(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Symphonium)
{
    ui->setupUi(this);

    //define window namesymphonium
    setWindowTitle(tr("Symphonium"));

    //define initial spacing between the library and the piano display
    ui->horMainSplitter->setStretchFactor(0,1);
    ui->horMainSplitter->setStretchFactor(1,6); //weird, needs 1/6 ratio to define a 1/4 ratio; for an unknown reason, it changed when i defined the final QWidget type ...

    //resize the main window to smth acceptable
    //QSize availableSize = qApp->desktop()->availableGeometry().size();
    int width = 1000;//availableSize.width()*.9;
    int height = 800;//availableSize.height()*.9;
    resize(width, height);

    //if there is one load the config stored in memory
    manager.options.loadConfig();

    //create the toolbar (since the QTdesigner doesnt give much options to do so)
    setupToolbar();

    //displayTheMIDILibrary
    displayMIDILibrary();

    //create and connect RTMIDI to ports in memory
    if (!manager.createRTMIDIobjects())
    { //cannot create RTmidi objects, software issue that a user cannot solve
        QMessageBox Msgbox;
        Msgbox.setWindowTitle("Technical Error");
        Msgbox.setTextFormat(Qt::RichText);
        Msgbox.setText("<center>Software error while creating RTMidi Objects. My apologies.<br>"
                        "You can watch the song and test the software but not play.<br>"
                        "Feel free to contact me by opening a <a href=\"https://github.com/ttdm/symphonium/issues\">Github issue</a>.<br></center>");
        Msgbox.exec();
    }
    else
    {
        manager.connectRTMIDIatStart();
    }


    if (!manager.isMIDIDeviceReady)
    {
        ui->statusbar->setStyleSheet("color: red");
        ui->statusbar->showMessage("The MIDI device in memory has not been found ! Not sending and receiving MIDI messages at the moment !");
    }

    //create a timer fonction to make the app work periodically.
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(4); //update at 250Hz.
    timerCount = 0;

    songDurationLabel = new QLabel(this);
    songDurationLabel->setText("");
    songDurationLabel->setFixedWidth(80);
    songDurationLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    songDurationLabel->show();

    ui->menubar->raise(); //probably hide a little error; if not some buttons are not selectable (caused by the library)
}

//update manager and display periodically
void Symphonium::update()
{
    //qDebug() <<"in update";
    manager.update(); //250Hz
    //if (timerCount%4 == 0) //60hz output seems jumpy; 250hz output seems nice, so lets let it that way
    {
        ui->displayArea->getParamsFromManager(&manager);
        ui->displayArea->update(); //60Hz
    }
    timerCount ++;
    if (timerCount > 3) timerCount = 0;

    //display good timer and the correct position of the pointer in the scroll bar.
    if (!manager.isMidiSelectedFileValid)
    {
        //hide time and put the scroll bar cursor on the left
        ui->slider->setValue(0);
        songDurationLabel->setText("");
    }
    else
    {
        //add a label with the song duration and current song time
        int maxmin = ((int)(manager.songDuration))/60;
        int maxsec = (int)(manager.songDuration)-60*maxmin;
        int currmin = ((int)(manager.songTime))/60;
        int currsec = (int)(manager.songTime)-60*currmin;
        songDurationLabel->setText(tr("%1'%2s/%3'%4s").arg(currmin).arg(currsec).arg(maxmin).arg(maxsec));
        int labelx = ui->slider->pos().x() + ui->slider->width() - 80;
        int labely = ui->slider->pos().y() + 29;
        songDurationLabel->move(QPoint(labelx, labely));

        //place the scroll bar cursor according to currentSongTime vs maxSongTime;
        double timeRatio = manager.songTime/manager.songDuration;
        ui->slider->setValue((int)(timeRatio*1000));
    }
}

//add the toolbar :
void Symphonium::setupToolbar()
{
    //game mode
    modeCombo = new QComboBox();
    modeCombo->addItem("Listen to the song",Qt::DisplayRole);
    modeCombo->addItem("Learn the song",Qt::DisplayRole);
    modeCombo->addItem("Play the song",Qt::DisplayRole);
    ui->toolBar->addWidget(modeCombo);
    connect(modeCombo, SIGNAL(activated(int)), this, SLOT(selectMode(int)));

    QComboBox *trackCombo = new QComboBox();
    trackCombo->addItem("All Tracks",Qt::DisplayRole);
    trackSelectionWidgetAddAction = ui->toolBar->addWidget(trackCombo);
    connect(trackCombo, SIGNAL(activated(int)), this, SLOT(selectTrackMode(int)));

    //song main controls
    trackSelectionFollowingWidgetAction = ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionPlay_Pause);
    ui->toolBar->addAction(ui->actionStop);

    //display controls
    ui->toolBar->addSeparator();
    QSpinBox *RTfactorSpin = new QSpinBox;
    RTfactorSpin->setRange(1,500);
    RTfactorSpin->setRange(1,500);
    RTfactorSpin->setSingleStep(10);
    RTfactorSpin->setPrefix(tr("Speed : "));
    RTfactorSpin->setSuffix(tr("%"));
    RTfactorSpin->setValue(100);
    RTfactorSpin->setFixedWidth(QFontMetrics(this->font()).horizontalAdvance("Speed : 100%")+25);
    ui->toolBar->addWidget(RTfactorSpin);
    connect(RTfactorSpin, SIGNAL(valueChanged(int)), this, SLOT(setRTFactor(int)));

    QDoubleSpinBox *displayDurationSpin = new QDoubleSpinBox;
    displayDurationSpin->setRange(1.,20.0);
    displayDurationSpin->setPrefix(tr("Display Duration : "));
    displayDurationSpin->setSuffix(tr("s"));
    displayDurationSpin->setSingleStep(1.0);
    displayDurationSpin->setValue(5.0);
    displayDurationSpin->setFixedWidth(QFontMetrics(this->font()).horizontalAdvance("Display Duration : 20.0s")+25);
    ui->toolBar->addWidget(displayDurationSpin);
    connect(displayDurationSpin, SIGNAL(valueChanged(double)), this, SLOT(setDisplayDuration(double)));

    //interval handling
    ui->toolBar->addSeparator();
    QCheckBox *useInterval = new QCheckBox;
    useInterval->setText("Restrict to Interval,");
    connect(useInterval, SIGNAL(stateChanged(int)), this, SLOT(useIntervalSwitch(int)));
    ui->toolBar->addWidget(useInterval);//checkbox
    QLabel *intervalFrom = new QLabel;
    intervalFrom->setText("From ");
    ui->toolBar->addWidget(intervalFrom);//label
    intervalBeginTime = new QTimeEdit;
    intervalBeginTime->setFixedWidth(QFontMetrics(this->font()).horizontalAdvance("mm:ss")+15);
    intervalBeginTime->setDisplayFormat(QString("mm:ss"));
    ui->toolBar->addWidget(intervalBeginTime);//timeedit
    connect(intervalBeginTime, SIGNAL(timeChanged(QTime)), this, SLOT(intervalBeginTimeChanged(QTime)));
    QLabel *intervalTo = new QLabel;
    intervalTo->setText(" To ");
    ui->toolBar->addWidget(intervalTo);//label
    intervalEndTime = new QTimeEdit;
    intervalEndTime->setFixedWidth(QFontMetrics(this->font()).horizontalAdvance("mm:ss")+15);
    intervalEndTime->setDisplayFormat(QString("mm:ss"));
    ui->toolBar->addWidget(intervalEndTime);//timeedit
    connect(intervalEndTime, SIGNAL(timeChanged(QTime)), this, SLOT(intervalEndTimeChanged(QTime)));

    //precise song controls
    ui->toolBar->addSeparator();
    QDoubleSpinBox *timeB4restarting = new QDoubleSpinBox;
    timeB4restarting->setRange(0.0,60.0);
    timeB4restarting->setPrefix(tr("Time before restarting : "));
    timeB4restarting->setSuffix(tr("s"));
    timeB4restarting->setSingleStep(0.5);
    timeB4restarting->setValue(manager.options.timeB4Restart);
    timeB4restarting->setFixedWidth(QFontMetrics(this->font()).horizontalAdvance("Time before restarting : 60.0s")+25);
    ui->toolBar->addWidget(timeB4restarting);
    connect(timeB4restarting, SIGNAL(valueChanged(double)), this, SLOT(changeTimeB4Restart(double)));

    //audio handling
    ui->toolBar->addSeparator();
    QSpinBox *soundLevelSpin = new QSpinBox;
    soundLevelSpin->setRange(0,500);
    soundLevelSpin->setRange(0,500);
    soundLevelSpin->setSingleStep(10);
    soundLevelSpin->setPrefix(tr("Volume : "));
    soundLevelSpin->setSuffix(tr("%"));
    soundLevelSpin->setValue(100);
    soundLevelSpin->setFixedWidth(QFontMetrics(this->font()).horizontalAdvance("Volume : 500%")+25);
    ui->toolBar->addWidget(soundLevelSpin);
    connect(soundLevelSpin, SIGNAL(valueChanged(int)), this, SLOT(setSoundLevel(int)));
    QCheckBox *restreamMIDIIn = new QCheckBox;
    restreamMIDIIn->setText("Forward MIDI Input to output");
    connect(restreamMIDIIn, SIGNAL(stateChanged(int)), this, SLOT(restreamMIDIin(int)));
    ui->toolBar->addWidget(restreamMIDIIn);
}

void Symphonium::setDisplayDuration(double duration)
{
    ui->displayArea->songTimeShowed = duration;
}

void Symphonium::setRTFactor(int fact)
{
    manager.RTFactor = (double)fact/100.0;
}

void Symphonium::setSoundLevel(int fact) //TODO important; change IT;
{
    manager.userVolumeMultiplier = (double)fact/100.0;
    manager.setVolume();
}

void Symphonium::selectMode(int answerId)
{
    if (answerId != 0 && !manager.isMIDIDeviceReady) // don't change the gamemode if there isnt any MIDI device
    {
        QMessageBox Msgbox;
          Msgbox.setText("You need to set a MIDI device in 'options->set MIDI device' before attempting to play the songs.");
            Msgbox.exec();
         modeCombo->setCurrentIndex(0);
    }
    else
    {
        manager.gameMode = answerId;
    }
}

void Symphonium::selectTrackMode(int answerId)
{
    manager.selectedTrack = answerId;
}

// add a file to the MIDI library through UI
void Symphonium::on_actionAdd_File_To_Lib_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());

    // If we have a file name that is a midi file, add it to the list.
    if (!fileName.isEmpty())
    {
        if (!addFileTolib(fileName))
        {
            QMessageBox Msgbox;
            Msgbox.setText("The selected file is not a valid MIDI file.");
            Msgbox.exec();
            return;
        }
        manager.options.fileLibrary.push_back(fileName);
        manager.options.isConfigModified = true;
    }
}

// add a folder to the MIDI library through UI
void Symphonium::on_actionAdd_Folder_To_Lib_triggered()
{
    //TODO small improvement; this dialog should show file but allow only directory selection; not trivial according to online Q&As
    QDir directory = QFileDialog::getExistingDirectory(this, tr("Select directory"));
    if (directory.path() == ".") return;
    bool fileAdded = addDirectoryToLib(directory);
    if (fileAdded)
    {
        manager.options.dirLibrary.push_back(directory);
        manager.options.isConfigModified = true;
    }
}

//
bool Symphonium::addDirectoryToLib(QDir directory)
{
    int addedFile = 0;
    QDirIterator it(directory.path(), QDir::Files);
    while (it.hasNext())
    {
        bool fileAdded = addFileTolib(directory.filePath(it.next()));
        if (fileAdded) addedFile ++;
    }

    if (addedFile == 0)
    {
        QMessageBox Msgbox;
          Msgbox.setText("No valid MIDI file found in the directory.");
            Msgbox.exec();
            return false;
    }
    return true;
}

bool Symphonium::addFileTolib(QString filePath)
{
    //check if MIDIFile can read the file. If yes continue; if not cancel.
    smf::MidiFile file;
    file.read(filePath.toStdString());
    if ( !file.status() )
    {
            return false;
    }

    //get the name of the file
    QString name = QFileInfo(filePath).fileName();
    if (name.lastIndexOf(".") != -1) name = name.left(name.lastIndexOf("."));

    // we fill the last empty row
    QTableWidgetItem *newFileName = new QTableWidgetItem(name);
    QTableWidgetItem *newFilePath = new QTableWidgetItem(filePath);
    QTableWidgetItem *newFileLength = new QTableWidgetItem(tr("?"));

    int rowCount = ui->libraryTableWidget->rowCount();
    ui->libraryTableWidget->setItem(rowCount -1, 0, newFileName);
    ui->libraryTableWidget->setItem(rowCount -1, 1, newFileLength);
    ui->libraryTableWidget->setItem(rowCount -1, 2, newFilePath);

    ui->libraryTableWidget->setRowCount(rowCount+1);

    //improvement2: parse the MIDI file to find its length and other infos
            //midifile.getFileDurationInSeconds(); midifile.getEventCount() (then filter event with notepressOn)
    return true;
}

void Symphonium::displayMIDILibrary()
{
    //create a table at the good size
    ui->libraryTableWidget->setColumnCount(3);

    //setup the table labels
    QStringList headers = { "Name", "length", "Path" };
    ui->libraryTableWidget->setHorizontalHeaderLabels(headers);

    //hide the path
    ui->libraryTableWidget->hideColumn(2);
    ui->libraryTableWidget->hideColumn(1);
    ui->libraryTableWidget->setColumnWidth(0,250);

    //create an empty line
    ui->libraryTableWidget->setRowCount(1);

    // add stuff from the config file
    if (manager.options.dirLibrary.size() != 0)
    {
        foreach (QDir dirName, manager.options.dirLibrary) {
           addDirectoryToLib(dirName);
        }
    }
    if (manager.options.fileLibrary.size() != 0)
    {
        foreach (QString fileName, manager.options.fileLibrary) {
           addFileTolib(fileName);
        }
    }
}

void Symphonium::closeEvent(QCloseEvent *event)
{
    if(!manager.options.isConfigModified) {event->accept(); return;}

    QMessageBox msgBox;
    msgBox.setText("Your library and options have been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    switch (ret) {
      case QMessageBox::Save:
          manager.options.saveConfig();
          event->accept();
          break;
      case QMessageBox::Discard:
          event->accept();
          break;
      case QMessageBox::Cancel:
          event->ignore();
          break;
      default:
          // should never be reached
            event->accept();
          break;
    }
}

void Symphonium::on_actionPlay_Pause_triggered()
{
    //check if the selected item can be played
    if (!manager.isMidiSelectedFileValid)
    {
        QMessageBox Msgbox;
          Msgbox.setText("Please select a file by clicking on it in the library on the left side of your screen.");
            Msgbox.exec();
            return;
    }

    //if yes, toggle the isMIDIplaying constant (play/pause behavior)
    manager.isMIDIPlaying = !manager.isMIDIPlaying;
}

void Symphonium::on_libraryTableWidget_itemSelectionChanged()
{
    //change the selected file to NOTVALID until checked otherwise :
    manager.isMidiSelectedFileValid = false;

    // if we select the empty last row (which makes it 0 items), return;
    if ( ui->libraryTableWidget->selectedItems().length() == 0 ) return;

    //get selected item row
    int sRow = ui->libraryTableWidget->selectedItems().at(0)->row();

    // last row being empty; return if the selected row is the last
    if (sRow >= ui->libraryTableWidget->rowCount()-1) return;

    //retrieve the MIDI file path.
    QString path = ui->libraryTableWidget->item(sRow,2)->text();

    manager.selectMIDIFile(path.toStdString());

    if (manager.isMidiSelectedFileValid)
    {
        ui->toolBar->removeAction(trackSelectionWidgetAddAction);
        QComboBox *trackCombo = new QComboBox();
        trackCombo->addItem("All Tracks",Qt::DisplayRole);
        for (int i = 0; i < manager.midifile.getTrackCount() ; i++ )
        {
            trackCombo->addItem("Track " + tr("%1").arg(i),Qt::DisplayRole);
        }
        trackSelectionWidgetAddAction = ui->toolBar->insertWidget(trackSelectionFollowingWidgetAction,trackCombo);
        connect(trackCombo, SIGNAL(activated(int)), this, SLOT(selectTrackMode(int)));
    }

    int minutes = std::floor(manager.songDuration/60.0);
    int seconds = manager.songDuration - 60 * minutes;

    intervalBeginTime->setMinimumTime(QTime(0,0,0));
    intervalBeginTime->setTime(QTime(0,0,0));
    intervalEndTime->setMinimumTime(QTime(0,0,1));
    intervalEndTime->setMaximumTime(QTime(0,minutes,seconds));
    intervalEndTime->setTime(QTime(0,minutes,seconds));

    //minus 1sec properly :
    if (seconds == 0) { seconds = 59; minutes--; }
    else seconds--;
    intervalBeginTime->setMaximumTime(QTime(0,minutes,seconds));


}

Symphonium::~Symphonium()
{
    delete ui;
}

void Symphonium::on_actionStop_triggered()
{
    if (manager.isIntervalOn)
    {
        manager.songTime = manager.intInitTime - manager.options.timeB4Restart/2.0;
    }
    else
    {
        manager.songTime = -manager.options.timeB4Restart/2.0;
    }
    manager.isMIDIPlaying = false;
}

void Symphonium::on_actionSkip_Fwd_triggered()
{
    if (manager.isMidiSelectedFileValid)
    {
        manager.songTime += manager.songDuration/10.0;
    }
}

void Symphonium::on_actionSkip_Back_triggered()
{
    if (manager.isMidiSelectedFileValid)
    {
        manager.songTime -= manager.songDuration/10.0;
        if (manager.songTime < -manager.options.timeB4Restart/2.0 ) manager.songTime = -manager.options.timeB4Restart/2.0;
    }
}

void Symphonium::on_actionLibrary_Panel_triggered()
{
    if ( ui->libraryTableWidget->size().width() * 10 < this->size().width())
    {
        QList<int> widths = {this->size().width()/5,4*this->size().width()/5};
        ui->horMainSplitter->setSizes(widths);
    }
    else
    {
        QList<int> widths = {0,this->size().width()};
        ui->horMainSplitter->setSizes(widths);
    }
}

void Symphonium::on_actionFull_Screen_triggered()
{
    if(isFullScreen()) {
         this->setWindowState(Qt::WindowNoState);
      } else {
         this->setWindowState(Qt::WindowFullScreen);
      }
}

void Symphonium::on_actionAbout_triggered()
{
    QMessageBox Msgbox;
    Msgbox.setWindowTitle("About Symphonium");
    Msgbox.setTextFormat(Qt::RichText);
    Msgbox.setText("<center> Symphonium is an open source and cross-platform project designed to help learning to play the piano.<br>"
                   "More information on it can be found on its <a href=\"https://symphonium.net\">website</a>.<br>"
                   "Its sources can be found on its <a href=\'https://github.com/ttdm/symphonium/\'>github repository</a>. Any help to improve the software is welcome !<br>"
                   "Initially developped by <a href=\"https://ttdm.github.io/\">TTDM</a>.</center>");
    Msgbox.exec();
}

void Symphonium::on_actionOnline_FAQ_triggered()
{
    QString FAQLink = "https://symphonium.net/faq/";
    QDesktopServices::openUrl(QUrl(FAQLink));
}

void Symphonium::on_slider_valueChanged(int value)
{
    if (manager.isMidiSelectedFileValid)
    {
        manager.songTime = (double)value/1000.0*manager.songDuration;
        if (value==0) manager.songTime = -manager.options.timeB4Restart/2.0;
    }
}

void Symphonium::on_actionselect_MIDI_device_triggered()
{
    midiDialog = new QDialog();
    midiDialog->setWindowTitle("Select your MIDI in/out devices");

    midiinCombo = new QComboBox();
    unsigned int nInPorts = manager.midiin->getPortCount();
    for ( unsigned int i=0; i<nInPorts+1; i++ ) { //+1 to allow empty input selec if only an output exist (which allow to listen to the songs))
          try {
          std::string portName = manager.midiin->getPortName(i);
            if (portName.find("RtMidi") != std::string::npos) continue; //don't add RTmidi ports.
            midiinCombo->addItem(QString::fromStdString(portName),Qt::DisplayRole);
        }
              catch ( RtMidiError &error ) {
            midiinCombo->addItem("",Qt::DisplayRole);
              }
    }

    midioutCombo = new QComboBox();
    unsigned int nOutPorts = manager.midiout->getPortCount();
    for ( unsigned int i=0; i<nOutPorts+1; i++ ) {
          try {
          std::string portName = manager.midiout->getPortName(i);
          if (portName.find("RtMidi") != std::string::npos) continue; //don't add RTmidi ports.
            midioutCombo->addItem(QString::fromStdString(portName),Qt::DisplayRole);
        }
              catch ( RtMidiError &error ) {
            midioutCombo->addItem("",Qt::DisplayRole);
              }
    }
    QLabel *topLabel = new QLabel(tr("Select your input MIDI Device :"));
    QLabel *botLabel = new QLabel(tr("Select your output MIDI Device :"));

    QPushButton *apply = new QPushButton;
    apply->setText("Apply");
    connect(apply, SIGNAL(clicked()), this, SLOT(finishMIDIdeviceSelection()));

    QPushButton *cancel = new QPushButton;
    cancel->setText("Cancel");
    connect(cancel, SIGNAL(clicked()), this, SLOT(cancelMIDIdeviceSelection()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(apply);
    buttonLayout->addWidget(cancel);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    layout->addWidget(midiinCombo);
    layout->addWidget(botLabel);
    layout->addWidget(midioutCombo);
    layout->addLayout(buttonLayout);
    midiDialog->setLayout(layout);

    midiDialog->show();
}

void Symphonium::finishMIDIdeviceSelection()
{
    midiDialog->hide();
    manager.isMIDIDeviceReady = false;

    //close the previously open ports
    if ( manager.midiin->isPortOpen() ) manager.midiin->closePort();
    if ( manager.midiout->isPortOpen() ) manager.midiout->closePort();

    //open the new ports
    //get the port number corresponding to the selected options in the combobox :
    unsigned int nInPorts = manager.midiin->getPortCount();
    int inPortNumber = -1;
    for ( unsigned int i=0; i<nInPorts+1; i++ ) {
        if ( manager.midiin->getPortName(i) == midiinCombo->itemData(midiinCombo->currentIndex()).toString().toStdString() ) inPortNumber = i;
    }
    unsigned int nOutPorts = manager.midiout->getPortCount();
    int outPortNumber = -1;
    for ( unsigned int i=0; i<nOutPorts+1; i++ ) {
        if ( manager.midiout->getPortName(i) == midioutCombo->itemData(midioutCombo->currentIndex()).toString().toStdString() ) outPortNumber = i;
    }

    if (!manager.connectRTMIDIobjects2ports(midiinCombo->currentIndex(),midioutCombo->currentIndex()))
    {
        QMessageBox Msgbox;
        Msgbox.setWindowTitle("Error while connecting to the selected devices");
        Msgbox.setTextFormat(Qt::RichText);
          Msgbox.setText("<center>Error while connecting to the selected devices. Please try to connect other MIDI devices.<br>"
                         "Please note that connecting ONLY to an output device will trigger this message but still allow to watch the MIDIFile and to listen to them.<br><br>"
                         "Feel free to ask for help by opening a <a href=\"https://github.com/ttdm/symphonium/issue\">github issue</a>.</center>");
            Msgbox.exec();
        return;
    }
    manager.isMIDIDeviceReady = true;
}

void Symphonium::cancelMIDIdeviceSelection()
{
    midiDialog->hide();
}

void Symphonium::changeTimeB4Restart(double value)
{
    if (manager.options.timeB4Restart == value ) return;

    manager.options.timeB4Restart = value;
    manager.options.isConfigModified = true;
}

void Symphonium::useIntervalSwitch(int state)
{
    manager.isIntervalOn = (state != 0);
}

void Symphonium::restreamMIDIin(int state)
{
    manager.restreamMIDIIn = (state != 0);
}

void Symphonium::intervalBeginTimeChanged(QTime t)
{
    manager.intInitTime = 60 * t.minute() + t.second();
    intervalEndTime->setMinimumTime(QTime(0,t.minute(),t.second()+1));
}
void Symphonium::intervalEndTimeChanged(QTime t)
{
    manager.intFinalTime = 60 * t.minute() + t.second();
    intervalBeginTime->setMaximumTime(QTime(0,t.minute(),t.second()-1));
}


