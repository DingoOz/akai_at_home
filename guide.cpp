#include "guide.h"

enum { KICK, CLAP, HATS, ROLL, BASS, PAD, ARP, LEAD };  // track order in defaultSong()
enum { FILTER, RESO, ATTACK, RELEASE, REVERB, CHORUS, VOLUME };

const char *kKnobHelp[kKnobs + 1] = {
    "<b>Filter</b> cuts the high frequencies. Down = muffled and dark, up = bright and open. Sweep it up in a build-up.",
    "<b>Resonance</b> boosts the sound right at the filter's cutoff, giving sweeps a whistly, squelchy edge.",
    "<b>Attack</b> is how long a note takes to fade in. Up = soft swells (pads), down = instant hits (bass, plucks).",
    "<b>Release</b> is how long a note rings on after it ends. Up = long, dreamy tails.",
    "<b>Reverb</b> sends the sound into a huge hall. Up = distant and ethereal, down = dry and punchy.",
    "<b>Chorus</b> layers slightly detuned copies of the sound to make it wider and shimmery.",
    "<b>Volume</b> sets this track's loudness in the mix.",
    "<b>Tempo</b> sets the speed of the whole song in beats per minute (BPM).",
};

const char *kChordNotes[7] = {"A C E", "B D F", "C E G", "D F A", "E G B", "F A C", "G B D"};

