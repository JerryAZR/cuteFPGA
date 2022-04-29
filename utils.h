#ifndef UTILS_H
#define UTILS_H

#include <QObject>

void setYosysEnv();
void logHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);
void removeWorkDir();
QString getTargetName(const QDate& yesterday);
QString getTargetName();
QString getTargetUrl(const QDate& yesterday);
QString getTargetPath();
QString getTargetFromPath(const QString& path);
QString getWorkDir();
void nativeOpen(const QString& target);

#endif // UTILS_H
