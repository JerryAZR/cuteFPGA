#include "synthesis.h"
#include "ui_synthesis.h"
#include "warndialog.h"
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
    _pnrLog = new QFile(workDir + "/nextpnr.log", this);
    _packLog = new QFile(workDir + "/packer.log", this);

    // Construct support list
    QFile configFile(":/config/chips.json");
    configFile.open(QFile::ReadOnly);
    QJsonDocument config = QJsonDocument::fromJson(configFile.readAll());
    configFile.close();
    QStringList supportedArch;
    foreach(QJsonValueRef archRef, config.array()) {
        static const QString nameKey("display_name");
        static const QString archKey("yosys_name");
        static const QString pnrKey("nextpnr_name");
        static const QString typeKey("device_types");
        static const QString packKey("packer");
        static const QString formatKey("packer_format");
        static const QString packerOpKey("packer_option");
        supportedArch.append(archRef[nameKey].toString());
        _yosysArchs.append(archRef[archKey].toString());
        _pnrArchs.append(archRef[pnrKey].toString());
        _packers.append(archRef[packKey].toString());
        _packerFormats.append(archRef[formatKey].toString());
        _packerOptions.append(archRef[packerOpKey].toString());
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

    _pnrRunner = new QProcess(this);
    _pnrRunner->setProcessChannelMode(QProcess::MergedChannels);
    _pnrRunner->setWorkingDirectory(workDir);

    _packRunner = new QProcess(this);
    _packRunner->setProcessChannelMode(QProcess::MergedChannels);
    _packRunner->setWorkingDirectory(workDir);

    // set up spinner dialog and connect signals
    _spinner = new SpinnerDialog(this);
    _spinner->addTask("Synthesis");
    _spinner->addTask("Place & Route");
    _spinner->addTask("Bitstream Generation");
    connect(_spinner, SIGNAL(rejected()), _yosysRunner, SLOT(kill()));
    connect(_spinner, SIGNAL(rejected()), _pnrRunner, SLOT(kill()));
    connect(_spinner, SIGNAL(rejected()), _packRunner, SLOT(kill()));

    connect(_yosysRunner, SIGNAL(readyRead()), this, SLOT(updateSynth()));
    connect(_yosysRunner, SIGNAL(finished(int)), this, SLOT(finishSynth(int)));
    connect(_yosysRunner, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onFailure(QProcess::ProcessError)));

    connect(_pnrRunner, SIGNAL(readyRead()), this, SLOT(updatePnR()));
    connect(_pnrRunner, SIGNAL(finished(int)), this, SLOT(finishPnR(int)));
    connect(_pnrRunner, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onFailure(QProcess::ProcessError)));

    connect(_packRunner, SIGNAL(readyRead()), this, SLOT(updatePack()));
    connect(_packRunner, SIGNAL(finished(int)), this, SLOT(finishPack(int)));
    connect(_packRunner, SIGNAL(errorOccurred(QProcess::ProcessError)),
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
    const static QString yosysCmd("synth_%1 -top %2 -json out.json");
    QString topLevel = ui->topLevelLine->text();
    QStringList options;
    int archIdx = ui->archSel->currentIndex();
    // get all files
    options.append("-p");
    options.append(yosysCmd.arg(_yosysArchs[archIdx], topLevel));
    bool emptyFileList = true;
    foreach(QCheckBox* child, this->findChildren<QCheckBox*>()) {
        options.append(child->text());
        emptyFileList = false;
    }
    // Validate inputs
    WarnDialog warnBox;
    warnBox.setWindowIcon(QIcon(":/icons/alert.svg"));
    if (emptyFileList || topLevel.isEmpty()) {
        warnBox.setWindowTitle("Error");
        warnBox.addLine("Please fix the following error(s)");
        if (emptyFileList) {
            warnBox.addLine("No source file added.");
        }
        if (topLevel.isEmpty()) {
            warnBox.addLine("Top level module not specified.");
        }
        warnBox.exec();
        return;
    }
    // Warn about non-critical inputs
    if (ui->pcfName->text().isEmpty()) {
        warnBox.setWindowTitle("Warning");
        warnBox.addLine("No pin map file selected.");
        warnBox.addLine("Would you like to select one now?");
        if (warnBox.exec() == QDialog::Accepted) {
            on_pcfBtn_clicked();
            // check again
            if (ui->pcfName->text().isEmpty()) {
                return;
            }
        }
    }

    // open log file
    _yosysLog->open(QFile::WriteOnly);

    // start process
    qInfo() << "Running yosys with the following options:";
    qInfo() << options;
    _yosysRunner->start("yosys", options);

    _spinner->reset();
    _spinner->show();
}

