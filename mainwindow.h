#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "synthesis.h"
#include "programmer.h"
#include "fancybutton.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showSynthScreen();
    void showProgScreen();

private:
    Ui::MainWindow *ui;
    Synthesis* _synth;
    Programmer* _prog;

    FancyButton* _synthTabBtn;
    FancyButton* _progTabBtn;
};
#endif // MAINWINDOW_H
