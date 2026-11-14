// Lessons: Liquid Drum & Bass, Ambient.
// Melodic rows count from the bottom, starting at 1: row 1 = A, 2 = B, 3 = C, 4 = D, 5 = E, 6 = F, 7 = G, 8 = A.
// (In code rows count from 0.)
#include "guide.h"

// ---------------------------------------------------------------- Liquid Drum & Bass
namespace dnb { enum { KICK, SNARE, HATS, BASS, KEYS, PAD, BELLS }; }

Song dnbSong() {
    using namespace dnb;
    Song s;
    s.bpm = 172;
    s.tracks = {
        drumTrack("Kick", 9, {"Kick"}, {36}, 125),
        drumTrack("Snare", 10, {"Snare"}, {38}, 110),
        drumTrack("Hats", 11, {"Closed hat", "Ride"}, {42, 51}, 75),
        synthTrack("Bass", 0, 39, 33, 6, 0, 50),      // Synth Bass 2 (loud with chorus)
        synthTrack("Keys", 1, 4, 57, 8, 80, 45),      // Electric Piano (Rhodes)
        synthTrack("Pad", 2, 94, 57, 16, 110, 55),    // Halo Pad
        synthTrack("Bells", 3, 8, 69, 4, 100, 60),    // Celesta
    };
    s.tracks[BELLS].bars = 4, s.tracks[BELLS].follow = false;
    s.sections = {
        {"Intro", 8, {false, false, true, false, true, true, false}},
        {"Build", 8, {false, true, true, false, true, true, true}},
        {"Drop", 16, {true, true, true, true, true, false, true}},
        {"Break", 8, {false, false, false, false, true, true, true}},
        {"Drop 2", 16, {true, true, true, true, true, true, true}},
        {"Outro", 8, {true, true, true, false, false, true, false}},
    };
    return s;
}

