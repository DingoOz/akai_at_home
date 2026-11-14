// Lessons: Trap, Pop.
// Melodic rows count from the bottom, starting at 1: row 1 = A, 2 = B, 3 = C, 4 = D, 5 = E, 6 = F, 7 = G, 8 = A.
// (In code rows count from 0.)
#include "guide.h"

// ---------------------------------------------------------------- Trap
namespace trap { enum { KICK, CLAP, HATS, B808, BELLS, CHOIR }; }

Song trapSong() {
    using namespace trap;
    Song s;
    s.bpm = 140;
    s.tracks = {
        drumTrack("Kick", 9, {"Kick"}, {36}, 120),
        drumTrack("Clap", 10, {"Clap", "Snare"}, {39, 38}, 100),
        drumTrack("Hats", 11, {"Closed hat", "Open hat"}, {42, 46}, 80),
        synthTrack("808", 0, 38, 33, 6, 0, 115),      // Synth Bass 1 standing in for an 808
        synthTrack("Bells", 1, 10, 69, 2, 70, 60),    // Music Box
        synthTrack("Choir", 2, 52, 57, 16, 100, 55),  // Choir Aahs
    };
    s.sections = {
        {"Intro", 8, {false, false, false, false, true, true}},
        {"Hook", 8, {true, true, true, true, true, true}},
        {"Verse", 8, {true, true, true, true, false, true}},
        {"Hook 2", 8, {true, true, true, true, true, true}},
        {"Bridge", 4, {false, false, true, false, true, true}},
        {"Hook 3", 8, {true, true, true, true, true, true}},
        {"Outro", 4, {false, false, false, false, true, true}},
    };
    return s;
}

std::vector<GuideStep> trapSteps() {
    using namespace trap;
    return {
        {"Welcome to Trap", R"(
<p><b>Trap</b> is dark and heavy: a booming <b>808</b> bass, rattling hi-hat rolls, and a "half-time" beat
that feels slow even though the tempo is fast. Melodies are short, eerie loops on bells or plucks.</p>)",
         KICK, {"tracks", "grid", "next"}, {}},
        tempoStep(140, 0, "Trap is set around 130-150 BPM, but the clap only lands once per bar, so it feels like half that speed.", KICK),
        {"Half-time beat", R"(
<p>Select <b>Clap</b> and click step <b>9</b> only. Select <b>Kick</b> and click <b>1 and 11</b>.</p>
<p><b>Listen:</b> one big clap in the middle of the bar. That's the "half-time" feel: heavy and slow.</p>)", CLAP, {},
         [](Song &s) { s.tracks[CLAP].set(0, {8}); s.tracks[KICK].set(0, {0, 10}); }},
        {"Hi-hat rolls", R"(
<p>Select <b>Hats</b>. <b>Closed hat</b> on every odd step <b>1, 3 ... 11</b>, then <b>every</b> step from
<b>13 to 16</b>.</p>
<p><b>Listen:</b> the sudden rattle at the end of the bar is the trap signature. Move the roll somewhere else,
or add an <b>Open hat</b> on 7, to change the bounce.</p>)", HATS, {},
         [](Song &s) { s.tracks[HATS].set(0, {0, 2, 4, 6, 8, 10, 12, 13, 14, 15}); }},
        {"Dark chords", R"(
<p>Set the chord boxes to <b>Am &rarr; Am &rarr; F &rarr; Em</b>. Staying on <b>Am</b> (A C E) for two bars
then sinking through <b>F</b> (F A C) to <b>Em</b> (E G B) sounds moody and tense.</p>)", B808, {"chords"},
         [](Song &s) { s.chords = {0, 0, 5, 4}; }},
        {"The 808", R"(
<p>Select <b>808</b>. Bottom row on <b>1 and 11</b>, exactly with the kicks. The 808 is a long, deep bass
note that rings out: raise <b>Release</b> (<b>K4</b>) to about 60 so it sustains.</p>
<p>It follows the chords, so it changes note every bar. Turn the <b>Volume</b> up loud on a good speaker
and feel it.</p>)", B808, {},
         [](Song &s) { s.tracks[B808].set(0, {0, 10}); s.tracks[B808].knobs[3] = 60; }},
        {"An eerie bell loop", R"(
<p>Select <b>Bells</b>. A short loop that repeats each bar and follows the chords:</p>
<p>Row 8 on <b>1</b>, row 5 on <b>4</b>, row 3 on <b>7</b>, row 5 on <b>9</b>, row 3 on <b>12</b>,
row 1 on <b>15</b>.</p>
<p>Use the green <b>Chord notes</b> rows to experiment: trap melodies are often just three or four notes.</p>)", BELLS, {"chordhelp"},
         [](Song &s) { Track &t = s.tracks[BELLS]; t.set(7, {0}), t.set(4, {3, 8}), t.set(2, {6, 11}), t.set(0, {14}); }},
        {"A dark choir", R"(
<p>Select <b>Choir</b>. Click rows <b>1, 3 and 5</b> on step <b>1</b>. Turn its <b>Filter</b> (<b>K1</b>)
down to about 70 so it sits in the shadows behind the bells.</p>)", CHOIR, {},
         [](Song &s) { Track &t = s.tracks[CHOIR]; t.set(0, {0}), t.set(2, {0}), t.set(4, {0}); t.knobs[0] = 70; }},
        arrangeStep("<b>Intro</b> (bells and choir) &rarr; <b>Hook</b> (everything) &rarr; <b>Verse</b> (beat without "
                    "the bells, room for a rapper) &rarr; <b>Hook 2</b> &rarr; <b>Bridge</b> &rarr; <b>Hook 3</b> &rarr; "
                    "<b>Outro</b>.", trapSong, KICK),
        finishStep(B808),
    };
}

