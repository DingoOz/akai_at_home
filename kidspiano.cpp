// Piano Pals: teaches primary school kids their first song (Jingle Bells) on the MPK mini Plus.
// Big coloured keys, a bouncing arrow on the next key, and the song as a road of coloured notes.
// It waits for the right key (any octave counts), so there is no rushing and no "game over".
#include "kids_song.h"
#include "voice.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath>
#include <fluidsynth.h>
#include <set>


static void star(QPainter &p, QPointF c, double r, QColor fill, double turn = 0) {
    QPainterPath path;
    for (int i = 0; i < 10; ++i) {
        double a = turn - M_PI / 2 + i * M_PI / 5, rr = i % 2 ? r * 0.45 : r;
        QPointF pt = c + QPointF(std::cos(a) * rr, std::sin(a) * rr);
        i ? path.lineTo(pt) : path.moveTo(pt);
    }
    path.closeSubpath();
    p.setPen(QPen(fill.darker(150), 2));
    p.setBrush(fill);
    p.drawPath(path);
}

class Piano : public QWidget {
public:
    explicit Piano(const QString &sf2);
    ~Piano() override;
    QString error;

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private:
    static int onMidi(void *data, fluid_midi_event_t *ev);
    void press(int key, bool on, bool fromMouse);
    void go(int lesson);
    void listen(int i, int gen);
    void jingle();
    QRectF whiteRect(int i) const;
    QRectF blackRect(int i) const;
    int keyAt(QPointF pos) const;  // on-screen key index 0..12, or -1
    int target() const { return at < int(L[les].notes.size()) ? L[les].notes[at].pc : -2; }
    void drawRoad(QPainter &p);
    void drawKeys(QPainter &p);

    fluid_settings_t *settings = nullptr;
    fluid_synth_t *synth = nullptr;
    fluid_audio_driver_t *audio = nullptr;
    fluid_midi_driver_t *midi = nullptr;

    std::vector<Lesson> L = lessons();
    int les = 0, at = 0;  // current lesson and the note we are waiting for
    int listenAt = -1, listenGen = 0, mouseKey = -1, wrong = -1;
    bool done = false, demo = false;  // demo: the song is being played to us, keys only make sound
    std::set<int> down;  // on-screen keys held right now
    QElapsedTimer wrongT;
    Voice voice;
    QTimer anim;
    double t = 0;
    std::vector<QPointF> snow;
    QPushButton *back, *hear, *again, *next, *talk;
};

