#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QFile>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

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

private:
    bool busy;
    QString _url;
    QString _localFile;
    QFile* _tmpFile;
    QNetworkAccessManager* _manager;
    QNetworkReply* _reply;
};

#endif // DOWNLOADER_H
