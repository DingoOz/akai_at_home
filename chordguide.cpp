#include "chordguide.h"
#include "guide.h"  // kChordNotes
#include <QGridLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <algorithm>

// ---------------------------------------------------------------- Piano
static int whiteIndex(int n) {  // white keys from C4 up to (not including) n
    int i = 0;
    for (int k = Piano::kLow; k < n; ++k) i += !Piano::black(k);
    return i;
}

QRectF Piano::keyRect(int n) const {
    const double w = whiteW(), h = height() - kTop;
    if (!black(n)) return {whiteIndex(n) * w, double(kTop), w, h};
    return {whiteIndex(n) * w - w * 0.3, double(kTop), w * 0.6, h * 0.6};  // straddles the white keys' edge
}

double Piano::centre(int n) const { return keyRect(n).center().x(); }

void Piano::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), palette().window());
    const QColor rootC(240, 140, 50), litC(80, 190, 110);
    QFont small = font();
    small.setPointSizeF(font().pointSizeF() * 0.85);
    for (bool blackPass : {false, true}) {  // white keys first, black ones on top
        for (int n = kLow; n <= kHigh; ++n) {
            if (black(n) != blackPass) continue;
            auto it = std::find(lit.begin(), lit.end(), n);
            QColor fill = it == lit.end() ? (blackPass ? QColor(30, 30, 30) : QColor(245, 245, 240))
                                          : (it == lit.begin() ? rootC : litC);
            if (n == pressed) fill = fill.darker(130);
            p.setPen(QPen(QColor(60, 60, 60), 1));
            p.setBrush(fill);
            p.drawRoundedRect(keyRect(n).adjusted(0.5, 0, -0.5, 0), 3, 3);
            if (!blackPass || it != lit.end()) {  // name every white key and every lit key
                QRectF r = keyRect(n);
                p.setFont(it != lit.end() ? QFont(font().family(), small.pointSize(), QFont::Bold) : small);
                p.setPen(blackPass || it != lit.end() ? Qt::white : QColor(90, 90, 90));
                if (it != lit.end() && !blackPass) p.setPen(Qt::black);
                p.drawText(r.adjusted(0, 0, 0, -6), Qt::AlignHCenter | Qt::AlignBottom, noteName(n).chopped(1));
            }
        }
    }
    p.setFont(QFont(font().family(), small.pointSize(), QFont::Bold));
    p.setPen(palette().text().color());
    for (size_t i = 1; i < lit.size(); ++i) {  // "3" between A and C: count every key, black ones too
        double a = centre(lit[i - 1]), b = centre(lit[i]);
        p.drawLine(QPointF(a, kTop - 6), QPointF(b, kTop - 6));
        p.drawText(QRectF(a, 0, b - a, kTop - 8), Qt::AlignCenter, QString::number(lit[i] - lit[i - 1]));
    }
}

void Piano::mousePressEvent(QMouseEvent *e) {
    for (bool blackFirst : {true, false})  // black keys sit on top
        for (int n = kLow; n <= kHigh; ++n)
            if (black(n) == blackFirst && keyRect(n).contains(e->position())) {
                pressed = n;
                if (onKey) onKey(n, true);
                return update();
            }
}

void Piano::mouseReleaseEvent(QMouseEvent *) {
    if (pressed >= 0 && onKey) onKey(pressed, false);
    pressed = -1;
    update();
}

// ---------------------------------------------------------------- Explainer
static const char *kExplainer = R"(
<h2>How chords work</h2>
<p>A <b>chord</b> is three (or more) notes played together. In this app every chord is built the same simple
way: pick a white key, <b>skip one</b>, take the next, <b>skip one</b>, take the next. So A, (skip B), C, (skip D),
E gives <b>A C E</b>: the chord <b>A minor</b>. The first note (orange on the keyboard) is the <b>root</b>, and
names the chord.</p>

<h3>Happy or sad? Count the keys</h3>
<p>The difference between a <b>major</b> (happy, bright) and a <b>minor</b> (sad, serious) chord is just the
gap between the first two notes. Count every key, black ones included (each key is one <b>half-step</b>):</p>
<ul>
<li><b>Minor</b> (written with an <b>m</b>, like <b>Am</b>): gaps of <b>3 then 4</b>. A&rarr;C is 3, C&rarr;E is 4.</li>
<li><b>Major</b> (just the letter, like <b>C</b>): gaps of <b>4 then 3</b>. C&rarr;E is 4, E&rarr;G is 3.</li>
<li><b>Diminished</b> (<b>dim</b>): <b>3 then 3</b>. Tense and unstable, used sparingly.</li>
</ul>
<p>Click the chord buttons below and look at the numbers above the keyboard.</p>

<h3>The key: seven chords that belong together</h3>
<p>All the songs here are in <b>A minor</b>, which uses only the <b>white keys</b>. Building a chord on each white
key gives exactly seven chords, and they all sound good together:
<b>Am, Bdim, C, Dm, Em, F, G</b>. Three are minor, three are major, one is diminished.
That's why the chord boxes in the top bar only offer these seven.</p>

<h3>A progression is a little journey</h3>
<p>A <b>chord progression</b> is the order the chords come in. Here each chord lasts one bar, and the four chord
boxes repeat round and round.</p>
<ul>
<li><b>Am is home.</b> Starting or ending on it feels settled, like resting.</li>
<li><b>Moving away</b> (to F, C, G...) builds a feeling of travel and tension.</li>
<li><b>Coming back</b> to Am feels like arriving. G and Em lead home especially strongly.</li>
<li><b>Shared notes make it smooth.</b> Am (A C E) and F (F A C) share two notes, so the change feels gentle.
Am to G shares none, so it's a bolder step.</li>
</ul>
<p>Each chord colours the same melody note differently. An E over Am sounds calm (E is in the chord), but the same
E over F sounds wistful and floating. That's why the green <b>Chord notes</b> rows change every bar.</p>

