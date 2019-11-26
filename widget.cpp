#include "widget.h"
#include "ui_widget.h"
#include "helpform.h"
#include <QUdpSocket>
#include <numeric>
#include <iostream>
#include <QVector>
#include <QMessageBox>
#include <QMenu>
#include <QClipboard>
#include <QTextCodec>
#include <QShortcut>
#include <QScreen>
#include <QPoint>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    mSocket = new QUdpSocket();     // left Socket
    sSocket = new QUdpSocket();     // right Socket
    bSocket = new QUdpSocket();     // Broadcast Socket
    mSocket->bind(QHostAddress::AnyIPv4,leftport);  // mSocket->bind(2001,QUdpSocket::ShareAddress);
    sSocket->bind(QHostAddress::AnyIPv4,rightport); // mSocket->close(); to close port
    bSocket->bind(QHostAddress::AnyIPv4,1021);      // Broadcast
    connect(ui->horizontalSlider1_1, SIGNAL(valueChanged(int)), ui->spinBox1_1, SLOT(setValue(int)));
    connect(ui->spinBox1_1, SIGNAL(valueChanged(int)), ui->horizontalSlider1_1, SLOT(setValue(int)));
    connect(ui->horizontalSlider1_2, SIGNAL(valueChanged(int)), ui->spinBox1_2, SLOT(setValue(int)));
    connect(ui->spinBox1_2, SIGNAL(valueChanged(int)), ui->horizontalSlider1_2, SLOT(setValue(int)));
    connect(ui->horizontalSlider1_3, SIGNAL(valueChanged(int)), ui->spinBox1_3, SLOT(setValue(int)));
    connect(ui->spinBox1_3, SIGNAL(valueChanged(int)), ui->horizontalSlider1_3, SLOT(setValue(int)));
    connect(ui->horizontalSlider1_4, SIGNAL(valueChanged(int)), ui->spinBox1_4, SLOT(setValue(int)));
    connect(ui->spinBox1_4, SIGNAL(valueChanged(int)), ui->horizontalSlider1_4, SLOT(setValue(int)));
    connect(ui->horizontalSlider1_5, SIGNAL(valueChanged(int)), ui->spinBox1_5, SLOT(setValue(int)));
    connect(ui->spinBox1_5, SIGNAL(valueChanged(int)), ui->horizontalSlider1_5, SLOT(setValue(int)));
    connect(ui->horizontalSlider1_6, SIGNAL(valueChanged(int)), ui->spinBox1_6, SLOT(setValue(int)));
    connect(ui->spinBox1_6, SIGNAL(valueChanged(int)), ui->horizontalSlider1_6, SLOT(setValue(int)));
    ui->spinBox1_6->setValue(100);
    ui->spinBox1_5->setValue(380);
    ui->spinBox1_4->setValue(100);
    ui->spinBox1_3->setValue(100);
    ui->spinBox1_2->setValue(85);
    ui->spinBox1_1->setValue(115);
    connect(mSocket,SIGNAL(readyRead()),this,SLOT(read_L()));
    connect(sSocket,SIGNAL(readyRead()),this,SLOT(read_R()));
    connect(bSocket,SIGNAL(readyRead()),this,SLOT(broadcastmessage()));
    myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(periodMessage()));
    myTimer->start(1000);
    ui->dateTimeEdit->setReadOnly(true);    // set datetimeEdit component readonly;
    setWindowTitle(QString::fromUtf8("NEBULA CHANGER, Clicking F1 for Help"));
    setWindowIcon(QIcon(":/sokit.png"));
    helpform * h = new helpform;
