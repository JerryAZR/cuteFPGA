#include "warndialog.h"
#include "ui_warndialog.h"
#include <QLabel>

WarnDialog::WarnDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WarnDialog)
{
    ui->setupUi(this);
}

WarnDialog::~WarnDialog()
{
    delete ui;
}

void WarnDialog::addLine(QString line)
{
    QLabel* label = new QLabel(line, this);
    ui->mainLayout->addWidget(label);
}
