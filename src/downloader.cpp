#include "downloader.h"
#include "utils.h"
#include <QStandardPaths>
#include <QDir>
#include <QUrl>

Downloader::Downloader(const QString& url, QObject *parent)
    : QObject{parent}
{
    busy = false;
    _url = url;
    _manager = new QNetworkAccessManager(this);
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
    QUrl url(_url);
    qInfo() << "Start download" << url;
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
}

void Downloader::finish()
{
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
    busy = false;
}

void Downloader::addData()
{
    _tmpFile->write(_reply->readAll());
}

void Downloader::updateProgress(qint64 received, qint64 total)
{
    emit progressUpdated(received * 100 / total);
}

void Downloader::terminate()
{
    qWarning() << "Download aborted";

    _reply->abort();
    _tmpFile->close();
    _tmpFile->remove();
    busy = false;
}

