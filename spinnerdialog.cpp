#include "spinnerdialog.h"
#include "ui_spinnerdialog.h"

SpinnerDialog::SpinnerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpinnerDialog)
{
    ui->setupUi(this);
}

SpinnerDialog::~SpinnerDialog()
{
    delete ui;
}

/**
 * @brief SpinnerDialog::addTask
 *
 * Add a task entry to the dialog
 *
 * @param taskName -- Name of the task to add
 * @param state -- Initial state of the task (default: pending / not started)
 */
void SpinnerDialog::addTask(QString taskName, int state)
{
    SpinnerTask* newTask = new SpinnerTask(state, this);
    newTask->setText(taskName);
    ui->mainLayout->addWidget(newTask);
}

/**
 * @brief SpinnerDialog::next
 *
 * Mark the current task as done and start the next one
 */
void SpinnerDialog::next()
{
    _current++;
    _refresh();
}

/**
 * @brief SpinnerDialog::reset
 *
 * Mark the first task as running and the rest as pending
 */
void SpinnerDialog::reset()
{
    _current = 0;
    _refresh();
    ui->errorLabel->setText("");
}

/**
 * @brief SpinnerDialog::fail
 *
 * Mark the running task (if exists) as failed
 *
 * @return Index of the failed task, or -1 if nothing failed.
 */
int SpinnerDialog::fail()
{
    QList<SpinnerTask*> children = this->findChildren<SpinnerTask*>();
    if (_current < children.length()) {
        children[_current]->setState(SpinnerTask::Fail);
        return _current;
    } else {
        return -1;
    }
}

/**
 * @brief SpinnerDialog::refresh
 *
 * Set all task states according to the current index
 */
void SpinnerDialog::_refresh()
{
    // Not found in the doc, but it appears that
    // findChildren() returns the children in the order they were added
    QList<SpinnerTask*> children = this->findChildren<SpinnerTask*>();
    for(int i = 0; i < children.length(); i++) {
        if (i < _current) {
            children[i]->setState(SpinnerTask::Done);
        } else if (i == _current) {
            children[i]->setState(SpinnerTask::Running);
        } else {
            children[i]->setState(SpinnerTask::Pending);
        }
    }
}

int SpinnerDialog::current() const
{
    return _current;
}

void SpinnerDialog::setCurrent(int newCurrent)
{
    _current = newCurrent;
    _refresh();
}

void SpinnerDialog::setError(const QString &error)
{
    ui->errorLabel->setText(error);
}
