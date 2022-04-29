#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QWidget>
#include <QProcess>
#include <QFile>
#include "spinnerdialog.h"

namespace Ui {
class Programmer;
}

class Programmer : public QWidget
{
    Q_OBJECT

public:
    explicit Programmer(QWidget *parent = nullptr);
    ~Programmer();

    void runProg();

private slots:
    void updateProg();
    void finishProg(int returnCode);
    void onFailure(QProcess::ProcessError error);

    void on_progBtn_clicked();

    void on_binBtn_clicked();

private:
    Ui::Programmer *ui;
    SpinnerDialog* _spinner;
    QProcess* _progRunner;
    QFile* _progLog;
    QStringList _programmers;
    QList<bool> _progUsePython;
//    QList<QStringList> _progOptions; // Not implemented
};

#endif // PROGRAMMER_H
