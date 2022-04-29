#ifndef WARNDIALOG_H
#define WARNDIALOG_H

#include <QDialog>

namespace Ui {
class WarnDialog;
}

class WarnDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WarnDialog(QWidget *parent = nullptr);
    ~WarnDialog();

    void addLine(QString line);

private:
    Ui::WarnDialog *ui;
};

#endif // WARNDIALOG_H