Piano::Piano(const QString &sf2) {
    settings = new_fluid_settings();
    fluid_settings_setnum(settings, "synth.gain", 0.7);
    fluid_settings_setstr(settings, "midi.driver", "alsa_seq");
    fluid_settings_setint(settings, "midi.autoconnect", 1);
    fluid_settings_setint(settings, "audio.period-size", 128);
    synth = new_fluid_synth(settings);
    if (fluid_synth_sfload(synth, sf2.toUtf8().constData(), 1) == FLUID_FAILED) {
        error = "Could not load a SoundFont (" + sf2 + ").\nTry: sudo apt install fluid-soundfont-gm";
        return;
    }
    for (const char *drv : {"pulseaudio", "alsa"}) {  // not "pipewire": broken in FluidSynth 2.3 (see ERRORS.md)
        fluid_settings_setstr(settings, "audio.driver", drv);
        if ((audio = new_fluid_audio_driver(settings, synth))) break;
    }
    if (!audio) { error = "Could not open any audio output."; return; }
    fluid_synth_program_change(synth, 0, 0);  // grand piano
    midi = new_fluid_midi_driver(settings, onMidi, this);

    setWindowTitle("Piano Pals");
    resize(1280, 820);
    setMinimumSize(900, 600);
    auto make = [this](const QString &text, const char *colour) {
        auto *b = new QPushButton(text, this);
        b->setFocusPolicy(Qt::NoFocus);
        b->setMinimumSize(170, 64);
        b->setStyleSheet(QString("QPushButton { font: bold 24px; color: white; background: %1; border: none;"
                                 " border-radius: 22px; padding: 6px 18px; }"
                                 "QPushButton:pressed { background: #333; } QPushButton:disabled { background: #bbb; }")
                             .arg(colour));
        return b;
    };
    back = make("◀ Back", "#7e57c2"), hear = make("♪ Listen", "#039be5");
    again = make("↺ Again", "#fb8c00"), next = make("Next ▶", "#43a047");
    auto *row = new QHBoxLayout;
    talk = make("Voice", "#ec407a");
    talk->setCheckable(true);
    talk->setStyleSheet(talk->styleSheet() + "QPushButton:!checked { background: #9e9e9e; }");
    talk->setToolTip("Read the instructions out loud");
    for (auto *b : {back, hear, again, talk}) row->addWidget(b);
    row->addStretch();
    row->addWidget(next);
    auto *l = new QVBoxLayout(this);
    l->addLayout(row);
    l->addStretch();
    connect(back, &QPushButton::clicked, this, [this] { go(les - 1); });
    connect(next, &QPushButton::clicked, this, [this] { go(les + 1); });
    connect(again, &QPushButton::clicked, this, [this] { go(les); });
    connect(hear, &QPushButton::clicked, this, [this] { voice.stop(), demo = true, listen(0, ++listenGen); });
    connect(talk, &QPushButton::toggled, this, [this](bool on) {
        talk->setText(on ? "Voice: on" : "Voice: off");
        QSettings().setValue("voice", on);
        voice.on = on;
        if (!on) voice.stop();
    });
    talk->setText("Voice: off");
    talk->setChecked(QSettings().value("voice", false).toBool());

    for (int i = 0; i < 60; ++i)
        snow.push_back({QRandomGenerator::global()->bounded(1.0), QRandomGenerator::global()->bounded(1.0)});
    connect(&anim, &QTimer::timeout, this, [this] {
        t += 0.033;
        for (QPointF &s : snow) {
            s.ry() += 0.0015;
            s.rx() += 0.0006 * std::sin(t + s.y() * 9);
            if (s.y() > 1) s = {QRandomGenerator::global()->bounded(1.0), 0};
        }
        update();
    });
    anim.start(33);
    go(0);
}

