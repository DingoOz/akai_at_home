// Lessons: Lo-fi Hip Hop, Synthwave.
// Melodic rows count from the bottom, starting at 1: row 1 = A, 2 = B, 3 = C, 4 = D, 5 = E, 6 = F, 7 = G, 8 = A.
// (In code rows count from 0.)
#include "guide.h"

// ---------------------------------------------------------------- Lo-fi Hip Hop
namespace lofi { enum { KICK, SNARE, HATS, KEYS, BASS, MELODY, RAIN }; }

Song lofiSong() {
    using namespace lofi;
    Song s;
    s.bpm = 80;
    s.tracks = {
        drumTrack("Kick", 9, {"Kick"}, {36}, 120),
        drumTrack("Snare", 10, {"Snare", "Rim"}, {38, 37}, 95),
        drumTrack("Hats", 11, {"Closed hat", "Open hat"}, {42, 46}, 75),
        synthTrack("Keys", 0, 4, 57, 14, 70, 80),     // Electric Piano
        synthTrack("Bass", 1, 33, 33, 4, 10, 105),    // Finger Bass
        synthTrack("Melody", 2, 11, 69, 4, 80, 70),   // Vibraphone
        synthTrack("Rain", 3, 96, 57, 16, 60, 45),    // FX Rain
    };
    s.tracks[MELODY].bars = 4, s.tracks[MELODY].follow = false;
    s.tracks[RAIN].follow = false;
    s.sections = {
        {"Intro", 4, {false, false, false, true, false, false, true}},
        {"Beat", 8, {true, true, true, true, true, false, true}},
        {"Melody", 8, {true, true, true, true, true, true, true}},
        {"Break", 4, {false, false, false, true, false, true, true}},
        {"Melody 2", 8, {true, true, true, true, true, true, true}},
        {"Outro", 4, {false, false, false, true, false, false, true}},
    };
    return s;
}

