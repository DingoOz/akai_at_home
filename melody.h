// Melody maker: a scratch melody of its own (not part of the song) that loops over the song.
// Play the MPK keys or click the on-screen keys while recording: every note snaps to the nearest
// 16th step. Import a track's notes to start from, and add the result to the song as a new track.
#pragma once
#include "engine.h"
#include "grid.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <functional>

class MelodyMaker : public QDialog {
public:
    MelodyMaker(Engine &engine, std::function<void()> songChanged, QWidget *parent);
    void reload();  // the song changed: refresh the track list and chord labels

private:
    static constexpr int kChannel = 15;  // MIDI channel of the melody, never used by song tracks
    QWidget *buildControls();
    void capture(int key, unsigned tick);
    void importTrack();
    void addToSong();
    void refresh();
    void keep() { undo = mel; }  // snapshot for Undo
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;
    void changeEvent(QEvent *) override;

    Engine &eng;
    std::function<void()> songChanged;
    Track mel, undo;
    long lastStep = -1;  // replace mode: notes played on the same step add up into a chord

    QComboBox *instrument, *octave, *length, *chordHelp, *backing, *source;
    QCheckBox *click, *replace;
    QPushButton *loopBtn, *recBtn, *barBtns[kMaxBars], *keys[kScaleRows];
    QSpinBox *offset;
    QLabel *status;
    Grid *grid;
};
