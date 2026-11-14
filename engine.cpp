#include "engine.h"
#include <QFile>
#include <QFileInfo>
#include <sndfile.h>
#include <algorithm>
#include <cmath>

static fluid_settings_t *makeSettings() {
    fluid_settings_t *s = new_fluid_settings();
    fluid_settings_setnum(s, "synth.gain", 0.4);
    fluid_settings_setnum(s, "synth.sample-rate", 44100);
    // Big, bright hall: most of the "ethereal" comes from here plus the Reverb knob.
    fluid_settings_setnum(s, "synth.reverb.room-size", 0.9);
    fluid_settings_setnum(s, "synth.reverb.damp", 0.2);
    fluid_settings_setnum(s, "synth.reverb.width", 1.0);
    fluid_settings_setnum(s, "synth.reverb.level", 0.9);
    return s;
}

static void addMod(fluid_synth_t *synth, int cc, int flags, int dest, double amount) {
    fluid_mod_t *m = new_fluid_mod();
    fluid_mod_set_source1(m, cc, FLUID_MOD_CC | FLUID_MOD_UNIPOLAR | FLUID_MOD_LINEAR | flags);
    fluid_mod_set_source2(m, FLUID_MOD_NONE, FLUID_MOD_GC);
    fluid_mod_set_dest(m, dest);
    fluid_mod_set_amount(m, amount);
    fluid_synth_add_default_mod(synth, m, FLUID_SYNTH_OVERWRITE);
    delete_fluid_mod(m);
}

// Wires the knob CCs to sound parameters. Plain General MIDI only wires reverb/chorus/volume,
// and only to 20% reverb; these make every knob audible.
// ponytail: amounts tuned by ear for the GM banks; adjust here if a knob feels too weak or wild.
static void addKnobMods(fluid_synth_t *synth) {
    addMod(synth, 74, FLUID_MOD_NEGATIVE, GEN_FILTERFC, -7200);  // Filter: 0 = dark, 127 = open
    addMod(synth, 71, 0, GEN_FILTERQ, 200);                      // Resonance: whistly peak at the cutoff
    addMod(synth, 73, 0, GEN_VOLENVATTACK, 7200);                // Attack: notes fade in
    addMod(synth, 72, 0, GEN_VOLENVRELEASE, 4800);               // Release: notes ring on
    addMod(synth, 91, 0, GEN_REVERBSEND, 1000);                  // Reverb send up to 100%
    addMod(synth, 93, 0, GEN_CHORUSSEND, 1000);                  // Chorus send up to 100%
}

static void setupTrack(fluid_synth_t *synth, int sfid, const Track &t) {
    fluid_synth_program_select(synth, t.channel, sfid, t.bank, t.program);
    for (int k = 0; k < kKnobs; ++k) fluid_synth_cc(synth, t.channel, kKnobCC[k], t.knobs[k]);
}

