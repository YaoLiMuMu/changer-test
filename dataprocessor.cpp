#include "dataprocessor.h"
#include "widget.h"

DataProcessor::DataProcessor(QObject *parent) : QObject(parent)
{

}

// process Datagrams
static unsigned char eleLock[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00};
static unsigned char km1[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00};
static unsigned char mode[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char kmii[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00};
static unsigned char km2[] = {0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00};

void DataProcessor::ReceiveDatagrams()
{
    lSocket = new QUdpSocket();     // left Socket
    rSocket = new QUdpSocket();     // right Socket
    lSocket->bind(leftport, QUdpSocket::ShareAddress);  // or bind(QHostAddress::AnyIPv4,leftport);
    rSocket->bind(rightport, QUdpSocket::ShareAddress);
    connect(lSocket,SIGNAL(readyRead()),this,SLOT(LprocessDatagrams()));
    connect(rSocket,SIGNAL(readyRead()),this,SLOT(RprocessDatagrams()));
}

void DataProcessor::LprocessDatagrams()
{
    qDebug() << "Process Datagrams thread ID: " << QThread::currentThreadId();
    while (lSocket->hasPendingDatagrams())
    {
        emit Send2UI(msg);
        msg.maplist.clear();
        msg.lcdnum.clear();
        msg.lcdisplay.clear();
        msg.Flag.clear();
        QByteArray array;
        QHostAddress address;
        quint16 port;
        QDateTime nowtime = QDateTime::currentDateTime();
        QString timeblock = nowtime.toString("hh:mm:ss");    // or "yyyy-MM-dd hh:mm:ss dddd"
        int size;
        size = int(lSocket->bytesAvailable());
        array.resize(size);    // or array.resize(Socket->pendingDatagramSize());
        lSocket->readDatagram(array.data(),size,&address,&port);
        if(array.isEmpty())
            continue;
        msg.maplist.insert("listWidget",(timeblock + "   " +array.toHex())); //  or ui->listWidget->insertItem(1, array);
        qDebug() << "port " << port << ": " << array.toHex();
        char sum = 0x00;    // cant use unsigned char, because QBytearray[] is char
        for (int i = 0; i < (size-1); ++i) {
            sum+=array.at(i);
        }   // sum cleck
        if (sum == array.at(size-1))
        {
            array.remove(size-1,1); // truncate sumcheck byte
            array.remove(0,2);  //  truncate 0x0200
            unsigned char buf[size-3];
            memcpy(buf,array,ulong(size-3));
            // show work mode
            if (arraycmp(mode, buf, sizeof (mode), sizeof (buf)) == 1)
            {
                switch (buf[size-4])
                {
                case 0:
                    msg.maplist.insert("label1_1", QString::fromUtf8("自动"));
                    msg.Flag.insert("bt2_6", true);
                    msg.Flag.insert("bt2_5", true);
                    continue;
                case 1:
                    msg.maplist.insert("label1_1", QString::fromUtf8("手动"));
                    msg.Flag.insert("manual_mode", true);
                    msg.Flag.insert("bt2_6", false);
                    msg.Flag.insert("bt2_5", false);
                    continue;
                default:
                    msg.maplist.insert("label1_1", QString::fromUtf8("F"));
                    continue;
                }
            }
            // show eletronic locks
            if (arraycmp(eleLock, buf, sizeof (eleLock), sizeof (buf)) == 1)
            {
                switch (buf[size-4])
                {
                case 0:
                    msg.maplist.insert("label1",QString::fromUtf8("锁定"));
                    continue;
                case 1:
                    msg.maplist.insert("label1",QString::fromUtf8("解锁"));
                    continue;
                default:
                    msg.maplist.insert("label1", "F");
                }
            }
            // show K1/K2
            if (arraycmp(km1, buf, sizeof (km1), sizeof (buf)) == 1)
            {
                switch (buf[size-4])
                {
                case 0:
                    msg.maplist.insert("label2_1",(QString::fromUtf8("弹开")));
                    continue;
                case 1:
                    msg.maplist.insert("label2_1",(QString::fromUtf8("吸合")));
                    continue;
                default:
                    msg.maplist.insert("label2_1","F");
                }
            }
            //show K3/K4
            if (arraycmp(km2, buf, sizeof (km2), sizeof (buf)) == 1)
            {
                switch (buf[size-4])
                {
                case 0:
                    msg.maplist.insert("label2_2",(QString::fromUtf8("弹开")));
                    continue;
                case 1:
                    msg.maplist.insert("label2_2",(QString::fromUtf8("吸合")));
                    continue;
                default:
                    msg.maplist.insert("label2_2","F");
                }
            }
            // show work status
            if (buf[0] == 0x36) // array start length byte!!
            {
                msg.lcdnum.insert("lcdNumber1_1", buf[52]);     // Power module #1 temperature
                msg.lcdnum.insert("lcdNumber1_2", buf[53]);     // Power module #2 temperature
                msg.lcdnum.insert("lcdNumber1_3", buf[54]);     // Power module #3 temperature
                qint32 temp1 = (buf[13]<<24)+(buf[14]<<16)+(buf[15]<<8)+buf[16];
                msg.maplist.insert("label2_5",(tr("%1 V").arg(temp1)));   // BMS Demand Voltage
                temp1 = (buf[17]<<24)+(buf[18]<<16)+(buf[19]<<8)+buf[20];
                msg.maplist.insert("label2_6",(tr("%1 V").arg(temp1)));   // BMS Demand Current
                qint32 temp2 = (buf[21]<<24)+(buf[22]<<16)+(buf[23]<<8)+buf[24];
                double temp3 = (double (temp2))/1000;
                msg.lcdisplay.insert("lcdNumber2_1", temp3);   // Output Voltage
                qint32 temp4 = (buf[25]<<24)+(buf[26]<<16)+(buf[27]<<8)+buf[28];
                double temp5 = (double (temp4))/1000;
                msg.lcdisplay.insert("lcdNumber2_2", temp5);   // Output Current
                int temp6 = (buf[29]<<8) + buf[30];
                double temp7 = (double(temp6))/10;
                msg.lcdisplay.insert("lcdNumber2_3", temp7);   // AB Voltage
                temp6 = (buf[31]<<8) + buf[32];
                temp7 = (double(temp6))/10;
                msg.lcdisplay.insert("lcdNumber2_4", temp7);   // BC Voltage
                temp6 = (buf[33]<<8) + buf[34];
                temp7 = (double(temp6))/10;
                msg.lcdisplay.insert("lcdNumber2_5", temp7);   // CA Voltage
                msg.lcdnum.insert("progressBar", buf[35]);     //  BMS SOC
                switch (buf[45])
                {
                case 6:
                    msg.maplist.insert("label2_12",(QString::fromUtf8("Loading")));
                    msg.Flag.insert("module_status", true);     // Power module Status
                    break;
                case 5:
                    msg.maplist.insert("label2_12",(QString::fromUtf8("Free")));
                    break;
                default:
                    msg.maplist.insert("label2_12","F");
                }
                switch (buf[10])
                {
                case 0:
                    msg.maplist.insert("label2_3",(QString::fromUtf8("Free")));  // Device Charing Status
                    continue;
                case 1:
                    msg.maplist.insert("lable2_3",(QString::fromUtf8("Loading")));
                    msg.Flag.insert("charging_status",true);
                    continue;
                default:
                    msg.maplist.insert("lable2_3","F");
                }
            }
            // system temprature
            if (buf[8] == 0x20 && buf[0] == 0x0f)
            {
                msg.lcdnum.insert("lcdNumber1_7", (buf[10]<<8)+buf[11]);     // Power module PT1000 temperature
                msg.lcdnum.insert("lcdNumber2_7", (buf[12]<<8)+buf[13]);     // anode temperature
                msg.lcdnum.insert("lcdNumber2_8", (buf[14]<<8)+buf[15]);     // cathode temperature
                continue;
            }
            // show Ki/Kii
            if (arraycmp(kmii, buf, sizeof (kmii), sizeof (buf)) == 1)
            {
                switch (buf[size-4])
                {
                case 0:
                    msg.maplist.insert("label1_2",(QString::fromUtf8("弹开")));
                    continue;
                case 1:
                    msg.maplist.insert("label1_2",(QString::fromUtf8("吸合")));
                    continue;
                default:
                    msg.maplist.insert("label1_2","F");
                }
            }
            // read device charge error message
            if (buf[0] == 0x25 && buf[8] == 0x34)
            {
                QString Bitemp;
                Bitemp = Bitemp.setNum(buf[16],2);
                if(Bitemp[15] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8("BMS异常: ")+QString::fromUtf8("BHM包超时(BHM_timeover)"));
                // System exception
                Bitemp = QString("%1%2%3%4%").arg(buf[18],8,2,QLatin1Char('0'))
                        .arg(buf[19],8,2,QLatin1Char('0'))
                        .arg(buf[20],8,2,QLatin1Char('0'))
                        .arg(buf[21],8,2,QLatin1Char('0'));   // arg 第二个参数表示字符位数, 第三个参数表示进制, 第四个参数表示自动补全的字符
                if(Bitemp[31] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 系统过温(overTemp)"));
                if(Bitemp[30] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 门禁开关(limitSwitch)"));
                if(Bitemp[29] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 急停开关(EmergencySwitch)"));
                if(Bitemp[28] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 服务器通讯超时(SeverCommTimeout)"));
                if(Bitemp[27] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 服务器启动(severStartCp)"));
                if(Bitemp[26] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 服务器停止(severStopCp)"));
                if(Bitemp[25] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 欠费停止充电(overCashStopCp)"));
                if(Bitemp[24] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 电表通讯异常(ecCommError)"));
                if(Bitemp[23] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 电表与SOC的结束相关的异常(SocEcError)"));
                if(Bitemp[22] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 枪头未归位(gunOffLine)"));
                if(Bitemp[21] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 车枪未连接(gunOffCar)"));
                if(Bitemp[20] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" RTC时间异常(RtcTimeError)"));
                if(Bitemp[19] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 防拆开关1(limitSwitch1)"));
                if(Bitemp[18] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 人工停止, APP停止，屏停止， 急停(manual_stopcp)"));
                if(Bitemp[17] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" BMS故障停止(fault_stopcp)"));
                if(Bitemp[16] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 自动停止(autoStopCp)"));
                if(Bitemp[15] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 记录上传失败(recordUploadFail)"));
                if(Bitemp[14] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 系统记录存储满(recordFull)"));
                if(Bitemp[13] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 程序升级中(update)"));
                if(Bitemp[12] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 禁止使用(Prohibit)"));
                if(Bitemp[11] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 继电器板CAN初始化出错(RelayCanError)"));
                if(Bitemp[10] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 继电器通讯板异常(RealyCommError)"));
                if(Bitemp[9] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 继电器板异常(RelayStaError)"));
                if(Bitemp[8] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 系统异常:  ")+QString::fromUtf8(" 交流接触器报警(AcContactorError)"));
//                Bitemp = Bitemp.setNum(buf[20],2);
            }
            // update real time
            if (buf[0] == 0x10 && buf[8] == 0x0d)
            {
                msg.maplist.insert("dateTimeEdit", (QString("%1-%2-%3 %4:%5:%6").arg((buf[10]<<8)+buf[11])
                        .arg(buf[12]).arg(buf[13]).arg(buf[14]).arg(buf[15]).arg(buf[16])));
                continue;
            }
            // read IoT code
//            if (buf[0] == 0x0f && buf[] == 0x00)
            if (buf[0] == 0x13 && buf[8] == 0x0b)
            {
                msg.lcdnum.insert("lcdNumber2_10", (buf[10]<<8)+buf[11]);     // anode resistance
                msg.lcdnum.insert("lcdNumber2_11", (buf[12]<<8)+buf[13]);     // cathode resistance
                msg.lcdisplay.insert("lcdNumber2_9", double((buf[14]<<8)+buf[15])/10);      // measurng voltage
                msg.lcdisplay.insert("lcdNumber2_12", double((buf[16]<<8)+buf[17])/10);     // anode insulation
                msg.lcdisplay.insert("lcdNumber2_13", double((buf[18]<<8)+buf[19])/10);     // cathode insulation
            }
        }
    }
}

void DataProcessor::RprocessDatagrams()
{
//    while (sSocket->hasPendingDatagrams())
//    {
//        QByteArray array;
//        QHostAddress address;
//        quint16 port;
//        QDateTime time = QDateTime::currentDateTime();
//        QString timeblock = time.toString("hh:mm:ss");    // or "yyyy-MM-dd hh:mm:ss dddd"
//        array.resize(sSocket->bytesAvailable());    // or array.resize(Socket->pendingDatagramSize());
//        int size = array.size();
//        sSocket->readDatagram(array.data(),array.size(),&address,&port);
//        if(array.isEmpty())
//            return;
//        ui->listWidget->addItem(timeblock + "   " +array.toHex()); //  or ui->listWidget->insertItem(1, array);
//        qDebug() << "port " << port << ": " << array.toHex();
//        char sum = 0x00;    // cant use unsigned char because QBytearray[] is char
//        for (int i = 0; i < (size-1); ++i) {
//            sum+=array.at(i);
//        }   // sum cleck
//        if (sum == array.at(size-1))
//        {
//            array.remove(size-1,1); // truncate sumcheck byte
//            array.remove(0,2);  //  truncate 0x0200
//            unsigned char buf[size-3];
//            memcpy(buf,array,size-3);
//            // show work mode
//            if (arraycmp(mode, buf, sizeof (mode), sizeof (buf)) == 1)
//            {
//                switch (buf[size-4])
//                {
//                case 0:
//                    ui->label1_1->setText(QString::fromUtf8("自动"));
//                    ui->pushButton2_6->setEnabled(true);
//                    ui->pushButton2_5->setEnabled(true);
//                    break;
//                case 1:
//                    ui->label1_1->setText(QString::fromUtf8("手动"));
////                    member.data()[0] = 0x55; // Working mode
//                    ui->pushButton2_6->setEnabled(false);
//                    ui->pushButton2_5->setEnabled(false);
//                    break;
//                default:
//                    ui->label1_1->setText(QString::fromUtf8("F "));
////                    member.data()[3] = 0x55;
//                    qDebug() << "Network is not connnected";
//                    return;
//                }
//            }
//            if (port == 2001)
//            {
//                // show eletronic locks
//                if (arraycmp(eleLock, buf, sizeof (eleLock), sizeof (buf)) == 1)
//                {
//                    if (buf[size-4] == 0x00)
//                    {
//                        ui->label1->setText(QString::fromUtf8("锁定"));
//                    }
//                    else {
//                        if (buf[size-4] == 0x01)
//                             ui->label1->setText(QString::fromUtf8("解锁"));
//                    }
//                }
//                // show K1/K2
//                if (arraycmp(km1, buf, sizeof (km1), sizeof (buf)) == 1)
//                {
//                    if (buf[size-4] == 0x00)
//                        ui->label2_1->setText(QString::fromUtf8("弹开"));
//                    else {
//                        if (buf[size-4] == 0x01)
//                             ui->label2_1->setText(QString::fromUtf8("吸合"));
//                    }
//                }
//                // show K3/K4
//                if (arraycmp(km2, buf, sizeof (km2), sizeof (buf)) == 1)
//                {
//                    if (buf[size-4] == 0x00)
//                        ui->label2_2->setText(QString::fromUtf8("弹开"));
//                    else {
//                        if (buf[size-4] == 0x01)
//                             ui->label2_2->setText(QString::fromUtf8("吸合"));
//                    }
//                }
//                // system temprature
//                if (buf[8] == 0x20 && buf[0] == 0x0f)
//                {
//                    ui->lcdNumber1_7->display((buf[10]<<8)+buf[11]);     // Power module PT1000 temperature
//                    ui->lcdNumber2_7->display((buf[12]<<8)+buf[13]);     // anode temperature
//                    ui->lcdNumber2_8->display((buf[14]<<8)+buf[15]);     // cathode temperature
//                }
//                // show work status
//                if (buf[0] == 0x36) // array start length byte!!
//                {
//                    ui->lcdNumber1_1->display(buf[52]);     // Power module #1 temperature
//                    ui->lcdNumber1_2->display(buf[53]);     // Power module #2 temperature
//                    ui->lcdNumber1_3->display(buf[54]);     // Power module #3 temperature
//                    if (buf[10] == 0x00)
//                        ui->label2_3->setText(QString::fromUtf8("Free"));  // Device Charing Status
//                    else if (buf[11] == 0x01) {
//                        ui->label2_3->setText(QString::fromUtf8("Loading"));
////                        member.data()[2] = 0x55;
//                    }
//                    quint32 temp1 = (buf[13]<<24)+(buf[14]<<16)+(buf[15]<<8)+buf[16];
//                    ui->label2_5->setText(tr("%1 V").arg(temp1));   // MBS Demand Voltage
//                    // MBS Demand Current
//                    quint32 temp2 = (buf[21]<<24)+(buf[22]<<16)+(buf[23]<<8)+buf[24];
//                    double temp3 = ((double) temp2)/1000;
//                    ui->lcdNumber2_1->display(temp3);   // Output Voltage
//                    quint32 temp4 = (buf[25]<<24)+(buf[26]<<16)+(buf[27]<<8)+buf[28];
//                    double temp5 = ((double) temp4)/1000;
//                    ui->lcdNumber2_2->display(temp5);   // Output Current
//                    int temp6 = (buf[29]<<8) + buf[30];
//                    double temp7 = ((double)temp6)/10;
//                    ui->lcdNumber2_3->display(temp7);   // AB Voltage
//                    temp6 = (buf[31]<<8) + buf[32];
//                    temp7 = ((double)temp6)/10;
//                    ui->lcdNumber2_4->display(temp7);   // BC Voltage
//                    temp6 = (buf[33]<<8) + buf[34];
//                    temp7 = ((double)temp6)/10;
//                    ui->lcdNumber2_5->display(temp7);   // CA Voltage
//                    ui->progressBar->setValue(buf[35]);     //  BMS SOC
//                    if(buf[45] == 0x06)
//                        ui->label2_12->setText(QString::fromUtf8("Loading"));
//                    if(buf[45 == 0x05])
//                        ui->label2_12->setText(QString::fromUtf8("Free"));
////                    member.data()[1] = buf[45];     // Power module Status
//                }
//                // show Ki/Kii
//                if (arraycmp(kmii, buf, sizeof (kmii), sizeof (buf)) == 1)
//                {
//                    if (buf[size-4] == 0x00)
//                        ui->label1_2->setText(QString::fromUtf8("弹开"));
//                    else {
//                        if (buf[size-4] == 0x01)
//                             ui->label1_2->setText(QString::fromUtf8("吸合"));
//                    }
//                }
//                if (buf[0] == 0x10 && buf[8] == 0x0d)
//                {
//                    ui->dateTimeEdit->setDateTime(QDateTime::fromString(QString("%1-%2-%3 %4:%5:%6")
//                                                                        .arg((buf[10]<<8)+buf[11])
//                            .arg(buf[12]).arg(buf[13]).arg(buf[14]).arg(buf[15]).arg(buf[16]), "yyyy-MM-dd hh:mm:ss"));
//                }
//                if (buf[0] == 0x13 && buf[8] == 0x0b)
//                {
//                    ui->lcdNumber2_10->display((buf[10]<<8)+buf[11]);     // anode resistance
//                    ui->lcdNumber2_11->display((buf[12]<<8)+buf[13]);     // cathode resistance
//                    ui->lcdNumber2_9->display((double(buf[14]<<8)+buf[15])/10);      // measurng voltage
//                    ui->lcdNumber2_12->display(double((buf[16]<<8)+buf[17])/10);     // anode insulation
//                    ui->lcdNumber2_13->display(double((buf[18]<<8)+buf[19])/10);     // cathode insulation
//                }
//            }
//            // right port
//            else {
//                // show eletronic locks
//                if (arraycmp(eleLock, buf, sizeof (eleLock), sizeof (buf)) == 1)
//                {
//                    if (buf[size-4] == 0x00)
//                        ui->label3_1->setText(QString::fromUtf8("锁定"));
//                    else {
//                        if (buf[size-4] == 0x01)
//                             ui->label3_1->setText(QString::fromUtf8("解锁"));
//                    }
//                }
//                // show K1/K2
//                if (arraycmp(km1, buf, sizeof (km1), sizeof (buf)) == 1)
//                {
//                    if (buf[size-4] == 0x00)
//                        ui->label3_2->setText(QString::fromUtf8("弹开"));
//                    else {
//                        if (buf[size-4] == 0x01)
//                             ui->label3_2->setText(QString::fromUtf8("吸合"));
//                    }
//                }
//                // show work status
//                if (buf[0] == 0x36)
//                {
//    //                ui->label1_10->setText(tr("%1 °C").arg(buf[54]));   //ui->label1_10->setStyleSheet("color:red;");
//                    ui->lcdNumber1_4->display(buf[52]);
//                    ui->lcdNumber1_5->display(buf[53]);
//                    ui->lcdNumber1_6->display(buf[54]);

//                }
//            }
//        }
//    }
}


bool DataProcessor::arraycmp(unsigned char arrayA[], unsigned char arrayB[], unsigned long a, unsigned long b)
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
