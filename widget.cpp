#include "widget.h"
#include "ui_widget.h"
#include <QUdpSocket>
#include <numeric>
#include <iostream>
#include <QVector>
#include <QMessageBox>
#include <QMenu>
#include <QClipboard>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    mSocket = new QUdpSocket();     // left Socket
    sSocket = new QUdpSocket();     // right Socket
    mSocket->bind(QHostAddress::AnyIPv4,leftport);  // mSocket->bind(2001,QHostAddress::ShareAdress);
    sSocket->bind(QHostAddress::AnyIPv4,rightport); // mSocket->close(); to close port
    connect(mSocket,SIGNAL(readyRead()),this,SLOT(read_L()));
    connect(sSocket,SIGNAL(readyRead()),this,SLOT(read_R()));
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
    emit sendDatagram(buf,Len1,leftport, mSocket);
    qDebug() << "Lock Messsage";
}

void Widget::on_radioButton2_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x00};
    emit sendDatagram(buf,Len1,leftport, mSocket);
    qDebug() << "Unlock Message";
}

void Widget::on_radioButton3_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x01};
    emit sendDatagram(buf,Len1,rightport, sSocket);
    qDebug() << "Lock Messsage";
}

void Widget::on_radioButton4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x03, 0xa0, 0x08, 0x00};
    emit sendDatagram(buf,Len1,rightport, sSocket);
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
    unsigned char buf1[] = {0x02, 0x00, 0x03, 0xa0, 0x32, 0x00};    // read working､ status
    unsigned char buf2[] = {0x02, 0x00, 0x03, 0xa0, 0x09, 0x00};    // read eletronic locks
    unsigned char buf3[] = {0x02, 0x00, 0x04, 0xa0, 0x00, 0x01, 0x00}; // read work mode
    unsigned char buf4[] = {0x02, 0x00, 0x03, 0xa0, 0x0f, 0x00}; // read K1/K2
    emit sendDatagram(buf1,Len1,leftport,mSocket);
    emit sendDatagram(buf1,Len1,rightport,sSocket);
    emit sendDatagram(buf2,Len1,leftport,mSocket);
    emit sendDatagram(buf2,Len1,rightport,sSocket);
    emit sendDatagram(buf3,Len2,leftport,mSocket);
    emit sendDatagram(buf3,Len2,rightport,sSocket);
    emit sendDatagram(buf4,Len1,leftport,mSocket);
    emit sendDatagram(buf4,Len1,rightport,sSocket);
}

// Read messages
void Widget::read_L()
{
    emit processPendingDatagrams(mSocket);
}

void Widget::read_R()
{
    emit processPendingDatagrams(sSocket);
}

