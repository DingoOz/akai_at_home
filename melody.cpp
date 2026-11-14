#include "melody.h"
#include "guide.h"  // kChordNotes
#include <QEvent>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <set>

MelodyMaker::MelodyMaker(Engine &engine, std::function<void()> onSongChanged, QWidget *parent)
    : QDialog(parent), eng(engine), songChanged(std::move(onSongChanged)),
      mel(synthTrack("Melody", kChannel, 4, 57, 2, 60, 100)) {  // Electric Piano, from A3
    mel.follow = false, mel.bars = 4;
    undo = mel;
    setWindowTitle("Melody maker");
    resize(820, 680);
    auto *help = new QLabel(
        "A scratch pad for melodies, separate from your tracks. <b>1.</b> Press <b>Loop</b>. <b>2.</b> Press "
        "<b>● Record</b> and play the MPK keys or click the keys below: every note snaps to the nearest 16th step and "
        "plays back next time round. Play over notes to replace them, or click squares in the grid. "
        "<b>3.</b> Happy? <b>Add to song</b>.<br><i>Black keys snap to the nearest white key (A minor).</i>");
    help->setWordWrap(true);

    loopBtn = new QPushButton("▶ Loop");
    recBtn = new QPushButton("● Record");
    recBtn->setCheckable(true);
    recBtn->setStyleSheet("QPushButton:checked { background: #c0392b; color: white; font-weight: bold; }");
    auto *undoBtn = new QPushButton("Undo"), *clearBtn = new QPushButton("Clear melody");
    undoBtn->setToolTip("Back to how the melody was before the last Record, Clear or Import");
    auto *buttons = new QHBoxLayout;
    for (auto *b : {loopBtn, recBtn, undoBtn, clearBtn}) b->setMinimumHeight(36), buttons->addWidget(b);
    status = new QLabel("Stopped");

    auto *bars = new QHBoxLayout;
    bars->addWidget(new QLabel("Showing:"));
    for (int b = 0; b < kMaxBars; ++b) {
        barBtns[b] = new QPushButton(QString("Bar %1").arg(b + 1));
        barBtns[b]->setCheckable(true);
        barBtns[b]->setStyleSheet("QPushButton:checked { font-weight: bold; background: palette(highlight); }");
        connect(barBtns[b], &QPushButton::clicked, this, [this, b] { grid->page = b, refresh(); });
        bars->addWidget(barBtns[b]);
    }
    bars->addStretch();
    grid = new Grid;
    grid->onToggle = [] {};

    auto *keyRow = new QHBoxLayout;
    keyRow->addSpacing(80);  // line up with the grid rows' labels
    for (int r = 0; r < kScaleRows; ++r) {
        keys[r] = new QPushButton;
        keys[r]->setMinimumHeight(60);
        keys[r]->setToolTip("Click to play this note (and record it while Record is on)");
        connect(keys[r], &QPushButton::pressed, this, [this, r] { eng.liveNote(noteFor(eng.song, mel, r, 0), true); });
        connect(keys[r], &QPushButton::released, this, [this, r] { eng.liveNote(noteFor(eng.song, mel, r, 0), false); });
        keyRow->addWidget(keys[r]);
    }

    auto *l = new QVBoxLayout(this);
    l->addWidget(help);
    l->addWidget(buildControls());
    l->addLayout(buttons);
    l->addWidget(status);
    l->addLayout(bars);
    l->addWidget(grid, 1);
    l->addWidget(new QLabel("Keys (low to high), green = notes of the chord:"));
    l->addLayout(keyRow);

    connect(loopBtn, &QPushButton::clicked, this, [this] { eng.playing() ? eng.stop() : eng.play(false); });
    connect(recBtn, &QPushButton::toggled, this, [this](bool on) {
        if (!on) return;
        keep(), lastStep = -1;
        QSignalBlocker quiet(&eng);  // play() first sends "stopped", which would switch Record straight off again
        if (!eng.playing()) eng.play(false);
    });
    connect(undoBtn, &QPushButton::clicked, this, [this] { std::swap(mel, undo), eng.setup(mel), refresh(); });
    connect(clearBtn, &QPushButton::clicked, this, [this] { keep(), mel.clear(), refresh(); });
    connect(&eng, &Engine::keyPressed, this, &MelodyMaker::capture);
    connect(&eng, &Engine::position, this, [this](int, int, int step, long songBar) {
        loopBtn->setText(step < 0 ? "▶ Loop" : "■ Stop");
        if (step < 0) recBtn->setChecked(false);
        if (!isVisible()) return;
        if (step >= 0 && grid->page != int(songBar % mel.bars)) grid->page = int(songBar % mel.bars), refresh();
        grid->setPlayhead(step);
        status->setText(step < 0 ? "Stopped"
                                 : QString("Bar %1 of %2 · %3%4").arg(grid->page + 1).arg(mel.bars)
                                       .arg(kChordNames[eng.song.chords[songBar % 4]])
                                       .arg(recBtn->isChecked() ? " · <b style='color:#e74c3c'>RECORDING</b>" : ""));
    });
    reload();
}

