#pragma once
#include "engine.h"
#include "grid.h"
#include "guide.h"
#include "highlight.h"
#include "chordguide.h"
#include "melody.h"
#include <QHash>
#include <QMenuBar>
#include <QTabWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDial>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextBrowser>
#include <QWidget>

class MainWindow : public QWidget {
public:
    explicit MainWindow(Engine &engine);

private:
    QWidget *buildTopBar();
    QMenuBar *buildMenu();
    QWidget *buildTrackPanel();
    QWidget *buildGuide();
    QWidget *buildKnobs();
    QWidget *buildSong();
    void refreshAll();
    void refreshTrack();
    void refreshTable();
    void refreshLearn();
    void showStep(int i);
    void songChanged();  // after the guide or a file load replaced parts of the song
    void updateHints(bool switchTab = false);  // outline what the current tutorial step still needs
    void setGuideFont(int pt);
    void showPlayingSection(int section);  // -1: none
    void openMelodyMaker();
    void openChordGuide();

    Engine &eng;
    int lessonIndex = 0;
    bool tutorialOn = true;
    std::vector<GuideStep> steps;
    QWidget *guidePanel;
    int stepIndex = 0;
    bool refreshing = false;
    int playingSection = -1;
    QHash<QString, QList<QWidget *>> named;  // highlight targets for GuideStep::show
    Highlighter *overlay = nullptr;  // null until the window is fully built
    QTabWidget *bottom;
    MelodyMaker *melody = nullptr;  // created on first use
    ChordGuide *chordGuide = nullptr;

    QPushButton *playBtn, *doItBtn, *backBtn, *nextBtn;
    QComboBox *mode, *instrument, *length, *chordHelp;
    QPushButton *barBtns[kMaxBars];
    QComboBox *chords[4];
    QSpinBox *bpm, *swing, *gate;
    QCheckBox *follow;
    QLabel *posLabel, *stepTitle, *stepCount, *noteLabel, *ccLabel, *knobHelp;
    QListWidget *tracks;
    Grid *grid;
    QTextBrowser *guideText;
    QDial *dials[kKnobs + 1];
    QLabel *dialValues[kKnobs + 1];
    QWidget *knobGraphs[kKnobs + 1];
    QPushButton *learnBtns[kKnobs + 1];
    QTableWidget *table;
};
