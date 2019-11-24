#ifndef HELPFORM_H
#define HELPFORM_H

#include <QDialog>

namespace Ui {
class helpform;
}

class helpform : public QDialog
{
    Q_OBJECT

public:
    explicit helpform(QWidget *parent = nullptr);
    ~helpform();
private:
    void init();

private:
    Ui::helpform *ui;
};

#endif // HELPFORM_H
