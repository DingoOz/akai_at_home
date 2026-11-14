// Step grid: one row per sound/note, 16 columns = one bar of 16th notes (the `page` bar). Click to toggle a step.
#pragma once
#include "song.h"
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <functional>

class Grid : public QWidget {
public:
    Track *track = nullptr;
    QStringList labels;
    int playhead = -1;
    int page = 0;  // which bar of a multi-bar pattern is shown
    std::vector<std::array<bool, kSteps * kMaxBars>> hints;  // tutorial: steps still to click
    std::function<void()> onToggle;
    std::vector<bool> chordRows;  // chord helper: rows whose note is in the chosen chord

    QSize minimumSizeHint() const override { return {kLabelW + kSteps * 28, 6 * 26}; }

    void setTrack(Track *t, const QStringList &l) { track = t, labels = l, update(); }
    void setPlayhead(int s) { if (s != playhead) playhead = s, update(); }


protected:
    static constexpr int kLabelW = 80;
    int rows() const { return track ? track->rows() : 0; }
    double cellW() const { return (width() - kLabelW) / double(kSteps); }
    double cellH() const { return height() / double(std::max(1, rows())); }

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), palette().base());
        if (!track) return;
        const QColor on = track->drum ? QColor(255, 140, 60) : QColor(90, 170, 255);
        for (int r = 0; r < rows(); ++r) {
            double y = (rows() - 1 - r) * cellH();  // bottom row = lowest sound
            bool chordRow = r < int(chordRows.size()) && chordRows[r];
            const QColor green(80, 200, 120);
            if (chordRow) p.fillRect(QRectF(0, y + 1, width(), cellH() - 2), QColor(80, 200, 120, 45));
            p.setPen(chordRow ? green : palette().text().color());
            p.drawText(QRectF(4, y, kLabelW - 8, cellH()), Qt::AlignVCenter | Qt::AlignRight,
                       chordRow ? "● " + labels.value(r) : labels.value(r));
            for (int s = 0; s < kSteps; ++s) {
                QRectF c(kLabelW + s * cellW() + 2, y + 2, cellW() - 4, cellH() - 4);
                bool hit = track->steps[r][page * kSteps + s];
                QColor fill = hit ? on : palette().button().color();
                if (!hit && chordRow) fill = QColor(60, 110, 75);  // a safe note to click
                if (!hit && s / 4 % 2) fill = fill.darker(115);  // shade every other beat
                if (s == playhead) fill = fill.lighter(150);
                p.setPen(Qt::NoPen);
                p.setBrush(fill);
                p.drawRoundedRect(c, 4, 4);
                if (r < int(hints.size()) && hints[r][page * kSteps + s]) {
                    p.setPen(QPen(QColor(255, 200, 0), 2, Qt::DashLine));
                    p.setBrush(Qt::NoBrush);
                    p.drawRoundedRect(c.adjusted(1, 1, -1, -1), 4, 4);
                }
            }
        }
    }

    void mousePressEvent(QMouseEvent *e) override {
        if (!track || e->position().x() < kLabelW) return;
        int s = int((e->position().x() - kLabelW) / cellW());
        int r = rows() - 1 - int(e->position().y() / cellH());
        if (s < 0 || s >= kSteps || r < 0 || r >= rows()) return;
        track->steps[r][page * kSteps + s] = !track->steps[r][page * kSteps + s];
        update();
        if (onToggle) onToggle();
    }
};
