// Optional spoken instructions. Uses Piper (natural voice, model in ~/.local/share/piper/*.onnx) when
// installed, otherwise speech-dispatcher's spd-say. Piper phrases are cached as WAVs in ~/.cache/piano_pals.
#pragma once
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <functional>

class Voice {
public:
    bool on = false;
    ~Voice() { stop(); }
    bool busy() const { return proc && proc->state() != QProcess::NotRunning; }

    void stop() {
        if (!proc) return;
        proc->disconnect();
        proc->kill();
        proc->waitForFinished(500);
        delete proc;
        proc = nullptr;
    }

    // Says `text`, then runs `after` (straight away when the voice is off). A new say() cuts the old one off.
    void say(const QString &text, std::function<void()> after = {}) {
        stop();
        if (!on) {
            if (after) after();
            return;
        }
        proc = new QProcess;
        QObject::connect(proc, &QProcess::finished, proc, [after] {
            if (after) QMetaObject::invokeMethod(qApp, after, Qt::QueuedConnection);  // not inside proc's own signal
        });
        QString model = findModel(), piper = QStandardPaths::findExecutable("piper");
        if (piper.isEmpty()) piper = QStandardPaths::findExecutable("piper", {QDir::homePath() + "/.local/bin"});
        if (model.isEmpty() || piper.isEmpty()) {
            proc->start("spd-say", {"-w", "-r", "-15", text});
            return;
        }
        QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir().mkpath(dir);
        QString wav = dir + "/" + QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Md5).toHex() + ".wav";
        // ponytail: --length-scale 1.15 = a little slower than normal for young listeners; tune to taste.
        proc->start("sh", {"-c",
                           "[ -s \"$2\" ] || { \"$3\" -m \"$1\" -f \"$2.tmp\" --length-scale 1.15 2>/dev/null"
                           " && mv \"$2.tmp\" \"$2\"; }; exec pw-play \"$2\"",
                           "sh", model, wav, piper});
        proc->write(text.toUtf8());
        proc->closeWriteChannel();
    }

private:
    QProcess *proc = nullptr;
    static QString findModel() {
        QDir d(QDir::homePath() + "/.local/share/piper");
        QStringList m = d.entryList({"*.onnx"});
        return m.isEmpty() ? QString() : d.filePath(m.first());
    }
};