Piano::~Piano() {
    if (midi) delete_fluid_midi_driver(midi);  // stop MIDI callbacks before the synth goes
    if (audio) delete_fluid_audio_driver(audio);
    if (synth) delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

// FluidSynth's MIDI thread: play the sound right away (no lag), tell the GUI thread about keys.
int Piano::onMidi(void *data, fluid_midi_event_t *ev) {
    auto *p = static_cast<Piano *>(data);
    int type = fluid_midi_event_get_type(ev);
    if (type == 0xC0) return FLUID_OK;  // keep the piano sound whatever the MPK sends
    if (fluid_midi_event_get_channel(ev) == 9) return fluid_synth_handle_midi_event(p->synth, ev);  // pads: drums
    fluid_midi_event_set_channel(ev, 0);
    if (type == 0x90 || type == 0x80) {
        int key = fluid_midi_event_get_key(ev);
        bool on = type == 0x90 && fluid_midi_event_get_velocity(ev) > 0;
        QMetaObject::invokeMethod(p, [p, key, on] { p->press(key, on, false); }, Qt::QueuedConnection);
    }
    return fluid_synth_handle_midi_event(p->synth, ev);
}

void Piano::go(int lesson) {
    les = std::clamp(lesson, 0, int(L.size()) - 1);
    at = 0, done = false, wrong = -1, listenAt = -1, ++listenGen;
    back->setEnabled(les > 0);
    next->setEnabled(les + 1 < int(L.size()));
    demo = L[les].notes[0].pc >= 0;  // hear it first, then play it
    hear->setEnabled(demo);
    // Read the step out (if the voice is on), then play the demo.
    int gen = listenGen;
    voice.say(L[les].title + " " + L[les].say + (demo ? " Listen first." : ""), [this, gen] {
        if (demo && gen == listenGen) QTimer::singleShot(voice.on ? 300 : 900, this, [this, gen] { listen(0, gen); });
    });
}

void Piano::press(int key, bool on, bool fromMouse) {
    int d = fromMouse ? key - 60 : key % 12;  // MPK keys in any octave count as the matching on-screen key
    if (fromMouse) on ? fluid_synth_noteon(synth, 0, key, 100) : fluid_synth_noteoff(synth, 0, key);
    if (!on) { down.erase(d); return; }
    down.insert(d);
    if (done || demo) return;
    int want = target();
    if (want == -1 || d % 12 == want) {
        wrong = -1;
        if (++at < int(L[les].notes.size())) return;
        done = true, jingle();
        voice.say(les + 1 < int(L.size()) ? "Well done!" : "You played Jingle Bells! You are a star!");
        if (les + 1 < int(L.size()))  // a short cheer, then straight on (unless they pressed Again or Back meanwhile)
            QTimer::singleShot(2500, this, [this, l = les] { if (les == l && done) go(les + 1); });
    } else {
        wrong = d, wrongT.start();
        if (!voice.busy()) voice.say(QString("Try the %1 key.").arg(QString(kColourName[want]).toLower()));
    }
}

void Piano::listen(int i, int gen) {
    if (gen != listenGen) return;
    if (i >= int(L[les].notes.size())) { listenAt = -1, demo = false, voice.say("Your turn!"); return; }
    const Note &n = L[les].notes[i];
    int key = 60 + n.pc, ms = int(n.beats * 520);  // ponytail: fixed demo tempo (~115 BPM), slow it here for little ones
    listenAt = i;
    fluid_synth_noteon(synth, 0, key, 90);
    QTimer::singleShot(ms * 9 / 10, this, [this, key] { fluid_synth_noteoff(synth, 0, key); });
    QTimer::singleShot(ms, this, [this, i, gen] { listen(i + 1, gen); });
}

void Piano::jingle() {  // sleigh bells (General MIDI drum 83) and a happy run up the keys
    int run[] = {60, 64, 67, 72};
    for (int i = 0; i < 4; ++i)
        QTimer::singleShot(300 + i * 140, this, [this, i, run] {
            fluid_synth_noteon(synth, 9, 83, 120);
            fluid_synth_noteon(synth, 0, run[i], 80);
            QTimer::singleShot(600, this, [this, i, run] { fluid_synth_noteoff(synth, 0, run[i]); });
        });
}

QRectF Piano::whiteRect(int i) const {
    double kw = width() * 0.84 / 8;
    return {width() * 0.08 + i * kw, height() * 0.58, kw, height() * 0.38};
}

QRectF Piano::blackRect(int i) const {
    QRectF w = whiteRect(kBlackAfter[i]);
    double bw = w.width() * 0.6;
    return {w.right() - bw / 2, w.top(), bw, w.height() * 0.6};
}

int Piano::keyAt(QPointF pos) const {
    for (int i = 0; i < 5; ++i) if (blackRect(i).contains(pos)) return kBlack[i];
    for (int i = 0; i < 8; ++i) if (whiteRect(i).contains(pos)) return kWhite[i];
    return -1;
}

void Piano::mousePressEvent(QMouseEvent *e) {
    if ((mouseKey = keyAt(e->position())) >= 0) press(60 + mouseKey, true, true);
}

void Piano::mouseReleaseEvent(QMouseEvent *) {
    if (mouseKey >= 0) press(60 + mouseKey, false, true), mouseKey = -1;
}

static QFont bold(double px) {
    QFont f("Ubuntu");
    f.setPixelSize(std::max(8, int(px)));
    f.setBold(true);
    return f;
}

void Piano::drawRoad(QPainter &p) {
    const auto &notes = L[les].notes;
    int n = int(notes.size()), cur = listenAt >= 0 ? listenAt : at;
    double r = std::min(width() / 24.0, height() * 0.042), gap = r * 2.9, y = height() * 0.40;
    int fit = std::max(1, int((width() - 2 * r) / gap)), shown = std::min(n, fit);
    int first = std::clamp(cur - fit / 3, 0, std::max(0, n - fit));  // keep the next note near the left third
    double x0 = (width() - (shown - 1) * gap) / 2;
    for (int i = first; i < first + shown; ++i) {
        const Note &nt = notes[i];
        bool now = i == cur && !done, past = i < at;
        QPointF c(x0 + (i - first) * gap, y - (now ? 8 * std::abs(std::sin(t * 4)) : 0));
        double rr = r * (now ? 1.3 + 0.06 * std::sin(t * 6) : 1);
        QColor col = nt.pc < 0 ? QColor("#90a4ae") : kColour[nt.pc];
        if (past) col.setAlpha(120);
        p.setPen(QPen(now ? QColor(Qt::white) : col.darker(130), now ? 6 : 2));
        p.setBrush(col);
        p.drawEllipse(c, rr, rr);
        p.setPen(nt.pc == 4 ? QColor("#5d4037") : QColor(Qt::white));
        p.setFont(bold(rr * 0.95));
        p.drawText(QRectF(c.x() - rr, c.y() - rr, 2 * rr, 2 * rr), Qt::AlignCenter, nt.pc < 0 ? "?" : kName[nt.pc]);
        if (past) star(p, c - QPointF(0, rr * 1.45), r * 0.42, QColor("#ffd600"), t);
        p.setPen(QColor(now ? "#0d47a1" : "#37474f"));
        p.setFont(bold(r * (now ? 0.7 : 0.55)));
        p.drawText(QRectF(c.x() - gap / 2, y + r * 1.4, gap, r), Qt::AlignCenter, nt.word);
    }
    p.setPen(QColor("#37474f"));
    p.setFont(bold(r));
    if (first > 0) p.drawText(QPointF(r * 0.2, y + r * 0.3), "…");
    if (first + shown < n) p.drawText(QPointF(width() - r * 1.2, y + r * 0.3), "…");
}

void Piano::drawKeys(QPainter &p) {
    int want = done ? -2 : listenAt >= 0 ? L[les].notes[listenAt].pc : demo ? -2 : target();
    bool oops = wrong >= 0 && wrongT.elapsed() < 700;
    double pulse = 0.5 + 0.5 * std::sin(t * 6);
    auto mix = [](QColor a, QColor b, double k) {
        return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * k, a.greenF() + (b.greenF() - a.greenF()) * k,
                                a.blueF() + (b.blueF() - a.blueF()) * k);
    };
    for (int i = 0; i < 8; ++i) {
        int d = kWhite[i];
        QRectF r = whiteRect(i);
        QColor fill = Qt::white;
        if (d == want) fill = mix(Qt::white, kColour[want], 0.35 + 0.5 * pulse);
        if (down.count(d) || (listenAt >= 0 && d == want)) fill = kColour[d % 12];
        if (oops && d == wrong) fill = QColor("#cfd8dc"), r.translate(5 * std::sin(t * 50), 0);
        p.setPen(QPen(QColor("#37474f"), 3));
        p.setBrush(fill);
        p.drawRoundedRect(r, 10, 10);
        double lr = r.width() * 0.26;
        QPointF lc(r.center().x(), r.bottom() - lr * 1.5);
        p.setPen(QPen(kColour[d % 12].darker(140), 2));
        p.setBrush(kColour[d % 12]);
        p.drawEllipse(lc, lr, lr);
        p.setPen(d % 12 == 4 ? QColor("#5d4037") : QColor(Qt::white));
        p.setFont(bold(lr * 1.1));
        p.drawText(QRectF(lc.x() - lr, lc.y() - lr, 2 * lr, 2 * lr), Qt::AlignCenter, kName[d % 12]);
        if (les >= 2 && d < 12 && finger(d)) {  // finger numbers once we have met our fingers
            p.setPen(QColor("#546e7a"));
            p.setFont(bold(lr * 0.9));
            p.drawText(QRectF(r.left(), lc.y() - lr * 3.2, r.width(), lr * 1.6), Qt::AlignCenter,
                       QString::number(finger(d)));
        }
    }
    for (int i = 0; i < 5; ++i) {
        QRectF r = blackRect(i);
        bool shake = oops && kBlack[i] == wrong;
        if (shake) r.translate(5 * std::sin(t * 50), 0);
        p.setPen(QPen(Qt::black, 2));
        p.setBrush(QColor(down.count(kBlack[i]) || shake ? "#78909c" : "#263238"));
        p.drawRoundedRect(r, 6, 6);
    }
    if (L[les].showC) {  // "two black keys together" bracket
        QRectF a = blackRect(0), b = blackRect(1);
        QRectF box(a.left() - 6, a.top() - 6, b.right() - a.left() + 12, a.height() + 12);
        p.setPen(QPen(QColor(255, 193, 7, int(120 + 135 * pulse)), 6));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(box, 10, 10);
        p.setPen(QColor("#e65100"));
        p.setFont(bold(height() * 0.028));
        p.drawText(QRectF(box.left() - 60, box.bottom() + 4, box.width() + 120, height() * 0.05), Qt::AlignCenter,
                   "2 black keys");
    }
    if (want < 0) return;
    QRectF k = whiteRect(std::find(kWhite, kWhite + 8, want) - kWhite);  // all song notes are white keys
    double aw = k.width() * 0.55, ah = height() * 0.065, tip = k.top() - 6 - 10 * std::abs(std::sin(t * 5));
    double x = k.center().x();
    QPolygonF arrow({{x, tip}, {x - aw / 2, tip - ah * .55}, {x - aw / 4, tip - ah * .55}, {x - aw / 4, tip - ah},
                     {x + aw / 4, tip - ah}, {x + aw / 4, tip - ah * .55}, {x + aw / 2, tip - ah * .55}});
    p.setPen(QPen(kColour[want].darker(160), 3));
    p.setBrush(kColour[want]);
    p.drawPolygon(arrow);
}

