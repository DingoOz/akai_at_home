// Main window top bar (transport, tempo, chords, files) and the menu bar.
#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QActionGroup>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>

QWidget *MainWindow::buildTopBar() {
    auto *w = new QWidget;
    auto *l = new QHBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);

    playBtn = new QPushButton("▶ Play");
    connect(playBtn, &QPushButton::clicked, this, [this] {
        if (eng.playing()) return eng.stop();
        eng.play(mode->currentIndex() == 1);
        if (mode->currentIndex() == 1) bottom->setCurrentWidget(table->parentWidget());  // watch the sections go by
        playBtn->setText("■ Stop");
        updateHints();
    });
    mode = new QComboBox;
    mode->addItems({"Loop", "Song"});
    mode->setToolTip("Loop repeats 4 bars of every unmuted track. Song plays the sections in the Song tab.");

    bpm = new QSpinBox;
    bpm->setRange(60, 180);
    connect(bpm, &QSpinBox::valueChanged, this, [this](int v) {
        eng.song.bpm = v;
        QSignalBlocker b(dials[kKnobs]);
        dials[kKnobs]->setValue(eng.knob(kKnobs));
        dialValues[kKnobs]->setText(QString("%1 BPM").arg(v));
        knobGraphs[kKnobs]->update();
        updateHints();
    });

    l->addWidget(playBtn);
    l->addWidget(mode);
    l->addWidget(new QLabel("BPM"));
    l->addWidget(bpm);
    swing = new QSpinBox;
    swing->setRange(0, 60);
    swing->setSuffix("%");
    swing->setToolTip("Swing: delays every second 16th note for a lazy, bouncy groove. 0 = straight (trance, techno).");
    connect(swing, &QSpinBox::valueChanged, this, [this](int v) { if (!refreshing) eng.song.swing = v, updateHints(); });
    l->addWidget(new QLabel("Swing"));
    l->addWidget(swing);
    l->addSpacing(16);
    l->addWidget(new QLabel("Chords:"));
    for (int i = 0; i < 4; ++i) {
        chords[i] = new QComboBox;
        for (int c = 0; c < 7; ++c) chords[i]->addItem(QString("%1 · %2").arg(kChordNames[c], kChordNotes[c]));
        chords[i]->setToolTip(QString("Chord for bar %1").arg(i + 1));
        connect(chords[i], &QComboBox::currentIndexChanged, this, [this, i](int c) {
            if (refreshing) return;
            eng.song.chords[i] = c;
            refreshTrack();  // note labels follow bar 1's chord
        });
        l->addWidget(chords[i]);
    }
    auto *chordHelpBtn = new QPushButton("?");
    chordHelpBtn->setFixedWidth(28);
    chordHelpBtn->setToolTip("How chords and chord progressions work, with a piano keyboard");
    connect(chordHelpBtn, &QPushButton::clicked, this, &MainWindow::openChordGuide);
    l->addWidget(chordHelpBtn);
    l->addSpacing(16);
    posLabel = new QLabel("Stopped");
    posLabel->setMinimumWidth(170);
    l->addWidget(posLabel);
    l->addStretch();

    auto *save = new QPushButton("Save…"), *load = new QPushButton("Open…"), *wav = new QPushButton("Export audio…");
    auto *midi = new QPushButton("Reconnect MPK"), *maker = new QPushButton("Melody maker…");
    maker->setToolTip("Loop the song and record a melody from the MPK keys, snapped to the beat");
    connect(maker, &QPushButton::clicked, this, &MainWindow::openMelodyMaker);
    l->addWidget(maker);
    named["save"] = {save}, named["export"] = {wav};
    midi->setToolTip("Click after plugging the MPK in, or if a note gets stuck");
    connect(save, &QPushButton::clicked, this, [this] {
        QString p = QFileDialog::getSaveFileName(this, "Save song", "my_track.json", "Songs (*.json)");
        if (!p.isEmpty() && !saveSong(eng.song, p)) QMessageBox::warning(this, "Save", "Could not save " + p);
    });
    connect(load, &QPushButton::clicked, this, [this] {
        QString p = QFileDialog::getOpenFileName(this, "Open song", {}, "Songs (*.json)");
        if (p.isEmpty()) return;
        Song s;
        if (!loadSong(s, p)) return void(QMessageBox::warning(this, "Open", "Not a song file: " + p));
        eng.stop();
        eng.song = s;
        songChanged();
    });
    connect(wav, &QPushButton::clicked, this, [this] {
        QString filter;
        QString p = QFileDialog::getSaveFileName(this, "Export the song as audio", "my_track.mp3",
            "MP3 (*.mp3);;WAV, uncompressed (*.wav);;Ogg Vorbis (*.ogg);;FLAC, lossless (*.flac);;AIFF (*.aiff)", &filter);
        if (p.isEmpty()) return;
        if (QFileInfo(p).suffix().isEmpty()) p += filter.section("*", 1, 1).chopped(1);  // "MP3 (*.mp3)" -> ".mp3"
        QString why;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        bool ok = eng.exportAudio(p, why);
        QApplication::restoreOverrideCursor();
        ok ? QMessageBox::information(this, "Export", "Saved " + p) : QMessageBox::warning(this, "Export", why);
    });
    connect(midi, &QPushButton::clicked, this, [this] { eng.reconnectMidi(); });
    for (auto *b : {save, load, wav, midi}) l->addWidget(b);
    return w;
}

QMenuBar *MainWindow::buildMenu() {
    auto *bar = new QMenuBar;
    QMenu *m = bar->addMenu("&Tutorial");
    QAction *show = m->addAction("Show tutorial");
    show->setCheckable(true);
    show->setChecked(tutorialOn);
    connect(show, &QAction::toggled, this, [this](bool on) {
        tutorialOn = on;
        guidePanel->setVisible(on);
        QSettings("akai_at_home", "studio").setValue("tutorial", on);
        updateHints(on);
    });
    m->addSeparator();
    m->addSection("Lessons: each starts a new song");
    auto *group = new QActionGroup(m);
    const auto &all = lessons();
    for (int i = 0; i < int(all.size()); ++i) {
        QAction *a = m->addAction(all[i].name);
        a->setToolTip(all[i].blurb);
        a->setCheckable(true);
        a->setChecked(i == lessonIndex);
        group->addAction(a);
        connect(a, &QAction::triggered, this, [this, i, show, group] {
            const Lesson &l = lessons()[i];
            auto answer = QMessageBox::question(this, l.name,
                QString("<b>%1</b><br>%2<br><br>Start this lesson? It replaces your current song with the lesson's "
                        "starting song, so <b>Save</b> first if you want to keep what you've made.").arg(l.name, l.blurb));
            if (answer != QMessageBox::Yes) return void(group->actions()[lessonIndex]->setChecked(true));
            eng.stop();
            lessonIndex = i;
            eng.song = l.start();
            steps = l.steps();
            songChanged();
            show->setChecked(true);
            showStep(0);
            QSettings("akai_at_home", "studio").setValue("lesson", i);
        });
    }
    m->setToolTipsVisible(true);
    bar->addMenu("T&ools")->addAction("Melody maker…", this, &MainWindow::openMelodyMaker);
    bar->addMenu("&Help")->addAction("How chords work…", this, &MainWindow::openChordGuide);
    return bar;
}
