// Lessons: Reggaeton, Disco.
// Melodic rows count from the bottom, starting at 1: row 1 = A, 2 = B, 3 = C, 4 = D, 5 = E, 6 = F, 7 = G, 8 = A.
// (In code rows count from 0.)
#include "guide.h"

// ---------------------------------------------------------------- Reggaeton
namespace reggaeton { enum { KICK, SNARE, HATS, BASS, GUITAR, PLUCK, PAD }; }

Song reggaetonSong() {
    using namespace reggaeton;
    Song s;
    s.bpm = 95;
    s.tracks = {
        drumTrack("Kick", 9, {"Kick"}, {36}, 120),
        drumTrack("Snare", 10, {"Snare", "Rim"}, {38, 37}, 100),
        drumTrack("Hats", 11, {"Closed hat", "Shaker"}, {42, 70}, 75),
        synthTrack("Bass", 0, 38, 33, 2, 0, 105),     // Synth Bass 1
        synthTrack("Guitar", 1, 24, 57, 2, 50, 80),   // Nylon Guitar
        synthTrack("Pluck", 2, 12, 69, 2, 60, 65),    // Marimba
        synthTrack("Pad", 3, 89, 57, 16, 90, 55),     // Warm Pad
    };
    s.sections = {
        {"Intro", 4, {false, false, false, false, true, false, true}},
        {"Verse", 8, {true, true, true, true, true, false, false}},
        {"Chorus", 8, {true, true, true, true, true, true, true}},
        {"Verse 2", 8, {true, true, true, true, true, false, false}},
        {"Chorus 2", 8, {true, true, true, true, true, true, true}},
        {"Outro", 4, {false, false, false, false, true, false, true}},
    };
    return s;
}

std::vector<GuideStep> reggaetonSteps() {
    using namespace reggaeton;
    return {
        {"Welcome to Reggaeton", R"(
<p><b>Reggaeton</b> is built on one famous rhythm, the <b>dembow</b>: <i>boom-ch-boom-chick</i>. Once you
have that, add a bass, a guitar or marimba hook and some warm chords, and it's ready for the dance floor.</p>)",
         KICK, {"tracks", "grid", "next"}, {}},
        tempoStep(95, 0, "Reggaeton is usually 88-100 BPM: relaxed but bouncy.", KICK),
        {"The kick", R"(
<p>Select <b>Kick</b> and click <b>1, 5, 9, 13</b>: a steady kick on every beat.</p>)", KICK, {},
         [](Song &s) { s.tracks[KICK].set(0, {0, 4, 8, 12}); }},
        {"The dembow", R"(
<p>Select <b>Snare</b>. On the Snare row click <b>4, 7, 12 and 15</b>.</p>
<p><b>Listen:</b> <i>boom-ch-boom-chick</i>. The snare lands just before and just after the beats, which gives
the rhythm its famous swing. Say it along: "boom ... ch-boom ... chick".</p>)", SNARE, {},
         [](Song &s) { s.tracks[SNARE].set(0, {3, 6, 11, 14}); }},
        {"Hats and shaker", R"(
<p>Select <b>Hats</b>: <b>Closed hat</b> on every odd step <b>1, 3 ... 15</b>, and <b>Shaker</b> on
<b>2, 6, 10, 14</b>.</p>)", HATS, {},
         [](Song &s) { s.tracks[HATS].set(0, {0, 2, 4, 6, 8, 10, 12, 14}); s.tracks[HATS].set(1, {1, 5, 9, 13}); }},
        {"Chords", R"(
<p>Set the chord boxes to <b>Am &rarr; Dm &rarr; G &rarr; C</b>: A C E, D F A, G B D, C E G. Two minor chords
then two major ones: it goes from moody to bright every time around.</p>)", GUITAR, {"chords"},
         [](Song &s) { s.chords = {0, 3, 6, 2}; }},
        {"Bass with the dembow", R"(
<p>Select <b>Bass</b>. Bottom row on <b>1, 4, 9 and 12</b>: the bass locks onto the kick and the dembow
snare together.</p>)", BASS, {},
         [](Song &s) { s.tracks[BASS].set(0, {0, 3, 8, 11}); }},
        {"Nylon guitar", R"(
<p>Select <b>Guitar</b>. Click rows <b>1, 3 and 5</b> on <b>3, 7, 11 and 15</b>, the "and" between the beats.
It strums the chords in the gaps the drums leave.</p>)", GUITAR, {},
         [](Song &s) { for (int r : {0, 2, 4}) s.tracks[GUITAR].set(r, {2, 6, 10, 14}); }},
        {"Marimba hook", R"(
<p>Select <b>Pluck</b>. A bouncy pattern that follows the chords:</p>
<p>Row 5 on <b>1 and 4</b>, row 3 on <b>7</b>, row 8 on <b>9</b>, row 5 on <b>12</b>, row 3 on <b>15</b>.</p>
<p>Notice it copies the dembow's timing (4, 7, 12, 15). Hooks that echo the drums feel extra catchy.</p>)", PLUCK, {"chordhelp"},
         [](Song &s) { Track &t = s.tracks[PLUCK]; t.set(4, {0, 3, 11}), t.set(2, {6, 14}), t.set(7, {8}); }},
        {"A warm pad", R"(
<p>Select <b>Pad</b>. Rows <b>1, 3 and 5</b> on step <b>1</b>, with <b>Attack</b> (<b>K3</b>) at about 50.
It's quiet glue underneath that you'd miss if it were gone. Mute it to check.</p>)", PAD, {},
         [](Song &s) { Track &t = s.tracks[PAD]; t.set(0, {0}), t.set(2, {0}), t.set(4, {0}); t.knobs[2] = 50; }},
        arrangeStep("<b>Intro</b> (guitar and pad) &rarr; <b>Verse</b> (dembow and bass) &rarr; <b>Chorus</b> (marimba "
                    "hook) &rarr; <b>Verse 2</b> &rarr; <b>Chorus 2</b> &rarr; <b>Outro</b>.", reggaetonSong, KICK),
        finishStep(PLUCK),
    };
}

