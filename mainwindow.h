#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "synthesis.h"
#include "programmer.h"
#include "fancybutton.h"
#include "settingsdialog.h"

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
    SettingsDialog* _settings;

    FancyButton* _synthTabBtn;
    FancyButton* _progTabBtn;
    FancyButton* _settingsBtn;
};
#endif // MAINWINDOW_H
