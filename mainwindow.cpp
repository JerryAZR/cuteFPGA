#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "synthesis.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _synth = new Synthesis();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _synth;
}

