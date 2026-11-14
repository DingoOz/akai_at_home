// Sound engine: FluidSynth synth + sequencer for the step patterns + MIDI input from the MPK.
#pragma once
#include "knobreader.h"
#include "song.h"
#include <QObject>
#include <QTimer>
#include <atomic>
#include <fluidsynth.h>

class Engine : public QObject {
    Q_OBJECT
public:
    explicit Engine(const QString &sf2);
    ~Engine() override;
    QString error;  // non-empty if the synth could not start
    QString sf2;

    Song song;
    std::vector<std::pair<QString, int>> presets;  // instrument name, bank * 128 + program
    std::array<int, kKnobs + 1> ccMap{70, 71, 72, 73, 74, 75, 76, 77};  // MPK knob CC -> knob index
    int learn = -1;  // knob waiting for an MPK knob to be turned
    KnobReader knobReader;

    void applyAll();               // instruments + knob values of every track to the synth
    void applyTrack(int t);
    void select(int t);            // the track the MPK keys and knobs play
    int selected() const { return sel; }
    void setKnob(int k, int v);    // on the selected track (k == kKnobs: tempo)
    int knob(int k) const;
    void play(bool songMode);
    void stop();
    bool playing() const { return timer.isActive(); }
    void reconnectMidi();
    bool exportAudio(const QString &path, QString &why);  // format from extension: wav mp3 ogg flac aiff

    // Melody maker: its own melody (not part of the song) played on top of the loop,
    // which song tracks play along, and a metronome click.
    const Track *extra = nullptr;
    enum Backing { All, DrumsOnly, Nothing } backing = All;
    bool click = false;
    void setup(const Track &t);              // instrument + knobs of a track that isn't in the song
    void playLiveOn(int ch) { liveChannel = ch; }  // where the MPK keys go (select() resets it)
    void liveNote(int key, bool on);
    void preview(int key, bool on);  // piano on its own channel, for the chord explainer         // on-screen keyboard: like pressing an MPK key
    double stepAt(unsigned tick) const;  // global step (fractional) sounding at sequencer time `tick`
    unsigned now() const;

    static int onMidi(void *data, fluid_midi_event_t *ev);

signals:
    void position(int section, int bar, int step, long songBar);  // step -1: stopped
    void knobChanged(int k, int v);
    void noteHit(const QString &name);
    void keyPressed(int key, unsigned tick);  // MPK key (not a drum pad) pressed at sequencer time `tick`
    void ccSeen(int cc, int value);

private:
    void pump();
    void handleCC(int cc, int v);

    fluid_settings_t *settings = nullptr;
    fluid_synth_t *synth = nullptr;
    fluid_audio_driver_t *audio = nullptr;
    fluid_midi_driver_t *midi = nullptr;
    fluid_sequencer_t *seq = nullptr;
    fluid_seq_id_t synthDest = -1;
    int sfid = -1, sel = 0;
    std::atomic<int> liveChannel{9};
    QTimer timer;
    bool songMode = false;
    long pos = 0;
    double nextMs = 0;
    int generation = 0;  // drops playhead updates queued before a stop
};
