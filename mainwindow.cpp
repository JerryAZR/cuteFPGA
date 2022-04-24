#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "fancybutton.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _synth = new Synthesis(this);
    _prog = new Programmer(this);
    _synthTabBtn = new FancyButton(":/icons/memory.svg", "Synthesis", this);
    _progTabBtn = new FancyButton(":/icons/usb-port.svg", "Program", this);
    _synthTabBtn->setCheckable(true);
    _progTabBtn->setCheckable(true);
    FancyButton* settingsBtn = new FancyButton(":/icons/cog.svg", "Settings", this);
    // Construct tool bar
    ui->mainTabBar->addWidget(_synthTabBtn);
    ui->mainTabBar->addWidget(_progTabBtn);
    ui->mainToolBar->addWidget(settingsBtn);

    // Initialize main layout contents
    ui->mainHLayout->addWidget(_synth);
    ui->mainHLayout->addWidget(_prog);

    // hide unused widgets and toggle selections
    _prog->hide();
    _synthTabBtn->setChecked(true);

    // bind signals and slots
    connect(_synthTabBtn, SIGNAL(clicked()), this, SLOT(showSynthScreen()));
    connect(_progTabBtn, SIGNAL(clicked()), this, SLOT(showProgScreen()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showSynthScreen()
{
    _synth->show();
    _prog->hide();

    _synthTabBtn->setChecked(true);
    _progTabBtn->setChecked(false);
}

void MainWindow::showProgScreen()
{
    _synth->hide();
    _prog->show();

    _synthTabBtn->setChecked(false);
    _progTabBtn->setChecked(true);
}