std::vector<GuideStep> dnbSteps() {
    using namespace dnb;
    return {
        {"Welcome to Liquid Drum & Bass", R"(
<p><b>Liquid drum &amp; bass</b> is fast drums under slow, soulful music. The drums race along at about 172 BPM,
while the chords and bass move gently. The contrast is what makes it feel exciting and relaxing at once.</p>)",
         KICK, {"tracks", "grid", "next"}, {}},
        tempoStep(172, 0, "Drum & bass lives at 170-176 BPM. That's fast! But the chords will change slowly.", KICK),
        {"The two-step beat", R"(
<p>Select <b>Kick</b> and click <b>1 and 11</b>. Select <b>Snare</b> and click <b>5 and 13</b>.</p>
<p><b>Listen:</b> <i>boom ... SNARE ... boom SNARE</i>. This "two-step" pattern is the backbone of drum &amp; bass.
The late second kick gives it a rolling, skipping feel.</p>)", KICK, {},
         [](Song &s) { s.tracks[KICK].set(0, {0, 10}); s.tracks[SNARE].set(0, {4, 12}); }},
        {"Hats and ride", R"(
<p>Select <b>Hats</b>: <b>Closed hat</b> on every odd step <b>1, 3 ... 15</b>, and <b>Ride</b> on
<b>3, 7, 11, 15</b> for a shimmering top end.</p>)", HATS, {},
         [](Song &s) { s.tracks[HATS].set(0, {0, 2, 4, 6, 8, 10, 12, 14}); s.tracks[HATS].set(1, {2, 6, 10, 14}); }},
        {"Soulful chords", R"(
<p>Set the chord boxes to <b>Am &rarr; Em &rarr; F &rarr; G</b>. <b>Em</b> (E minor) = E G B: another soft,
minor chord. Climbing from F up to G lifts you back to the start.</p>)", KEYS, {"chords"},
         [](Song &s) { s.chords = {0, 4, 5, 6}; }},
        {"Rhodes keys", R"(
<p>Select <b>Keys</b>. Click rows <b>1, 3, 5 and 7</b> on steps <b>1 and 9</b>: two lush 7th chords per bar.
Liquid DnB borrows these from soul and jazz.</p>)", KEYS, {},
         [](Song &s) { for (int r : {0, 2, 4, 6}) s.tracks[KEYS].set(r, {0, 8}); }},
        {"A smooth bass", R"(
<p>Select <b>Bass</b>. Bottom row on <b>1 and 11</b>, with the kicks. Its Gate is 6, so each note is long.</p>
<p>Make it a "Reese" bass: raise <b>Chorus</b> (<b>K6</b>) to about 70 and lower <b>Filter</b> (<b>K1</b>) to
about 80. Chorus makes it wide and growly, filter keeps it warm.</p>)", BASS, {},
         [](Song &s) { Track &t = s.tracks[BASS]; t.set(0, {0, 10}); t.knobs[5] = 70, t.knobs[0] = 80; }},
        {"Halo pad", R"(
<p>Select <b>Pad</b>. Click row <b>1</b> and row <b>5</b> on step <b>1</b>, and raise <b>Attack</b> (<b>K3</b>)
to about 80 so it swells slowly, the opposite of the fast drums.</p>)", PAD, {},
         [](Song &s) { s.tracks[PAD].set(0, {0}), s.tracks[PAD].set(4, {0}); s.tracks[PAD].knobs[2] = 80; }},
        {"Bell melody", R"(
<p>Select <b>Bells</b>, 4 bars long (use <b>Bar 1-4</b>). Two notes per bar, from each bar's chord:</p>
<p><b>Bar 1</b> (Am): E5 on 1, A5 on 7 &nbsp; <b>Bar 2</b> (Em): G5 on 1, E5 on 7<br>
<b>Bar 3</b> (F): F5 on 1, C5 on 7 &nbsp; <b>Bar 4</b> (G): G5 on 1, D5 on 7</p>)", BELLS, {"bars", "chordhelp"},
         [](Song &s) {
             Track &t = s.tracks[BELLS];
             t.set(4, {0, 22}), t.set(7, {6}), t.set(6, {16, 48}), t.set(5, {32}), t.set(2, {38}), t.set(3, {54});
         }},
        arrangeStep("<b>Intro</b> (keys, pad, hats) &rarr; <b>Build</b> (snare joins) &rarr; <b>Drop</b> (kick and bass) "
                    "&rarr; <b>Break</b> &rarr; <b>Drop 2</b> &rarr; <b>Outro</b>.", dnbSong, KICK),
        finishStep(KEYS),
    };
}

// ---------------------------------------------------------------- Ambient
namespace ambient { enum { DRONE, PAD, CHOIR, BELLS, TEXTURE }; }

Song ambientSong() {
    using namespace ambient;
    Song s;
    s.bpm = 70;
    s.tracks = {
        synthTrack("Drone", 0, 92, 45, 16, 110, 65),    // Bowed Pad, from A2
        synthTrack("Pad", 1, 89, 57, 16, 120, 70),      // Warm Pad
        synthTrack("Choir", 2, 91, 69, 16, 120, 50),    // Choir Pad
        synthTrack("Bells", 3, 10, 69, 6, 127, 55),     // Music Box
        synthTrack("Texture", 4, 99, 57, 16, 110, 45),  // Atmosphere
    };
    s.tracks[DRONE].follow = false;
    s.tracks[BELLS].bars = 4, s.tracks[BELLS].follow = false;
    s.sections = {
        {"Dawn", 8, {true, false, false, false, true}},
        {"Rise", 8, {true, true, false, false, true}},
        {"Glow", 8, {true, true, true, true, true}},
        {"Drift", 8, {true, true, false, true, false}},
        {"Glow 2", 8, {true, true, true, true, true}},
        {"Fade", 8, {true, false, false, false, true}},
    };
    return s;
}

