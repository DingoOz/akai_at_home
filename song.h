// The song data: tracks of 16-step patterns, a 4-chord progression, and an arrangement of sections.
#pragma once
#include <QString>
#include <QStringList>
#include <array>
#include <functional>
#include <vector>

constexpr int kSteps = 16;     // steps per bar
constexpr int kMaxBars = 4;    // longest pattern
constexpr int kScaleRows = 8;  // melodic tracks: one octave of A minor, bottom to top
constexpr int kKnobs = 7;      // per-track sound settings, see kKnobNames
extern const char *kKnobNames[kKnobs + 1];  // +1: the global tempo knob
extern const int kKnobCC[kKnobs];           // MIDI CC each knob sends to the synth
extern const char *kChordNames[7];          // diatonic chords of A minor

struct Track {
    QString name;
    int channel = 0, bank = 0, program = 0;
    bool drum = false;               // drum: each row is a fixed drum sound; melodic: rows are scale notes
    int base = 57;                   // melodic: MIDI note of the bottom row (an A)
    QStringList drumNames;           // drum: row labels, bottom to top
    std::vector<int> drumNotes;      // drum: MIDI note per row
    int gate = 1;                    // how many steps each note lasts
    int bars = 1;                    // pattern length: 1, 2 or 4 bars
    bool follow = true;              // melodic: move with the chord progression
    bool mute = false;
    int velocity = 100;
    std::array<int, kKnobs> knobs{127, 0, 0, 0, 40, 0, 100};
    std::vector<std::array<bool, kSteps * kMaxBars>> steps;  // [row][bar * 16 + step]

    int rows() const { return drum ? int(drumNotes.size()) : kScaleRows; }
    void clear() { steps.assign(rows(), {}); }
    void set(int row, std::initializer_list<int> stepList) { for (int s : stepList) steps.at(row).at(s) = true; }
};

struct Section {
    QString name;
    int bars = 4;
    std::vector<bool> on;  // per track
};

struct Song {
    int bpm = 138;
    int swing = 0;  // % of a step that every second 16th is delayed: 0 = straight, 30+ = lazy groove
    std::array<int, 4> chords{0, 5, 2, 6};  // one chord per bar, index into kChordNames
    std::vector<Track> tracks;
    std::vector<Section> sections;
};

struct StepInfo { int section = -1, bar = 0, step = 0; long songBar = 0; };  // bar: within section (or loop)

QString noteName(int key);
int noteFor(const Song &song, const Track &t, int row, int bar);
int rowFor(const Song &song, const Track &t, int key, int bar);  // melodic row closest to a played key (same note name first)
std::array<int, 3> chordKeys(int chord);  // MIDI notes of a chord (index into kChordNames), root between C4 and B4
bool inChord(int chord, int key);  // is MIDI note `key` one of the 3 notes of chord (index into kChordNames)?
QStringList rowLabels(const Song &song, const Track &t, int bar);
Song defaultSong();  // the trance lesson's tracks and arrangement, patterns empty
// Track builders for lesson starter songs. Programs are General MIDI numbers counted from 0.
Track drumTrack(QString name, int ch, QStringList names, std::vector<int> notes, int volume);
Track synthTrack(QString name, int ch, int program, int base, int gate, int reverb, int volume);

// Calls note(track, key, velocity, gateSteps) for every note that starts at global step `pos`.
// songMode follows the arrangement, otherwise loops 4 bars of all unmuted tracks.
// Returns false once pos is past the end of the song.
bool songStep(const Song &song, long pos, bool songMode, StepInfo &info,
              const std::function<void(const Track &, int, int, int)> &note);

// The notes of one track that start at global step `pos` (part of songStep).
void trackStep(const Song &song, const Track &t, long pos, const std::function<void(const Track &, int, int, int)> &note);

bool saveSong(const Song &song, const QString &path);
bool loadSong(Song &song, const QString &path);
