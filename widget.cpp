#include "widget.h"
#include "ui_widget.h"
#include <QUdpSocket>
#include <numeric>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    mSocket = new QUdpSocket();
    connect(ui->radioButton1,SIGNAL(clicked()),this,SLOT(LeftLock()));
    connect(ui->radioButton2,SIGNAL(clicked()),this,SLOT(LeftLock()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_radioButton3_clicked()
{
    QByteArray ba;
    ba.resize(7);
    ba[0]=0x02;
    ba[1]=0x00;
    ba[2]=0x03;
    ba[3]=0xa0;
    ba[4]=0x08;
    ba[5]=0x01;
    ba[6]=ba[1] + ba[2] + ba[3] + ba[4] + ba[5] + ba[0];
    mSocket->writeDatagram(ba,7,QHostAddress("192.168.1.60"),2002);
}

void Widget::on_radioButton4_clicked()
{
    QByteArray ba;
    ba.resize(7);
    ba[0]=0x02;
    ba[1]=0x00;
    ba[2]=0x03;
    ba[3]=0xa0;
    ba[4]=0x08;
    ba[5]=0x00;
    ba[6]=ba[1] + ba[2] + ba[3] + ba[4] + ba[5] + ba[0];
    mSocket->writeDatagram(ba,7,QHostAddress("192.168.1.60"),2002);
}

void Widget::on_radioButton1_clicked()
{
    QByteArray ba;
    ba.resize(7);
    ba.resize(7);
    ba[0]=0x02;
    ba[1]=0x00;
    ba[2]=0x03;
    ba[3]=0xa0;
    ba[4]=0x08;
    ba[5]=0x01;
    ba[6]=ba[1] + ba[2] + ba[3] + ba[4] + ba[5] + ba[0];
    mSocket->writeDatagram(ba,7,QHostAddress("192.168.1.60"),2001);
}

void Widget::on_radioButton2_clicked()
{
    QByteArray dat;
    dat.resize(7);
    dat[0]=0x02;
    dat[1]=0x00;
    dat[2]=0x03;
    dat[3]=0xa0;
    dat[4]=0x08;
    dat[5]=0x00;
    dat[6]=dat[1] + dat[2] + dat[3] + dat[4] + dat[5] + dat[0];
    mSocket->writeDatagram(dat,7,QHostAddress("192.168.1.60"),2001);
}