//    QRect rec = QGuiApplication::screenAt(this->pos())->geometry();
//    QPoint topLeft = QPoint((rec.width() / 2) - (h->size().width() ), (rec.height() / 2) - (h->size().height() / 2));
//    h->setGeometry(QRect(topLeft, h->size()));
    QShortcut * k = new QShortcut(QKeySequence(Qt::Key_F1), this);
    connect(k, SIGNAL(activated()), h, SLOT(exec()));
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
    QByteArray ba= QByteArray::fromHex("55");
    bSocket->writeDatagram(ba,1,QHostAddress::Broadcast,1021);
}
void Widget::broadcastmessage()
{
//    ui->comboBox->clear();
    QByteArray array;
    array.resize(bSocket->bytesAvailable());
    QHostAddress recvaddress;
    quint16 port;
    bSocket->readDatagram(array.data(),array.size(),&recvaddress,&port);
//    QByteArray array = QByteArray::fromHex("4e6562756c615f4368617267696e6733c0a8013cac54c3a1e23c");
    quint8 val1 = array.data()[16];
    quint8 val2 = array.data()[17];
    quint8 val3 = array.data()[18];
    quint8 val4 = array.data()[19];
    QString Ipstr = QString::asprintf("%d.%d.%d.%d" ,val1,val2,val3,val4);
    QString macstr = (array.mid(20,10).toHex());
    QString string(array.remove(16,10));
    if (Device_Node.contains(macstr) || macstr == "")
        return;
    Device_Node.insert(macstr, Ipstr);
    foreach(const QString &str, Device_Node.keys())
        {
        ui->comboBox->addItem("Name: " + string + " IP: " + Ipstr + " Mac: " + macstr,Device_Node.value(str));
    }
    member.data()[4] = 0x55;
}
// period send datagram
static unsigned char buf8[] = {0x02, 0x00, 0x03, 0xa0, 0x0d, 0x00}; // read real time
static unsigned char buf1[] = {0x02, 0x00, 0x03, 0xa0, 0x32, 0x00};    // read working､ status
static unsigned char buf2[] = {0x02, 0x00, 0x03, 0xa0, 0x09, 0x00};    // read eletronic locks
static unsigned char buf3[] = {0x02, 0x00, 0x04, 0xa0, 0x00, 0x01, 0x00}; // read work mode
static unsigned char buf4[] = {0x02, 0x00, 0x03, 0xa0, 0x0f, 0x00}; // read K1/K2
static unsigned char buf5[] = {0x02, 0x00, 0x03, 0xa0, 0x11, 0x00}; // read Ki/Kii
static unsigned char buf6[] = {0x02, 0x00, 0x03, 0xa0, 0x10, 0x00}; // read K3/K4
static unsigned char buf7[] = {0x02, 0x00, 0x03, 0xa0, 0x20, 0x00}; // read system temprature
static unsigned char buf9[] = {0x02, 0x00, 0x03, 0xa0, 0x0b, 0x00}; // show insulation detection
void Widget::periodMessage()
{
    if(member.at(4) != 0x00)
    {
        emit sendDatagram(buf8,Len1,leftport,mSocket);
        emit sendDatagram(buf1,Len1,leftport,mSocket);
        emit sendDatagram(buf1,Len1,rightport,sSocket);
        emit sendDatagram(buf2,Len1,leftport,mSocket);
        emit sendDatagram(buf2,Len1,rightport,sSocket);
        emit sendDatagram(buf3,Len2,leftport,mSocket);
        emit sendDatagram(buf3,Len2,rightport,sSocket);
        emit sendDatagram(buf4,Len1,leftport,mSocket);
        emit sendDatagram(buf4,Len1,rightport,sSocket);
        emit sendDatagram(buf5,Len1,leftport,mSocket);
        emit sendDatagram(buf6,Len1,leftport,mSocket);
        emit sendDatagram(buf6,Len1,rightport,sSocket);
        emit sendDatagram(buf7,Len1,leftport,mSocket);
        emit sendDatagram(buf7,Len1,rightport,sSocket);
        emit sendDatagram(buf9,Len1,leftport,mSocket);
        emit sendDatagram(buf9,Len1,rightport,sSocket);
    }
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
static unsigned char eleLock[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00};
static unsigned char km1[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00};
static unsigned char mode[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char kmii[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00};
static unsigned char km2[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00};
void Widget::processPendingDatagrams(QUdpSocket * rSocket)
{
    while (rSocket->hasPendingDatagrams())
    {
        QByteArray array;
        QHostAddress address;
        quint16 port;
        QDateTime time = QDateTime::currentDateTime();
        QString timeblock = time.toString("hh:mm:ss");    // or "yyyy-MM-dd hh:mm:ss dddd"
        array.resize(rSocket->bytesAvailable());    // or array.resize(Socket->pendingDatagramSize());
        int size = array.size();
        rSocket->readDatagram(array.data(),array.size(),&address,&port);
        if(array.isEmpty())
            return;
        ui->listWidget->addItem(timeblock + "   " +array.toHex()); //  or ui->listWidget->insertItem(1, array);
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
            // show work mode
            if (arraycmp(mode, buf, sizeof (mode), sizeof (buf)) == 1)
            {
                switch (buf[size-4])
                {
                case 0:
                    ui->label1_1->setText(QString::fromUtf8("自动"));
                    ui->pushButton2_6->setEnabled(true);
                    ui->pushButton2_5->setEnabled(true);
                    break;
                case 1:
                    ui->label1_1->setText(QString::fromUtf8("手动"));
                    member.data()[0] = 0x55; // Working mode
                    ui->pushButton2_6->setEnabled(false);
                    ui->pushButton2_5->setEnabled(false);
                    break;
                default:
                    ui->label1_1->setText(QString::fromUtf8("F "));
                    member.data()[3] = 0x55;
                    qDebug() << "Network is not connnected";
                    return;
                }
            }
            if (port == 2001)
            {
                // show eletronic locks
                if (arraycmp(eleLock, buf, sizeof (eleLock), sizeof (buf)) == 1)
                {
                    if (buf[size-4] == 0x00)
                    {
                        ui->label1->setText(QString::fromUtf8("锁定"));
                    }
                    else {
                        if (buf[size-4] == 0x01)
                             ui->label1->setText(QString::fromUtf8("解锁"));
                    }
                }
                // show K1/K2
                if (arraycmp(km1, buf, sizeof (km1), sizeof (buf)) == 1)
                {
                    if (buf[size-4] == 0x00)
                        ui->label2_1->setText(QString::fromUtf8("弹开"));
                    else {
                        if (buf[size-4] == 0x01)
                             ui->label2_1->setText(QString::fromUtf8("吸合"));
                    }
                }
                // show K3/K4
                if (arraycmp(km2, buf, sizeof (km2), sizeof (buf)) == 1)
                {
                    if (buf[size-4] == 0x00)
                        ui->label2_2->setText(QString::fromUtf8("弹开"));
                    else {
                        if (buf[size-4] == 0x01)
                             ui->label2_2->setText(QString::fromUtf8("吸合"));
                    }
                }
                // system temprature
                if (buf[8] == 0x20 && buf[0] == 0x0f)
                {
                    ui->lcdNumber1_7->display((buf[10]<<8)+buf[11]);     // Power module PT1000 temperature
                    ui->lcdNumber2_7->display((buf[12]<<8)+buf[13]);     // anode temperature
                    ui->lcdNumber2_8->display((buf[14]<<8)+buf[15]);     // cathode temperature
                }
                // show work status
                if (buf[0] == 0x36) // array start length byte!!
                {
                    ui->lcdNumber1_1->display(buf[52]);     // Power module #1 temperature
                    ui->lcdNumber1_2->display(buf[53]);     // Power module #2 temperature
                    ui->lcdNumber1_3->display(buf[54]);     // Power module #3 temperature
                    if (buf[10] == 0x00)
                        ui->label2_3->setText(QString::fromUtf8("Free"));  // Device Charing Status
                    else if (buf[11] == 0x01) {
                        ui->label2_3->setText(QString::fromUtf8("Loading"));
                        member.data()[2] = 0x55;
                    }
                    quint32 temp1 = (buf[13]<<24)+(buf[14]<<16)+(buf[15]<<8)+buf[16];
                    ui->label2_5->setText(tr("%1 V").arg(temp1));   // MBS Demand Voltage
                    // MBS Demand Current
                    quint32 temp2 = (buf[21]<<24)+(buf[22]<<16)+(buf[23]<<8)+buf[24];
                    double temp3 = ((double) temp2)/1000;
                    ui->lcdNumber2_1->display(temp3);   // Output Voltage
                    quint32 temp4 = (buf[25]<<24)+(buf[26]<<16)+(buf[27]<<8)+buf[28];
                    double temp5 = ((double) temp4)/1000;
                    ui->lcdNumber2_2->display(temp5);   // Output Current
                    int temp6 = (buf[29]<<8) + buf[30];
                    double temp7 = ((double)temp6)/10;
                    ui->lcdNumber2_3->display(temp7);   // AB Voltage
                    temp6 = (buf[31]<<8) + buf[32];
                    temp7 = ((double)temp6)/10;
                    ui->lcdNumber2_4->display(temp7);   // BC Voltage
                    temp6 = (buf[33]<<8) + buf[34];
                    temp7 = ((double)temp6)/10;
                    ui->lcdNumber2_5->display(temp7);   // CA Voltage
                    ui->progressBar->setValue(buf[35]);     //  BMS SOC
                    if(buf[45] == 0x06)
                        ui->label2_12->setText(QString::fromUtf8("Loading"));
                    if(buf[45 == 0x05])
                        ui->label2_12->setText(QString::fromUtf8("Free"));
                    member.data()[1] = buf[45];     // Power module Status
                }
                // show Ki/Kii
                if (arraycmp(kmii, buf, sizeof (kmii), sizeof (buf)) == 1)
                {
                    if (buf[size-4] == 0x00)
                        ui->label1_2->setText(QString::fromUtf8("弹开"));
                    else {
                        if (buf[size-4] == 0x01)
                             ui->label1_2->setText(QString::fromUtf8("吸合"));
                    }
                }
                if (buf[0] == 0x10 && buf[8] == 0x0d)
                {
                    ui->dateTimeEdit->setDateTime(QDateTime::fromString(QString("%1-%2-%3 %4:%5:%6")
                                                                        .arg((buf[10]<<8)+buf[11])
                            .arg(buf[12]).arg(buf[13]).arg(buf[14]).arg(buf[15]).arg(buf[16]), "yyyy-MM-dd hh:mm:ss"));
                }
                if (buf[0] == 0x13 && buf[8] == 0x0b)
                {
                    ui->lcdNumber2_10->display((buf[10]<<8)+buf[11]);     // anode resistance
                    ui->lcdNumber2_11->display((buf[12]<<8)+buf[13]);     // cathode resistance
                    ui->lcdNumber2_9->display((double(buf[14]<<8)+buf[15])/10);      // measurng voltage
                    ui->lcdNumber2_12->display(double((buf[16]<<8)+buf[17])/10);     // anode insulation
                    ui->lcdNumber2_13->display(double((buf[18]<<8)+buf[19])/10);     // cathode insulation
                }
            }
            // right port
            else {
                // show eletronic locks
                if (arraycmp(eleLock, buf, sizeof (eleLock), sizeof (buf)) == 1)
                {
                    if (buf[size-4] == 0x00)
                        ui->label3_1->setText(QString::fromUtf8("锁定"));
                    else {
                        if (buf[size-4] == 0x01)
                             ui->label3_1->setText(QString::fromUtf8("解锁"));
                    }
                }
                // show K1/K2
                if (arraycmp(km1, buf, sizeof (km1), sizeof (buf)) == 1)
                {
                    if (buf[size-4] == 0x00)
                        ui->label3_2->setText(QString::fromUtf8("弹开"));
                    else {
                        if (buf[size-4] == 0x01)
                             ui->label3_2->setText(QString::fromUtf8("吸合"));
                    }
                }
                // show work status
                if (buf[0] == 0x36)
                {
    //                ui->label1_10->setText(tr("%1 °C").arg(buf[54]));   //ui->label1_10->setStyleSheet("color:red;");
                    ui->lcdNumber1_4->display(buf[52]);
                    ui->lcdNumber1_5->display(buf[53]);
                    ui->lcdNumber1_6->display(buf[54]);

                }
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
            {
                e = 0;
                break;
            }
            count++;
        }
    else {
        e = 0;
    }
    return e;
}

// send Messages for call
void Widget::sendDatagram(unsigned char buf[], short Length, quint16 port, QUdpSocket *Socket)
{
        unsigned char *p = sumCheck(buf,Length);
        QByteArray ba(reinterpret_cast<char*>(p), Length);
        Socket->writeDatagram(ba,Length,QHostAddress(stripAdress),port);
        delete p;
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
    if(ui->checkBox1_2->isChecked())
        buf[5] = 0x02;
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "K3/K4 turn off";
}

void Widget::on_radioButton2_4_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x06, 0x01};
    emit sendDatagram(buf,Len2,leftport,mSocket);
    if(ui->checkBox1_2->isChecked())
        buf[5] = 0x02;
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
    unsigned char buf[14] = {0x02, 0x00, 0x0b, 0xa0, 0x03, 0x00}; // no define element default set to 0x00
    if(ui->checkBox1_1->isChecked())
        buf[4] = 0x1d;
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
    if(member.at(1) == 0x06)        // member.at(1) == 0x06 Power Modele is working
        buf[5] = 0x01;
    emit sendDatagram(buf,15,leftport,mSocket);
    if(member.at(1) == 0x05)        // member.at(1) == 0x06 Power Modele is free
    {
        ui->pushButton2_2->setEnabled(true);
    }
    qDebug() << "Loading voltage and current";
}

// process Qstring to QByteArray
QByteArray Widget::processQString(QString item, int k) // or can try str.toLatin1()/str.toUtf8()
{
    QByteArray dat;
    int val = item.toInt() * k; // int < 2147483648
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
    if(ui->checkBox1_1->isChecked())
        buf[4] = 0x1d;
    emit sendDatagram(buf,15,leftport,mSocket);
    if(member.at(1) == 0x05)
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
    str = str.right(str.size()-11);
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
    ui->label2_11->setText(QString::fromUtf8("秒 / s "));
}

void Widget::on_radioButton2_8_clicked()
{
    ui->label2_11->setText(QString::fromUtf8("元/Yuan"));
}

void Widget::on_radioButton2_6_clicked()
{
    ui->label2_11->setText(QString::fromUtf8("KW·h/ ° "));
}

void Widget::on_radioButton2_5_clicked()
{
    ui->label2_11->setText(QString::fromUtf8("    "));
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

void Widget::on_radioButton2_11_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x05, 0x00};
    emit sendDatagram(buf,Len2,leftport,mSocket);
    if(ui->checkBox1_3->isChecked())
        emit sendDatagram(buf,Len2,rightport,sSocket);
    ui->label2_10->setText(QString::fromUtf8("Close_inValid"));
    qDebug() << "Fans turn off";
}