// ---------------------------------------------------------------- Disco
namespace disco { enum { KICK, CLAP, HATS, BASS, GUITAR, STRINGS, BRASS }; }

Song discoSong() {
    using namespace disco;
    Song s;
    s.bpm = 120;
    s.tracks = {
        drumTrack("Kick", 9, {"Kick"}, {36}, 125),
        drumTrack("Clap", 10, {"Clap", "Snare"}, {39, 38}, 100),
        drumTrack("Hats", 11, {"Closed hat", "Open hat"}, {42, 46}, 80),
        synthTrack("Bass", 0, 33, 33, 1, 0, 105),       // Finger Bass
        synthTrack("Guitar", 1, 28, 57, 1, 30, 70),     // Muted Electric Guitar
        synthTrack("Strings", 2, 48, 57, 8, 80, 70),    // String Ensemble
        synthTrack("Brass", 3, 61, 57, 2, 60, 65),      // Brass Section
    };
    s.sections = {
        {"Intro", 4, {true, false, true, false, false, false, false}},
        {"Groove", 8, {true, true, true, true, true, false, false}},
        {"Strings", 8, {true, true, true, true, true, true, false}},
        {"Break", 4, {false, false, true, true, true, false, false}},
        {"Full", 16, {true, true, true, true, true, true, true}},
        {"Outro", 4, {true, false, true, false, false, true, false}},
    };
    return s;
}

std::vector<GuideStep> discoSteps() {
    using namespace disco;
    return {
        {"Welcome to Disco", R"(
<p><b>Disco</b> is 70s dance-floor joy: a four-on-the-floor kick, a "tss" hi-hat between every beat, a bouncing
<b>octave bass</b>, funky muted guitar, sweeping strings and brass stabs.</p>)", KICK, {"tracks", "grid", "next"}, {}},
        tempoStep(120, 0, "Disco sits around 110-125 BPM, the tempo that started dance music.", KICK),
        {"Kick and clap", R"(
<p>Select <b>Kick</b>: <b>1, 5, 9, 13</b>. Select <b>Clap</b>: the Clap row on <b>5 and 13</b>.</p>)", KICK, {},
         [](Song &s) { s.tracks[KICK].set(0, {0, 4, 8, 12}); s.tracks[CLAP].set(0, {4, 12}); }},
        {"The disco hi-hat", R"(
<p>Select <b>Hats</b>. <b>Open hat</b> on <b>3, 7, 11, 15</b>: the famous <i>tss</i> between every kick.
<b>Closed hat</b> on <b>2, 6, 10, 14</b> and <b>4, 8, 12, 16</b> to keep it busy and sparkling.</p>)", HATS, {},
         [](Song &s) { s.tracks[HATS].set(1, {2, 6, 10, 14}); s.tracks[HATS].set(0, {1, 3, 5, 7, 9, 11, 13, 15}); }},
        {"Chords", R"(
<p>Set the chord boxes to <b>Am &rarr; Dm &rarr; Am &rarr; Dm</b>: A C E and D F A. Just two chords rocking
back and forth: disco and funk love a groove that doesn't go anywhere, so you can keep dancing.</p>)", BASS, {"chords"},
         [](Song &s) { s.chords = {0, 3, 0, 3}; }},
        {"Octave bass", R"(
<p>Select <b>Bass</b>. Bottom row on <b>1, 5, 9, 13</b> and top row (one octave up) on <b>3, 7, 11, 15</b>.</p>
<p><b>Listen:</b> <i>low-high-low-high</i>. Jumping octaves is THE disco bassline.</p>)", BASS, {},
         [](Song &s) { s.tracks[BASS].set(0, {0, 4, 8, 12}); s.tracks[BASS].set(7, {2, 6, 10, 14}); }},
        {"Funky guitar", R"(
<p>Select <b>Guitar</b> (muted electric). Click rows <b>3 and 5</b> on every even step <b>2, 4 ... 16</b>.
Short, choppy 16ths like this are called "chicken scratch" guitar.</p>)", GUITAR, {},
         [](Song &s) { for (int r : {2, 4}) s.tracks[GUITAR].set(r, {1, 3, 5, 7, 9, 11, 13, 15}); }},
        {"String swells", R"(
<p>Select <b>Strings</b>. Rows <b>1, 3, 5 and 8</b> on steps <b>1 and 9</b>. Raise <b>Attack</b> (<b>K3</b>) to
about 70 so each chord sweeps in. Big orchestral strings are what made disco feel glamorous.</p>)", STRINGS, {},
         [](Song &s) { Track &t = s.tracks[STRINGS]; for (int r : {0, 2, 4, 7}) t.set(r, {0, 8}); t.knobs[2] = 70; }},
        {"Brass stabs", R"(
<p>Select <b>Brass</b>. Rows <b>1, 3 and 5</b> on <b>7 and 15</b>: short, punchy hits just before beats 3 and 1.
They make the groove jump.</p>)", BRASS, {},
         [](Song &s) { for (int r : {0, 2, 4}) s.tracks[BRASS].set(r, {6, 14}); }},
        arrangeStep("<b>Intro</b> (kick and hats) &rarr; <b>Groove</b> (bass and guitar) &rarr; <b>Strings</b> &rarr; "
                    "<b>Break</b> (drums drop to hats) &rarr; <b>Full</b> (brass joins) &rarr; <b>Outro</b>.", discoSong, KICK),
        finishStep(BASS),
    };
}