// ---------------------------------------------------------------- Pop
namespace pop { enum { KICK, SNARE, HATS, BASS, PIANO, STRINGS, LEAD }; }

Song popSong() {
    using namespace pop;
    Song s;
    s.bpm = 118;
    s.tracks = {
        drumTrack("Kick", 9, {"Kick"}, {36}, 120),
        drumTrack("Snare", 10, {"Clap", "Snare"}, {39, 38}, 100),
        drumTrack("Hats", 11, {"Closed hat", "Tambourine"}, {42, 54}, 75),
        synthTrack("Bass", 0, 34, 33, 2, 0, 100),       // Picked Bass
        synthTrack("Piano", 1, 0, 57, 3, 60, 80),       // Grand Piano
        synthTrack("Strings", 2, 48, 57, 16, 90, 60),   // String Ensemble
        synthTrack("Lead", 3, 73, 69, 3, 70, 70),       // Flute, standing in for a singer
    };
    s.tracks[LEAD].bars = 4, s.tracks[LEAD].follow = false;
    s.sections = {
        {"Intro", 4, {false, false, false, false, true, false, false}},
        {"Verse", 8, {true, false, true, true, true, false, false}},
        {"Pre-chorus", 4, {true, true, true, true, true, true, false}},
        {"Chorus", 8, {true, true, true, true, true, true, true}},
        {"Verse 2", 8, {true, true, true, true, true, false, false}},
        {"Chorus 2", 8, {true, true, true, true, true, true, true}},
        {"Outro", 4, {false, false, false, false, true, true, false}},
    };
    return s;
}