std::vector<GuideStep> guideSteps() {
    return {
        {"Welcome", R"(
<p>We'll build an <b>ethereal trance</b> track one layer at a time: a dreamy pad and arpeggio first,
then the drums and bass, then a slow, soft melody. Then we'll arrange it into a song with a breakdown
and a drop.</p>
<p><b>The screen:</b> tracks on the left, the <b>step grid</b> in the middle (one bar, 16 steps,
each step is a 16th note, every 4 steps is one beat), and knobs and song arrangement at the bottom.</p>
<p>Press <b>▶ Play</b> (top left) any time to hear your loop; it glows whenever the music is stopped.</p>
<p>Each part tells you what to click. A <b>glowing yellow outline</b> shows where to look, and
<b>dashed squares</b> in the grid show which steps to click. If you're stuck, <b>Do it for me</b> does that step for you.
Too small to read? Use <b>A+</b> above this text.
Click <b>Next</b> to start.</p>)", PAD, {"tracks", "grid", "knobs", "doit", "next"}, {}},

        {"Set the tempo", R"(
<p>Tempo is measured in <b>BPM</b> (beats per minute). Big-room trance runs at 138-140, but dreamy,
emotional trance feels better a little slower. Set <b>BPM</b> at the top to <b>132</b>.</p>
<p>Press <b>Play</b> (mode: <i>Loop</i>). Nothing plays yet because the grid is empty. Leave it playing.</p>
<p><b>MPK:</b> knob <b>K8</b> is the tempo knob. Turn it and watch the BPM change.</p>)", PAD, {"bpm", "play", "mode"},
         [](Song &s) { s.bpm = 132; }},

        {"The chord progression", R"(
<p>Ethereal music starts with <b>chords</b>, not drums. A <b>chord</b> is several notes played at
the same time. The four boxes at the top are the <b>chord progression</b>, one chord per bar:
<b>Am &rarr; F &rarr; C &rarr; G</b>. This emotional, uplifting progression is behind countless
trance anthems.</p>
<p><b>A vs Am:</b> a letter on its own, like <b>A</b> or <b>C5</b>, is a single <b>note</b> (one key).
A chord name starts with the note it's built on, and an <b>m</b> means <b>minor</b>:</p>
<ul>
<li><b>Am</b> = "A minor" = the notes <b>A + C + E</b> together. Minor chords sound sad, soft, wistful.</li>
<li><b>F</b> = "F major" = <b>F + A + C</b>. No "m" means <b>major</b>: bright, hopeful.</li>
<li><b>C</b> = <b>C + E + G</b> &nbsp; and &nbsp; <b>G</b> = <b>G + B + D</b>, both major.</li>
</ul>
<p>Each chord box also shows its notes. Moving between sad (Am) and hopeful (F, C, G) chords is what
gives trance its bittersweet feel.</p>
<p>Tracks with <b>Follow chords</b> ticked move along with it automatically: you write one bar and
the chords do the rest.</p>)", PAD, {"chords"},
         [](Song &s) { s.chords = {0, 5, 2, 6}; }},

        {"A lush pad", R"(
<p><b>Pad</b> is selected. A pad is a long, soft chord that fills the background.</p>
<p>On step <b>1</b>, click four rows: <b>A, C, E and G</b> (the bottom row and every second row above it).
Three notes make a plain chord; the fourth (the <b>7th</b>) makes it dreamy and unresolved. The <b>Gate</b>
is 16, so the chord holds for the whole bar.</p>
<p>Now make it breathe: in the <b>Knobs</b> tab, raise <b>Attack</b> to about 60 (it fades in) and
<b>Release</b> to about 70 (it fades out slowly). On the MPK, those are <b>K3</b> and <b>K4</b>.</p>
<p><b>To hear it:</b> press <b>▶ Play</b> at the top left (mode <i>Loop</i>). It plays the 4 bars over and
over, with the chord changing each bar. Press it again (<b>■ Stop</b>) to stop. Turn the knobs while it plays
to hear them change the sound.</p>
<p><b>Try:</b> change bar 3's chord to <b>Dm</b> and listen to the mood darken. Change it back to C.</p>)", PAD, {"gate"},
         [](Song &s) {
             Track &t = s.tracks[PAD];
             t.set(0, {0}), t.set(2, {0}), t.set(4, {0}), t.set(6, {0});
             t.knobs[ATTACK] = 60, t.knobs[RELEASE] = 70, t.knobs[CHORUS] = 60;
         }},

        {"Play along on the MPK", R"(
<p>The MPK keys always play the <b>selected track's</b> sound. With <b>Pad</b> selected, press some keys:
you hear them straight away through your speakers, even when the loop is stopped.</p>
<p><b>To play along:</b> press <b>▶ Play</b> so the pad loops in the background, then play keys on top.
Nothing when you press a key? Click <b>Reconnect MPK</b> at the top right, and check the big letter in the
Knobs tab changes when you press a key.</p>
<p><b>The secret:</b> this song is in <b>A minor</b>, and A minor uses <b>only the white keys</b>.
Any white keys sound good over it. Try holding <b>A</b>, then <b>F</b>, <b>C</b> and <b>G</b> along
with the chords. Play <b>slowly</b>: in ethereal music, space between notes matters.</p>
<p>The big letter in the Knobs tab shows which note you hit. The MPK's <b>Octave</b> buttons shift
the keyboard up or down.</p>)", PAD, {"note", "instrument"}, {}},

        {"A sparkly arpeggio", R"(
<p>Select <b>Arp</b>. An arpeggio plays a chord one note at a time.</p>
<p>Climb the chord on every step: <b>A</b> (bottom row) on 1, 5, 9, 13; <b>C</b> (3rd row) on
2, 6, 10, 14; <b>E</b> (5th row) on 3, 7, 11, 15; top <b>A</b> on 4, 8, 12, 16. You get a staircase pattern.</p>
<p><b>Listen:</b> shimmering, glassy movement on top of the pad.</p>)", ARP, {},
         [](Song &s) {
             Track &t = s.tracks[ARP];
             t.set(0, {0, 4, 8, 12}), t.set(2, {1, 5, 9, 13}), t.set(4, {2, 6, 10, 14}), t.set(7, {3, 7, 11, 15});
             t.knobs[RELEASE] = 40, t.knobs[REVERB] = 90;
         }},

        {"The filter sweep", R"(
<p>With <b>Arp</b> selected, turn <b>Filter</b> (<b>K1</b> on the MPK) down. It goes soft and
muffled, as if it's in the next room.</p>
<p>Turning it <b>slowly</b> back up over a few bars builds tension. It's the classic sound of a trance
build-up. <b>Resonance</b> (<b>K2</b>) adds a whistly edge, but keep it low for a gentle mood.</p>
<p><b>Do it for me</b> sets the filter fairly low, for a softer arp you can open up later.</p>)", ARP, {},
         [](Song &s) { s.tracks[ARP].knobs[FILTER] = 70; }},

        {"A slow, soft melody", R"(
<p>Select <b>Lead</b>. A melody needs room, so its <b>Length</b> is <b>4 bars</b>, one bar per chord.
Use the <b>Bar 1-4</b> buttons above the grid to move between them. <b>Follow chords</b> is off:
we choose each note to fit that bar's chord. The <b>Gate</b> is 6, so every note is long and singing.</p>
<p>Only three notes per bar:</p>
<p><b>Bar 1</b> (Am): E5 on step 1, D5 on 7, C5 on 11<br>
<b>Bar 2</b> (F): C5 on 1, A4 on 7, C5 on 11<br>
<b>Bar 3</b> (C): E5 on 1, G5 on 7, E5 on 11<br>
<b>Bar 4</b> (G): D5 on 1, B4 on 9, then silence</p>
<p><b>Helper:</b> the <b>green rows</b> are the notes of the bar's chord (set by <b>Chord notes</b> above the grid).
Notes on green rows always fit; the others are fine as quick passing notes.</p>
<p><b>Listen:</b> it's a question (bars 1-2) and an answer (bars 3-4) that settles on the G chord and leads
back around. The sound is a soft bell-pad, not a sharp synth. Also try <i>Pan Flute</i>, <i>Choir</i>
or <i>Voice</i> from the Instrument menu.</p>)", LEAD, {"length", "bars", "follow", "gate", "chordhelp"},
         [](Song &s) {
             Track &t = s.tracks[LEAD];
             t.set(4, {0, 32, 42}), t.set(3, {6, 48}), t.set(2, {10, 16, 26}), t.set(0, {22}), t.set(6, {38}), t.set(1, {56});
         }},

        {"Space: reverb and chorus", R"(
<p>Ethereal music sounds like it's in a huge hall. With <b>Lead</b> selected, move <b>Reverb</b>
(<b>K5</b>) to 0, then back up to about 110. That's the difference between a demo and a mood.</p>
<p><b>Chorus</b> (<b>K6</b>) widens it; <b>Release</b> (<b>K4</b>) lets each note fade into the next;
a little <b>Attack</b> (<b>K3</b>) takes the edge off the start of each note. Keeping <b>Filter</b> a bit
below the top keeps it soft rather than piercing.</p>)", LEAD, {},
         [](Song &s) { s.tracks[LEAD].knobs = {85, 0, 15, 70, 110, 50, 60}; }},

        {"The kick", R"(
<p>Now the beat. Select <b>Kick</b> and click steps <b>1, 5, 9 and 13</b> (the first step of each beat):
"four on the floor".</p>
<p><b>Listen:</b> the dreamy loop suddenly has a pulse. Notice how much one sound changes the feel.</p>
<p><b>MPK:</b> hit the pads. If they're on MIDI channel 10 (the factory setting) they play drums over the loop.</p>)", KICK, {},
         [](Song &s) { s.tracks[KICK].set(0, {0, 4, 8, 12}); }},

        {"A rolling bass", R"(
<p>Select <b>Bass</b>. Trance bass rolls in the gaps <i>around</i> the kick, so the two never clash.</p>
<p>On the bottom <b>A</b> row, click steps <b>2, 3, 6, 7, 10, 11, 14, 15</b>. On the top <b>A</b> row
(one octave up), click <b>4, 8, 12, 16</b>. It follows the chords, so it changes note every bar.</p>)", BASS, {},
         [](Song &s) { s.tracks[BASS].set(0, {1, 2, 5, 6, 9, 10, 13, 14}); s.tracks[BASS].set(7, {3, 7, 11, 15}); }},

        {"Clap and hats, sparingly", R"(
<p>Select <b>Clap</b> and click steps <b>5 and 13</b> (beats 2 and 4).</p>
<p>Select <b>Hats</b>. On the <b>Open hat</b> row, click <b>3, 7, 11, 15</b>, between the kicks: the
classic trance <i>boom-tss</i>. On <b>Closed hat</b>, just <b>4, 8, 12, 16</b>.</p>
<p><b>Less is more:</b> filling every step with hats makes the track feel rushed and busy, not dreamy.
In the song these only come in halfway through, so the start can breathe.</p>)", HATS, {},
         [](Song &s) {
             s.tracks[CLAP].set(0, {4, 12});
             s.tracks[HATS].set(1, {2, 6, 10, 14}), s.tracks[HATS].set(0, {3, 7, 11, 15});
         }},

        {"A build-up that speeds up", R"(
<p>Select <b>Snare roll</b>. Its Length is 4 bars and it gets faster each bar, pulling you towards the drop:</p>
<p><b>Bar 1:</b> steps 1, 5, 9, 13 &nbsp; <b>Bar 2:</b> every odd step (1, 3, 5 ... 15)
&nbsp; <b>Bars 3-4:</b> all 16 steps</p>
<p>It only plays in the <b>Build-up</b> section. Mute it now (untick it) or the loop gets noisy.</p>)", ROLL, {"length", "bars"},
         [](Song &s) {
             Track &t = s.tracks[ROLL];
             t.set(0, {0, 4, 8, 12, 16, 18, 20, 22, 24, 26, 28, 30});
             for (int i = 32; i < 64; ++i) t.steps[0][i] = true;
             t.mute = true;
         }},

        {"Hear the layers", R"(
<p>Mute and unmute the tracks one at a time to hear what each layer adds. Try just Pad + Arp + Lead with
no drums: that's the dreamy <b>breakdown</b> sound. Then add the drums back one by one.</p>
<p>Want a different flavour? Each track has an <b>Instrument</b> menu with every sound in the
SoundFont. Try <i>Choir</i>, <i>Atmosphere</i> or <i>Synth Strings</i> on the Pad.</p>)", PAD, {"tracks", "instrument"}, {}},

        {"Arrange the song", R"(
<p>Open the <b>Song</b> tab. A track is a list of <b>sections</b>, each a number of bars with some
tracks switched on. Layers arrive gradually, so the energy grows:</p>
<p><b>Intro</b> (pad + arp only) &rarr; <b>Groove</b> (kick and bass join) &rarr; <b>Groove 2</b>
(clap and hats) &rarr; <b>Breakdown</b> (drums out, the melody appears) &rarr; <b>Build-up</b>
(snare roll) &rarr; <b>Drop</b> (everything) &rarr; <b>Outro</b>.</p>
<p>Tick and untick boxes to change who plays where. Keep bar counts multiples of 4 so the chords and the
melody line up.</p>)", KICK, {},
         [](Song &s) { s.sections = defaultSong().sections; }},

        {"Play the whole track", R"(
<p>Set the mode at the top to <b>Song</b> and press <b>Play</b>. Mutes still apply, so tick the
<b>Snare roll</b> back on first. The playhead walks through every section, about two minutes.</p>
<p>During the build-up, try sweeping <b>K1</b> on the Arp from closed to open, timed so it's fully open
on the drop.</p>
<p><b>Save</b> keeps your project to work on later. <b>Export audio</b> renders the song to an audio
file (MP3, WAV, Ogg, FLAC or AIFF) you can play anywhere or share.</p>)", ROLL, {"mode", "play", "save", "export"},
         [](Song &s) { s.tracks[ROLL].mute = false; }},

        {"Where next", R"(
<p><b>Make it yours:</b></p>
<ul>
<li>Write your own 4-bar melody. Pick notes that are in each bar's chord, leave gaps, and repeat ideas.</li>
<li>Change the progression: try <b>Am F C Em</b> or <b>F G Am Am</b> (then re-check the melody notes).</li>
<li>Swap instruments: a choir pad plus a bell arp sounds completely different.</li>
<li>Use the MPK's built-in <b>arpeggiator</b> and <b>Note Repeat</b> while holding keys or pads.</li>
<li>Better sounds: <code>sudo apt install fluid-soundfont-gm</code> and restart.</li>
</ul>)", LEAD, {"chords", "instrument"}, {}},
    };
}

