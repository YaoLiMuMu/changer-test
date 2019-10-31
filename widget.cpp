#include "widget.h"
#include "ui_widget.h"
#include <QUdpSocket>
#include <numeric>

const short Len1 = 7;
const short Len2 = 8;
const int leftport = 2001;
const int rightport = 2002;
const QString stripAdress = "192.168.1.60";

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    mSocket = new QUdpSocket();
    mSocket->bind(QHostAddress::AnyIPv4,leftport);  // mSocket->bind(2001,QHostAddress::ShareAdress);
//    mSocket->bind(QHostAddress::AnyIPv4,rightport);
    connect(mSocket,SIGNAL(readyRead()),this,SLOT(read_data()));
    myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(periodMessage()));
    myTimer->start(1000);
}

Widget::~Widget()
{
    delete ui;
}

// Eletronic lock
void Widget::on_radioButton1_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x01};
    unsigned char *p = sumCheck(buf,Len1);
    QByteArray ba((char*)p, Len1);  // or QByteArray ba = QByteArray::fromHex("020003a00801ae");
    mSocket->writeDatagram(ba,Len1,QHostAddress(stripAdress),leftport);
    qDebug() << "Lock Messsage" << ba.toHex();

}

void Widget::on_radioButton2_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x00};
    unsigned char *p = sumCheck(buf, Len1);
    QByteArray ba((char*)p, Len1);
    mSocket->writeDatagram(ba,Len1,QHostAddress(stripAdress),leftport);
    qDebug() << "Unlock Message" << ba.toHex();
}

void Widget::on_radioButton3_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x01};
    unsigned char *p = sumCheck(buf, Len1);
    QByteArray ba((char*)p, Len1);
    mSocket->writeDatagram(ba,Len1,QHostAddress(stripAdress),rightport);
    qDebug() << "Lock Messsage" << ba.toHex();
}

void Widget::on_radioButton4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x00};
    unsigned char *p =sumCheck(buf,Len1);
    QByteArray ba((char*)p, Len1);
    mSocket->writeDatagram(ba,Len1,QHostAddress(stripAdress),rightport);
    qDebug() << "Unlock Message" << ba.toHex();
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
    unsigned char *p1 = sumCheck(buf1, Len1);
    unsigned char *p2 = sumCheck(buf2, Len1);
    QByteArray ba1((char*)p1, Len1);
    QByteArray ba2((char*)p2, Len1);
//    mSocket->writeDatagram(ba1,Len1,QHostAddress(stripAdress),leftport);
//    mSocket->writeDatagram(ba1,Len1,QHostAddress(stripAdress),rightport);
    mSocket->writeDatagram(ba2,Len1,QHostAddress(stripAdress),leftport);
}

// Read messages
void Widget::read_data()
{
    QByteArray array;
    QHostAddress address;
    quint16 port;
    array.resize(mSocket->bytesAvailable());    // or array.resize(mSocket->pendingDatagramSize());
    int size = array.size();
    mSocket->readDatagram(array.data(),array.size(),&address,&port);
    ui->listWidget->addItem(array);
    qDebug() << "first array " << array.toHex();
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
        if (strcmp((char*) eleLock1, (char*) buf) == 0)
            ui->label1->setText(QString::fromLocal8Bit("锁止"));
        if (strcmp((char*) eleLock2, (char*) buf) == 0)
            ui->label1->setText(QString::fromLocal8Bit("解锁"));
        qDebug() << "\n size" << array.length()
                 << "\n array" << array.toHex();
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

// Manual/Automatic
void Widget::on_radioButton1_1_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x00, 0x00, 0x01};
    unsigned char *p =sumCheck(buf,Len2);
    QByteArray ba((char*)p, Len2);
    mSocket->writeDatagram(ba,Len2,QHostAddress(stripAdress),rightport);
    qDebug() << "Manual mode" << ba.toHex();
}

void Widget::on_radioButton1_2_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x00, 0x00, 0x00};
    unsigned char *p =sumCheck(buf,Len2);
    QByteArray ba((char*)p, Len2);
    mSocket->writeDatagram(ba,Len2,QHostAddress(stripAdress),rightport);
    qDebug() << "Automatic mode" << ba.toHex();
}

// K1/K2 output contactors control
void Widget::on_radioButton2_1_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x03, 0x00};
    unsigned char *p =sumCheck(buf,Len2);
    QByteArray ba((char*)p, Len2);
    mSocket->writeDatagram(ba,Len2,QHostAddress(stripAdress),rightport);
    qDebug() << "K1/K2 turn off" << ba.toHex();
}

void Widget::on_radioButton2_2_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x03, 0x01};
    unsigned char *p =sumCheck(buf,Len2);
    QByteArray ba((char*)p, Len2);
    mSocket->writeDatagram(ba,Len2,QHostAddress(stripAdress),rightport);
    qDebug() << "K1/K2 turn on" << ba.toHex();
}

// K3/K4 auxiliary contactors control
void Widget::on_radioButton2_3_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x06, 0x00};
    unsigned char *p =sumCheck(buf,Len2);
    QByteArray ba((char*)p, Len2);
    mSocket->writeDatagram(ba,Len2,QHostAddress(stripAdress),rightport);
    qDebug() << "K3/K4 turn off" << ba.toHex();
}

void Widget::on_radioButton2_4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x06, 0x01};
    unsigned char *p =sumCheck(buf,Len2);
    QByteArray ba((char*)p, Len2);
    mSocket->writeDatagram(ba,Len2,QHostAddress(stripAdress),rightport);
    qDebug() << "K3/K4 turn on" << ba.toHex();
}

// Ki/Kii support contactors control
void Widget::on_radioButton1_3_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x01, 0x00};
    unsigned char *p =sumCheck(buf,Len2);
    QByteArray ba((char*)p, Len2);
    mSocket->writeDatagram(ba,Len2,QHostAddress(stripAdress),rightport);
    qDebug() << "Ki/Kii turn off" << ba.toHex();
}

void Widget::on_radioButton1_4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x01, 0x01};
    unsigned char *p =sumCheck(buf,Len2);
    QByteArray ba((char*)p, Len2);
    mSocket->writeDatagram(ba,Len2,QHostAddress(stripAdress),rightport);
    qDebug() << "Ki/Kii turn on" << ba.toHex();
}
