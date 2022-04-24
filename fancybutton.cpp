#include "fancybutton.h"
#include "ui_fancybutton.h"
#include <QIcon>

FancyButton::FancyButton(QWidget *parent) :
    QPushButton(parent),
    ui(new Ui::FancyButton)
{
    ui->setupUi(this);
    ui->icon->setPixmap(QIcon(":/icons/memory.svg").pixmap(32, 32));
    ui->label->setText("Synthesis");
}

FancyButton::FancyButton(const QIcon& icon, const QString& label, QWidget *parent) :
    QPushButton(parent),
    ui(new Ui::FancyButton)
{
    ui->setupUi(this);
    setIcon(icon);
    setLabel(label);
}

FancyButton::FancyButton(const QPixmap& pixmap, const QString& label, QWidget *parent) :
    QPushButton(parent),
    ui(new Ui::FancyButton)
{
    ui->setupUi(this);
    setIcon(pixmap);
    setLabel(label);
}

FancyButton::FancyButton(const QString& path, const QString& label, QWidget *parent) :
    QPushButton(parent),
    ui(new Ui::FancyButton)
{
    ui->setupUi(this);
    setIcon(path);
    setLabel(label);
}

FancyButton::~FancyButton()
{
    delete ui;
}

void FancyButton::setIcon(const QIcon &icon)
{
    ui->icon->setPixmap(icon.pixmap(32,32));
}

void FancyButton::setIcon(const QPixmap &pixmap)
{
    ui->icon->setPixmap(pixmap);
}

void FancyButton::setIcon(const QString &path)
{
    ui->icon->setPixmap(QIcon(path).pixmap(32, 32));
}

void FancyButton::setLabel(const QString &label)
{
    ui->label->setText(label);
}
