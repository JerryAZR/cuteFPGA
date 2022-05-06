#include "installer.h"
#include "utils.h"
#include <QDebug>
#include <QRegularExpression>
#include <QFile>

Installer::Installer(const QString& target, QObject *parent)
    : QObject{parent}
{
#ifdef Q_OS_WIN
    _exeName = target;
#else
    _exeName = "tar";
    _archive = target;
#endif
    _instProc = new QProcess(this);
    _instProc->setProcessChannelMode(QProcess::MergedChannels);
    _instProc->setWorkingDirectory(getWorkDir());

    connect(_instProc, SIGNAL(readyRead()), this, SLOT(updateProgress()));
    connect(_instProc, SIGNAL(finished(int)), this, SLOT(finishProgress(int)));
    connect(_instProc, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onFailure(QProcess::ProcessError)));
}

void Installer::updateProgress()
{
#ifdef Q_OS_WIN
    const static QRegularExpression re("(\\d+)%");
    QString tmp = _instProc->readAll();
    if (tmp.length() > 0) {
        QRegularExpressionMatch match = re.match(tmp);
        if (match.hasMatch()) {
            // Assume success because it's taken from a regex match
            int progress = match.captured(1).toInt();
            emit progressUpdated(progress);
        }
    }
#else
    const static QRegularExpression re("(\\d+)");
    QString tmp = _instProc->readAll();
    if (tmp.length() > 0) {
        QRegularExpressionMatch match = re.match(tmp);
        if (match.hasMatch()) {
            // Assume success because it's taken from a regex match
            int progress = match.captured(1).toInt();
            int estimate = (progress / 1600);
            estimate = estimate > 100 ? 100 : estimate;
            emit progressUpdated(estimate);
        }
    }
#endif
}

void Installer::finishProgress(int exitCode)
{
    if (exitCode == 0) {
        // Program ended normally
        emit installed();
    } else {
        qCritical() << "Terminated. Return code:" << exitCode;
        qCritical() << _instProc->errorString();
        emit failed(exitCode);
    }
    // Delete the installer exe or archive to save disk space
#ifdef Q_OS_WIN
    QFile::remove(_exeName);
#else
    QFile::remove(_archive);
#endif
}

void Installer::onFailure(QProcess::ProcessError error)
{
    qCritical() << "Error:";
    qCritical() << error;
    emit failed(error + 42); // some offset to help with troubleshooting
}

void Installer::terminate()
{
    _instProc->kill();
}

const QString &Installer::exeName() const
{
    return _exeName;
}

void Installer::setExeName(const QString &newExeName)
{
    _exeName = newExeName;
}


void Installer::run()
{
#ifdef Q_OS_WIN
    _instProc->start(_exeName, QStringList() << "-y");
#else
    QStringList options;
    options << "-zxf" << _archive << "--checkpoint=1600";
    _instProc->start(_exeName, options);
#endif
}