<h3>In this app</h3>
<p>Tracks with <b>Follow chords</b> ticked (bass, pads, arps) move with the progression automatically: you write
the pattern once, and it fits every chord. Melodies usually don't follow; instead you pick notes that suit each
bar's chord (green rows) for a calm sound, or other notes for tension.</p>
<p>Try the famous progressions below: press <b>Hear it</b>, and <b>Use in my song</b> to put one into the
chord boxes (you can always change them back).</p>
)";

// ---------------------------------------------------------------- Dialog
ChordGuide::ChordGuide(Engine &engine, std::function<void()> onSongChanged, QWidget *parent)
    : QDialog(parent), eng(engine), songChanged(std::move(onSongChanged)) {
    setWindowTitle("How chords work");
    resize(900, 900);
    auto *text = new QTextBrowser;
    QFont f = text->font();
    f.setPointSize(std::max(10, QSettings("akai_at_home", "studio").value("guideFont", 11).toInt() - 2));
    text->setFont(f);
    text->setHtml(kExplainer);

    piano = new Piano;
    piano->onKey = [this](int key, bool on) { silence(), eng.preview(key, on); };
    info = new QLabel("Click a chord, or a key on the piano to hear it.");
    info->setWordWrap(true);
    info->setTextFormat(Qt::RichText);

    auto *chordRow = new QHBoxLayout;
    for (int c = 0; c < 7; ++c) {
        auto *b = new QPushButton(QString("%1\n%2").arg(kChordNames[c], kChordNotes[c]));
        b->setMinimumHeight(48);
        connect(b, &QPushButton::clicked, this, [this, c] { silence(), showChord(c, true); });
        chordRow->addWidget(b);
    }

    struct Prog { const char *name; std::vector<int> chords; };
    const std::vector<Prog> progs = {
        {"Epic / trance", {0, 5, 2, 6}},
        {"Pop's \"four chords\"", {2, 6, 0, 5}},
        {"Heroic climb", {0, 6, 5, 6}},
        {"Classic minor", {0, 3, 4, 0}},
        {"Rising home", {5, 6, 0, 0}},
        {"Jazzy / lo-fi", {3, 6, 2, 0}},
    };
    auto *progGrid = new QGridLayout;
    auto addRow = [&](int row, const QString &name, std::function<std::vector<int>()> chords, bool canUse) {
        QStringList names;
        for (int c : chords()) names << kChordNames[c];
        progGrid->addWidget(new QLabel("<b>" + name + "</b>"), row, 0);
        auto *label = new QLabel(canUse ? names.join(" → ") : "whatever is in the chord boxes");
        progGrid->addWidget(label, row, 1);
        auto *hearBtn = new QPushButton("▶ Hear it");
        connect(hearBtn, &QPushButton::clicked, this, [this, chords] { hear(chords()); });
        progGrid->addWidget(hearBtn, row, 2);
        if (!canUse) return;
        auto *use = new QPushButton("Use in my song");
        connect(use, &QPushButton::clicked, this, [this, chords] {
            std::vector<int> c = chords();
            std::copy(c.begin(), c.end(), eng.song.chords.begin());
            songChanged();
            info->setText("Your song's chord boxes now say <b>" + QStringList([&] {
                QStringList l; for (int x : c) l << kChordNames[x]; return l; }()).join(" → ") + "</b>.");
        });
        progGrid->addWidget(use, row, 3);
    };
    addRow(0, "Your song right now", [this] { return std::vector<int>(eng.song.chords.begin(), eng.song.chords.end()); }, false);
    for (int i = 0; i < int(progs.size()); ++i)
        addRow(i + 1, progs[i].name, [c = progs[i].chords] { return c; }, true);
    progGrid->setColumnStretch(1, 1);

    auto *l = new QVBoxLayout(this);
    l->addWidget(text, 1);
    l->addWidget(piano);
    l->addWidget(info);
    l->addLayout(chordRow);
    l->addWidget(new QLabel("<b>Famous progressions</b> (one chord per bar):"));
    l->addLayout(progGrid);

    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, [this] {
        for (int k : sounding) eng.preview(k, false);
        sounding.clear();
        if (queue.empty()) return;
        showChord(queue.front(), true);
        queue.erase(queue.begin());
    });
    showChord(0, false);
}

void ChordGuide::showChord(int c, bool play) {
    auto k = chordKeys(c);
    piano->lit.assign(k.begin(), k.end());
    piano->update();
    int gap = k[1] - k[0];
    const char *kind = c == 1 ? "diminished: tense" : gap == 3 ? "minor: sad, serious" : "major: happy, bright";
    info->setText(QString("<b style='font-size:14pt'>%1</b> &nbsp; = %2 &nbsp; · gaps %3 + %4 half-steps &nbsp; · %5")
                      .arg(kChordNames[c], kChordNotes[c]).arg(gap).arg(k[2] - k[1]).arg(kind));
    if (!play) return;
    for (int n : k) eng.preview(n, true), sounding.push_back(n);
    timer.start(queue.empty() ? 1500 : 1200);
}

void ChordGuide::hear(std::vector<int> chords) {
    silence();
    queue = std::move(chords);
    timer.start(0);
}

void ChordGuide::silence() {
    timer.stop();
    queue.clear();
    for (int k : sounding) eng.preview(k, false);
    sounding.clear();
}