std::vector<GuideStep> lofiSteps() {
    using namespace lofi;
    return {
        {"Welcome to Lo-fi Hip Hop", R"(
<p><b>Lo-fi hip hop</b> is the "chill beats to study to" sound: slow, lazy, swung drums, jazzy chords, and
everything a bit muffled and dusty, like an old record. It's about mood, not perfection.</p>)",
         KEYS, {"tracks", "grid", "next"}, {}},
        tempoStep(80, 35, "Lo-fi sits around 70-90 BPM. Lots of swing is the secret: it makes the drums feel lazy and human.", KICK),
        {"Jazzy chords", R"(
<p>Set the chord boxes to <b>Dm &rarr; G &rarr; C &rarr; Am</b>.</p>
<p><b>Dm</b> (D minor) = D F A, <b>G</b> (major) = G B D, <b>C</b> = C E G, <b>Am</b> = A C E. The move
Dm &rarr; G &rarr; C is the most famous chord change in jazz (called a "two-five-one"). It sounds like
coming home.</p>)", KEYS, {"chords"}, [](Song &s) { s.chords = {3, 6, 2, 0}; }},
        {"Dusty electric piano", R"(
<p>Select <b>Keys</b>. On step <b>1</b>, click rows <b>1, 3, 5 and 7</b>: four-note "7th" chords that sound
jazzy and soft. The Gate is 14, so they ring for most of the bar.</p>
<p>Now make it lo-fi: turn <b>Filter</b> (<b>K1</b>) down to about <b>60</b>. The highs disappear and it
sounds like it's playing from an old radio in the next room.</p>)", KEYS, {},
         [](Song &s) { Track &t = s.tracks[KEYS]; for (int r : {0, 2, 4, 6}) t.set(r, {0}); t.knobs[0] = 60; }},
        {"Boom-bap drums", R"(
<p>Select <b>Kick</b> and click <b>1, 8 and 11</b>. Select <b>Snare</b> and click the Snare row on
<b>5 and 13</b>.</p>
<p><b>Listen:</b> <i>boom ... bap ... boom-boom bap</i>. The kicks don't fall on every beat, which makes it
sound like a drummer, not a machine.</p>)", KICK, {},
         [](Song &s) { s.tracks[KICK].set(0, {0, 7, 10}); s.tracks[SNARE].set(0, {4, 12}); }},
        {"Swung hats", R"(
<p>Select <b>Hats</b> and click the <b>Closed hat</b> row on every odd step: <b>1, 3, 5 ... 15</b>. Add an
<b>Open hat</b> on <b>15</b> instead of the closed one there if you like.</p>
<p><b>Listen:</b> set <b>Swing</b> to 0: stiff and robotic. Back to 35: lazy and nodding. Same notes,
different feel.</p>)", HATS, {"swing"},
         [](Song &s) { s.tracks[HATS].set(0, {0, 2, 4, 6, 8, 10, 12}); s.tracks[HATS].set(1, {14}); }},
        {"A round bass", R"(
<p>Select <b>Bass</b>. Bottom row on step <b>1</b>, row 5 on step <b>11</b>. Two notes is enough: the
bass follows the chords, so it changes every bar by itself.</p>)", BASS, {},
         [](Song &s) { s.tracks[BASS].set(0, {0}), s.tracks[BASS].set(4, {10}); }},
        {"A vibraphone melody", R"(
<p>Select <b>Melody</b>. It's 4 bars long; use <b>Bar 1-4</b> to move between them. Two gentle notes per bar,
each from that bar's chord:</p>
<p><b>Bar 1</b> (Dm): F5 on 3, D5 on 11 &nbsp; <b>Bar 2</b> (G): G5 on 3, B4 on 11<br>
<b>Bar 3</b> (C): E5 on 3, C5 on 9 &nbsp; <b>Bar 4</b> (Am): A4 on 1, C5 on 9</p>
<p>Starting off the beat (on step 3) makes it feel relaxed and conversational.</p>)", MELODY, {"bars", "chordhelp"},
         [](Song &s) {
             Track &t = s.tracks[MELODY];
             t.set(5, {2}), t.set(3, {10}), t.set(6, {18}), t.set(1, {26}), t.set(4, {34}), t.set(2, {40, 56}), t.set(0, {48});
         }},
        {"Rain in the background", R"(
<p>Select <b>Rain</b> and click the bottom row on step <b>1</b>. A quiet layer of rain noise is a lo-fi
classic: it fills the silence and adds to the cosy mood. Keep its <b>Volume</b> (<b>K7</b>) low.</p>)", RAIN, {},
         [](Song &s) { s.tracks[RAIN].set(0, {0}); }},
        arrangeStep("<b>Intro</b> (keys and rain) &rarr; <b>Beat</b> (drums and bass) &rarr; <b>Melody</b> &rarr; "
                    "<b>Break</b> (no drums) &rarr; <b>Melody 2</b> &rarr; <b>Outro</b>.", lofiSong, KEYS),
        finishStep(KEYS),
    };
}

// ---------------------------------------------------------------- Synthwave
namespace synthwave { enum { KICK, SNARE, HATS, BASS, PAD, ARP, LEAD }; }

Song synthwaveSong() {
    using namespace synthwave;
    Song s;
    s.bpm = 100;
    s.tracks = {
        drumTrack("Kick", 9, {"Kick"}, {36}, 120),
        drumTrack("Snare", 10, {"Snare", "Clap"}, {38, 39}, 100),
        drumTrack("Hats", 11, {"Closed hat", "Open hat"}, {42, 46}, 75),
        synthTrack("Bass", 0, 38, 33, 1, 0, 95),      // Synth Bass 1
        synthTrack("Pad", 1, 50, 57, 16, 90, 35),     // Synth Strings (loud with chorus)
        synthTrack("Arp", 2, 80, 69, 1, 60, 50),      // Square Lead
        synthTrack("Lead", 3, 62, 69, 4, 80, 60),     // Synth Brass
    };
    s.tracks[SNARE].knobs[4] = 70;
    s.tracks[LEAD].bars = 4, s.tracks[LEAD].follow = false;
    s.sections = {
        {"Intro", 8, {false, false, false, false, true, true, false}},
        {"Drive", 8, {true, true, true, true, true, true, false}},
        {"Chorus", 8, {true, true, true, true, true, true, true}},
        {"Break", 8, {false, false, false, true, true, true, false}},
        {"Chorus 2", 16, {true, true, true, true, true, true, true}},
        {"Outro", 8, {true, true, false, false, true, true, false}},
    };
    return s;
}

