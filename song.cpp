#include "song.h"
#include <algorithm>
#include <cstdlib>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

const char *kKnobNames[kKnobs + 1] = {"Filter", "Resonance", "Attack", "Release", "Reverb", "Chorus", "Volume", "Tempo"};
const int kKnobCC[kKnobs] = {74, 71, 73, 72, 91, 93, 7};
const char *kChordNames[7] = {"Am", "Bdim", "C", "Dm", "Em", "F", "G"};
static const int kChordShift[7] = {0, 1, 2, 3, 4, -2, -1};  // scale steps from A; F and G sit below A
static const int kMinor[7] = {0, 2, 3, 5, 7, 8, 10};         // A natural minor = the white keys from A

QString noteName(int key) {
    static const char *names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return QString("%1%2").arg(names[key % 12]).arg(key / 12 - 1);
}

int noteFor(const Song &song, const Track &t, int row, int bar) {
    if (t.drum) return t.drumNotes[row];
    int degree = row + (t.follow ? kChordShift[song.chords[bar % 4]] : 0);
    int octave = degree >= 0 ? degree / 7 : (degree - 6) / 7;
    return t.base + 12 * octave + kMinor[degree - 7 * octave];
}

int rowFor(const Song &song, const Track &t, int key, int bar) {
    int best = 0, bestScore = 1 << 30;
    for (int r = 0; r < t.rows(); ++r) {
        int d = std::abs(noteFor(song, t, r, bar) - key), pc = std::min(d % 12, 12 - d % 12);
        int score = pc * 1000 + d;  // a C anywhere on the keyboard lands on the C row; sharps snap to a neighbour
        if (score < bestScore) best = r, bestScore = score;
    }
    return best;
}

std::array<int, 3> chordKeys(int chord) {
    std::array<int, 3> k;
    for (int i = 0; i < 3; ++i) {
        int d = chord + 2 * i;  // root, 3rd, 5th: every other white key
        k[i] = 69 + 12 * (d / 7) + kMinor[d % 7];  // from A4
    }
    if (k[0] >= 72) for (int &n : k) n -= 12;  // every root lands between C4 and B4
    return k;
}

bool inChord(int chord, int key) {
    for (int i = 0; i < 3; ++i)  // a chord stacks every other scale note: root, 3rd, 5th
        if ((9 + kMinor[(chord + 2 * i) % 7]) % 12 == key % 12) return true;  // 9 = A
    return false;
}

QStringList rowLabels(const Song &song, const Track &t, int bar) {
    if (t.drum) return t.drumNames;
    QStringList l;
    for (int r = 0; r < t.rows(); ++r) l << noteName(noteFor(song, t, r, bar));
    return l;
}

Track drumTrack(QString name, int ch, QStringList names, std::vector<int> notes, int volume) {
    Track t;
    t.name = name, t.channel = ch, t.bank = 128, t.drum = true, t.drumNames = names, t.drumNotes = notes;
    t.knobs[4] = 15, t.knobs[6] = volume;
    t.clear();
    return t;
}

Track synthTrack(QString name, int ch, int program, int base, int gate, int reverb, int volume) {
    Track t;
    t.name = name, t.channel = ch, t.program = program, t.base = base, t.gate = gate;
    t.knobs[4] = reverb, t.knobs[6] = volume;
    t.clear();
    return t;
}

Song defaultSong() {
    Song s;
    s.bpm = 132;
    // Programs are General MIDI numbers counted from 0. Volumes balance the mix: GM leads are much louder than drums.
    // Drums are split into separate tracks so the arrangement can bring them in one at a time.
    s.tracks = {
        drumTrack("Kick", 9, {"Kick", "Crash"}, {36, 49}, 127),
        drumTrack("Clap", 10, {"Clap", "Snare"}, {39, 38}, 100),
        drumTrack("Hats", 11, {"Closed hat", "Open hat"}, {42, 46}, 85),
        drumTrack("Snare roll", 12, {"Snare"}, {38}, 80),
        synthTrack("Bass", 0, 38, 33, 1, 0, 100),   // Synth Bass 1, from A1
        synthTrack("Pad", 1, 89, 57, 16, 100, 72),  // Warm Pad, from A3
        synthTrack("Arp", 2, 98, 69, 1, 70, 50),    // Crystal, from A4
        synthTrack("Lead", 3, 88, 69, 6, 110, 60),  // New Age (soft bell-pad), from A4
    };
    s.tracks[3].bars = 4;  // the roll speeds up over 4 bars
    Track &lead = s.tracks[7];
    lead.bars = 4, lead.follow = false;  // a real 4-bar melody, written against the chords
    lead.knobs = {85, 0, 15, 70, 110, 50, 60};
    //                 Kick   Clap   Hats   Roll   Bass   Pad    Arp    Lead
    s.sections = {
        {"Intro", 8, {false, false, false, false, false, true, true, false}},
        {"Groove", 8, {true, false, false, false, true, true, true, false}},
        {"Groove 2", 8, {true, true, true, false, true, true, true, false}},
        {"Breakdown", 8, {false, false, false, false, false, true, true, true}},
        {"Build-up", 4, {false, false, false, true, false, true, true, true}},
        {"Drop", 16, {true, true, true, false, true, true, true, true}},
        {"Outro", 8, {true, false, false, false, false, true, true, false}},
    };
    return s;
}

