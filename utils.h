#ifndef UTILS_H
#define UTILS_H

#include <QObject>

void setYosysEnv();
void logHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);

#endif // UTILS_H
