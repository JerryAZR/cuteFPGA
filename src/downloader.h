#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QFile>
#ifdef Q_OS_WIN
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#else
#include <QProcess>
#endif

/**
 * @brief The Downloader class
 *
 * QtNetwork does not use the correct libssl version on linux
 * so we will use wget instead
 */
class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(const QString& url = "", QObject *parent = nullptr);

    const QString &target() const;
    void setTarget(const QString &newTarget);

    const QString &localFile() const;
    void setLocalFile(const QString &newLocalFile);

signals:
    void progressUpdated(int progress);
    void finished();

public slots:
    void run();
    void finish();
    void addData();
    void updateProgress(qint64 received, qint64 total);
    void terminate();
    void onFailure();

private:
    bool busy;
    QString _url;
    QString _localFile;
#ifdef Q_OS_WIN
    QFile* _tmpFile;
    QNetworkAccessManager* _manager;
    QNetworkReply* _reply;
#else
    QProcess* _process;
#endif
};

#endif // DOWNLOADER_H