std::vector<GuideStep> synthwaveSteps() {
    using namespace synthwave;
    return {
        {"Welcome to Synthwave", R"(
<p><b>Synthwave</b> sounds like an 80s movie: night drives, neon lights. A pulsing synth bass, a big
echoing snare, glittery arpeggios and bold synth-brass melodies.</p>)", KICK, {"tracks", "grid", "next"}, {}},
        tempoStep(100, 0, "Synthwave usually runs at 80-118 BPM: steady and cruising, not frantic.", KICK),
        {"Kick and a huge snare", R"(
<p>Select <b>Kick</b> and click <b>1 and 9</b>. Select <b>Snare</b> and click the Snare row on
<b>5 and 13</b>.</p>
<p>The snare already has lots of <b>Reverb</b> (<b>K5</b>): that giant, roomy 80s snare. Turn it down to 0
and back up to hear what it adds.</p>)", KICK, {},
         [](Song &s) { s.tracks[KICK].set(0, {0, 8}); s.tracks[SNARE].set(0, {4, 12}); }},
        {"Hats", R"(
<p>Select <b>Hats</b>: <b>Closed hat</b> on every odd step <b>1, 3, 5 ... 15</b>. Simple and steady.</p>)", HATS, {},
         [](Song &s) { s.tracks[HATS].set(0, {0, 2, 4, 6, 8, 10, 12, 14}); }},
        {"Chords", R"(
<p>Set the chord boxes to <b>Am &rarr; G &rarr; F &rarr; G</b>. <b>Am</b> (A minor) = A C E, <b>G</b> =
G B D, <b>F</b> = F A C. Going back and forth like this feels like driving forward on an endless road.</p>)",
         BASS, {"chords"}, [](Song &s) { s.chords = {0, 6, 5, 6}; }},
        {"Pulsing bass", R"(
<p>Select <b>Bass</b>. On the bottom row click every odd step: <b>1, 3, 5 ... 15</b>. An even, pulsing 8th-note
bass is the engine of synthwave.</p>)", BASS, {},
         [](Song &s) { s.tracks[BASS].set(0, {0, 2, 4, 6, 8, 10, 12, 14}); }},
        {"Synth strings", R"(
<p>Select <b>Pad</b>. On step <b>1</b>, click rows <b>1, 3 and 5</b>. Raise <b>Chorus</b> (<b>K6</b>) to about
80: chorus makes it wide and wobbly, very 80s. A little <b>Attack</b> (<b>K3</b>, about 40) softens the start.</p>)", PAD, {},
         [](Song &s) { Track &t = s.tracks[PAD]; t.set(0, {0}), t.set(2, {0}), t.set(4, {0}); t.knobs[5] = 80, t.knobs[2] = 40; }},
        {"Neon arpeggio", R"(
<p>Select <b>Arp</b>. Make it climb up and down the chord, one note per step:</p>
<p>Row 1 on <b>1, 7, 9, 15</b>; row 3 on <b>2, 6, 8, 10, 14, 16</b>; row 5 on <b>3, 5, 11, 13</b>;
row 8 on <b>4, 12</b>.</p>)", ARP, {},
         [](Song &s) {
             Track &t = s.tracks[ARP];
             t.set(0, {0, 6, 8, 14}), t.set(2, {1, 5, 7, 9, 13, 15}), t.set(4, {2, 4, 10, 12}), t.set(7, {3, 11});
         }},
        {"Synth-brass melody", R"(
<p>Select <b>Lead</b>, a 4-bar melody (use <b>Bar 1-4</b>). Three bold notes per bar:</p>
<p><b>Bar 1</b> (Am): A5 on 1, G5 on 7, E5 on 9<br><b>Bar 2</b> (G): D5 on 1, G5 on 7, B4 on 9<br>
<b>Bar 3</b> (F): C5 on 1, F5 on 7, A5 on 9<br><b>Bar 4</b> (G): G5 on 1, D5 on 9</p>)", LEAD, {"bars", "chordhelp"},
         [](Song &s) {
             Track &t = s.tracks[LEAD];
             t.set(7, {0, 40}), t.set(6, {6, 22, 48}), t.set(4, {8}), t.set(3, {16, 56}), t.set(1, {24}), t.set(2, {32}), t.set(5, {38});
         }},
        arrangeStep("<b>Intro</b> (pad and arp) &rarr; <b>Drive</b> (drums and bass) &rarr; <b>Chorus</b> (the lead) "
                    "&rarr; <b>Break</b> &rarr; <b>Chorus 2</b> &rarr; <b>Outro</b>.", synthwaveSong, KICK),
        finishStep(LEAD),
    };
}
