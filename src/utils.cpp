#include "utils.h"
#include <qglobal.h>
#include <QDebug>
#include <QDir>
#include <QTextStream>
#include <QStringList>
#include <QDate>
#include <QProcess>

const static char* appPath(".cutefpga");

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
    QDir appDir(QDir::home().absoluteFilePath(appPath));
    // Create the app root if not exist
    if (!appDir.exists()) {
        QDir::home().mkdir(appPath);
    }

    yosysRoot = appDir.absoluteFilePath("oss-cad-suite");
    qputenv("YOSYSHQ_ROOT", QDir::toNativeSeparators(yosysRoot).toUtf8());

    // Update PATH (prepend)
    QString binDir = appDir.absoluteFilePath("oss-cad-suite/bin");
    QString libDir = appDir.absoluteFilePath("oss-cad-suite/lib");
    QString pyDir = appDir.absoluteFilePath("oss-cad-suite/py3bin");
    QString sysPath;
    sysPath.append(QDir::toNativeSeparators(binDir));
    sysPath.append(separator);
    sysPath.append(QDir::toNativeSeparators(libDir));
    sysPath.append(separator);
    sysPath.append(QDir::toNativeSeparators(pyDir));
    sysPath.append(separator);
    sysPath.append(qgetenv("PATH"));
    qputenv("PATH", sysPath.toUtf8());

    // Set python executable
#ifdef Q_OS_WIN
    QString pyPath = appDir.absoluteFilePath("py3bin/python3.exe");
    qputenv("PYTHON_EXECUTABLE", QDir::toNativeSeparators(pyPath).toUtf8());
#else
    // TODO: Add Linux code
    qInfo() << "Modified path:" << sysPath;
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
    Q_UNUSED(ctx);
    Q_UNUSED(msg);
#endif
}


QString getWorkDir()
{
    return QDir::home().absoluteFilePath(appPath);
}

/**
 * @brief removeWorkDir
 *
 * Delete the data folder created on startup
 */
void removeWorkDir()
{
    QDir appDir(QDir::home().absoluteFilePath(appPath));
    // NOTE: Use a thread(pool) because this could take long
    appDir.removeRecursively();
}

/**
 * @brief getTargetName
 * @param date
 * @return File name of the target exe or tarball
 */
QString getTargetName(const QDate &date)
{
#if (defined Q_OS_WIN)
    return QString("oss-cad-suite-%1-%2-%3.%4")
            .arg("windows", "x64", date.toString("yyyyMMdd"), "exe");
#elif (defined Q_OS_LINUX)
    return QString("oss-cad-suite-%1-%2-%3.%4")
            .arg("linux", "x64", date.toString("yyyyMMdd"), "tgz");
#endif
}

QString getTargetName()
{
#if (defined Q_OS_WIN)
    return QString("oss-cad-suite-%1-%2.%3")
            .arg("windows", "x64", "exe");
#elif (defined Q_OS_LINUX)
    return QString("oss-cad-suite-%1-%2.%3")
            .arg("linux", "x64", "tgz");
#endif
}

/**
 * @brief getTargetUrl
 * @param date
 * @return URL of the target exe or tarball
 */
QString getTargetUrl(const QDate &date)
{
    // YYYY-MM-DD
    QString repo =
            QString("https://github.com/YosysHQ/oss-cad-suite-build/releases/download/%1-%2-%3/")
            .arg(date.year(), 4, 10, QLatin1Char('0'))
            .arg(date.month(), 2, 10, QLatin1Char('0'))
            .arg(date.day(), 2, 10, QLatin1Char('0'));

    return repo.append(getTargetName(date));
}

/**
 * @brief getTargetPath
 * @param date
 * @return Absolute path of the target exe or tarball
 */
QString getTargetPath()
{
    QDir appDir(QDir::home().absoluteFilePath(appPath));
    return appDir.absoluteFilePath(getTargetName());
}

/**
 * @brief getTargetFromPath
 * @param path
 * @return File name of the target with the given path
 *          (e.g. "/home/user/target.tgz" ==> "target.tgz")
 */
QString getTargetFromPath(const QString &path)
{
    QStringList separated = path.split("/");
    return separated.last();
}

/**
 * @brief nativeOpen
 *
 * Open target file with OS default application
 *
 * @param target
 */
void nativeOpen(const QString &target)
{
    QProcess* process = new QProcess();
    QProcess::connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
#if (defined Q_OS_WIN)
    QStringList options;
    options << "/c" << "start" << QDir::toNativeSeparators(target);
    process->start("cmd", options);
#elif (defined Q_OS_DARWIN)
    process->start("open", QStringList() << QDir::toNativeSeparators(target));
#elif (defined Q_OS_LINUX)
    process->start("xdg-open", QStringList() << QDir::toNativeSeparators(target));
#endif
}
