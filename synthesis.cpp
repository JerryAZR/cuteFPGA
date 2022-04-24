#include "synthesis.h"
#include "ui_synthesis.h"
#include <QFileDialog>
#include <QApplication>
#include <QStyle>
#include <QDebug>
#include <QCheckBox>

Synthesis::Synthesis(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Synthesis)
{
    ui->setupUi(this);
    ui->pcfBtn->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
}

Synthesis::~Synthesis()
{
    delete ui;
}

bool Synthesis::containsFile(const QString &fname)
{
    // We have set the file name as the object name when creating the object,
    // so findChild can help determine if the file already exists.
    QObject* target = this->findChild<QObject*>(fname);
    if (target) {
        // child found
        return true;
    } else {
        return false;
    }
}

QList<QCheckBox*> Synthesis::getSelectedFiles()
{
    QList<QCheckBox*> checkedList;

    foreach(QCheckBox* child, this->findChildren<QCheckBox*>()) {
        if (child && child->isChecked()) {
            checkedList.append(child);
        }
    }

    return checkedList;
}

void Synthesis::on_pcfBtn_clicked()
{
    QFileDialog fileDialog(this);
    QStringList pcfNames;

    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter("PCF Files (*.pcf)");

    if (fileDialog.exec() == QFileDialog::Accepted) {
        pcfNames = fileDialog.selectedFiles();
    }

    if (pcfNames.length() > 0) {
        ui->pcfName->setText(pcfNames[0]);
    }
}

void Synthesis::on_removeSelBtn_clicked()
{
    QList<QCheckBox*> selected = getSelectedFiles();

    foreach(QCheckBox* file, selected) {
        ui->fileListLayout->removeWidget(file);
        delete file;
    }
}

void Synthesis::on_addFileBtn_clicked()
{
    QFileDialog fileDialog(this);
    QStringList hdlNames;

    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setNameFilter("HDL Files (*.v *.sv)");

    if (fileDialog.exec() == QFileDialog::Accepted) {
        hdlNames = fileDialog.selectedFiles();
    }

    foreach(const QString& fname, hdlNames) {
        if (!containsFile(fname)) {
            QCheckBox* newEntry = new QCheckBox(fname, this);
            newEntry->setObjectName(fname);
            ui->fileListLayout->addWidget(newEntry);
        }
    }
}

