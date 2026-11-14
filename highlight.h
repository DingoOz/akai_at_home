// Pulsing yellow outline drawn on top of the widgets the current tutorial step talks about.
#pragma once
#include <QPainter>
#include <QPointer>
#include <QTimer>
#include <QWidget>
#include <cmath>

class Highlighter : public QWidget {
public:
    explicit Highlighter(QWidget *host) : QWidget(host) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        connect(&timer, &QTimer::timeout, this, [this] {
            phase += 0.15;
            setGeometry(parentWidget()->rect());
            raise();
            QRegion now = region();
            update(now + last);  // repaint only around the outlines, not the whole window
            last = now;
        });
        timer.start(50);
    }
    void setTargets(const QList<QPointer<QWidget>> &t) { targets = t; }

protected:
    QRegion region() const {
        QRegion r;
        for (const QRect &rect : rects()) r += rect.adjusted(-4, -4, 4, 4);
        return r;
    }
    QList<QRect> rects() const {
        QList<QRect> out;
        for (const auto &w : targets)
            if (w && w->isVisible()) out << QRect(w->mapTo(parentWidget(), QPoint(0, 0)), w->size()).adjusted(-3, -3, 3, 3);
        return out;
    }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QColor c(255, 200, 0);
        c.setAlphaF(0.7 + 0.3 * std::sin(phase));
        p.setPen(QPen(c, 4));
        for (const QRect &r : rects()) p.drawRoundedRect(r, 6, 6);
    }

private:
    QList<QPointer<QWidget>> targets;
    QTimer timer;
    QRegion last;
    double phase = 0;
};
