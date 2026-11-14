// The bottom panels of the main window: MPK knobs and the song arrangement table.
#include "knobgraph.h"
#include "mainwindow.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSettings>
#include <QVBoxLayout>

// Drag up/down to turn (like music software), instead of QDial's circular drag,
// where a small slip across the bottom gap jumps straight from 0 to 127.
class VDial : public QDial {
    int startY = 0, startValue = 0;
    void mousePressEvent(QMouseEvent *e) override { startY = int(e->position().y()), startValue = value(); }
    void mouseMoveEvent(QMouseEvent *e) override { setValue(startValue + (startY - int(e->position().y())) / 2); }
    void mouseReleaseEvent(QMouseEvent *) override {}
};

QWidget *MainWindow::buildKnobs() {
    auto *w = new QWidget;
    auto *l = new QHBoxLayout(w);
    auto *knobs = new QGridLayout;
    for (int k = 0; k <= kKnobs; ++k) {
        auto *name = new QLabel(QString("<b>K%1</b> %2").arg(k + 1).arg(kKnobNames[k]));
        dials[k] = new VDial;
        dials[k]->setToolTip("Drag up or down, or use the mouse wheel");
        dials[k]->setRange(0, 127);
        dials[k]->setNotchesVisible(true);
        dials[k]->setFixedSize(64, 64);
        dialValues[k] = new QLabel;
        learnBtns[k] = new QPushButton;
        learnBtns[k]->setToolTip("Click, then turn the MPK knob you want to control this");
        connect(dials[k], &QDial::valueChanged, this, [this, k](int v) { eng.setKnob(k, v); });
        connect(learnBtns[k], &QPushButton::clicked, this, [this, k] {
            eng.learn = k;
            learnBtns[k]->setText("Turn a knob…");
        });
        knobs->addWidget(name, 0, k, Qt::AlignHCenter);
        knobs->addWidget(dials[k], 1, k, Qt::AlignHCenter);
        knobs->addWidget(knobGraphs[k] = new KnobGraph(dials[k], k), 2, k, Qt::AlignHCenter);
        knobs->addWidget(dialValues[k], 3, k, Qt::AlignHCenter);
        knobs->addWidget(learnBtns[k], 4, k);
    }
    refreshLearn();

    auto *info = new QVBoxLayout;
    knobHelp = new QLabel("Turn a knob on the MPK (or drag one here) to hear what it does to the <b>selected track</b>.");
    knobHelp->setWordWrap(true);
    knobHelp->setMinimumWidth(260);
    ccLabel = new QLabel("Last MPK knob: none yet");
    noteLabel = new QLabel("♪");
    noteLabel->setAlignment(Qt::AlignCenter);
    noteLabel->setStyleSheet("font-size: 36pt; font-weight: bold;");
    noteLabel->setToolTip("The last note you played on the MPK");
    info->addWidget(knobHelp, 1);
    info->addWidget(noteLabel);
    info->addWidget(ccLabel);

    QSettings settings("akai_at_home", "studio");
    auto *mode = new QComboBox;
    mode->addItems({"Auto-detect", "Normal (absolute)", "Endless (relative)"});
    mode->setToolTip("Endless MPK knobs send small steps, not positions. Auto-detect works it out after a couple of turns.");
    auto *speed = new QSpinBox;
    speed->setRange(1, 8);
    speed->setToolTip("How much an endless knob changes a setting per click. Lower = finer control.");
    connect(mode, &QComboBox::currentIndexChanged, this, [this](int i) {
        eng.knobReader.mode = KnobReader::Mode(i);
        QSettings("akai_at_home", "studio").setValue("knobMode", i);
    });
    connect(speed, &QSpinBox::valueChanged, this, [this](int v) {
        eng.knobReader.speed = v;
        QSettings("akai_at_home", "studio").setValue("knobSpeed", v);
    });
    mode->setCurrentIndex(settings.value("knobMode", 0).toInt());
    speed->setValue(settings.value("knobSpeed", 2).toInt());
    eng.knobReader.mode = KnobReader::Mode(mode->currentIndex());
    eng.knobReader.speed = speed->value();
    auto *row = new QHBoxLayout;
    row->addWidget(new QLabel("MPK knob type:"));
    row->addWidget(mode);
    row->addWidget(new QLabel("Speed:"));
    row->addWidget(speed);
    row->addStretch();
    info->addLayout(row);

    l->addLayout(knobs);
    l->addLayout(info, 1);
    return w;
}

