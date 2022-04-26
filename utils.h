#ifndef UTILS_H
#define UTILS_H

#include <QObject>

void setYosysEnv();
void logHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);
void removeWorkDir();
QString getTargetName(const QDate& yesterday);
QString getTargetUrl(const QDate& yesterday);
QString getTargetPath(const QDate& yesterday);
QString getTargetFromPath(const QString& path);
QString getWorkDir();

#endif // UTILS_H
