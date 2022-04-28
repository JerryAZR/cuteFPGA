#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include <QWidget>
#include <QCheckBox>
#include <QProcess>
#include <QList>
#include <QFile>

namespace Ui {
class Synthesis;
}

class Synthesis : public QWidget
{
    Q_OBJECT

public:
    explicit Synthesis(QWidget *parent = nullptr);
    ~Synthesis();

    bool containsFile(const QString& fname);
    QList<QCheckBox*> getSelectedFiles();
    void runSynth();
    void runPnR();
    void runPack();

private slots:
    void on_pcfBtn_clicked();
    void on_removeSelBtn_clicked();
    void on_addFileBtn_clicked();
    void on_synthBtn_clicked();

    void updateSynth();
    void finishSynth(int exitCode);
    void onFailure(QProcess::ProcessError error);

private:
    Ui::Synthesis *ui;
    QProcess* _yosysRunner;
    QFile* _yosysLog;
};

#endif // SYNTHESIS_H
