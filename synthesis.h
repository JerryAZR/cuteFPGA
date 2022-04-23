#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include <QWidget>

namespace Ui {
class Synthesis;
}

class Synthesis : public QWidget
{
    Q_OBJECT

public:
    explicit Synthesis(QWidget *parent = nullptr);
    ~Synthesis();

private:
    Ui::Synthesis *ui;
};

#endif // SYNTHESIS_H
