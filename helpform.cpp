#include "helpform.h"
#include "ui_helpform.h"
#include <QShortcut>
#include <QTextStream>

helpform::helpform(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::helpform)
{
    ui->setupUi(this);
    setWindowTitle(QString::fromLocal8Bit("Help, Clicking F1 for exit"));
    init();
}

helpform::~helpform()
{
    delete ui;
}

void helpform::init()
{
    QShortcut* k = new QShortcut(QKeySequence(Qt::Key_F1), this);
    connect(k, SIGNAL(activated()), this, SLOT(close()));

}