// process Datagrams
void Widget::processPendingDatagrams(QUdpSocket *Socket)
{
    unsigned char eleLock[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00};
    unsigned char km1[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00};
    unsigned char mode[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char mode2[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    QByteArray array;
    QHostAddress address;
    quint16 port;
    QDateTime time = QDateTime::currentDateTime();
    QString timeblock = time.toString("hh:mm:ss");    // or "yyyy-MM-dd hh:mm:ss dddd"
    array.resize(Socket->bytesAvailable());    // or array.resize(Socket->pendingDatagramSize());
    int size = array.size();
    Socket->readDatagram(array.data(),array.size(),&address,&port);
    ui->listWidget->addItem(timeblock + array.toHex()); //  or ui->listWidget->insertItem(1, array);
    qDebug() << "port " << port << ": " << array.toHex();
    char sum = 0x00;    // cant use unsigned char because QBytearray[] is char
    for (int i = 0; i < (size-1); ++i) {
        sum+=array.at(i);
    }   // sum cleck
    if (sum == array.at(size-1))
    {
        array.remove(size-1,1); // truncate sumcheck byte
        array.remove(0,2);  //  truncate 0x0200
        unsigned char buf[size-3];
        memcpy(buf,array,size-3);
        if (port == 2001)
        {
            // show eletronic locks
            if (arraycmp(eleLock, buf, sizeof (eleLock), sizeof (buf)) == 1)
            {
                if (buf[size-4] == 0x00)
                    ui->label1->setText(QString::fromLocal8Bit("锁定"));
                else {
                    if (buf[size-4] == 0x01)
                         ui->label1->setText(QString::fromLocal8Bit("解锁"));
                }
            }
            // show K1/K2
            if (arraycmp(km1, buf, sizeof (km1), sizeof (buf)) == 1)
            {
                if (buf[size-4] == 0x00)
                    ui->label2_1->setText(QString::fromLocal8Bit("弹开"));
                else {
                    if (buf[size-4] == 0x01)
                         ui->label2_1->setText(QString::fromLocal8Bit("吸合"));
                }
            }
            // show work status
            if (buf[0] == 0x36)
            {
                ui->label1_5->setText(tr("%1 °C").arg(buf[52]));    // Power module #1 temperature
                ui->label1_6->setText(tr("%1 °C").arg(buf[53]));    // Power module #2 temperature
                ui->label1_7->setText(tr("%1 °C").arg(buf[54]));    // Power module #3 temperature
                if (buf[10] == 0x00)
                    ui->label2_3->setText(QString::fromLocal8Bit("Free"));  // Device Charing Status
                else if (buf[10] == 0x01) {
                    ui->label2_3->setText(QString::fromLocal8Bit("Loading"));
                    member.data()[2] = 0x55;
                }
                quint32 temp1 = (buf[13]<<24)+(buf[14]<<16)+(buf[15]<<8)+buf[16];
                ui->label2_5->setText(tr("%1 V").arg(temp1));   // MBS Demand Voltage
                // MBS Demand Current
                quint32 temp2 = (buf[20]<<24)+(buf[21]<<16)+(buf[22]<<8)+buf[23];
                double temp3 = ((double) temp2)/1000;
                ui->lcdNumber2_1->display(750.10);   // Output Voltage
                quint32 temp4 = (buf[24]<<24)+(buf[25]<<16)+(buf[26]<<8)+buf[27];
                double temp5 = ((double) temp4)/1000;
                ui->lcdNumber2_2->display(150.10);   // Output Current
                ui->progressBar->setValue(0);
                member.data()[1] = buf[44];     // Power module Status
            }
            // show work mode
            if (arraycmp(mode, buf, sizeof (mode), sizeof (buf)) == 1)
            {
                if (buf[size-4] == 0x00)
                    ui->label1_1->setText(QString::fromLocal8Bit("自动"));
                else {
                    if (buf[size-4] == 0x01)
                    {
                        ui->label1_1->setText(QString::fromLocal8Bit("手动"));
                        member.data()[0] = 55; // Working mode
                        ui->pushButton2_6->setEnabled(false);
                        ui->pushButton2_5->setEnabled(false);
                    }
                }
            }
        }
        else {
            // show eletronic locks
            if (arraycmp(eleLock, buf, sizeof (eleLock), sizeof (buf)) == 1)
            {
                if (buf[size-4] == 0x00)
                    ui->label3_1->setText(QString::fromLocal8Bit("锁定"));
                else {
                    if (buf[size-4] == 0x01)
                         ui->label3_1->setText(QString::fromLocal8Bit("解锁"));
                }
            }
            // show K1/K2
            if (arraycmp(km1, buf, sizeof (km1), sizeof (buf)) == 1)
            {
                if (buf[size-4] == 0x00)
                    ui->label3_2->setText(QString::fromLocal8Bit("弹开"));
                else {
                    if (buf[size-4] == 0x01)
                         ui->label3_2->setText(QString::fromLocal8Bit("吸合"));
                }
            }
            // show work status
            if (buf[0] == 0x36)
            {
                ui->label1_8->setText(tr("%1 °C").arg(buf[52]));
                ui->label1_9->setText(tr("%1 °C").arg(buf[53]));
                ui->label1_10->setText(tr("%1 °C").arg(buf[54]));   //ui->label1_10->setStyleSheet("color:red;");

            }
        }
    }
}

// sum Check for messages
unsigned char * Widget::sumCheck(unsigned char dat[], short Length)
{
    short len = Length-1;
    unsigned char * temp = new unsigned char[Length];
    temp[len] = 0x00;
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
    if((a+1) == b)
        while (e && count < a) {
            if (arrayA[count] != arrayB[count])
                e = 0;
            count++;
        }
    return e;
}

// send Messages for call
void Widget::sendDatagram(unsigned char buf[], short Length, quint16 port, QUdpSocket *Socket)
{
        unsigned char *p = sumCheck(buf,Length);
        QByteArray ba(reinterpret_cast<char*>(p), Length);
        Socket->writeDatagram(ba,Length,QHostAddress(stripAdress),port);
        qDebug() << QDateTime::currentDateTime() <<"Sending successfully " << ba.toHex();
}

// Manual/Automatic
void Widget::on_radioButton1_1_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x00, 0x00, 0x01};
    emit sendDatagram(buf,Len2,rightport,mSocket);
    qDebug() << "Manual mode";
}

void Widget::on_radioButton1_2_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x00, 0x00, 0x00};
    emit sendDatagram(buf,Len2,rightport,mSocket);
    qDebug() << "Automatic mode";
}

