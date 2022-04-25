#ifndef INSTALLER_H
#define INSTALLER_H

#include <QObject>
#include <QProcess>

class Installer : public QObject
{
    Q_OBJECT
public:
    explicit Installer(QString target, QObject *parent = nullptr);

    void run();

signals:
    void progressUpdated(int progress);
    void installed();
    void failed(int exitCode);

public slots:
    void updateProgress();
    void finishProgress(int exitCode);
    void onFailure(QProcess::ProcessError error);
    void terminate();

private:
    QString _exeName;
    QProcess* _instProc;

};

#endif // INSTALLER_H
