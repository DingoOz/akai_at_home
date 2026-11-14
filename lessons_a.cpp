// Lessons: Deep House, Techno, Lo-fi Hip Hop.
// Melodic rows count from the bottom: row 0 = A, 1 = B, 2 = C, 3 = D, 4 = E, 5 = F, 6 = G, 7 = A (octave up).
#include "guide.h"

// ---------------------------------------------------------------- Deep House
namespace house { enum { KICK, CLAP, HATS, BASS, KEYS, PAD, VOCAL }; }

Song houseSong() {
    using namespace house;
    Song s;
    s.bpm = 122;
    s.tracks = {
        drumTrack("Kick", 9, {"Kick"}, {36}, 127),
        drumTrack("Clap", 10, {"Clap", "Snare"}, {39, 38}, 95),
        drumTrack("Hats", 11, {"Closed hat", "Open hat", "Shaker"}, {42, 46, 70}, 80),
        synthTrack("Bass", 0, 39, 33, 2, 0, 105),     // Synth Bass 2
        synthTrack("Keys", 1, 4, 57, 3, 60, 85),      // Electric Piano
        synthTrack("Pad", 2, 89, 57, 16, 90, 60),     // Warm Pad
        synthTrack("Vocal", 3, 53, 69, 12, 100, 70),  // Voice Oohs
    };
    s.tracks[VOCAL].bars = 4, s.tracks[VOCAL].follow = false;
    s.sections = {
        {"Intro", 8, {false, false, true, false, false, true, false}},
        {"Groove", 8, {true, true, true, true, false, true, false}},
        {"Main", 8, {true, true, true, true, true, true, true}},
        {"Break", 8, {false, false, false, false, true, true, true}},
        {"Main 2", 16, {true, true, true, true, true, true, true}},
        {"Outro", 8, {true, false, true, false, false, true, false}},
    };
    return s;
}

std::vector<GuideStep> houseSteps() {
    using namespace house;
    return {
        {"Welcome to Deep House", R"(
<p><b>Deep house</b> is warm and soulful: a steady four-on-the-floor kick, off-beat hi-hats, jazzy
electric-piano chords, a rolling bassline and soft vocal "oohs". It's music for dancing slowly at sunset.</p>
<p>Follow each part, or press <b>Do it for me</b> if you get stuck.</p>)", KICK, {"tracks", "grid", "next"}, {}},
        tempoStep(122, 12, "Deep house sits around 120-124 BPM, with a touch of swing for a human, laid-back feel.", KICK),
        {"The beat", R"(
<p>Select <b>Kick</b> and click steps <b>1, 5, 9, 13</b> (every beat). Then select <b>Clap</b> and click
<b>5 and 13</b> (beats 2 and 4).</p>
<p><b>Listen:</b> <i>boom, clap, boom, clap</i>. Every house track is built on this.</p>)", KICK, {},
         [](Song &s) { s.tracks[KICK].set(0, {0, 4, 8, 12}); s.tracks[CLAP].set(0, {4, 12}); }},
        {"Hi-hats and shaker", R"(
<p>Select <b>Hats</b>. Click the <b>Open hat</b> row on <b>3, 7, 11, 15</b> (between the kicks), and the
<b>Shaker</b> row on every even step: <b>2, 4, 6 ... 16</b>.</p>
<p><b>Listen:</b> the shaker bounces because of the swing. Set <b>Swing</b> to 0 and back to 12 to hear
the difference: 0 sounds stiff, 12 sounds human.</p>)", HATS, {"swing"},
         [](Song &s) { s.tracks[HATS].set(1, {2, 6, 10, 14}); s.tracks[HATS].set(2, {1, 3, 5, 7, 9, 11, 13, 15}); }},
        {"Jazzy chords", R"(
<p>Deep house loves soft minor chords. Set the chord boxes to <b>Am &rarr; Dm &rarr; F &rarr; Em</b>.</p>
<p>A chord is several notes together. <b>Am</b> (A minor) = A C E, <b>Dm</b> = D F A, <b>F</b> (major) =
F A C, <b>Em</b> = E G B. Minor chords sound wistful; this progression drifts without ever feeling finished,
which keeps you dancing.</p>)", KEYS, {"chords"},
         [](Song &s) { s.chords = {0, 3, 5, 4}; }},
        {"Electric piano stabs", R"(
<p>Select <b>Keys</b> (an electric piano). Click the bottom row and every second row above it (four notes:
a dreamy <b>7th chord</b>) on steps <b>1 and 7</b>. The second hit lands just before a beat, which gives the
groove its push.</p>
<p>Now turn the <b>Filter</b> (<b>K1</b>) down a little, to about 90, so the piano sits softly behind the
drums.</p>)", KEYS, {"chordhelp"},
         [](Song &s) {
             Track &t = s.tracks[KEYS];
             for (int r : {0, 2, 4, 6}) t.set(r, {0, 6});
             t.knobs[0] = 90;
         }},
        {"A rolling bassline", R"(
<p>Select <b>Bass</b>. House bass dodges the kick: bottom row on <b>3 and 11</b>, top row (an octave up)
on <b>8</b>, and row 5 (the 5th of the chord) on <b>15</b>.</p>
<p><b>Listen:</b> the bass follows the chords and answers the kick instead of fighting it.</p>)", BASS, {},
         [](Song &s) { Track &t = s.tracks[BASS]; t.set(0, {2, 10}), t.set(7, {7}), t.set(4, {14}); }},
        {"A warm pad", R"(
<p>Select <b>Pad</b>. On step <b>1</b>, click rows 1, 3 and 5 from the bottom (a plain chord). Its
<b>Gate</b> is 16 so it holds all bar. Raise <b>Attack</b> (<b>K3</b>) to about 70 so it swells in gently.</p>)", PAD, {},
         [](Song &s) { Track &t = s.tracks[PAD]; t.set(0, {0}), t.set(2, {0}), t.set(4, {0}); t.knobs[2] = 70; }},
        {"Vocal oohs", R"(
<p>Select <b>Vocal</b>. It's 4 bars long (use <b>Bar 1-4</b> above the grid) with one long note per bar,
each a note of that bar's chord:</p>
<p><b>Bar 1</b>: E5 on step 1 &nbsp; <b>Bar 2</b>: F5 on step 1 &nbsp; <b>Bar 3</b>: C5 on step 1
&nbsp; <b>Bar 4</b>: G5 on step 1</p>
<p>The green <b>Chord notes</b> rows show safe notes for each bar if you want to add more.</p>)", VOCAL, {"bars", "chordhelp"},
         [](Song &s) { Track &t = s.tracks[VOCAL]; t.set(4, {0}), t.set(5, {16}), t.set(2, {32}), t.set(6, {48}); }},
        arrangeStep("<b>Intro</b> (pad and hats) &rarr; <b>Groove</b> (kick, clap, bass) &rarr; <b>Main</b> (keys and "
                    "vocal join) &rarr; <b>Break</b> (drums out) &rarr; <b>Main 2</b> &rarr; <b>Outro</b>.", houseSong, KICK),
        finishStep(KEYS),
    };
}

