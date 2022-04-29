#ifndef INSTALLER_H
#define INSTALLER_H

#include <QObject>
#include <QProcess>

class Installer : public QObject
{
    Q_OBJECT
public:
    explicit Installer(const QString& target = "", QObject *parent = nullptr);

    void run();

    const QString &exeName() const;
    void setExeName(const QString &newExeName);

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
