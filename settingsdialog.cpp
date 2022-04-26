#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "utils.h"
#include <QDate>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QDate yesterday = QDate::currentDate().addDays(-1);
    _installer = new Installer(getTargetPath(yesterday), this);
    _downloader = new Downloader(getTargetUrl(yesterday), this);
    _downloader->setLocalFile(getTargetPath(yesterday));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::clearConnections()
{
    foreach(QMetaObject::Connection conn, _activeConnections) {
        disconnect(conn);
    }
    _activeConnections.clear();
}

void SettingsDialog::init()
{
    ui->barLabel->setText("");
    // Enable buttons
    ui->buttonBox->setDisabled(false);
    ui->installBtn->setDisabled(false);
    ui->progressBar->setValue(0);
    ui->progressBar->hide();

    this->exec();
}

void SettingsDialog::startWorking(QString work)
{
    // disable all buttons
    ui->buttonBox->setDisabled(true);
    ui->installBtn->setDisabled(true);
    // show progress bar
    ui->progressBar->show();
    ui->progressBar->setValue(0);
    // set workload name
    ui->barLabel->setText(work);
}

void SettingsDialog::stopWorking(int returnCode)
{
    static QString errorMsg("Process terminated with code %1");
    if (returnCode == 0) {
        // success
        ui->barLabel->setText("Done");
        ui->progressBar->setValue(ui->progressBar->maximum());
    } else {
        ui->barLabel->setText(errorMsg.arg(QString::number(returnCode)));
    }
    // enable all buttons
    ui->buttonBox->setDisabled(false);
    ui->installBtn->setDisabled(false);
    // disconnect all signal-slots
    clearConnections();
}

void SettingsDialog::startInstall()
{
    // Set progress bar signal-slot connections
    clearConnections();
    _activeConnections.append(connect(_installer, SIGNAL(progressUpdated(int)),
                                      ui->progressBar, SLOT(setValue(int))));
    _activeConnections.append(connect(_installer, SIGNAL(installed()),
                                      this, SLOT(stopWorking())));
    _activeConnections.append(connect(_installer, SIGNAL(failed(int)),
                                      this, SLOT(stopWorking(int))));
    _activeConnections.append(connect(this, SIGNAL(rejected()),
                                      _installer, SLOT(terminate())));
    startWorking("Installing...");
    _installer->run();
}

void SettingsDialog::startDownload()
{
    clearConnections();
    _activeConnections.append(connect(_downloader, SIGNAL(progressUpdated(int)),
                                      ui->progressBar, SLOT(setValue(int))));
    _activeConnections.append(connect(_downloader, SIGNAL(finished()),
                                      this, SLOT(startInstall())));
    _activeConnections.append(connect(this, SIGNAL(rejected()),
                                      _downloader, SLOT(terminate())));
    startWorking("Downloading...");
    _downloader->run();
}

void SettingsDialog::on_installBtn_clicked()
{
    startDownload();
}