std::vector<GuideStep> popSteps() {
    using namespace pop;
    return {
        {"Welcome to Pop", R"(
<p><b>Pop</b> is all about being catchy and familiar: a clear beat, friendly major chords, a hook you can
hum, and a <b>verse &rarr; chorus</b> structure that builds to the big moment.</p>)", PIANO, {"tracks", "grid", "next"}, {}},
        tempoStep(118, 0, "Upbeat pop usually sits at 100-125 BPM: easy to clap along to.", KICK),
        {"The four-chord song", R"(
<p>Set the chord boxes to <b>C &rarr; G &rarr; Am &rarr; F</b>.</p>
<p><b>C</b> (C major) = C E G, <b>G</b> = G B D, <b>Am</b> (A minor) = A C E, <b>F</b> = F A C. These four
chords are in hundreds of hit songs. Three are major (happy) and one is minor (a touch of sadness).
That mix is why it works so well.</p>)", PIANO, {"chords"}, [](Song &s) { s.chords = {2, 6, 0, 5}; }},
        {"Piano on every beat", R"(
<p>Select <b>Piano</b>. Click rows <b>1, 3 and 5</b> on steps <b>1, 5, 9 and 13</b>: a chord on every beat.
The rows move with the chords, so the piano plays C, then G, then Am, then F.</p>)", PIANO, {"chordhelp"},
         [](Song &s) { for (int r : {0, 2, 4}) s.tracks[PIANO].set(r, {0, 4, 8, 12}); }},
        {"The beat", R"(
<p>Select <b>Kick</b>: <b>1, 9 and 11</b>. Select <b>Snare</b>: the Clap row on <b>5 and 13</b>.</p>
<p>Select <b>Hats</b>: <b>Closed hat</b> on every odd step <b>1, 3 ... 15</b>, and <b>Tambourine</b> on
<b>5 and 13</b> to brighten the backbeat.</p>)", KICK, {},
         [](Song &s) {
             s.tracks[KICK].set(0, {0, 8, 10});
             s.tracks[SNARE].set(0, {4, 12});
             s.tracks[HATS].set(0, {0, 2, 4, 6, 8, 10, 12, 14}), s.tracks[HATS].set(1, {4, 12});
         }},
        {"Bass on the root", R"(
<p>Select <b>Bass</b>. Bottom row on every odd step <b>1, 3 ... 15</b>. Pop bass often just plays the root
note of each chord (the note it's named after) in steady 8th notes.</p>)", BASS, {},
         [](Song &s) { s.tracks[BASS].set(0, {0, 2, 4, 6, 8, 10, 12, 14}); }},
        {"Strings for the chorus", R"(
<p>Select <b>Strings</b>. Rows <b>1, 3 and 5</b> on step <b>1</b>, and raise <b>Attack</b> (<b>K3</b>) to about
60 so they swell. In the song they only come in for the pre-chorus and chorus, to make them feel bigger.</p>)", STRINGS, {},
         [](Song &s) { Track &t = s.tracks[STRINGS]; t.set(0, {0}), t.set(2, {0}), t.set(4, {0}); t.knobs[2] = 60; }},
        {"The hook", R"(
<p>Select <b>Lead</b> (4 bars, use <b>Bar 1-4</b>). This is the part people sing along to:</p>
<p><b>Bar 1</b> (C): E5 on 1, G5 on 5, E5 on 9, C5 on 13<br><b>Bar 2</b> (G): D5 on 1, B4 on 5, D5 on 9<br>
<b>Bar 3</b> (Am): C5 on 1, E5 on 5, A5 on 9, E5 on 13<br><b>Bar 4</b> (F): F5 on 1, E5 on 5, C5 on 9</p>
<p>Almost every note is on the beat and in the chord (green rows), which is what makes it so singable.</p>)",
         LEAD, {"bars", "chordhelp"},
         [](Song &s) {
             Track &t = s.tracks[LEAD];
             t.set(4, {0, 8, 36, 44, 52}), t.set(6, {4}), t.set(2, {12, 32, 56}), t.set(3, {16, 24}), t.set(1, {20});
             t.set(7, {40}), t.set(5, {48});
         }},
        arrangeStep("<b>Intro</b> (piano) &rarr; <b>Verse</b> (beat and bass, quieter) &rarr; <b>Pre-chorus</b> (strings "
                    "build) &rarr; <b>Chorus</b> (the hook) &rarr; <b>Verse 2</b> &rarr; <b>Chorus 2</b> &rarr; <b>Outro</b>.",
                    popSong, PIANO),
        finishStep(LEAD),
    };
}
