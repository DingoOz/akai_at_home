// Tiny picture under each knob showing what it does to the sound. Drawn from the dial's value (0..127),
// shapes are illustrative, not measured.
#pragma once
#include "song.h"
#include <QDial>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include <cmath>

class KnobGraph : public QWidget {
    QDial *dial;
    int kind;  // knob index, see kKnobNames

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setPen(palette().mid().color());
        p.drawRect(rect().adjusted(0, 0, -1, -1));
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(3, 3, -3, -3);
        const double v = dial->value() / 127.0, W = r.width(), H = r.height();
        const QColor c = palette().highlight().color();
        auto at = [&](double x, double y) { return QPointF(r.left() + x * W, r.bottom() - y * H); };  // 0..1 coords
        QPainterPath path;
        auto curve = [&](auto f) {  // y = f(x), x and y in 0..1
            path.moveTo(at(0, f(0.0)));
            for (double x = 0; x <= 1.0; x += 1.0 / W) path.lineTo(at(x, f(x)));
        };
        switch (kind) {
        case 0:  // Filter: a low-pass curve, highs above the cutoff fade away
        case 1: {  // Resonance: a peak at the cutoff
            const double cut = kind == 0 ? 0.08 + 0.85 * v : 0.55, peak = kind == 1 ? v : 0.0;
            curve([&](double x) {
                double d = (x - cut) / 0.08;
                return std::clamp(0.6 / std::sqrt(1 + std::pow((x / cut), 8)) + 0.4 * peak * std::exp(-d * d), 0.0, 1.0);
            });
            break;
        }
        case 2: {  // Attack: how slowly a note fades in
            const double len = 0.05 + 0.9 * v;
            path.moveTo(at(0, 0)), path.lineTo(at(len, 1)), path.lineTo(at(1, 1));
            break;
        }
        case 3: {  // Release: how long a note rings after you let go
            const double len = 0.05 + 0.9 * v;
            curve([&](double x) { return std::exp(-4 * x / len); });
            break;
        }
        case 4:  // Reverb: the dry hit, then the echo of the room
            p.setPen(QPen(c, 2));
            p.drawLine(at(0.06, 0), at(0.06, 1));
            curve([&](double x) { return x < 0.12 ? 0.0 : 0.7 * v * std::exp(-3 * (x - 0.12)); });
            break;
        case 5:  // Chorus: copies of the sound, slightly out of tune, spreading wider
            for (double d : {-1.0, 1.0}) {
                curve([&](double x) { return 0.5 + 0.4 * std::sin(x * 6 * M_PI * (1 + d * 0.15 * v)); });
                p.setPen(QPen(QColor(c.red(), c.green(), c.blue(), 110), 1.5));
                p.drawPath(path);
                path = {};
            }
            curve([](double x) { return 0.5 + 0.4 * std::sin(x * 6 * M_PI); });
            break;
        case 6:  // Volume: how big the wave is
            curve([&](double x) { return 0.5 + 0.48 * v * std::sin(x * 4 * M_PI); });
            break;
        default: {  // Tempo: one tick per beat in a fixed stretch of time
            const double bpm = 60 + std::lround(v * 120);
            p.setPen(QPen(c, 2));
            for (double t = 0.05; t < 1; t += 60.0 / bpm / 1.5) p.drawLine(at(t, 0.15), at(t, 0.85));  // 1.5 s wide
            return;
        }
        }
        if (kind == 2 || kind == 3) {  // filled envelope shapes
            QPainterPath fill = path;
            fill.lineTo(at(1, 0)), fill.lineTo(at(0, 0));
            p.fillPath(fill, QColor(c.red(), c.green(), c.blue(), 70));
        }
        p.setPen(QPen(c, 2));
        p.drawPath(path);
    }

public:
    KnobGraph(QDial *d, int k) : dial(d), kind(k) {
        setFixedSize(72, 34);
        static const char *tips[] = {
            "Loudness of low (left) to high (right) frequencies: the filter cuts off the highs",
            "A boost right at the filter's cutoff: higher = more whistly",
            "Volume of a note as it starts: a slow slope means it fades in",
            "Volume after you let go: a long tail means it rings on",
            "The note (spike) and the room's echo after it: higher = more echo",
            "Copies of the sound slightly out of tune: wider = shimmering and thicker",
            "How big the sound wave is: louder = bigger",
            "One line per beat: faster tempo = beats closer together",
        };
        setToolTip(tips[k]);
    }
};