GuideStep tempoStep(int bpm, int swing, const QString &why, int track) {
    QString swingText = swing ? QString(" and <b>Swing</b> to <b>%1%</b>. Swing delays every second 16th note, "
                                        "so the groove bounces instead of marching").arg(swing)
                              : QString(" and <b>Swing</b> to <b>0%</b> (straight, machine-tight timing)");
    return {"Set the tempo", QString(R"(
<p>Tempo is measured in <b>BPM</b> (beats per minute). %1</p>
<p>Set <b>BPM</b> to <b>%2</b>%3.</p>
<p>Then press <b>▶ Play</b> (mode <i>Loop</i>). Nothing plays yet because the grid is empty. Leave it playing
while you build, so you hear every change. <b>MPK:</b> knob <b>K8</b> also sets the tempo.</p>)").arg(why).arg(bpm).arg(swingText),
            track, {"bpm", "swing", "play", "mode"},
            [bpm, swing](Song &s) { s.bpm = bpm, s.swing = swing; }};
}

GuideStep arrangeStep(const QString &sections, std::function<Song()> start, int track) {
    return {"Arrange the song", QString(R"(
<p>Open the <b>Song</b> tab. A track is a list of <b>sections</b>, each some bars long with some tracks
switched on. Bringing layers in and out over time is what turns a loop into a song:</p>
<p>%1</p>
<p>Tick and untick boxes to change who plays where, and edit the bar counts (keep them multiples of 4 so
the chords and melodies line up).</p>)").arg(sections),
            track, {"table"}, [start](Song &s) { s.sections = start().sections; }};
}