std::vector<GuideStep> ambientSteps() {
    using namespace ambient;
    return {
        {"Welcome to Ambient", R"(
<p><b>Ambient</b> music has no drums and no hurry. It's about space, slowness and texture: sounds that swell,
float and fade into a huge reverb. Nothing should feel rushed. Great for winding down.</p>)",
         DRONE, {"tracks", "grid", "next"}, {}},
        tempoStep(70, 0, "There's no beat to dance to, but a slow tempo means the chords change slowly and gently.", DRONE),
        {"A drone", R"(
<p>Select <b>Drone</b> and click the bottom row on step <b>1</b>. A <b>drone</b> is one low note that never
changes. Everything else floats above it. ("Follow chords" is off, so it stays put.)</p>)", DRONE, {"follow"},
         [](Song &s) { s.tracks[DRONE].set(0, {0}); }},
        {"Gentle chords", R"(
<p>Set the chord boxes to <b>Am &rarr; F &rarr; C &rarr; Em</b>: A C E, then F A C, then C E G, then E G B.
Each chord shares notes with the next, so they melt into each other.</p>)", PAD, {"chords"},
         [](Song &s) { s.chords = {0, 5, 2, 4}; }},
        {"A slow-swelling pad", R"(
<p>Select <b>Pad</b>. Click rows <b>1, 3 and 5</b> on step <b>1</b>. Then turn <b>Attack</b> (<b>K3</b>) up to
about <b>100</b> and <b>Release</b> (<b>K4</b>) to about <b>100</b>.</p>
<p><b>Listen:</b> each chord now fades in slowly and fades out long after. No sharp edges anywhere.</p>)", PAD, {},
         [](Song &s) { Track &t = s.tracks[PAD]; t.set(0, {0}), t.set(2, {0}), t.set(4, {0}); t.knobs[2] = 100, t.knobs[3] = 100; }},
        {"A distant choir", R"(
<p>Select <b>Choir</b>. Click row <b>5</b> on step <b>1</b>: one high note per chord, like voices far away.
Raise <b>Attack</b> (<b>K3</b>) to about 90.</p>)", CHOIR, {},
         [](Song &s) { s.tracks[CHOIR].set(4, {0}); s.tracks[CHOIR].knobs[2] = 90; }},
        {"Music-box bells", R"(
<p>Select <b>Bells</b>, 4 bars long (use <b>Bar 1-4</b>). Very few notes, at unexpected moments:</p>
<p><b>Bar 1</b> (Am): A5 on 4, E5 on 10 &nbsp; <b>Bar 2</b> (F): F5 on 6<br>
<b>Bar 3</b> (C): E5 on 3, C5 on 12 &nbsp; <b>Bar 4</b> (Em): G5 on 4</p>
<p>Silence is part of the music here.</p>)", BELLS, {"bars", "chordhelp"},
         [](Song &s) { Track &t = s.tracks[BELLS]; t.set(7, {3}), t.set(4, {9, 34}), t.set(5, {21}), t.set(2, {43}), t.set(6, {51}); }},
        {"Space", R"(
<p>With <b>Bells</b> selected, turn <b>Release</b> (<b>K4</b>) up to about 110. Reverb is already at the
maximum. Each bell now rings into a huge, endless room.</p>
<p>Try <b>Filter</b> (<b>K1</b>) on the Pad too: down makes it darker and further away.</p>)", BELLS, {},
         [](Song &s) { s.tracks[BELLS].knobs[3] = 110; }},
        {"Texture", R"(
<p>Select <b>Texture</b> and click the bottom row on step <b>1</b>: a soft, airy background that makes the
space feel alive.</p>)", TEXTURE, {}, [](Song &s) { s.tracks[TEXTURE].set(0, {0}); }},
        arrangeStep("<b>Dawn</b> (drone and texture) &rarr; <b>Rise</b> (pad) &rarr; <b>Glow</b> (choir and bells) "
                    "&rarr; <b>Drift</b> &rarr; <b>Glow 2</b> &rarr; <b>Fade</b>.", ambientSong, DRONE),
        finishStep(PAD),
    };
}
