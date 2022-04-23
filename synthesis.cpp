#include "synthesis.h"
#include "ui_synthesis.h"

Synthesis::Synthesis(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Synthesis)
{
    ui->setupUi(this);
}

Synthesis::~Synthesis()
{
    delete ui;
}
