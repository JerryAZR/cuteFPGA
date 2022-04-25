#include "mainwindow.h"
#include "utils.h"
#include "installer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setYosysEnv();
    Installer inst("oss-cad-suite-windows-x64-20220424.exe");
    inst.run();

    MainWindow w;
    w.show();
    return a.exec();
}
