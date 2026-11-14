// "How chords work": an explainer with a clickable piano keyboard that shows (and plays) each chord
// and lets you hear famous progressions and copy one into your song.
#pragma once
#include "engine.h"
#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <functional>

// Two octaves of piano, C4 to B5. Lit keys are drawn in colour (the root orange), with the number of
// half-steps between neighbouring lit keys written above them.
class Piano : public QWidget {
public:
    static constexpr int kLow = 60, kHigh = 83;
    std::vector<int> lit;  // sorted, lit[0] is the root
    std::function<void(int key, bool on)> onKey;

    QSize minimumSizeHint() const override { return {560, 170}; }
    static bool black(int n) { int pc = n % 12; return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10; }

protected:
    static constexpr int kTop = 24;  // strip above the keys for the half-step counts
    double whiteW() const { return width() / 14.0; }
    double centre(int n) const;  // x of a key's middle
    QRectF keyRect(int n) const;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    int pressed = -1;
};

class ChordGuide : public QDialog {
public:
    ChordGuide(Engine &engine, std::function<void()> songChanged, QWidget *parent);

private:
    void showChord(int chord, bool play);
    void hear(std::vector<int> chords);  // play a progression, one chord at a time
    void silence();

    Engine &eng;
    std::function<void()> songChanged;
    Piano *piano;
    QLabel *info;
    QTimer timer;
    std::vector<int> queue, sounding;
};
