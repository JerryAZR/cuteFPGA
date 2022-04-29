#ifndef FANCYBUTTON_H
#define FANCYBUTTON_H

#include <QPushButton>

namespace Ui {
class FancyButton;
}

class FancyButton : public QPushButton
{
    Q_OBJECT

public:
    explicit FancyButton(QWidget *parent = nullptr);
    explicit FancyButton(const QIcon& icon, const QString& label, QWidget *parent = nullptr);
    explicit FancyButton(const QPixmap& pixmap, const QString& label, QWidget *parent = nullptr);
    explicit FancyButton(const QString& path, const QString& label, QWidget *parent = nullptr);
    ~FancyButton();

    void setIcon(const QIcon& icon);
    void setIcon(const QPixmap& pixmap);
    void setIcon(const QString& path);

    void setLabel(const QString& label);

private:
    Ui::FancyButton *ui;
};

#endif // FANCYBUTTON_H
