#include "downloader.h"
#include "utils.h"
#include <QStandardPaths>
#include <QDir>
#ifdef Q_OS_WIN
#include <QUrl>
#else
#include <QRegularExpression>
#endif

Downloader::Downloader(const QString& url, QObject *parent)
    : QObject{parent}
{
    busy = false;
    _url = url;
#ifdef Q_OS_WIN
    _manager = new QNetworkAccessManager(this);
#else
    _process = new QProcess(this);
    _process->setProcessChannelMode(QProcess::MergedChannels);
    _process->setWorkingDirectory(getWorkDir());
    connect(_process, SIGNAL(readyRead()), this, SLOT(addData()));
    connect(_process, SIGNAL(finished(int)), this, SLOT(finish()));
    connect(_process, SIGNAL(errorOccurred()), this, SLOT(onFailure()));
#endif
    QDir defaultDownload(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    _localFile = defaultDownload.absoluteFilePath(getTargetFromPath(url));

}

/*-----------------------------------------------*/
/*             Getters and setters               */
/*-----------------------------------------------*/
const QString &Downloader::target() const
{
    return _url;
}

void Downloader::setTarget(const QString &newTarget)
{
    _url = newTarget;
}

const QString &Downloader::localFile() const
{
    return _localFile;
}

void Downloader::setLocalFile(const QString &newLocalFile)
{
    _localFile = newLocalFile;
}

/**
 * @brief Downloader::run
 *
 * Start the download process
 */
void Downloader::run()
{
    if (busy) {
        qCritical() << "Download manager is busy";
        return;
    }
    busy = true;
    qInfo() << "Start download" << _url;
#ifdef Q_OS_WIN
    QUrl url(_url);
    QNetworkRequest request(url);
    // Not sure who's managing this pointer.
    // Since the IDE generates no warnings I'll assume this is good.
    _reply = _manager->get(request);
    connect(_reply, SIGNAL(readyRead()), this, SLOT(addData()));
    connect(_reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(updateProgress(qint64,qint64)));
    connect(_reply, SIGNAL(finished()), this, SLOT(finish()));
    // Open temporary file
    _tmpFile = new QFile(_localFile + ".tmp");
    _tmpFile->open(QFile::WriteOnly);
#else
    QStringList options;
    options << _url << "-O" << _localFile;
    qInfo() << "Running wget with options:" << options;
    _process->start("wget", options);
#endif
}

void Downloader::finish()
{
#ifdef Q_OS_WIN
    if (_reply->isOpen()) { // Normal exit
        _tmpFile->write(_reply->readAll());
        _tmpFile->close();
        _reply->close();
        // remove existing file to avoid conflict
        QFile::remove(_localFile);

        if (!_tmpFile->rename(_localFile)) {
            qCritical() << "Unable to rename file to" << _localFile;
        } else {
            qInfo() << "file saved to" << _localFile;
            emit finished();
        }
    } else {
        qWarning() << "Nothing to stop";
    }
#else
    qInfo() << "file saved to" << _localFile;
    emit finished();
#endif
    busy = false;
}

void Downloader::addData()
{
#ifdef Q_OS_WIN
    _tmpFile->write(_reply->readAll());
#else
    const static QRegularExpression re("(\\d+)%");
    QString tmp = _process->readAll();
    if (tmp.length() > 0) {
        QRegularExpressionMatch match = re.match(tmp);
        if (match.hasMatch()) {
            // Assume success because it's taken from a regex match
            int progress = match.captured(1).toInt();
            emit progressUpdated(progress);
        }
    }
#endif
}

void Downloader::updateProgress(qint64 received, qint64 total)
{
    emit progressUpdated(received * 100 / total);
}

void Downloader::terminate()
{
    qWarning() << "Download aborted";
#ifdef Q_OS_WIN
    _reply->abort();
    _tmpFile->close();
    _tmpFile->remove();
#else
    _process->kill();
#endif
    busy = false;
}

void Downloader::onFailure()
{
#ifndef Q_OS_WIN
    qCritical() << "Download failed";
    qCritical() << _process->errorString();
#endif
}

