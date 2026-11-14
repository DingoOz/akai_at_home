// The step-by-step lesson that builds an ethereal trance track from an empty song.
#pragma once
#include "song.h"
#include <functional>
#include <vector>

struct GuideStep {
    QString title, html;
    int track;                           // track to show while on this step
    QStringList show;                    // UI parts to highlight (names in MainWindow::named)
    std::function<void(Song &)> doIt;    // "Do it for me"; empty if there is nothing to click
};

std::vector<GuideStep> guideSteps();  // the ethereal trance lesson

struct Lesson {
    QString name, blurb;
    std::function<Song()> start;                   // the lesson's starting song: tracks + arrangement, no notes
    std::function<std::vector<GuideStep>()> steps;
};
const std::vector<Lesson> &lessons();  // [0] is the trance lesson

// Shared lesson parts, so every style ends the same way.
GuideStep tempoStep(int bpm, int swing, const QString &why, int track);
GuideStep arrangeStep(const QString &sections, std::function<Song()> start, int track);
GuideStep finishStep(int track);
extern const char *kKnobHelp[kKnobs + 1];  // what each knob does, in plain words
extern const char *kChordNotes[7];         // the notes in each chord of kChordNames, e.g. "A C E"
