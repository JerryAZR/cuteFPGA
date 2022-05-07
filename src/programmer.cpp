#include "programmer.h"
#include "ui_programmer.h"
#include "utils.h"
#include "warndialog.h"
#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>

void addUdevRule();

Programmer::Programmer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Programmer)
{
    ui->setupUi(this);

    QString workDir = getWorkDir();
    QFile configFile(":/config/programmers.json");
    configFile.open(QFile::ReadOnly);
    QJsonDocument config = QJsonDocument::fromJson(configFile.readAll());
    configFile.close();
    QStringList progNames;
    foreach(QJsonValueRef progRef, config.array()) {
        // Example:
        //    {
        //        "name": "webFPGA",
        //        "use_python": true,
        //        "programmer": ":/scripts/webfpga.py",
        //        "options": []
        //    }
        static const QString nameKey("name");
        static const QString pyKey("use_python");
        static const QString progKey("programmer");
        progNames.append(progRef[nameKey].toString());
        _programmers.append(progRef[progKey].toString());
        _progUsePython.append(progRef[pyKey].toBool());
    }
    ui->progSel->addItems(progNames);
    // Set up programmer process
    _progLog = new QFile(workDir + "/programmer.log", this);
    _progRunner = new QProcess(this);
    _progRunner->setProcessChannelMode(QProcess::MergedChannels);
    _progRunner->setWorkingDirectory(workDir);

    // Set up spinner dialog and connect signals
    _spinner = new SpinnerDialog(this);
    _spinner->addTask("Programming device...");
    connect(_spinner, SIGNAL(rejected()), _progRunner, SLOT(kill()));

    connect(_progRunner, SIGNAL(readyRead()), this, SLOT(updateProg()));
    connect(_progRunner, SIGNAL(finished(int)), this, SLOT(finishProg(int)));
    connect(_progRunner, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onFailure(QProcess::ProcessError)));
}

Programmer::~Programmer()
{
    delete ui;
}

void Programmer::runProg()
{
    int progIdx = ui->progSel->currentIndex();
    QString bitstream = ui->binLine->text();
    QString command;
    QStringList options;

    addUdevRule();

    if (bitstream.isEmpty()) {
        // Might be better to show a warning here
        WarnDialog warnBox;
        warnBox.setWindowIcon(QIcon(":/icons/alert.svg"));
        warnBox.setWindowTitle("Error");
        warnBox.addLine("Please fix the following error(s):");
        warnBox.addLine("No bitstream file selected.");
        warnBox.exec();
        return;
    }

    // TODO: add support for custom options
    if (_progUsePython[progIdx]) {
        command = "python3";
        QFile pyFile(_programmers[progIdx]);
        pyFile.open(QFile::ReadOnly);
        options << "-c" << pyFile.readAll();
    } else {
        command = _programmers[progIdx];
    }

    // Add bitstream file as the last option
    options << bitstream;

    // Open log file
    _progLog->open(QFile::WriteOnly);

    // Start the process
    _spinner->reset();
    _spinner->show();
    qInfo() << "Running" << command << "with the following options:";
    qInfo() << options;
    _progRunner->start(command, options);
}

void Programmer::setBinPath(QString binPath)
{
    ui->binLine->setText(QDir::toNativeSeparators(binPath));
}

void Programmer::updateProg()
{
    QString log = _progRunner->readAll();
    _progLog->write(log.toUtf8());
}

void Programmer::finishProg(int returnCode)
{
    _progLog->close();
    if (returnCode == 0) {
        qInfo() << "Device programmed.";

        _spinner->next();
        _spinner->setError("Device programmed.");
    } else {
        qCritical() << "Terminated. Return code:" << returnCode;
        qCritical() << _progRunner->errorString();
        _spinner->fail();
        _spinner->setError(_progRunner->errorString());
        nativeOpen(QFileInfo(*_progLog).absoluteFilePath());
    }
}

void Programmer::onFailure(QProcess::ProcessError error)
{
    qCritical() << "Error:";
    qCritical() << error;
    qCritical() << "programmer: " << _progRunner->errorString();

    _spinner->fail();
    _spinner->setError(_progRunner->errorString());

    // close all files
    _progLog->close();
}

void Programmer::on_progBtn_clicked()
{
    runProg();
}


void Programmer::on_binBtn_clicked()
{
    QFileDialog fileDialog(this);
    QStringList pcfNames;

    const QStringList filters({"Bitstream files (*.bin *.bit)",
                               "Any files (*)"
                              });

    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilters(filters);

    if (fileDialog.exec() == QFileDialog::Accepted) {
        pcfNames = fileDialog.selectedFiles();
    }

    if (pcfNames.length() > 0) {
        ui->binLine->setText(pcfNames[0]);
    }
}

void addUdevRule() {
#ifdef Q_OS_LINUX
    QFile rule("/etc/udev/rules.d/60-webfpga.rules");
    if (rule.exists()) {
        return;
    }
    if (system("echo 'SUBSYSTEM==\"usb\", ATTR{idVendor}==\"16d0\", \
ATTR{idProduct}==\"0e6c\", TAG+=\"uaccess\"' \
| pkexec tee /etc/udev/rules.d/60-webfpga.rules > /dev/null")) {
        qCritical() << "Failed to create udev rule";
    } else {
        if (system("udevadm control --reload-rules && udevadm trigger")) {
            qCritical() << "Failed to reload udev rules";
        }
    }
#endif
}