// K1/K2 output contactors control
void Widget::on_radioButton2_1_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x03, 0x00};
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "K1/K2 turn off";
}

void Widget::on_radioButton2_2_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x03, 0x01};
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "K1/K2 turn on";
}

// K3/K4 auxiliary contactors control
void Widget::on_radioButton2_3_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x06, 0x00};
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "K3/K4 turn off";
}

void Widget::on_radioButton2_4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x06, 0x01};
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "K3/K4 turn on";
}

// Ki/Kii support contactors control
void Widget::on_radioButton1_3_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x01, 0x00};
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "Ki/Kii turn off";
}

void Widget::on_radioButton1_4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x01, 0x01};
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "Ki/Kii turn on";
}

// Loading Voltage and current
void Widget::on_pushButton2_1_clicked()
{
    if (ui->lineEdit2_1->text().isEmpty() || ui->lineEdit2_2->text().isEmpty())
        {
        QMessageBox::information(this,"Warning","Please Input Voltage and Current");
        return;
    }
    unsigned char buf[14] = {0x02, 0x00, 0x0b, 0xa0, 0x03, 0x00}; // no define element set to 0x00
    QString output = ui->lineEdit2_1->text();
    QByteArray ba = processQString(output,1000);
    int val = 9;
    for (int i = (ba.length()-1); i >= 0; i=i-1)
    {
        buf[val] = ba.data()[i];
        val = val -1;
    }
    output = ui->lineEdit2_2->text();
    ba = processQString(output,1000);
    val = 13;
    for (int i = (ba.length()-1); i >= 0; i=i-1)
    {
        buf[val] = ba.data()[i];
        val = val -1;
    }
    if(member.at(1) == 0x01)
        buf[5] = 0x01;
    emit sendDatagram(buf,15,leftport,mSocket);
    if(member.at(1) == 0x01)
    {
        ui->pushButton2_2->setEnabled(true);
    }
    qDebug() << "Loading voltage and current";
}

// process Qstring to QByteArray
QByteArray Widget::processQString(QString item, int k) // or can try str.toLatin1()/str.toUtf8()
{
    QByteArray dat;
    int val = item.toInt()*k; // int < 2147483648
    item = item.setNum(val, 16);    // or String str = Interger.toHexString(val) ; use String
    if (item.length()%2 != 0)
        item = '0' + item;
    dat.resize(item.length()/2);    // can't use sizeof get Qstring size, because will get Qstring object point size
    char ctempor = 0;
    for (int i = 0; i < item.length()/2; i++)
    {
        if (item.toLocal8Bit().data()[2*i] >= '0' && item.toLocal8Bit().data()[2*i] <= '9')
            ctempor = item.toLocal8Bit().data()[2*i] -48;
        else {
            ctempor = 0xa + (item.toLocal8Bit().data()[2*i] - 'a');
        }
        dat.data()[i] = ctempor;
        if (item.toLocal8Bit().data()[2*i+1] >= '0' && item.toLocal8Bit().data()[2*i+1] <= '9')
            ctempor = item.toLocal8Bit().data()[2*i+1] -48;
        else {
            ctempor = 0xa + (item.toLocal8Bit().data()[2*i+1] - 'a');
        }
        dat.data()[i] = (dat.data()[i] << 4) | ctempor;
    }
    return dat;
}

