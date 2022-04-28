#include "synthesis.h"
#include "ui_synthesis.h"
#include "utils.h"
#include <QFileDialog>
#include <QApplication>
#include <QStyle>
#include <QDebug>
#include <QCheckBox>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

Synthesis::Synthesis(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Synthesis)
{
    ui->setupUi(this);

    // initialize log files
    QString workDir = getWorkDir();
    _yosysLog = new QFile(workDir + "/yosys.log", this);

    // Construct support list
    QFile configFile(":/config/support.json");
    configFile.open(QFile::ReadOnly);
    QJsonDocument config = QJsonDocument::fromJson(configFile.readAll());
    configFile.close();
    QStringList supportedArch;
    foreach(QJsonValueRef archRef, config.array()) {
        static const QString nameKey("display_name");
        static const QString archKey("yosys_name");
        static const QString typeKey("device_types");
        supportedArch.append(archRef[nameKey].toString());
        _architectures.append(archRef[archKey].toString());
        QStringList tmpList;
        foreach(QJsonValueRef typeRef, archRef[typeKey].toArray()) {
            tmpList.append(typeRef.toString());
        }
        _devices.append(tmpList);
    }
    ui->archSel->addItems(supportedArch);
    ui->typeSel->addItems(_devices[0]);
    // Initialize runners
    _yosysRunner = new QProcess(this);
    _yosysRunner->setProcessChannelMode(QProcess::MergedChannels);
    _yosysRunner->setWorkingDirectory(workDir);

    connect(_yosysRunner, SIGNAL(readyRead()), this, SLOT(updateSynth()));
    connect(_yosysRunner, SIGNAL(finished(int)), this, SLOT(finishSynth(int)));
    connect(_yosysRunner, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onFailure(QProcess::ProcessError)));
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

void Synthesis::runSynth()
{
    static QString yosysCmd("synth_%1 -top %2 -json out.json");
    QStringList cmd;
    // get all files
    cmd.append("-p");
    int archIdx = ui->archSel->currentIndex();
    cmd.append(yosysCmd.arg(_architectures[archIdx], "fpga_top"));
    foreach(QCheckBox* child, this->findChildren<QCheckBox*>()) {
        cmd.append(child->text());
    }
    // open log file
    _yosysLog->open(QFile::WriteOnly);
    _yosysRunner->start("yosys", cmd);
}

void Synthesis::runPnR()
{

}

void Synthesis::runPack()
{

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


void Synthesis::on_synthBtn_clicked()
{
    runSynth();
}

void Synthesis::updateSynth()
{
    QString log = _yosysRunner->readAll();
    _yosysLog->write(log.toUtf8());
}

void Synthesis::finishSynth(int exitCode)
{
    if (exitCode == 0) {
        // Program ended normally
        qInfo() << "Synthesis complete";
    } else {
        qCritical() << "Terminated. Return code:" << exitCode;
        qCritical() << _yosysRunner->errorString();
    }
    _yosysLog->close();
}

void Synthesis::onFailure(QProcess::ProcessError error)
{
    qCritical() << "Error:";
    qCritical() << error;
    qCritical() << _yosysRunner->errorString();

    // close all files
    _yosysLog->close();
}


void Synthesis::on_archSel_currentIndexChanged(int index)
{
    ui->typeSel->clear();
    ui->typeSel->addItems(_devices[index]);
}

