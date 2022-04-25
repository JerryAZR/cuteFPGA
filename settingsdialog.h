#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QMetaObject>
#include <QList>
#include "installer.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void clearConnections();
    void startWorking(QString work);

public slots:
    void init();
    void stopWorking(int returnCode = 0);

private slots:
    void on_installBtn_clicked();

private:
    Ui::SettingsDialog *ui;
    Installer* _installer;
    QList<QMetaObject::Connection> _activeConnections;
};

#endif // SETTINGSDIALOG_H
