#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include <QWidget>
#include <QCheckBox>
#include <QList>

namespace Ui {
class Synthesis;
}

class Synthesis : public QWidget
{
    Q_OBJECT

public:
    explicit Synthesis(QWidget *parent = nullptr);
    ~Synthesis();

    bool containsFile(const QString& fname);
    QList<QCheckBox*> getSelectedFiles();


private slots:
    void on_pcfBtn_clicked();
    void on_removeSelBtn_clicked();
    void on_addFileBtn_clicked();

private:
    Ui::Synthesis *ui;
};

#endif // SYNTHESIS_H
