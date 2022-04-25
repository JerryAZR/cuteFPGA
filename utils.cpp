#include "utils.h"
#include <qglobal.h>
#include <QDebug>
#include <QDir>
#include <QTextStream>

/**
 * @brief setYosysEnv
 *
 * :: Find root dir
 * @if not defined YOSYSHQ_ROOT (
 *    for /f %%i in ("%~dp0") do @set YOSYSHQ_ROOT=%%~fi
 *
 * @set prompt=[OSS CAD Suite] $p$g
 * @set SSL_CERT_FILE=%YOSYSHQ_ROOT%etc\cacert.pem
 *
 * @set PATH=%YOSYSHQ_ROOT%bin;%YOSYSHQ_ROOT%lib;%YOSYSHQ_ROOT%py3bin;%PATH%
 * @set PYTHON_EXECUTABLE=%YOSYSHQ_ROOT%py3bin\python3.exe
 * @set QT_PLUGIN_PATH=%YOSYSHQ_ROOT%lib\qt5\plugins
 * @set QT_LOGGING_RULES=*=false
 *
 * @set GTK_EXE_PREFIX=%YOSYSHQ_ROOT%
 * @set GTK_DATA_PREFIX=%YOSYSHQ_ROOT%
 * @set GDK_PIXBUF_MODULEDIR=%YOSYSHQ_ROOT%lib\gdk-pixbuf-2.0\2.10.0\loaders
 * @set GDK_PIXBUF_MODULE_FILE=%YOSYSHQ_ROOT%lib\gdk-pixbuf-2.0\2.10.0\loaders.cache
 * @gdk-pixbuf-query-loaders.exe --update-cache
 *
 */
void setYosysEnv()
{
    // Set yosys root
    QString yosysRoot = qgetenv("YOSYSHQ_ROOT");
    if (yosysRoot.length() > 0) {
        qInfo() << "YOSYSHQ_ROOT already set";
        qInfo() << yosysRoot;
        return;
    }

#ifdef Q_OS_WIN
    QChar separator(';');
#else
    QChar separator(':');
#endif

    QDir appDir = QDir();

    yosysRoot = appDir.absoluteFilePath("oss-cad-suite");
    qputenv("YOSYSHQ_ROOT", QDir::toNativeSeparators(yosysRoot).toUtf8());

    // Update PATH
    QString binDir = appDir.absoluteFilePath("oss-cad-suite/bin");
    QString libDir = appDir.absoluteFilePath("oss-cad-suite/lib");
    QString pyDir = appDir.absoluteFilePath("oss-cad-suite/py3bin");
    QString sysPath = qgetenv("PATH");
    sysPath.append(separator);
    sysPath.append(QDir::toNativeSeparators(binDir));
    sysPath.append(separator);
    sysPath.append(QDir::toNativeSeparators(libDir));
    sysPath.append(separator);
    sysPath.append(QDir::toNativeSeparators(pyDir));
    sysPath.append(separator);
    qputenv("PATH", sysPath.toUtf8());

    // Set python executable
#ifdef Q_OS_WIN
    QString pyPath = appDir.absoluteFilePath("py3bin/python3.exe");
    qputenv("PYTHON_EXECUTABLE", QDir::toNativeSeparators(pyPath).toUtf8());
#else
    // TODO: Add Linux code
#endif

    // Note that not all environments are set
    // I'll add those if necessary
}

void logHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
#ifdef QT_DEBUG
    QTextStream out(stdout);
    QString form("(%1:%2) -- %3");
    switch (type) {
        case QtDebugMsg:
            out << QString("DEBUG ")
                << form.arg(ctx.file, QString::number(ctx.line), msg) << Qt::endl;
            break;
        case QtInfoMsg:
            out << QString("INFO ")
                << form.arg(ctx.file, QString::number(ctx.line), msg) << Qt::endl;
            break;
        case QtWarningMsg:
            out << QString("WARNING ")
                << form.arg(ctx.file, QString::number(ctx.line), msg) << Qt::endl;
            break;
        case QtCriticalMsg:
            out << QString("CRITICAL ")
                << form.arg(ctx.file, QString::number(ctx.line), msg) << Qt::endl;
            break;
        case QtFatalMsg:
            out << QString("FATAL ")
                << form.arg(ctx.file, QString::number(ctx.line), msg) << Qt::endl;
            break;
    }

#else
    Q_UNUSED(type);
    Q_UNUSED(context);
    Q_UNUSED(msg);
#endif
}