Engine::Engine(const QString &path) : sf2(path), song(defaultSong()) {
    settings = makeSettings();
    fluid_settings_setstr(settings, "midi.driver", "alsa_seq");
    fluid_settings_setint(settings, "midi.autoconnect", 1);
    // ponytail: fixed buffer size; raise to 256/512 if audio crackles, lower means less key-to-sound lag.
    fluid_settings_setint(settings, "audio.period-size", 128);
    synth = new_fluid_synth(settings);
    addKnobMods(synth);
    sfid = fluid_synth_sfload(synth, sf2.toUtf8().constData(), 1);
    if (sfid == FLUID_FAILED) {
        error = "Could not load a SoundFont (" + sf2 + ").\nTry: sudo apt install fluid-soundfont-gm";
        return;
    }
    // No "pipewire": its driver in FluidSynth 2.3.x fails (missing pw_init); PulseAudio goes through PipeWire anyway.
    for (const char *drv : {"pulseaudio", "alsa"}) {
        fluid_settings_setstr(settings, "audio.driver", drv);
        if ((audio = new_fluid_audio_driver(settings, synth))) break;
    }
    if (!audio) {
        error = "Could not open any audio output (PulseAudio/PipeWire or ALSA).";
        return;
    }
    seq = new_fluid_sequencer2(0);  // clocked by the synth's audio, so the beat never drifts
    synthDest = fluid_sequencer_register_fluidsynth(seq, synth);

    fluid_sfont_t *sf = fluid_synth_get_sfont_by_id(synth, sfid);
    fluid_sfont_iteration_start(sf);
    while (fluid_preset_t *p = fluid_sfont_iteration_next(sf)) {
        int bank = fluid_preset_get_banknum(p);
        presets.push_back({QString(bank == 128 ? "Drums: %1" : "%1").arg(fluid_preset_get_name(p)),
                           bank * 128 + fluid_preset_get_num(p)});
    }
    applyAll();
    select(0);
    timer.setTimerType(Qt::PreciseTimer);
    connect(&timer, &QTimer::timeout, this, &Engine::pump);
    reconnectMidi();
}