GuideStep finishStep(int track) {
    return {"Play it, save it, share it", R"(
<p>Set the mode at the top to <b>Song</b> and press <b>▶ Play</b>. The Song tab highlights each section
as it plays. Turn the MPK knobs on different tracks while it plays: that's how producers "perform" a mix.</p>
<p><b>Save</b> keeps the project to keep working on. <b>Export audio</b> makes an MP3 (or WAV, Ogg, FLAC)
you can play anywhere.</p>
<p><b>Make it yours:</b> change the chords, swap instruments, write your own melody on the green
<b>Chord notes</b> rows, or try another style from the <b>Tutorial</b> menu.</p>)",
            track, {"mode", "play", "save", "export"}, {}};
}

// Each style lives in lessons_*.cpp.
Song houseSong(); std::vector<GuideStep> houseSteps();
Song technoSong(); std::vector<GuideStep> technoSteps();
Song lofiSong(); std::vector<GuideStep> lofiSteps();
Song synthwaveSong(); std::vector<GuideStep> synthwaveSteps();
Song dnbSong(); std::vector<GuideStep> dnbSteps();
Song ambientSong(); std::vector<GuideStep> ambientSteps();
Song trapSong(); std::vector<GuideStep> trapSteps();
Song popSong(); std::vector<GuideStep> popSteps();
Song reggaetonSong(); std::vector<GuideStep> reggaetonSteps();
Song discoSong(); std::vector<GuideStep> discoSteps();

