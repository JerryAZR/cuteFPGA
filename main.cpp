#include "mainwindow.h"
#include "utils.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setYosysEnv();

    MainWindow w;
    w.show();
    return a.exec();
}
