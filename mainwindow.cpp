#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(Engine &engine) : eng(engine) {
    setWindowTitle("Akai at Home - Studio");
    resize(1250, 820);
    QSettings settings("akai_at_home", "studio");
    lessonIndex = qBound(0, settings.value("lesson", 0).toInt(), int(lessons().size()) - 1);
    tutorialOn = settings.value("tutorial", true).toBool();
    steps = lessons()[lessonIndex].steps();
    if (lessonIndex != 0) eng.song = lessons()[lessonIndex].start(), eng.applyAll();

    grid = new Grid;
    auto *clear = new QPushButton("Clear this track");
    connect(clear, &QPushButton::clicked, this, [this] { eng.song.tracks[eng.selected()].clear(); grid->update(); });
    auto *center = new QWidget;
    auto *cl = new QVBoxLayout(center);
    cl->addWidget(new QLabel("<b>Step grid</b>: 1 bar = 16 steps, shaded blocks are beats. Click a square to turn a step on or off."));
    auto *bars = new QHBoxLayout;
    bars->addWidget(new QLabel("Showing:"));
    for (int b = 0; b < kMaxBars; ++b) {
        barBtns[b] = new QPushButton(QString("Bar %1").arg(b + 1));
        barBtns[b]->setCheckable(true);
        barBtns[b]->setAutoExclusive(true);
        barBtns[b]->setStyleSheet("QPushButton:checked { font-weight: bold; background: palette(highlight); }");
        connect(barBtns[b], &QPushButton::clicked, this, [this, b] { grid->page = b; refreshTrack(); });
        bars->addWidget(barBtns[b]);
    }
    bars->addStretch();
    bars->addWidget(new QLabel("Chord notes:"));
    chordHelp = new QComboBox;
    chordHelp->addItems({"Off", "This bar's chord"});
    for (int c = 0; c < 7; ++c) chordHelp->addItem(QString("%1 · %2").arg(kChordNames[c], kChordNotes[c]));
    chordHelp->setCurrentIndex(1);
    chordHelp->setToolTip("Melody helper: green rows are the notes of this chord. They always sound good together with it.");
    connect(chordHelp, &QComboBox::currentIndexChanged, this, [this] { refreshTrack(); });
    bars->addWidget(chordHelp);
    cl->addLayout(bars);
    cl->addWidget(grid, 1);
    cl->addWidget(clear, 0, Qt::AlignLeft);

    bottom = new QTabWidget;
    bottom->addTab(buildKnobs(), "Knobs (MPK)");
    bottom->addTab(buildSong(), "Song");

    auto *middle = new QSplitter(Qt::Horizontal);
    middle->addWidget(buildTrackPanel());
    middle->addWidget(center);
    middle->addWidget(guidePanel = buildGuide());
    guidePanel->setVisible(tutorialOn);
    middle->setStretchFactor(1, 1);

    auto *vertical = new QSplitter(Qt::Vertical);
    vertical->addWidget(middle);
    vertical->addWidget(bottom);
    vertical->setStretchFactor(0, 1);

    auto *root = new QVBoxLayout(this);
    root->setMenuBar(buildMenu());
    root->addWidget(buildTopBar());
    root->addWidget(vertical, 1);

    connect(&eng, &Engine::position, this, [this](int section, int bar, int step, long songBar) {
        bool onPage = songBar % eng.song.tracks[eng.selected()].bars == grid->page;
        grid->setPlayhead(onPage ? step : -1);
        showPlayingSection(step < 0 ? -1 : section);
        if (step < 0) { playBtn->setText("▶ Play"); posLabel->setText("Stopped"); updateHints(); return; }
        playBtn->setText("■ Stop");  // also when the melody maker started it
        posLabel->setText(section < 0 ? QString("Loop · bar %1/4 · %2").arg(bar + 1).arg(kChordNames[eng.song.chords[bar]])
                                      : QString("%1 · bar %2/%3").arg(eng.song.sections[section].name).arg(bar + 1)
                                            .arg(eng.song.sections[section].bars));
    });
    connect(&eng, &Engine::knobChanged, this, [this](int k, int v) {
        QSignalBlocker b(dials[k]);
        dials[k]->setValue(v);
        knobGraphs[k]->update();
        dialValues[k]->setText(k == kKnobs ? QString("%1 BPM").arg(eng.song.bpm) : QString::number(v));
        if (k == kKnobs) { QSignalBlocker b2(bpm); bpm->setValue(eng.song.bpm); }
        if (refreshing) return;
        knobHelp->setText(kKnobHelp[k]);
        updateHints();
    });
    connect(&eng, &Engine::noteHit, noteLabel, &QLabel::setText);
    connect(&eng, &Engine::ccSeen, this, [this](int cc, int v) {
        ccLabel->setText(QString("Last MPK knob: CC %1 = %2").arg(cc).arg(v));
        refreshLearn();
    });

    named["grid"] = {grid};
    named["tracks"] = {tracks};
    named["instrument"] = {instrument};
    named["gate"] = {gate};
    named["length"] = {length};
    named["follow"] = {follow};
    named["chordhelp"] = {chordHelp};
    named["bars"] = {barBtns[0], barBtns[1], barBtns[2], barBtns[3]};
    named["play"] = {playBtn};
    named["mode"] = {mode};
    named["bpm"] = {bpm};
    named["swing"] = {swing};
    named["chords"] = {chords[0], chords[1], chords[2], chords[3]};
    named["knobs"] = {bottom->widget(0)};
    named["table"] = {table};
    named["note"] = {noteLabel};
    named["doit"] = {doItBtn};
    named["next"] = {nextBtn};
    overlay = new Highlighter(this);
    grid->onToggle = [this] { updateHints(); };

    refreshAll();
    showStep(0);
}