const std::vector<Lesson> &lessons() {
    static const std::vector<Lesson> all = {
        {"Ethereal Trance", "Dreamy pads, a sparkly arpeggio and a slow melody, 132 BPM.", defaultSong, guideSteps},
        {"Deep House", "Warm electric-piano chords, a rolling bass and soft vocal 'oohs', 122 BPM.", houseSong, houseSteps},
        {"Techno", "Hypnotic and driving: one chord, a squelchy acid line and knob tweaking, 130 BPM.", technoSong, technoSteps},
        {"Lo-fi Hip Hop", "Chilled study beats: swung drums, jazzy chords, muffled and dusty, 80 BPM.", lofiSong, lofiSteps},
        {"Synthwave", "80s retro: pulsing bass, big reverb snare, neon arps and synth brass, 100 BPM.", synthwaveSong, synthwaveSteps},
        {"Liquid Drum & Bass", "Fast breakbeat drums under slow, soulful Rhodes chords, 172 BPM.", dnbSong, dnbSteps},
        {"Ambient", "No drums: slow-swelling pads, a drone, choir and music-box bells, 70 BPM.", ambientSong, ambientSteps},
        {"Trap", "Half-time beat, rattling hi-hat rolls, a booming 808 and dark bells, 140 BPM.", trapSong, trapSteps},
        {"Pop", "The famous four-chord song: piano, strings and a catchy hook, 118 BPM.", popSong, popSteps},
        {"Reggaeton", "The dembow rhythm with guitar and marimba plucks, 95 BPM.", reggaetonSong, reggaetonSteps},
        {"Disco", "Octave bass, offbeat hi-hats, funky guitar and string swells, 120 BPM.", discoSong, discoSteps},
    };
    return all;
}