void Piano::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0, QColor("#81d4fa"));
    sky.setColorAt(1, QColor("#e1f5fe"));
    p.fillRect(rect(), sky);
    p.setPen(Qt::NoPen);
    for (size_t i = 0; i < snow.size(); ++i) {
        QPointF s(snow[i].x() * width(), snow[i].y() * height());
        if (done) star(p, s, 9, QColor::fromHsv(int(i * 37) % 360, 200, 255), t + i);
        else p.setBrush(QColor(255, 255, 255, 200)), p.drawEllipse(s, 4, 4);
    }
    const Lesson &ls = L[les];
    p.setPen(QColor("#0d47a1"));
    p.setFont(bold(height() * 0.055));
    p.drawText(QRectF(0, 88, width(), height() * 0.08), Qt::AlignCenter,
               QString("Step %1 of %2:  %3").arg(les + 1).arg(L.size()).arg(ls.title));
    bool oops = wrong >= 0 && wrongT.elapsed() < 1500 && target() >= 0;
    QString say = ls.say;
    QColor col("#263238");
    if (done) col = QColor("#2e7d32"), say = les + 1 < int(L.size()) ? "★ Well done! ★" : "★ You played Jingle Bells! You are a star! ★";
    else if (demo) col = QColor("#0277bd"), say = "Listen and watch first...";
    else if (oops) col = QColor("#e65100"), say = QString("Oops! Try the %1 key, %2.").arg(kColourName[target()], kName[target()]);
    else if (hear->isEnabled()) say = "Your turn! " + say;
    p.setPen(col);
    p.setFont(bold(height() * 0.032));
    p.drawText(QRectF(width() * 0.05, 88 + height() * 0.08, width() * 0.9, height() * 0.1),
               Qt::AlignCenter | Qt::TextWordWrap,
               say);
    drawRoad(p);
    drawKeys(p);
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setApplicationName("piano_pals"), app.setOrganizationName("akai_at_home");  // QSettings, voice cache
    QStringList candidates = {"/usr/share/sounds/sf2/FluidR3_GM.sf2", "/usr/share/sounds/sf2/default-GM.sf2"};
    if (argc > 1) candidates.prepend(argv[1]);
    QString sf2;
    for (const QString &c : candidates)
        if (QFile::exists(c)) { sf2 = c; break; }
    Piano w(sf2);
    if (!w.error.isEmpty()) return QMessageBox::critical(nullptr, "Piano Pals", w.error), 1;
    w.show();
    return app.exec();
}