void MainWindow::updateHints(bool switchTab) {
    if (!overlay) return;
    if (!tutorialOn) {
        grid->hints.clear();
        grid->update();
        return overlay->setTargets({});
    }
    const GuideStep &g = steps[stepIndex];
    QList<QWidget *> on;
    for (const QString &n : g.show) on += named.value(n);
    int sel = eng.selected();
    if (g.track != sel) on << tracks;  // "select the X track"
    if (stepIndex > 0 && !eng.playing()) on << playBtn;  // you need Play to hear the loop

    // Whatever "Do it for me" would change, and you haven't done yet, gets outlined.
    Song want = eng.song;
    if (g.doIt) g.doIt(want);
    const Song &have = eng.song;
    grid->hints.clear();
    if (want.tracks.size() == have.tracks.size()) {
        for (size_t t = 0; t < have.tracks.size(); ++t) {
            const Track &w = want.tracks[t], &h = have.tracks[t];
            bool missing = false;
            auto cells = w.steps;
            for (size_t r = 0; r < cells.size() && r < h.steps.size(); ++r)
                for (int c = 0; c < kSteps * kMaxBars; ++c) {
                    cells[r][c] = w.steps[r][c] && !h.steps[r][c];
                    missing |= cells[r][c];
                    if (cells[r][c] && int(t) == sel && c / kSteps != grid->page) on << barBtns[c / kSteps];
                }
            if (w.mute != h.mute || (missing && int(t) != sel)) on << tracks;
            if (int(t) != sel) continue;
            grid->hints = cells;
            for (int k = 0; k < kKnobs; ++k)
                if (std::abs(w.knobs[k] - h.knobs[k]) > 8) on << dials[k];
        }
    }
    if (want.bpm != have.bpm) on << bpm;
    if (want.swing != have.swing) on << swing;
    for (int i = 0; i < 4; ++i)
        if (want.chords[i] != have.chords[i]) on << chords[i];
    bool sectionsDiffer = want.sections.size() != have.sections.size();
    for (size_t i = 0; !sectionsDiffer && i < want.sections.size(); ++i)
        sectionsDiffer = want.sections[i].bars != have.sections[i].bars || want.sections[i].on != have.sections[i].on;
    if (sectionsDiffer) on << table;
    grid->update();

    QList<QPointer<QWidget>> targets;
    for (QWidget *w : on) {
        targets << w;
        for (int i = 0; switchTab && i < bottom->count(); ++i)
            if (bottom->widget(i)->isAncestorOf(w) || bottom->widget(i) == w) bottom->setCurrentIndex(i);
    }
    overlay->setTargets(targets);
}