void Synthesis::runPnR()
{
    int archIdx = ui->archSel->currentIndex();
    QString command = QString("nextpnr-%1").arg(_pnrArchs[archIdx]);
    QString outFormat = _packerFormats[archIdx];
    QStringList options;
    QString type = ui->typeSel->currentText();
    QString package = ui->packageLine->text().trimmed().toLower();
    QString pcf = ui->pcfName->text();

    // Add user-defined options
    if (!type.isEmpty()) {
        options << "--" + type;
    }
    if (!package.isEmpty()) {
        options << "--package" << package;
    }
    if (!pcf.isEmpty()) {
        options << "--pcf" << pcf;
    }

    // Add other options
    options << "--json" << "out.json";
    options << "--" + outFormat << "out." + outFormat;

    // Open log file
    _pnrLog->open(QFile::WriteOnly);

    // start process
    qInfo() << "Running" << command << "with the following options:";
    qInfo() << options;
    _pnrRunner->start(command, options);
}

void Synthesis::runPack()
{
    int archIdx = ui->archSel->currentIndex();
    QString packer = _packers[archIdx];
    QString packerOp = _packerOptions[archIdx];
    QStringList options;
    QString inFile = "out." + _packerFormats[archIdx];
    if (!packerOp.isEmpty()) {
        options << packerOp;
    }
    options << inFile << "out.bin";

    // Open log file
    _packLog->open(QFile::WriteOnly);

    // start process
    qInfo() << "Running" << packer << "with the following options:";
    qInfo() << options;
    _packRunner->start(packer, options);
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

void Synthesis::on_removeAllBtn_clicked()
{
    foreach(QCheckBox* file, this->findChildren<QCheckBox*>()) {
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
    _yosysLog->close();
    // DO place & route only if yosys ended normally.
    if (exitCode == 0) {
        // Program ended normally
        qInfo() << "Synthesis complete";

        _spinner->next();
        runPnR();
    } else {
        qCritical() << "Terminated. Return code:" << exitCode;
        qCritical() << _yosysRunner->errorString();
        _spinner->fail();
        nativeOpen(QFileInfo(*_yosysLog).absoluteFilePath());
    }

}

void Synthesis::updatePnR()
{
    QString log = _pnrRunner->readAll();
    _pnrLog->write(log.toUtf8());
}

void Synthesis::finishPnR(int exitCode)
{
    _pnrLog->close();
    // Do packing only if nextpnr ended normally.
    if (exitCode == 0) {
        // Program ended normally
        qInfo() << "Place and route complete";

        _spinner->next();
        runPack();
    } else {
        qCritical() << "Terminated. Return code:" << exitCode;
        qCritical() << _pnrRunner->errorString();
        _spinner->fail();
        nativeOpen(QFileInfo(*_pnrLog).absoluteFilePath());
    }
}

void Synthesis::updatePack()
{
    QString log = _packRunner->readAll();
    _packLog->write(log.toUtf8());
    qInfo() << log;
}

void Synthesis::finishPack(int exitCode)
{
    _packLog->close();
    if (exitCode == 0) {
        // Program ended normally
        qInfo() << "Packing complete";
        _spinner->next();
    } else {
        qCritical() << "Terminated. Return code:" << exitCode;
        qCritical() << _packRunner->errorString();
        _spinner->fail();
        nativeOpen(QFileInfo(*_packLog).absoluteFilePath());
    }
}

void Synthesis::onFailure(QProcess::ProcessError error)
{
    qCritical() << "Error:";
    qCritical() << error;
    qCritical() << "yosys" << _yosysRunner->errorString();
    qCritical() << "nextpnr" << _pnrRunner->errorString();
    qCritical() << "packer" << _packRunner->errorString();

    int failIdx = _spinner->fail();
    QList<QProcess*> tmpList = {_yosysRunner, _pnrRunner, _packRunner};
    _spinner->setError(tmpList[failIdx]->errorString());

    // close all files
    _yosysLog->close();
    _pnrLog->close();
    _packLog->close();
}

/**
 * @brief Synthesis::on_archSel_currentIndexChanged
 *
 * Update the device type drop down list to match the selected architecture
 *
 * @param index -- the selected device architecture index
 */
void Synthesis::on_archSel_currentIndexChanged(int index)
{
    ui->typeSel->clear();
    ui->typeSel->addItems(_devices[index]);
}