QWidget *MainWindow::buildSong() {
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);
    l->addWidget(new QLabel("Each row is a section of the song. Tick which tracks play in it. Plays in <b>Song</b> mode."));
    table = new QTableWidget;
    table->verticalHeader()->hide();
    connect(table, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *it) {
        if (refreshing) return;
        Section &s = eng.song.sections[it->row()];
        if (it->column() == 0) s.name = it->text();
        else if (it->column() == 1) s.bars = qBound(1, it->text().toInt(), 64);
        else s.on[it->column() - 2] = it->checkState() == Qt::Checked;
        updateHints();
    });
    l->addWidget(table, 1);
    auto *add = new QPushButton("Add section"), *remove = new QPushButton("Remove selected section");
    connect(add, &QPushButton::clicked, this, [this] {
        eng.song.sections.push_back({"New section", 4, std::vector<bool>(eng.song.tracks.size(), true)});
        refreshTable();
    });
    connect(remove, &QPushButton::clicked, this, [this] {
        int r = table->currentRow();
        if (r < 0 || eng.song.sections.size() <= 1) return;
        eng.stop();
        eng.song.sections.erase(eng.song.sections.begin() + r);
        refreshTable();
    });
    auto *buttons = new QHBoxLayout;
    buttons->addWidget(add);
    buttons->addWidget(remove);
    buttons->addStretch();
    l->addLayout(buttons);
    return w;
}

void MainWindow::refreshTable() {
    refreshing = true;
    const Song &s = eng.song;
    QStringList headers{"Section", "Bars"};
    for (const Track &t : s.tracks) headers << t.name;
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount(int(s.sections.size()));
    int totalBars = 0;
    for (int r = 0; r < int(s.sections.size()); ++r) {
        table->setItem(r, 0, new QTableWidgetItem(s.sections[r].name));
        table->setItem(r, 1, new QTableWidgetItem(QString::number(s.sections[r].bars)));
        totalBars += s.sections[r].bars;
        for (int c = 0; c < int(s.tracks.size()); ++c) {
            auto *it = new QTableWidgetItem;
            it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
            it->setCheckState(s.sections[r].on[c] ? Qt::Checked : Qt::Unchecked);
            table->setItem(r, c + 2, it);
        }
    }
    table->setToolTip(QString("%1 bars, about %2 seconds at %3 BPM").arg(totalBars).arg(totalBars * 240 / s.bpm).arg(s.bpm));
    table->resizeColumnsToContents();
    refreshing = false;
    int playing = playingSection;
    playingSection = -1;
    showPlayingSection(playing);  // the rebuilt rows lost their highlight
}

void MainWindow::showPlayingSection(int section) {
    if (section == playingSection) return;
    refreshing = true;  // colouring items fires itemChanged
    for (int row : {playingSection, section}) {
        if (row < 0 || row >= table->rowCount()) continue;
        bool on = row == section;
        for (int c = 0; c < table->columnCount(); ++c) {
            QTableWidgetItem *it = table->item(row, c);
            if (!it) continue;
            it->setBackground(on ? QBrush(QColor(255, 200, 0, 90)) : QBrush());
            QFont f = it->font();
            f.setBold(on);
            it->setFont(f);
        }
    }
    playingSection = section;
    refreshing = false;
    if (section >= 0) table->scrollToItem(table->item(section, 0));
}

void MainWindow::refreshLearn() {
    for (int k = 0; k <= kKnobs; ++k)
        learnBtns[k]->setText(eng.learn == k ? "Turn a knob…"
                              : eng.ccMap[k] < 0 ? "Learn (none)" : QString("Learn (CC %1)").arg(eng.ccMap[k]));
}

void MainWindow::setGuideFont(int pt) {
    pt = qBound(8, pt, 32);
    QFont f = guideText->font();
    f.setPointSize(pt);
    guideText->setFont(f);
    stepTitle->setStyleSheet(QString("font-size: %1pt; font-weight: bold;").arg(pt + 4));
    QSettings("akai_at_home", "studio").setValue("guideFont", pt);
}