bool songStep(const Song &song, long pos, bool songMode, StepInfo &info,
              const std::function<void(const Track &, int, int, int)> &note) {
    long bar = pos / kSteps;
    info = {-1, int(bar % 4), int(pos % kSteps), bar};
    if (songMode) {
        long start = 0;
        for (info.section = 0; info.section < int(song.sections.size()); ++info.section) {
            if (bar < start + song.sections[info.section].bars) break;
            start += song.sections[info.section].bars;
        }
        if (info.section == int(song.sections.size())) return false;
        info.bar = int(bar - start);
    }
    for (size_t i = 0; i < song.tracks.size(); ++i) {
        const Track &t = song.tracks[i];
        if (t.mute || (songMode && !song.sections[info.section].on[i])) continue;
        trackStep(song, t, pos, note);
    }
    return true;
}

void trackStep(const Song &song, const Track &t, long pos, const std::function<void(const Track &, int, int, int)> &note) {
    int s = int(pos % (kSteps * t.bars));
    for (int r = 0; r < t.rows(); ++r)
        if (t.steps[r][s]) note(t, noteFor(song, t, r, int(pos / kSteps)), t.velocity, t.gate);
}

bool saveSong(const Song &song, const QString &path) {
    QJsonArray tracks, sections, chords;
    for (int c : song.chords) chords << c;
    for (const Track &t : song.tracks) {
        QJsonArray knobs, steps, notes;
        for (int k : t.knobs) knobs << k;
        for (int n : t.drumNotes) notes << n;
        for (const auto &row : t.steps) {
            QString s;
            for (bool b : row) s += b ? 'x' : '.';
            steps << s;
        }
        tracks << QJsonObject{{"name", t.name}, {"channel", t.channel}, {"bank", t.bank}, {"program", t.program},
                              {"drum", t.drum}, {"base", t.base}, {"drumNames", QJsonArray::fromStringList(t.drumNames)},
                              {"drumNotes", notes}, {"gate", t.gate}, {"bars", t.bars}, {"follow", t.follow}, {"mute", t.mute},
                              {"velocity", t.velocity}, {"knobs", knobs}, {"steps", steps}};
    }
    for (const Section &sec : song.sections) {
        QJsonArray on;
        for (bool b : sec.on) on << b;
        sections << QJsonObject{{"name", sec.name}, {"bars", sec.bars}, {"on", on}};
    }
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    QJsonObject root{{"bpm", song.bpm}, {"swing", song.swing}, {"chords", chords}, {"tracks", tracks}, {"sections", sections}};
    return f.write(QJsonDocument(root).toJson()) > 0;
}

bool loadSong(Song &song, const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
    if (!root["tracks"].isArray() || root["tracks"].toArray().isEmpty()) return false;
    Song s;
    s.bpm = qBound(60, root["bpm"].toInt(138), 180);
    s.swing = qBound(0, root["swing"].toInt(0), 60);
    for (int i = 0; i < 4; ++i) s.chords[i] = qBound(0, root["chords"][i].toInt(), 6);
    for (const auto &v : root["tracks"].toArray()) {
        const QJsonObject o = v.toObject();
        Track t;
        t.name = o["name"].toString(), t.channel = qBound(0, o["channel"].toInt(), 15);
        t.bank = o["bank"].toInt(), t.program = qBound(0, o["program"].toInt(), 127);
        t.drum = o["drum"].toBool(), t.base = qBound(0, o["base"].toInt(57), 108);
        for (const auto &n : o["drumNotes"].toArray()) t.drumNotes.push_back(qBound(0, n.toInt(), 127));
        for (const auto &n : o["drumNames"].toArray()) t.drumNames << n.toString();
        while (t.drumNames.size() < int(t.drumNotes.size())) t.drumNames << "?";
        if (t.drum && t.drumNotes.empty()) continue;
        t.gate = qBound(1, o["gate"].toInt(1), kSteps), t.follow = o["follow"].toBool(true);
        t.mute = o["mute"].toBool(), t.velocity = qBound(1, o["velocity"].toInt(100), 127);
        t.bars = qBound(1, o["bars"].toInt(1), kMaxBars);
        if (t.bars == 3) t.bars = 4;  // must divide the 4-bar chord loop
        for (int k = 0; k < kKnobs; ++k) t.knobs[k] = qBound(0, o["knobs"][k].toInt(t.knobs[k]), 127);
        t.clear();
        QJsonArray steps = o["steps"].toArray();
        for (int r = 0; r < t.rows() && r < steps.size(); ++r)
            for (int c = 0; c < kSteps * kMaxBars; ++c) t.steps[r][c] = steps[r].toString().mid(c, 1) == "x";
        s.tracks.push_back(t);
    }
    for (const auto &v : root["sections"].toArray()) {
        const QJsonObject o = v.toObject();
        Section sec{o["name"].toString(), qBound(1, o["bars"].toInt(4), 64), {}};
        for (size_t i = 0; i < s.tracks.size(); ++i) sec.on.push_back(o["on"][int(i)].toBool());
        s.sections.push_back(sec);
    }
    if (s.tracks.empty()) return false;
    song = s;
    return true;
}