QWidget *MainWindow::buildTrackPanel() {
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);
    l->addWidget(new QLabel("<b>Tracks</b> (untick to mute)"));
    tracks = new QListWidget;
    connect(tracks, &QListWidget::currentRowChanged, this, [this](int r) {
        if (r < 0 || refreshing) return;
        eng.select(r);
        refreshTrack();
    });
    connect(tracks, &QListWidget::itemChanged, this, [this](QListWidgetItem *it) {
        if (refreshing) return;
        eng.song.tracks[tracks->row(it)].mute = it->checkState() != Qt::Checked;
        updateHints();
    });
    l->addWidget(tracks, 1);

    instrument = new QComboBox;
    for (const auto &[name, id] : eng.presets) instrument->addItem(name, id);
    connect(instrument, &QComboBox::activated, this, [this](int i) {
        Track &t = eng.song.tracks[eng.selected()];
        int id = instrument->itemData(i).toInt();
        t.bank = id / 128, t.program = id % 128;
        eng.applyTrack(eng.selected());
    });
    gate = new QSpinBox;
    gate->setRange(1, kSteps);
    gate->setToolTip("How many steps each note lasts");
    connect(gate, &QSpinBox::valueChanged, this, [this](int v) { if (!refreshing) eng.song.tracks[eng.selected()].gate = v; });
    follow = new QCheckBox("Follow chords");
    follow->setToolTip("Move this track's notes with the chord progression");
    connect(follow, &QCheckBox::toggled, this, [this](bool on) {
        if (refreshing) return;
        eng.song.tracks[eng.selected()].follow = on;
        refreshTrack();
    });

    auto *form = new QFormLayout;
    form->addRow("Instrument", instrument);
    form->addRow("Gate (steps)", gate);
    length = new QComboBox;
    length->addItems({"1 bar", "2 bars", "4 bars"});
    length->setToolTip("How long this track's pattern is before it repeats. Melodies need room: try 4 bars.");
    connect(length, &QComboBox::activated, this, [this](int i) {
        eng.song.tracks[eng.selected()].bars = 1 << i;
        refreshTrack();
    });
    form->addRow("Length", length);
    form->addRow(follow);
    l->addLayout(form);
    w->setMinimumWidth(240);
    return w;
}

QWidget *MainWindow::buildGuide() {
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);
    stepTitle = new QLabel;
    stepTitle->setWordWrap(true);
    stepCount = new QLabel;
    guideText = new QTextBrowser;
    auto *smaller = new QPushButton("A−"), *bigger = new QPushButton("A+");
    smaller->setToolTip("Smaller tutorial text");
    bigger->setToolTip("Bigger tutorial text");
    connect(smaller, &QPushButton::clicked, this, [this] { setGuideFont(guideText->font().pointSize() - 1); });
    connect(bigger, &QPushButton::clicked, this, [this] { setGuideFont(guideText->font().pointSize() + 1); });
    setGuideFont(QSettings("akai_at_home", "studio").value("guideFont", 11).toInt());
    auto *sizeRow = new QHBoxLayout;
    sizeRow->addWidget(stepCount, 1);
    sizeRow->addWidget(smaller);
    sizeRow->addWidget(bigger);
    backBtn = new QPushButton("◀ Back");
    doItBtn = new QPushButton("Do it for me");
    nextBtn = new QPushButton("Next ▶");
    connect(backBtn, &QPushButton::clicked, this, [this] { showStep(stepIndex - 1); });
    connect(nextBtn, &QPushButton::clicked, this, [this] { showStep(stepIndex + 1); });
    connect(doItBtn, &QPushButton::clicked, this, [this] {
        steps[stepIndex].doIt(eng.song);
        songChanged();
        showStep(stepIndex);  // re-select the step's track
    });
    auto *buttons = new QHBoxLayout;
    buttons->addWidget(backBtn);
    buttons->addWidget(doItBtn);
    buttons->addWidget(nextBtn);
    l->addWidget(stepTitle);
    l->addLayout(sizeRow);
    l->addWidget(guideText, 1);
    l->addLayout(buttons);
    w->setMinimumWidth(340);
    return w;
}