// ---------------------------------------------------------------- Techno
namespace techno { enum { KICK, HATS, PERC, RUMBLE, ACID, STAB, ATMOS }; }

Song technoSong() {
    using namespace techno;
    Song s;
    s.bpm = 130;
    s.tracks = {
        drumTrack("Kick", 9, {"Kick"}, {36}, 127),
        drumTrack("Hats", 10, {"Closed hat", "Open hat"}, {42, 46}, 85),
        drumTrack("Perc", 11, {"Clap", "Rim"}, {39, 37}, 90),
        synthTrack("Rumble", 0, 39, 33, 1, 40, 100),  // Synth Bass 2 with reverb = techno "rumble"
        synthTrack("Acid", 1, 81, 45, 1, 20, 70),     // Saw lead, from A2
        synthTrack("Stab", 2, 90, 57, 1, 110, 65),    // Polysynth
        synthTrack("Atmos", 3, 99, 57, 16, 100, 55),  // Atmosphere
    };
    s.sections = {
        {"Intro", 8, {true, true, false, false, false, false, true}},
        {"Build", 8, {true, true, true, true, false, false, true}},
        {"Peak", 16, {true, true, true, true, true, true, false}},
        {"Break", 8, {false, false, false, false, true, true, true}},
        {"Peak 2", 16, {true, true, true, true, true, true, false}},
        {"Outro", 8, {true, true, false, false, false, false, true}},
    };
    return s;
}

