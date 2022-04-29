#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include <QWidget>
#include <QCheckBox>
#include <QProcess>
#include <QList>
#include <QFile>
#include "spinnerdialog.h"

namespace Ui {
class Synthesis;
}

class Synthesis : public QWidget
{
    Q_OBJECT

public:
    explicit Synthesis(QWidget *parent = nullptr);
    ~Synthesis();

private:
    bool containsFile(const QString& fname);
    QList<QCheckBox*> getSelectedFiles();
    void runSynth();
    void runPnR();
    void runPack();

private slots:
    void on_pcfBtn_clicked();
    void on_removeSelBtn_clicked();
    void on_removeAllBtn_clicked();
    void on_addFileBtn_clicked();
    void on_synthBtn_clicked();

    void updateSynth();
    void finishSynth(int exitCode);
    void updatePnR();
    void finishPnR(int exitCode);
    void updatePack();
    void finishPack(int exitCode);
    void onFailure(QProcess::ProcessError error);

    void on_archSel_currentIndexChanged(int index);

private:
    Ui::Synthesis* ui;
    SpinnerDialog* _spinner;
    QProcess* _yosysRunner;
    QProcess* _pnrRunner;
    QProcess* _packRunner;
    QFile* _yosysLog;
    QFile* _pnrLog;
    QFile* _packLog;
    QStringList _yosysArchs;
    QStringList _pnrArchs;
    QStringList _packers;
    QStringList _packerOptions;
    QStringList _packerFormats;
    QList<QStringList> _devices;
};

#endif // SYNTHESIS_H