Engine::~Engine() {
    if (midi) delete_fluid_midi_driver(midi);  // stop MIDI callbacks before tearing down the synth
    if (seq) delete_fluid_sequencer(seq);
    if (audio) delete_fluid_audio_driver(audio);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

void Engine::applyAll() {
    for (size_t t = 0; t < song.tracks.size(); ++t) applyTrack(int(t));
}

void Engine::applyTrack(int t) { setupTrack(synth, sfid, song.tracks[t]); }

void Engine::select(int t) {
    sel = std::clamp(t, 0, int(song.tracks.size()) - 1);
    liveChannel = song.tracks[sel].channel;
}

int Engine::knob(int k) const {
    return k == kKnobs ? int(std::lround((song.bpm - 60) * 127 / 120.0)) : song.tracks[sel].knobs[k];
}

void Engine::setKnob(int k, int v) {
    if (k == kKnobs) {
        song.bpm = 60 + int(std::lround(v * 120 / 127.0));  // 60..180 BPM
    } else {
        song.tracks[sel].knobs[k] = v;
        fluid_synth_cc(synth, song.tracks[sel].channel, kKnobCC[k], v);
    }
    emit knobChanged(k, v);
}

void Engine::reconnectMidi() {
    if (midi) delete_fluid_midi_driver(midi);
    fluid_synth_all_notes_off(synth, -1);
    midi = new_fluid_midi_driver(settings, onMidi, this);
}

// Runs on FluidSynth's MIDI thread. Notes go straight to the synth for low lag;
// knob moves are handed to the GUI thread.
int Engine::onMidi(void *data, fluid_midi_event_t *ev) {
    auto *e = static_cast<Engine *>(data);
    int type = fluid_midi_event_get_type(ev), ch = fluid_midi_event_get_channel(ev);
    if (type == 0xC0) return FLUID_OK;  // keep the tracks' instruments; ignore MPK program changes
    if (type == 0xB0) {
        int cc = fluid_midi_event_get_control(ev), v = fluid_midi_event_get_value(ev);
        if (cc != 1 && cc != 64) {  // mod wheel and sustain pedal play through as usual
            QMetaObject::invokeMethod(e, [e, cc, v] { e->handleCC(cc, v); }, Qt::QueuedConnection);
            return FLUID_OK;
        }
    }
    if (type == 0x90 && fluid_midi_event_get_velocity(ev) > 0) {
        int key = fluid_midi_event_get_key(ev);
        unsigned tick = e->now();  // stamp it here: the GUI thread may get to it a few ms later
        QMetaObject::invokeMethod(e, [e, key, tick, ch] {
            emit e->noteHit(noteName(key));
            if (ch != 9) emit e->keyPressed(key, tick);
        }, Qt::QueuedConnection);
    }
    // Pads on channel 10 stay drums; everything else plays the selected track's sound.
    if (ch != 9 && type >= 0x80 && type <= 0xE0) fluid_midi_event_set_channel(ev, e->liveChannel);
    return fluid_synth_handle_midi_event(e->synth, ev);
}

void Engine::handleCC(int cc, int v) {
    emit ccSeen(cc, v);
    if (learn >= 0) {
        for (int &m : ccMap) if (m == cc) m = -1;  // one MPK knob drives one setting
        ccMap[learn] = cc;
        learn = -1;
    }
    for (int k = 0; k <= kKnobs; ++k)
        if (ccMap[k] == cc) setKnob(k, knobReader.read(cc, v, knob(k)));
}

void Engine::play(bool song_) {
    stop();
    songMode = song_;
    pos = 0;
    nextMs = fluid_sequencer_get_tick(seq) + 50;
    timer.start(20);
    pump();
}

unsigned Engine::now() const { return seq ? fluid_sequencer_get_tick(seq) : 0; }

// pump() has scheduled up to step `pos`, which starts at `nextMs`; count back from there.
double Engine::stepAt(unsigned tick) const { return pos - (nextMs - tick) / (15000.0 / song.bpm); }

void Engine::setup(const Track &t) { setupTrack(synth, sfid, t); }

void Engine::liveNote(int key, bool on) {
    if (!on) return void(fluid_synth_noteoff(synth, liveChannel, key));
    fluid_synth_noteon(synth, liveChannel, key, 100);
    emit noteHit(noteName(key));
    emit keyPressed(key, now());
}

// ponytail: fixed channel 14; clashes only if a song grows to 14+ tracks.
void Engine::preview(int key, bool on) {
    if (!on) return void(fluid_synth_noteoff(synth, 14, key));
    fluid_synth_program_select(synth, 14, sfid, 0, 0);  // Grand Piano
    fluid_synth_cc(synth, 14, 91, 40);
    fluid_synth_noteon(synth, 14, key, 90);
}

void Engine::stop() {
    timer.stop();
    ++generation;
    if (seq) fluid_sequencer_remove_events(seq, -1, synthDest, -1);
    fluid_synth_all_notes_off(synth, -1);
    emit position(-1, 0, -1, 0);
}

// Schedules every step that starts within the next 150 ms, sample-accurate via the sequencer.
void Engine::pump() {
    unsigned now = fluid_sequencer_get_tick(seq);
    fluid_event_t *ev = new_fluid_event();
    fluid_event_set_source(ev, -1);
    fluid_event_set_dest(ev, synthDest);
    while (nextMs < now + 150) {
        double stepMs = 15000.0 / song.bpm;
        StepInfo info;
        auto note = [&](const Track &t, int key, int vel, int gate) {
            if (&t != extra && (backing == Nothing || (backing == DrumsOnly && !t.drum))) return;
            fluid_event_note(ev, t.channel, key, vel, unsigned(gate * stepMs * 0.92));
            fluid_sequencer_send_at(seq, ev, unsigned(nextMs + (pos % 2 ? song.swing / 100.0 * stepMs : 0)), 1);
        };
        bool more = songStep(song, pos, songMode, info, note);
        if (extra && more && !songMode) trackStep(song, *extra, pos, note);
        if (click && more && pos % 4 == 0) {  // wood block on every beat, higher on beat 1
            fluid_event_note(ev, 9, pos % 16 ? 77 : 76, 110, 60);
            fluid_sequencer_send_at(seq, ev, unsigned(nextMs), 1);
        }
        int delay = std::max(0, int(nextMs - now)), gen = generation;
        if (!more) {
            timer.stop();
            QTimer::singleShot(delay + 3000, this, [this, gen] { if (gen == generation) stop(); });
            break;
        }
        QTimer::singleShot(delay, this, [this, gen, info] {
            if (gen == generation) emit position(info.section, info.bar, info.step, info.songBar);
        });
        nextMs += stepMs;
        ++pos;
    }
    delete_fluid_event(ev);
}

// Renders the whole arrangement offline (faster than real time) with a private synth.
bool Engine::exportAudio(const QString &path, QString &why) {
    struct Ev { long sample; bool on; int ch, key, vel; };
    std::vector<Ev> evs;
    const double stepSamples = 44100 * 15.0 / song.bpm;
    StepInfo info;
    for (long p = 0; songStep(song, p, true, info, [&](const Track &t, int key, int vel, int gate) {
             long start = long((p + (p % 2 ? song.swing / 100.0 : 0)) * stepSamples);
             evs.push_back({start, true, t.channel, key, vel});
             evs.push_back({start + long(gate * stepSamples * 0.92), false, t.channel, key, 0});
         }); ++p) {}
    if (evs.empty()) { why = "The song is silent: add some steps and switch tracks on in the arrangement."; return false; }
    std::stable_sort(evs.begin(), evs.end(), [](const Ev &a, const Ev &b) {
        return a.sample != b.sample ? a.sample < b.sample : !a.on && b.on;  // note-offs first
    });

    // File type from the extension. libsndfile does the encoding, including MP3 (via LAME).
    const QString ext = QFileInfo(path).suffix().toLower();
    int format = ext == "flac" ? SF_FORMAT_FLAC | SF_FORMAT_PCM_16
               : ext == "ogg"  ? SF_FORMAT_OGG | SF_FORMAT_VORBIS
               : ext == "mp3"  ? SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III
               : ext == "aiff" ? SF_FORMAT_AIFF | SF_FORMAT_PCM_16
               : ext == "wav"  ? SF_FORMAT_WAV | SF_FORMAT_PCM_16 : 0;
    if (!format) { why = "Unknown file type ." + ext + ": use .wav, .mp3, .ogg, .flac or .aiff"; return false; }
    SF_INFO sfinfo{};
    sfinfo.samplerate = 44100, sfinfo.channels = 2, sfinfo.format = format;
    SNDFILE *out = sf_open(QFile::encodeName(path).constData(), SFM_WRITE, &sfinfo);
    if (!out) { why = "Could not write " + path + ": " + sf_strerror(nullptr); return false; }
    double quality = 0.2;  // 0 = best quality / biggest file, 1 = smallest; ~190 kbps MP3, ~q7 Ogg
    sf_command(out, SFC_SET_COMPRESSION_LEVEL, &quality, sizeof quality);

    fluid_settings_t *s = makeSettings();
    fluid_synth_t *syn = new_fluid_synth(s);
    addKnobMods(syn);
    int id = fluid_synth_sfload(syn, sf2.toUtf8().constData(), 1);
    bool ok = id != FLUID_FAILED;
    if (ok) {
        for (const Track &t : song.tracks) setupTrack(syn, id, t);
        const int block = 64;
        std::vector<float> buf(2 * block);
        long end = evs.back().sample + 4 * 44100;  // let the reverb tail ring out
        size_t i = 0;
        for (long t = 0; ok && t < end; t += block) {
            for (; i < evs.size() && evs[i].sample <= t; ++i)
                evs[i].on ? fluid_synth_noteon(syn, evs[i].ch, evs[i].key, evs[i].vel)
                          : fluid_synth_noteoff(syn, evs[i].ch, evs[i].key);
            fluid_synth_write_float(syn, block, buf.data(), 0, 2, buf.data(), 1, 2);  // interleaved stereo
            ok = sf_writef_float(out, buf.data(), block) == block;
        }
        if (!ok) why = QString("Could not write ") + path + ": " + sf_strerror(out);
    } else {
        why = "Could not load the SoundFont " + sf2;
    }
    sf_close(out);
    delete_fluid_synth(syn);
    delete_fluid_settings(s);
    if (!ok) QFile::remove(path);  // don't leave a half-written file behind
    return ok;
}