void MainWindow::refreshAll() {
    refreshing = true;
    bpm->setValue(eng.song.bpm);
    swing->setValue(eng.song.swing);
    for (int i = 0; i < 4; ++i) chords[i]->setCurrentIndex(eng.song.chords[i]);
    tracks->clear();
    for (const Track &t : eng.song.tracks) {
        auto *it = new QListWidgetItem(t.name, tracks);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
        it->setCheckState(t.mute ? Qt::Unchecked : Qt::Checked);
    }
    tracks->setCurrentRow(eng.selected());
    refreshing = false;
    refreshTable();
    refreshTrack();
}

void MainWindow::refreshTrack() {
    refreshing = true;
    Track &t = eng.song.tracks[eng.selected()];
    if (grid->page >= t.bars) grid->page = 0;
    for (int b = 0; b < kMaxBars; ++b) {
        barBtns[b]->setEnabled(b < t.bars);
        barBtns[b]->setChecked(b == grid->page);
    }
    length->setCurrentIndex(t.bars == 4 ? 2 : t.bars - 1);
    grid->setTrack(&t, rowLabels(eng.song, t, grid->page));
    int chord = chordHelp->currentIndex() == 1 ? eng.song.chords[grid->page % 4] : chordHelp->currentIndex() - 2;
    grid->chordRows.assign(t.rows(), false);
    for (int r = 0; r < t.rows() && chord >= 0 && !t.drum; ++r)
        grid->chordRows[r] = inChord(chord, noteFor(eng.song, t, r, grid->page));
    chordHelp->setEnabled(!t.drum);
    instrument->setCurrentIndex(instrument->findData(t.bank * 128 + t.program));
    gate->setValue(t.gate);
    follow->setChecked(t.follow);
    follow->setEnabled(!t.drum);
    for (int k = 0; k <= kKnobs; ++k) emit eng.knobChanged(k, eng.knob(k));
    refreshing = false;
    updateHints();
}

void MainWindow::showStep(int i) {
    stepIndex = qBound(0, i, int(steps.size()) - 1);
    const GuideStep &s = steps[stepIndex];
    stepTitle->setText(s.title);
    stepCount->setText(QString("Part %1 of %2").arg(stepIndex + 1).arg(steps.size()));
    guideText->setHtml(s.html);
    doItBtn->setEnabled(bool(s.doIt));
    backBtn->setEnabled(stepIndex > 0);
    nextBtn->setEnabled(stepIndex + 1 < int(steps.size()));
    tracks->setCurrentRow(s.track);
    updateHints(true);
}

void MainWindow::songChanged() {
    eng.applyAll();
    eng.select(qBound(0, eng.selected(), int(eng.song.tracks.size()) - 1));
    refreshAll();
    if (melody) melody->reload();
}

void MainWindow::openChordGuide() {
    if (!chordGuide) chordGuide = new ChordGuide(eng, [this] { songChanged(); }, this);
    chordGuide->show();
    chordGuide->raise();
    chordGuide->activateWindow();
}

void MainWindow::openMelodyMaker() {
    if (!melody) melody = new MelodyMaker(eng, [this] { songChanged(); }, this);
    melody->show();
    melody->raise();
    melody->activateWindow();
}