std::vector<GuideStep> technoSteps() {
    using namespace techno;
    return {
        {"Welcome to Techno", R"(
<p><b>Techno</b> is hypnotic and driving. It repeats one idea for a long time and changes it slowly with
the knobs, so the MPK is your main instrument here. Expect dark, metallic sounds and a pounding kick.</p>)",
         KICK, {"tracks", "grid", "next"}, {}},
        tempoStep(130, 0, "Techno runs at 125-135 BPM with straight, machine-like timing.", KICK),
        {"The kick", R"(
<p>Select <b>Kick</b> and click <b>1, 5, 9, 13</b>. In techno the kick is the loudest thing in the
track: everything else is built around it.</p>)", KICK, {},
         [](Song &s) { s.tracks[KICK].set(0, {0, 4, 8, 12}); }},
        {"Hats", R"(
<p>Select <b>Hats</b>. <b>Open hat</b> on <b>3, 7, 11, 15</b> (the off-beats), <b>Closed hat</b> on every
even step <b>2, 4 ... 16</b>. The closed hats tick between everything and make it drive.</p>)", HATS, {},
         [](Song &s) { s.tracks[HATS].set(1, {2, 6, 10, 14}); s.tracks[HATS].set(0, {1, 3, 5, 7, 9, 11, 13, 15}); }},
        {"Percussion", R"(
<p>Select <b>Perc</b>. <b>Clap</b> on <b>5 and 13</b>. <b>Rim</b> on <b>4, 12 and 15</b>: an off-centre
click that keeps the loop from sounding too square.</p>)", PERC, {},
         [](Song &s) { s.tracks[PERC].set(0, {4, 12}); s.tracks[PERC].set(1, {3, 11, 14}); }},
        {"One chord", R"(
<p>Techno often stays on <b>one chord</b> the whole time. Set all four chord boxes to <b>Am</b>
(A minor: the notes A, C and E). Instead of chord changes, the movement comes from rhythm and knobs.</p>)",
         RUMBLE, {"chords"}, [](Song &s) { s.chords = {0, 0, 0, 0}; }},
        {"The rumble", R"(
<p>Select <b>Rumble</b>. On the bottom row, click every step <b>except</b> the kicks (1, 5, 9, 13).
Then turn its <b>Filter</b> (<b>K1</b>) down to about 60.</p>
<p>This bass has reverb on it, which smears into a dark, rolling <i>rumble</i> under the kick: the classic
modern techno low end.</p>)", RUMBLE, {},
         [](Song &s) {
             Track &t = s.tracks[RUMBLE];
             t.set(0, {1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15});
             t.knobs[0] = 60;
         }},
        {"An acid line", R"(
<p>Select <b>Acid</b>. Click: bottom row on <b>1, 4, 9, 15</b>; top row on <b>3 and 11</b>; row 3 on
<b>7</b>; row 4 on <b>12</b>. Jumping between low and high notes is what makes acid lines bounce.</p>)", ACID, {},
         [](Song &s) { Track &t = s.tracks[ACID]; t.set(0, {0, 3, 8, 14}), t.set(7, {2, 10}), t.set(2, {6}), t.set(3, {11}); }},
        {"Squelch: filter and resonance", R"(
<p>This is the heart of techno. With <b>Acid</b> selected, set <b>Filter</b> (<b>K1</b>) to about 40 and
<b>Resonance</b> (<b>K2</b>) to about 100.</p>
<p>Now slowly turn <b>K1</b> up and down while it plays. That squelchy, talking sound is <i>acid</i>.
Performing it live over a long loop is what techno producers do.</p>)", ACID, {},
         [](Song &s) { s.tracks[ACID].knobs[0] = 40, s.tracks[ACID].knobs[1] = 100; }},
        {"A dub stab", R"(
<p>Select <b>Stab</b>. Click rows 1, 3 and 5 from the bottom on steps <b>7 and 15</b>. Lots of
<b>Reverb</b> (<b>K5</b>) and a little <b>Release</b> (<b>K4</b>, about 60) make each stab echo away into
the distance.</p>)", STAB, {},
         [](Song &s) { Track &t = s.tracks[STAB]; for (int r : {0, 2, 4}) t.set(r, {6, 14}); t.knobs[3] = 60; }},
        {"Atmosphere", R"(
<p>Select <b>Atmos</b>. Click the bottom row and row 5 on step <b>1</b>: a long, eerie background chord.
It plays in the intro and the break, when the drums are away.</p>)", ATMOS, {},
         [](Song &s) { s.tracks[ATMOS].set(0, {0}), s.tracks[ATMOS].set(4, {0}); }},
        arrangeStep("<b>Intro</b> (kick, hats, atmosphere) &rarr; <b>Build</b> (perc and rumble) &rarr; <b>Peak</b> "
                    "(acid and stabs) &rarr; <b>Break</b> (no drums) &rarr; <b>Peak 2</b> &rarr; <b>Outro</b>.", technoSong, KICK),
        finishStep(ACID),
    };
}
