// Akai at Home: a beginner's step sequencer + guided trance lesson for the Akai MPK mini Plus.
#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    // Optional arg: path to any .sf2. Otherwise prefer the nicer FluidR3 set if installed.
    QStringList candidates = {"/usr/share/sounds/sf2/FluidR3_GM.sf2", "/usr/share/sounds/sf2/default-GM.sf2"};
    if (argc > 1) candidates.prepend(argv[1]);
    QString sf2;
    for (const QString &p : candidates)
        if (QFile::exists(p)) { sf2 = p; break; }

    Engine engine(sf2);
    if (!engine.error.isEmpty()) {
        QMessageBox::critical(nullptr, "Akai at Home", engine.error);
        return 1;
    }
    MainWindow win(engine);
    win.show();
    return app.exec();
}
