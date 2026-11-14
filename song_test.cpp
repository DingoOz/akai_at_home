// Self-check for the song logic: chord following, arrangement length, guide steps, save/load.
#undef NDEBUG  // keep asserts in Release builds
#include "guide.h"
#include "knobreader.h"
#include <QDir>
#include <cassert>
#include <cstdio>

int main() {
    Song s = defaultSong();
    const Track &bass = s.tracks[4];  // base A1 = 33, chords Am F C G
    assert(noteFor(s, bass, 0, 0) == 33);  // A1
    assert(noteFor(s, bass, 0, 1) == 29);  // F1 (F sits below A)
    assert(noteFor(s, bass, 0, 2) == 36);  // C2
    assert(noteFor(s, bass, 0, 3) == 31);  // G1
    assert(noteFor(s, s.tracks[5], 2, 1) == 57 && noteFor(s, s.tracks[5], 4, 1) == 60);  // pad over F: A3, C4
    assert(noteName(60) == "C4");
    assert(rowFor(s, bass, 33, 0) == 0 && rowFor(s, bass, 45, 0) == 7);  // A1 bottom row, A2 top row
    assert(rowFor(s, bass, 60, 0) == 2 && rowFor(s, bass, 72, 0) == 2);  // any C -> the C row
    assert(noteFor(s, bass, rowFor(s, bass, 65, 1), 1) % 12 == 5);       // F over the F chord, rows shifted
    assert((chordKeys(0) == std::array<int, 3>{69, 72, 76}));  // Am = A4 C5 E5
    assert((chordKeys(5) == std::array<int, 3>{65, 69, 72}));  // F = F4 A4 C5
    for (int c = 0; c < 7; ++c) for (int k : chordKeys(c)) assert(inChord(c, k) && k >= 60 && k < 84);
    assert(inChord(0, 69) && inChord(0, 72) && inChord(0, 76) && !inChord(0, 71));  // Am = A C E, not B
    assert(inChord(5, 65) && inChord(5, 69) && inChord(5, 60) && !inChord(5, 64));  // F = F A C, not E

    for (const GuideStep &g : guideSteps()) if (g.doIt) g.doIt(s);
    int bars = 0;
    for (const Section &sec : s.sections) bars += sec.bars;
    StepInfo info;
    auto none = [](const Track &, int, int, int) {};
    assert(songStep(s, bars * kSteps - 1, true, info, none) && info.section == int(s.sections.size()) - 1);
    assert(!songStep(s, bars * kSteps, true, info, none));
    int kicks = 0;
    songStep(s, 0, false, info, [&](const Track &t, int key, int, int) { kicks += t.drum && key == 36; });
    assert(kicks == 1);
    int lead = 0;  // 4-bar lead: bar 2 starts on C5 (72), not bar 1's E5 again
    songStep(s, 16, false, info, [&](const Track &t, int key, int, int) { if (t.name == "Lead") lead = key; });
    assert(lead == 72);

    // Every lesson: "Do it for me" on every step must fit its starter song, and the song must play to the end.
    for (const Lesson &l : lessons()) {
        Song ls = l.start();
        for (const GuideStep &g : l.steps()) {
            assert(g.track >= 0 && g.track < int(ls.tracks.size()));
            if (g.doIt) g.doIt(ls);  // Track::set throws on a bad row/step
        }
        for (const Section &sec : ls.sections) assert(sec.on.size() == ls.tracks.size());
        long notes = 0, p = 0;
        StepInfo li;
        while (songStep(ls, p++, true, li, [&](const Track &, int key, int, int) { assert(key >= 0 && key < 128); ++notes; })) {}
        for (const Track &t : ls.tracks) {
            bool any = false;
            for (const auto &row : t.steps) for (bool b : row) any |= b;
            if (!any) std::printf("%s: track %s has no notes\n", qPrintable(l.name), qPrintable(t.name)), assert(false);
        }
        assert(notes > 100);
    }
    assert(lessons().size() == 11);

    QString path = QDir::temp().filePath("akai_song_test.json");
    Song back;
    assert(saveSong(s, path) && loadSong(back, path));
    assert(back.tracks.size() == s.tracks.size() && back.sections.size() == s.sections.size());
    for (size_t i = 0; i < s.tracks.size(); ++i)
        assert(back.tracks[i].steps == s.tracks[i].steps && back.tracks[i].knobs == s.tracks[i].knobs);
    QFile::remove(path);
    KnobReader kr;  // endless knob: 1 = +1 click, 127 = -1 click, never a jump to 0 or 127
    int v = 64;
    for (int m : {1, 1, 1, 1, 127, 127}) v = kr.read(70, m, v);
    assert(v == 64 + 3 * 2 - 2 * 2);  // first 1 is held back until the knob type is clear
    KnobReader ab;  // normal knob: positions pass straight through
    v = 0;
    for (int m : {40, 41, 42, 43}) v = ab.read(71, m, v);
    assert(v == 43);
    std::puts("song_test: ok");
}