void Widget::on_radioButton2_12_clicked()
{
    unsigned char buf[] = {0x02, 0x00, 0x04, 0xa0, 0x04, 0x05, 0x01};
    emit sendDatagram(buf,Len2,leftport,mSocket);
    if(ui->checkBox1_3->isChecked())
        emit sendDatagram(buf,Len2,rightport,sSocket);
    ui->label2_10->setText(QString::fromUtf8("Open_inValid"));
    qDebug() << "Fans turn on";
}

void Widget::on_comboBox_currentIndexChanged(const QString &arg1)
{
    stripAdress = ui->comboBox->currentData().toString();
    member.data()[4] = 0x55;
    qDebug() << "mac and ip" << arg1;
}

void Widget::on_pushButton_2_clicked()
{
    unsigned char buf[9] = {0x02, 0x00, 0x06, 0xa0, 0x27};
    QDateTime time = QDateTime::currentDateTime();
    quint32 timeT = time.toTime_t();
    buf[8] = (uchar)(0x000000ff & timeT);
    buf[7] = (uchar)((0x0000ff00 & timeT)>>8);
    buf[6] = (uchar)((0x00ff0000 & timeT)>>16);
    buf[5] = (uchar)((0xff000000 & timeT)>>24);
    emit sendDatagram(buf,10,leftport,mSocket);
    qDebug() << "Synchronization time is completed";
}

