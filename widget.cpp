#include "widget.h"
#include "ui_widget.h"
#include <QUdpSocket>
#include <numeric>
#include <iostream>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    mSocket = new QUdpSocket();
    mSocket->bind(QHostAddress::AnyIPv4,leftport);  // mSocket->bind(2001,QHostAddress::ShareAdress);
//    mSocket->bind(QHostAddress::AnyIPv4,rightport);
//    mSocket->close();
    connect(mSocket,SIGNAL(readyRead()),this,SLOT(read_data()));
    myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(periodMessage()));
    myTimer->start(1000);
    setWindowTitle(tr("NEBULA CHANGER"));
}

Widget::~Widget()
{
    delete ui;
}

// Eletronic lock
void Widget::on_radioButton1_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x01};
    emit sendDatagram(buf,Len1,leftport);
    qDebug() << "Lock Messsage";
}

void Widget::on_radioButton2_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x00};
    emit sendDatagram(buf,Len1,leftport);
    qDebug() << "Unlock Message";
}

void Widget::on_radioButton3_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x01};
    emit sendDatagram(buf,Len1,rightport);
    qDebug() << "Lock Messsage";
}

void Widget::on_radioButton4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x00};
    emit sendDatagram(buf,Len1,rightport);
    qDebug() << "Unlock Message";
}

// Broadcast
void Widget::on_pushButton_clicked()
{
    QByteArray ba = QByteArray::fromHex("55");
    mSocket->writeDatagram(ba,1,QHostAddress("255.255.255.255"),1021);
}

void Widget::periodMessage()
{
    unsigned char buf1[] = {0x02, 0x00, 0x03, 0xa0, 0x32, 0x00};
    unsigned char buf2[] = {0x02, 0x00, 0x03, 0xa0, 0x09, 0x00};    // read eletronic locks
    unsigned char buf3[] = {0x02, 0x00, 0x04, 0xa0, 0x00, 0x01, 0x00}; // read work mode
    emit sendDatagram(buf1,Len1,leftport);
    emit sendDatagram(buf1,Len1,rightport);
    emit sendDatagram(buf2,Len1,leftport);
    emit sendDatagram(buf2,Len1,rightport);
    emit sendDatagram(buf3,Len2,leftport);
    emit sendDatagram(buf3,Len2,rightport);
}

// Read messages
void Widget::read_data()
{
    QByteArray array;
    QHostAddress address;
    quint16 port;
    QDateTime time = QDateTime::currentDateTime();
    QString frametime = time.toString("hh:mm:ss");    // or "yyyy-MM-dd hh:mm:ss dddd"
    array.resize(mSocket->bytesAvailable());    // or array.resize(mSocket->pendingDatagramSize());
    int size = array.size();
    mSocket->readDatagram(array.data(),array.size(),&address,&port);
    ui->listWidget->addItem(frametime);
    ui->listWidget->addItem(array.toHex()); //  or ui->listWidget->insertItem(1, array);
//    qDebug() << "port" << port;
    char sum = 0x00;    // cant use unsigned char because QBytearray[] is char
    for (int i = 0; i < (size-1); ++i) {
        sum+=array.at(i);
    }
    if (sum == array.at(size-1))
    {
        array.remove(size-1,1); // truncate sumcheck byte
        array.remove(0,2);  //  truncate 0x0200
        unsigned char buf[size-3];
        memcpy(buf,array,size-3);
        unsigned char eleLock1[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00};
        unsigned char eleLock2[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x01};
        unsigned char mode1[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned char mode2[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        // show eletronic locks
        if (arraycmp(eleLock1, buf, sizeof (eleLock1), sizeof (buf)) == 1)
            ui->label1->setText(QString::fromLocal8Bit("锁定"));
        else {
            if (arraycmp(eleLock2, buf, sizeof (eleLock1), sizeof (buf)) == 1)
                ui->label1->setText(QString::fromLocal8Bit("解锁"));
        }
        // show work mode
        if (arraycmp(mode1, buf, sizeof (mode1), sizeof (buf)) == 1)
            ui->label1_1->setText(QString::fromLocal8Bit("自动模式"));
        else {
            if (arraycmp(mode2, buf, sizeof (mode2), sizeof (buf)) == 1)
                ui->label1_1->setText(QString::fromLocal8Bit("手动模式"));
        }
//        qDebug() << "\n array" << array.toHex();
    }
}

// sum Check for messages
unsigned char * Widget::sumCheck(unsigned char dat[], short Length)
{
    short len = Length-1;
    unsigned char * temp = new unsigned char[Length];
    for (int i = 0;i < len;i++)
    {

        temp[i] = dat[i];
        temp[len] = dat[i] + temp[len];
    }
    return temp;
}

// compare two arrays
bool Widget::arraycmp(unsigned char arrayA[], unsigned char arrayB[], unsigned long a, unsigned long b)
{
    bool e = 1;
    unsigned long count = 0;
    if(a == b)
        while (e && count < a) {
            if (arrayA[count] != arrayB[count])
                e = 0;
            count++;
        }
    return e;
}

// send Messages for call
void Widget::sendDatagram(unsigned char buf[], short Length, quint16 port)
{
        unsigned char *p = sumCheck(buf,Length);
        QByteArray ba(reinterpret_cast<char*>(p), Length);
        mSocket->writeDatagram(ba,Length,QHostAddress(stripAdress),port);
        qDebug() << "Sending successfully " << ba.toHex();
}

// Manual/Automatic
void Widget::on_radioButton1_1_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x00, 0x00, 0x01};
    emit sendDatagram(buf,Len2,rightport);
    qDebug() << "Manual mode";
}

void Widget::on_radioButton1_2_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x00, 0x00, 0x00};
    emit sendDatagram(buf,Len2,rightport);
    qDebug() << "Automatic mode";
}

// K1/K2 output contactors control
void Widget::on_radioButton2_1_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x03, 0x00};
    emit sendDatagram(buf,Len2,leftport);
    qDebug() << "K1/K2 turn off";
}

void Widget::on_radioButton2_2_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x03, 0x01};
    emit sendDatagram(buf,Len2,leftport);
    qDebug() << "K1/K2 turn on";
}

// K3/K4 auxiliary contactors control
void Widget::on_radioButton2_3_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x06, 0x00};
    emit sendDatagram(buf,Len2,leftport);
    qDebug() << "K3/K4 turn off";
}

void Widget::on_radioButton2_4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x06, 0x01};
    emit sendDatagram(buf,Len2,leftport);
    qDebug() << "K3/K4 turn on";
}

// Ki/Kii support contactors control
void Widget::on_radioButton1_3_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x01, 0x00};
    emit sendDatagram(buf,Len2,leftport);
    qDebug() << "Ki/Kii turn off";
}

void Widget::on_radioButton1_4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x01, 0x01};
    emit sendDatagram(buf,Len2,leftport);
    qDebug() << "Ki/Kii turn on";
}
