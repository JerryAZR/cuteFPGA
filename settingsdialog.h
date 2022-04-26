#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QMetaObject>
#include <QList>
#include "installer.h"
#include "downloader.h"

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
    void startInstall();
    void startDownload();

private slots:
    void on_installBtn_clicked();

private:
    Ui::SettingsDialog *ui;
    Installer* _installer;
    Downloader* _downloader;
    QList<QMetaObject::Connection> _activeConnections;
};

#endif // SETTINGSDIALOG_H