// stop changer output
void Widget::on_pushButton2_2_clicked()
{
    unsigned char buf[14] = {0x02, 0x00, 0x0b, 0xa0, 0x03, 0x02};
    emit sendDatagram(buf,15,leftport,mSocket);
    if(member.at(1) == 0x00)
        ui->pushButton2_2->setEnabled(false);
    qDebug() << "Stopping output";
}
// Listwight custom context menu for copy and clear
void Widget::on_listWidget_customContextMenuRequested(const QPoint &pos)
{
        QListWidgetItem * curItem = ui->listWidget->itemAt(pos);
        if(curItem == nullptr)
            return;
        QMenu * popMenu = new QMenu( this );
        QAction * cleanSeed = new QAction(tr("Clean"), this);
        QAction * copySeed = new QAction(tr("Copy"), this);
        popMenu->addAction(copySeed);
        popMenu->addAction(cleanSeed);
        connect(copySeed, SIGNAL(triggered()), this, SLOT(copySeedSlot()));
        connect(cleanSeed, SIGNAL(triggered()), this, SLOT(cleanSeedSlot()));
        popMenu->exec(QCursor::pos());
        delete popMenu;
        delete copySeed;
        delete cleanSeed;
}
// clean listwidget item
void Widget::cleanSeedSlot()
{
    int ch = QMessageBox::warning(this, "Warning",
                                  "Are you sure to clean Messages ?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (ch != QMessageBox::Yes)
        return;
    QListWidgetItem * item = ui->listWidget->currentItem();
    if(item == nullptr)
        return;
    ui->listWidget->clear();
}
// copy listwidget item
void Widget::copySeedSlot()
{
    QListWidgetItem * item = ui->listWidget->currentItem();
    if(item == nullptr)
        return;
    QString str = ui->listWidget->currentItem()->text();
    QApplication::clipboard()->setText(str);
}
// Automatic charging mode､
void Widget::on_pushButton2_5_clicked()
{
    if(member.at(0) == 55)
    {
        QMessageBox::information(this,"Warning","Please Setting Mode ad Automatic Charging");
        return;
    }
    if(ui->radioButton2_5->isChecked())
    {
        unsigned char buf[] = {0x02, 0x00, 0x08, 0xa0, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        emit sendDatagram(buf,12,leftport,mSocket);
        if(member.at(2) == 0x55)
        {
            ui->pushButton2_5->setEnabled(false);
            ui->pushButton2_6->setEnabled(true);
        }
        qDebug() << "Automatic Charging to Full";
    }
    if(ui->radioButton2_7->isChecked())
    {
        if(ui->lineEdit2_5->text().isEmpty())
        {
            QMessageBox::information(this,"Warning","Please Setting the Charging Time");
            return;
        }
        unsigned char buf[] = {0x02, 0x00, 0x08, 0xa0, 0x31, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
        QString output = ui->lineEdit2_5->text();
        QByteArray ba = processQString(output,1);
        int val = 9;
        for (int i = (ba.length()-1); i >= 0; i=i-1)
        {
            buf[val] = ba.data()[i];
            val = val -1;
        }
        emit sendDatagram(buf,12,leftport,mSocket);
        if(member.at(2) == 0x55)
        {
            ui->pushButton2_5->setEnabled(false);
            ui->pushButton2_6->setEnabled(true);
        }
        qDebug() << "Automatic Charging for Setting Time";
    }
    if(ui->radioButton2_8->isChecked())
    {
        if(ui->lineEdit2_5->text().isEmpty())
        {
            QMessageBox::information(this,"Warning","Please Setting the Charging Fee");
            return;
        }
        unsigned char buf[] = {0x02, 0x00, 0x08, 0xa0, 0x31, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
        QString output = ui->lineEdit2_5->text();
        QByteArray ba = processQString(output,100);
        int val = 9;
        for (int i = (ba.length()-1); i >= 0; i=i-1)
        {
            buf[val] = ba.data()[i];
            val = val -1;
        }
        emit sendDatagram(buf,12,leftport,mSocket);
        if(member.at(2) == 0x55)
        {
            ui->pushButton2_5->setEnabled(false);
            ui->pushButton2_6->setEnabled(true);
        }
        qDebug() << "Automatic Charging to Fee";
    }
    if(ui->radioButton2_6->isChecked())
    {
        if(ui->lineEdit2_5->text().isEmpty())
        {
            QMessageBox::information(this,"Warning","Please Setting the Charging Level");
            return;
        }
        unsigned char buf[] = {0x02, 0x00, 0x08, 0xa0, 0x31, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01};
        QString output = ui->lineEdit2_5->text();
        QByteArray ba = processQString(output,100);
        int val = 9;
        for (int i = (ba.length()-1); i >= 0; i=i-1)
        {
            buf[val] = ba.data()[i];
            val = val -1;
        }
        emit sendDatagram(buf,12,leftport,mSocket);
        if(member.at(2) == 0x55)
        {
            ui->pushButton2_5->setEnabled(false);
            ui->pushButton2_6->setEnabled(true);
        }
        qDebug() << "Automatic Charging to Level";
    }
}

void Widget::on_radioButton2_7_clicked()
{
    ui->label2_11->setText(QString::fromLocal8Bit("秒 / s "));
}

void Widget::on_radioButton2_8_clicked()
{
    ui->label2_11->setText(QString::fromLocal8Bit("元/Yuan"));
}

void Widget::on_radioButton2_6_clicked()
{
    ui->label2_11->setText(QString::fromLocal8Bit("KW·h/ ° "));
}

void Widget::on_radioButton2_5_clicked()
{
    ui->label2_11->setText(QString::fromLocal8Bit("    "));
}
// Stop Automatic mode charging
void Widget::on_pushButton2_6_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x08, 0xa0, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    emit sendDatagram(buf,12,leftport,mSocket);
    if(member.at(2) == 0x00)
    {
        ui->pushButton2_6->setEnabled(false);
        ui->pushButton2_5->setEnabled(true);
    }
    qDebug() << "Stopping Charging";
}