QWidget *MelodyMaker::buildControls() {
    instrument = new QComboBox;
    for (const auto &[name, id] : eng.presets)
        if (id < 128 * 128) instrument->addItem(name, id);
    instrument->setCurrentIndex(instrument->findData(mel.bank * 128 + mel.program));
    connect(instrument, &QComboBox::activated, this, [this] {
        int id = instrument->currentData().toInt();
        mel.bank = id / 128, mel.program = id % 128;
        eng.setup(mel);
    });
    octave = new QComboBox;
    for (int base : {33, 45, 57, 69}) octave->addItem(QString("From %1").arg(noteName(base)), base);
    octave->setToolTip("Which octave the 8 rows cover. Moving it moves the whole melody up or down.");
    connect(octave, &QComboBox::activated, this, [this] { mel.base = octave->currentData().toInt(), refresh(); });
    length = new QComboBox;
    length->addItems({"1 bar", "2 bars", "4 bars"});
    length->setToolTip("How long the melody is before it repeats. 4 bars = once round the 4 chords.");
    connect(length, &QComboBox::activated, this, [this](int i) { mel.bars = 1 << i, refresh(); });
    chordHelp = new QComboBox;
    chordHelp->addItem("Off");
    chordHelp->addItem("This bar's chord");
    for (int c = 0; c < 7; ++c) chordHelp->addItem(QString("%1 · %2").arg(kChordNames[c], kChordNotes[c]));
    chordHelp->setCurrentIndex(1);
    chordHelp->setToolTip("Shows the notes of a chord in green. Notes from the chord always sound good over it.");
    connect(chordHelp, &QComboBox::currentIndexChanged, this, [this] { refresh(); });

    backing = new QComboBox;
    backing->addItems({"All tracks", "Drums only", "Nothing (just the melody)"});
    backing->setToolTip("Which song tracks play along. Only changes what you hear here, not the song.");
    connect(backing, &QComboBox::currentIndexChanged, this, [this](int i) { eng.backing = Engine::Backing(i); });
    click = new QCheckBox("Metronome click");
    connect(click, &QCheckBox::toggled, this, [this](bool on) { eng.click = on; });
    replace = new QCheckBox("Playing over old notes replaces them");
    replace->setChecked(true);
    replace->setToolTip("On: a note you play wipes whatever was on that step. Off: notes add up (overdub).");
    offset = new QSpinBox;
    offset->setRange(0, 400);
    offset->setSuffix(" ms");
    offset->setValue(QSettings("akai_at_home", "studio").value("melodyOffset", 0).toInt());
    offset->setToolTip("If notes keep landing a step late (Bluetooth headphones add delay), raise this. "
                       "Try 150-250 ms for Bluetooth, 0 for wired speakers.");
    connect(offset, &QSpinBox::valueChanged, this, [](int v) { QSettings("akai_at_home", "studio").setValue("melodyOffset", v); });

    source = new QComboBox;
    auto *importBtn = new QPushButton("Import");
    importBtn->setToolTip("Copy that track's notes and sound into the melody maker. The track itself is not changed.");
    auto *addBtn = new QPushButton("Add to song as a new track");
    connect(importBtn, &QPushButton::clicked, this, &MelodyMaker::importTrack);
    connect(addBtn, &QPushButton::clicked, this, &MelodyMaker::addToSong);
    auto *importRow = new QHBoxLayout;
    importRow->addWidget(source, 1);
    importRow->addWidget(importBtn);

    auto *w = new QWidget;
    auto *cols = new QHBoxLayout(w);
    cols->setContentsMargins(0, 0, 0, 0);
    auto *left = new QFormLayout, *right = new QFormLayout;
    left->addRow("Instrument", instrument);
    left->addRow("Octave", octave);
    left->addRow("Length", length);
    left->addRow("Chord notes", chordHelp);
    left->addRow("Start from track", importRow);
    right->addRow("Play along with", backing);
    right->addRow("", click);
    right->addRow("", replace);
    right->addRow("Timing fix", offset);
    right->addRow("", addBtn);
    cols->addLayout(left, 1);
    cols->addSpacing(16);
    cols->addLayout(right, 1);
    return w;
}

