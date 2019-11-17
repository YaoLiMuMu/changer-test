#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QTimer>
#include <QDateTime>
#include <QDataStream>
#include <QFile>

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

    void periodMessage();

    unsigned char * sumCheck(unsigned char dat[], short Length);

    bool arraycmp(unsigned char arrayA[], unsigned char arrayB[], unsigned long a, unsigned long b);

    void read_L();

    void read_R();

    void sendDatagram(unsigned char buf[], short Length, quint16 port, QUdpSocket *Socket);

    void processPendingDatagrams(QUdpSocket *Socket);

    QByteArray processQString(QString item, int k);

    void on_pushButton_clicked();

    void on_radioButton1_1_clicked();

    void on_radioButton1_2_clicked();

    void on_radioButton2_1_clicked();

    void on_radioButton2_2_clicked();

    void on_radioButton2_3_clicked();

    void on_radioButton2_4_clicked();

    void on_radioButton1_3_clicked();

    void on_radioButton1_4_clicked();

    void on_pushButton2_1_clicked();

    void on_pushButton2_2_clicked();

    void on_listWidget_customContextMenuRequested(const QPoint &pos);

    void cleanSeedSlot();

    void copySeedSlot();

    void on_pushButton2_5_clicked();

    void on_radioButton2_7_clicked();

    void on_radioButton2_8_clicked();

    void on_radioButton2_6_clicked();

    void on_radioButton2_5_clicked();

    void on_pushButton2_6_clicked();

private:
    Ui::Widget *ui;
    QUdpSocket *mSocket;
    QUdpSocket *sSocket;
    QTimer *myTimer;
    short Len1 = 7;
    short Len2 = 8;
    quint16 leftport = 2001;
    quint16 rightport = 2002;
    QString stripAdress = "192.168.1.60";
    QByteArray member = QByteArray::fromHex("000000");  // at.(0): working mode //at.(1):Power module Status // at.(2): Device charging Status
};

#endif // WIDGET_H