void Widget::on_pushButton2_8_clicked()
{
    unsigned char buf[6] = {0x02, 0x00, 0x03, 0xa0, 0x0a, 0x01};
    emit sendDatagram(buf,Len1,leftport,mSocket);
    qDebug() << "Insulation detection finishing";
}

void Widget::on_pushButton2_7_clicked()
{
    unsigned char buf[6] = {0x02, 0x00, 0x03, 0xa0, 0x0a};
    emit sendDatagram(buf,Len1,leftport,mSocket);
    qDebug() << "Insulation detection starting";
}

void Widget::on_horizontalSlider1_2_valueChanged(int value)
{
    uchar buf[8] = {0x02, 0x00, 0x05, 0xa0, 0x2f};
    buf[6] = (uchar)(ui->spinBox1_2->value() & 0x000000ff);
    buf[5] = (uchar)((ui->spinBox1_2->value() & 0x0000ff00)>>8);
    buf[7] = (uchar)value;
    emit sendDatagram(buf,9,leftport,mSocket);
    qDebug() << "Set undervoltage protection value";
}

void Widget::on_horizontalSlider1_1_valueChanged(int value)
{
    uchar buf[8] = {0x02, 0x00, 0x05, 0xa0, 0x30};
    buf[6] = (uchar)(ui->spinBox1_1->value() & 0x000000ff);
    buf[5] = (uchar)((ui->spinBox1_1->value() & 0x0000ff00)>>8);
    buf[7] = (uchar)value;
    emit sendDatagram(buf,9,leftport,mSocket);
    qDebug() << "Set overvoltage protection value";
}

void Widget::on_horizontalSlider1_3_valueChanged(int value)
{
    uchar buf[7] = {0x02, 0x00, 0x04, 0xa0, 0x1f};
    buf[6] = (uchar)value;
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "Constant power limit";
}

void Widget::on_horizontalSlider1_4_valueChanged(int value)
{
    uchar buf[12] = {0x02, 0x00, 0x09, 0xa0, 0x33};
    buf[6] = (uchar)ui->spinBox1_6->value();
    buf[8] = (uchar)value;
    buf[10] = buf[8];
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "Pole temperature protection";
}

void Widget::on_horizontalSlider1_6_valueChanged(int value)
{
    uchar buf[12] = {0x02, 0x00, 0x09, 0xa0, 0x33};
    buf[6] = (uchar)value;
    buf[8] = (uchar)ui->spinBox1_4->value();
    buf[10] = buf[8];
    emit sendDatagram(buf,Len2,leftport,mSocket);
    qDebug() << "Power module temperature protection";
}