void MelodyMaker::capture(int key, unsigned tick) {
    if (!recBtn->isChecked() || !eng.playing()) return;
    long step = std::lround(eng.stepAt(tick - std::min<unsigned>(tick, offset->value())));  // snap to the nearest 16th
    if (step < 0) return;
    int s = int(step % (kSteps * mel.bars));
    if (replace->isChecked() && step != lastStep)
        for (auto &row : mel.steps) row[s] = false;
    lastStep = step;
    mel.steps[rowFor(eng.song, mel, key, 0)][s] = true;
    grid->update();
}

// Copies the notes as they sound: a track that follows the chords becomes 4 bars of fixed notes.
void MelodyMaker::importTrack() {
    if (source->currentIndex() < 0) return;
    const Track &t = eng.song.tracks[source->currentData().toInt()];
    keep();
    mel.bank = t.bank, mel.program = t.program, mel.gate = t.gate, mel.knobs = t.knobs, mel.velocity = t.velocity;
    mel.base = t.base, mel.bars = t.follow ? 4 : t.bars;
    mel.clear();
    for (int s = 0; s < kSteps * mel.bars; ++s)
        for (int r = 0; r < t.rows(); ++r)
            if (t.steps[r][s % (kSteps * t.bars)])
                mel.steps[rowFor(eng.song, mel, noteFor(eng.song, t, r, s / kSteps), 0)][s] = true;
    eng.setup(mel);
    instrument->setCurrentIndex(instrument->findData(mel.bank * 128 + mel.program));
    if (octave->findData(mel.base) < 0) octave->addItem(QString("From %1").arg(noteName(mel.base)), mel.base);
    refresh();
}

void MelodyMaker::addToSong() {
    std::set<int> used{9, 14, kChannel};  // 9: drums, 14: chord explainer piano
    for (const Track &t : eng.song.tracks) used.insert(t.channel);
    int ch = 0;
    while (used.count(ch)) ++ch;
    if (ch >= 16) return void(QMessageBox::warning(this, "Melody maker", "The song already uses every MIDI channel."));
    Track t = mel;
    t.channel = ch;
    int n = 1;
    for (const Track &o : eng.song.tracks) n += o.name.startsWith("Melody");
    t.name = n == 1 ? "Melody" : QString("Melody %1").arg(n);
    eng.song.tracks.push_back(t);
    for (Section &sec : eng.song.sections) sec.on.push_back(true);
    eng.select(int(eng.song.tracks.size()) - 1);
    songChanged();
    QMessageBox::information(this, "Melody maker", QString("Added <b>%1</b> to the song. It plays in every section: "
        "switch it off in the <b>Song</b> tab where you don't want it.").arg(t.name));
}

void MelodyMaker::refresh() {
    if (grid->page >= mel.bars) grid->page = 0;
    for (int b = 0; b < kMaxBars; ++b) barBtns[b]->setEnabled(b < mel.bars), barBtns[b]->setChecked(b == grid->page);
    length->setCurrentIndex(mel.bars == 4 ? 2 : mel.bars - 1);
    octave->setCurrentIndex(octave->findData(mel.base));
    int chord = chordHelp->currentIndex() == 1 ? eng.song.chords[grid->page % 4] : chordHelp->currentIndex() - 2;
    grid->chordRows.assign(kScaleRows, false);
    for (int r = 0; r < kScaleRows; ++r) {
        int key = noteFor(eng.song, mel, r, 0);
        bool in = chord >= 0 && inChord(chord, key);
        grid->chordRows[r] = in;
        keys[r]->setText(noteName(key));
        keys[r]->setStyleSheet(in ? "background: #3c7a4f; color: white; font-weight: bold;" : "");
    }
    grid->setTrack(&mel, rowLabels(eng.song, mel, 0));
}

void MelodyMaker::reload() {
    source->clear();
    for (int i = 0; i < int(eng.song.tracks.size()); ++i)
        if (!eng.song.tracks[i].drum) source->addItem(eng.song.tracks[i].name, i);
    if (isActiveWindow()) eng.playLiveOn(kChannel);
    refresh();
}

void MelodyMaker::showEvent(QShowEvent *) {
    eng.extra = &mel;
    eng.setup(mel);
    reload();
}

// The MPK plays the melody while this window is in front, and the selected track otherwise.
void MelodyMaker::changeEvent(QEvent *e) {
    if (e->type() == QEvent::ActivationChange)
        isActiveWindow() ? eng.playLiveOn(kChannel) : eng.select(eng.selected());
    QDialog::changeEvent(e);
}

void MelodyMaker::hideEvent(QHideEvent *) {  // closed by the X button or Escape
    recBtn->setChecked(false);
    click->setChecked(false);
    backing->setCurrentIndex(0);  // the main window's Play should hear every track again
    eng.extra = nullptr;
    eng.select(eng.selected());
}
