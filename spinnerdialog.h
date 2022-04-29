#ifndef SPINNERDIALOG_H
#define SPINNERDIALOG_H

#include <QDialog>
#include "spinnertask.h"

namespace Ui {
class SpinnerDialog;
}

class SpinnerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpinnerDialog(QWidget *parent = nullptr);
    ~SpinnerDialog();

    void addTask(QString taskName, int state = SpinnerTask::Pending);
    void next();
    void reset();
    int fail();

    int current() const;
    void setCurrent(int newCurrent);
    void setError(const QString& error);

private:
    void _refresh();

private:
    Ui::SpinnerDialog *ui;
    int _current;
};

#endif // SPINNERDIALOG_H
