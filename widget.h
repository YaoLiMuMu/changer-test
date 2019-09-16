#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:

    void on_radioButton3_clicked();

    void on_radioButton4_clicked();

    void on_radioButton1_clicked();

    void on_radioButton2_clicked();

private:
    Ui::Widget *ui;
    QUdpSocket *mSocket;
};

#endif // WIDGET_H
