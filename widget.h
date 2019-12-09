#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QTimer>
#include <QDateTime>
#include <QDataStream>
#include <QFile>
#include <QThread>
#include <dataprocessor.h>

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
    void UpdateUI(Msg msg);

    void disconnect();

    void on_radioButton3_clicked();

    void on_radioButton4_clicked();

    void on_radioButton1_clicked();

    void on_radioButton2_clicked();

    void periodMessage();

    unsigned char * sumCheck(unsigned char dat[], short Length);

    bool arraycmp(unsigned char arrayA[], unsigned char arrayB[], unsigned long a, unsigned long b);

    void sendDatagram(unsigned char buf[], short Length, quint16 port, QUdpSocket *Socket);

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

    void broadcastmessage();

    void on_pushButton2_5_clicked();

    void on_radioButton2_7_clicked();

    void on_radioButton2_8_clicked();

    void on_radioButton2_6_clicked();

    void on_radioButton2_5_clicked();

    void on_pushButton2_6_clicked();

    void on_radioButton2_11_clicked();

    void on_radioButton2_12_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_pushButton_2_clicked();

    void on_pushButton2_8_clicked();

    void on_pushButton2_7_clicked();

    void on_horizontalSlider1_2_valueChanged(int value);

    void on_horizontalSlider1_1_valueChanged(int value);

    void on_horizontalSlider1_3_valueChanged(int value);

    void on_horizontalSlider1_4_valueChanged(int value);

    void on_horizontalSlider1_6_valueChanged(int value);

    void on_pushButton_3_clicked();

    void on_radioButton1_5_clicked();

    void on_radioButton1_6_clicked();

    void on_horizontalSlider1_7_valueChanged(int value);

private:
    Ui::Widget *ui;
    QUdpSocket *mSocket;
    QUdpSocket *sSocket;
    QUdpSocket *bSocket;
    QUdpSocket *vSocket;
    DataProcessor *dataprocess;
    QThread * thread;
    QTimer *myTimer;
    short Len1 = 7;
    short Len2 = 8;
    quint16 leftport = 2001;
    quint16 rightport = 2002;
    QString stripAdress;
    QMap<QString, QString> Device_Node;
    quint8 timeback = 1;
    bool member[5] ={0,0,0,0,0};
    //member[0]: working mode
    //member[1]: Power module Status
    //member[2]: Device charging Status
    //member[3]: Device network connect status
    //member[4]: Broadcast Flag
};

#endif // WIDGET_H
