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
    vSocket = new QUdpSocket();     // read version Socket
    lSocket->bind(leftport, QUdpSocket::ShareAddress);  // or bind(QHostAddress::AnyIPv4,leftport);
    rSocket->bind(rightport, QUdpSocket::ShareAddress);
    vSocket->bind(2000, QUdpSocket::ShareAddress);
    connect(lSocket,SIGNAL(readyRead()),this,SLOT(LprocessDatagrams()));
    connect(rSocket,SIGNAL(readyRead()),this,SLOT(RprocessDatagrams()));
    connect(vSocket,SIGNAL(readyRead()),this,SLOT(VprocessDatagrams()));
}

void DataProcessor::VprocessDatagrams()
{
    while (vSocket->hasPendingDatagrams())
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
        size = int(vSocket->bytesAvailable());
        array.resize(size);    // or array.resize(Socket->pendingDatagramSize());
        vSocket->readDatagram(array.data(),size,&address,&port);
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
            // read Software version
            if (array.at(0) == '\x1b' && array.at(1) == '\x99')
            {
                QString str(array.mid(4,14));
                msg.maplist.insert("label1_6",str);
            }
        }
    }
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
                    msg.Flag.insert("manual_mode", false);
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
                    msg.Flag.insert("module_status", false);
                    break;
                default:
                    msg.maplist.insert("label2_12","F");
                }
                switch (buf[10])
                {
                case 0:
                    msg.maplist.insert("label2_3",(QString::fromUtf8("Free")));  // Device Charing Status
                    msg.Flag.insert("charging_status",false);
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
                Bitemp = QString("%1%2%3%4%5%6%7%8").arg(buf[10],8,2,QLatin1Char('0'))
                        .arg(buf[11],8,2,QLatin1Char('0'))
                        .arg(buf[12],8,2,QLatin1Char('0'))
                        .arg(buf[13],8,2,QLatin1Char('0'))
                        .arg(buf[14],8,2,QLatin1Char('0'))
                        .arg(buf[15],8,2,QLatin1Char('0'))
                        .arg(buf[16],8,2,QLatin1Char('0'))
                        .arg(buf[17],8,2,QLatin1Char('0'));// arg 第二个参数表示字符位数, 第三个参数表示进制, 第四个参数表示自动补全的字符
                if(Bitemp[63] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" DC外侧电压异常>10V(DC_low10V_handshake)"));
                if(Bitemp[62] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BHM包超时(BHM_timeover)"));
                if(Bitemp[61] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BRM包超时(BRM_timeover)"));
                if(Bitemp[60] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BCP包超时(BCP_Timeout)"));
                if(Bitemp[59] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BRO_Timeout"));
                if(Bitemp[58] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BCS_Timeout"));
                if(Bitemp[57] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BCL_Timeout"));
                if(Bitemp[56] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BST_Timeout"));
                if(Bitemp[55] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BSD_Timeout"));
                if(Bitemp[54] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 枪过温(gun_overtemp)"));
                if(Bitemp[53] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 单体动力蓄电池异常(BSM_CellVoltage)"));
//                if(Bitemp[52] == '1')
//                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" "));
                if(Bitemp[51] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 整车动力电池荷电状态(BSM_WholeVoltage)"));
                if(Bitemp[50] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 过流(BSM_OverCurrent)"));
                if(Bitemp[49] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 过温(BSM_OverTemperature)"));
                if(Bitemp[48] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 动力电池绝缘状态异常(BSM_InsulationStatus)"));
                if(Bitemp[47] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 输出连接器状态(BSM_OutputConnectStatus)"));
                if(Bitemp[46] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 电子锁异常(E_Lock)"));
                if(Bitemp[45] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" KIKII闭合或断开故障(KIKIIError)"));
                if(Bitemp[44] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" K1K2故障(K1K2Error)"));
                if(Bitemp[43] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" K1K2平衡故障_预充电压异常(K1K2BalancedError)"));
                if(Bitemp[42] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" K3K4故障(K3K4Error)"));
                if(Bitemp[41] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS中止达到SOC值(BST_SOCSet)"));
                if(Bitemp[40] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS达到电压设定值(BST_VoltageSet)"));
//                if(Bitemp[39] == '1')
//                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BHM包超时(BHM_timeover)"));
                if(Bitemp[38] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS达到单体电压设定值(BST_CellVoltSet)"));
                if(Bitemp[37] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS收到充电桩中止(BST_ChargerStop)"));
                if(Bitemp[36] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS正常停止(BST_STOP_NORMAL)"));
//                if(Bitemp[35] == '1')
//                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" "));
                if(Bitemp[34] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS绝缘故障(BST_Insulation)"));
                if(Bitemp[33] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS输出连接器过温故障(BST_OutputConnectOverTemp)"));
                if(Bitemp[32] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS元件、输出连接器过温故障(BST_BMSComponentOverTemp)"));
                if(Bitemp[31] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS充电连接故障(BST_ChargeConnectFault)"));
                if(Bitemp[30] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 电池组过温故障(BST_BatteryOverTemp)"));
                if(Bitemp[29] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 高压继电器故障(BST_HighVoltRelayFault)"));
                if(Bitemp[28] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 检测点2电压故障(BST_CheckPoint2Fault)"));
                if(Bitemp[27] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 其它故障"));
                if(Bitemp[26] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 过流故障(BST_OverCurrent)"));
//                if(Bitemp[25] == '1')
//                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" "));
                if(Bitemp[24] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 电压异常(BST_VoltageAbnormal)"));
                if(Bitemp[23] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" CRM_00超时(BEM_CRM_00_Timeou)"));
//                if(Bitemp[22] == '1')
//                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" "));
                if(Bitemp[21] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" CRM AA回超时(BEM_CRM_AA_Timeout)"));
                if(Bitemp[20] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BEM_CTS_CML_Timeout"));
                if(Bitemp[19] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" CRO超时(BEM_CRO_Timeout)"));
                if(Bitemp[18] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BEM_CCS_Timeout"));
                if(Bitemp[17] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BEM_CST_Timeout"));
                if(Bitemp[16] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BEM_CSD_Timeout"));
                if(Bitemp[15] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" BMS SOC达到100(BCS_SOC_FULL)"));
                if(Bitemp[14] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 服务器中止(PC_STOP)"));
                if(Bitemp[13] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" BMS异常:  ")+QString::fromUtf8(" 最高电压或电流超过充电桩范围(BMS_BHM_Para_Invalid)"));
                // System exception
                Bitemp = QString("%1%2%3%4%").arg(buf[18],8,2,QLatin1Char('0'))
                        .arg(buf[19],8,2,QLatin1Char('0'))
                        .arg(buf[20],8,2,QLatin1Char('0'))
                        .arg(buf[21],8,2,QLatin1Char('0'));
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
                // Touch Screen Status
                Bitemp = QString("%1%2%3%4%").arg(buf[22],8,2,QLatin1Char('0'))
                        .arg(buf[23],8,2,QLatin1Char('0'))
                        .arg(buf[24],8,2,QLatin1Char('0'))
                        .arg(buf[25],8,2,QLatin1Char('0'));
                if(Bitemp[31] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 屏幕状态:  ")+QString::fromUtf8(" 正常(run_normal)"));
                if(Bitemp[30] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 屏幕状态:  ")+QString::fromUtf8(" 通讯异常(commonError)"));
                if(Bitemp[29] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 屏幕状态:  ")+QString::fromUtf8(" 屏启动标志(startCp)"));
                if(Bitemp[28] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 屏幕状态:  ")+QString::fromUtf8(" 屏停止标志(StopCp)"));
                // Power module Status
                Bitemp = QString("%1%2%3%4%").arg(buf[26],8,2,QLatin1Char('0'))
                        .arg(buf[27],8,2,QLatin1Char('0'))
                        .arg(buf[28],8,2,QLatin1Char('0'))
                        .arg(buf[29],8,2,QLatin1Char('0'));
                if(Bitemp[31] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 模块告警(alarm)"));
                if(Bitemp[30] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 模块故障(fault)"));
                if(Bitemp[29] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 其他故障(other)"));
                if(Bitemp[28] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 泄放(Discharge)"));
                if(Bitemp[27] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 风机(Fan)"));
                if(Bitemp[26] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 直流输出短路(OutShortCircuit)"));
                if(Bitemp[25] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 过温(temp)"));
                if(Bitemp[24] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 直流输出欠压(DCoutputUV)"));
                if(Bitemp[23] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 直流输出过压(DCoutputOV)"));
                if(Bitemp[22] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 模块关机DC无输出(stop)"));//交流输入故障(AcInput)
//                if(Bitemp[21] == '1')
//                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" ()"));
                if(Bitemp[20] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 模块上电中(PowerUp)"));
                if(Bitemp[19] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 模块通讯异常(ModelCommError)"));
                if(Bitemp[18] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 模块CAN芯片初始化异常(CanInitError)"));
//                if(Bitemp[17] == '1')
//                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" "));
                if(Bitemp[16] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 允许充电异常(AllowChargeError)"));
                if(Bitemp[15] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 程序监控到输入欠压报警(ViewInputUV_Alarm)"));
                if(Bitemp[14] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 程序监控输入电压保护(ViewInputUV_Portect)"));
                if(Bitemp[13] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 程序监控到输出电压过压，充电状态与需求电压比较(ViewOutputOV)"));
                if(Bitemp[12] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 程序监控到输入过压报警(ViewInputOV)"));
                if(Bitemp[11] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 程序监控到输入过压保护(ViewInputOV_protect)"));
                if(Bitemp[10] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 手动加载时, 程序监控到输出过流大于150或300A(ViewOutputOI_manual)"));
                if(Bitemp[9] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 充电过流BCL(ChangeOutputOI)"));
                if(Bitemp[8] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 充电过程中输出电压超过750V(ChangeOutputMaxOV)"));
                if(Bitemp[7] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 充电过程中输出电压低于200V(ChangeOutputMinUV)"));
                if(Bitemp[6] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 充电模块状态:  ")+QString::fromUtf8(" 模块进出风口温度偏差大于15度(TempOffsetMax)"));
                // Insulation dectection Module status
                Bitemp = QString("%1%2%3%4%").arg(buf[34],8,2,QLatin1Char('0'))
                        .arg(buf[35],8,2,QLatin1Char('0'))
                        .arg(buf[36],8,2,QLatin1Char('0'))
                        .arg(buf[37],8,2,QLatin1Char('0'));
                if(Bitemp[31] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 绝缘模块状态:  ")+QString::fromUtf8(" 正常(run_normal)"));
                if(Bitemp[30] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 绝缘模块状态:  ")+QString::fromUtf8(" 通讯异常(commonError)"));
                if(Bitemp[29] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 绝缘模块状态:  ")+QString::fromUtf8(" 校验异常(checkSumError)"));
                if(Bitemp[28] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 绝缘模块状态:  ")+QString::fromUtf8(" 读取电压异常(ReadVError)"));
                if(Bitemp[27] == '1')
                    msg.maplist.insert("listWidget2", QString::fromUtf8(" 绝缘模块状态:  ")+QString::fromUtf8(" 放电异常(DischangeErr)"));
            }
            // read IoT code
            if (buf[0] == 0x20 && buf[8] == 0x06)
            {
                msg.maplist.insert("label1_19",QString("%1%2%3%4%5%6%7%8%9%10%11\n%12%13%14%15%16%17%18%19%20%21")
                                    .arg(buf[10],2,16,QLatin1Char('0'))
                                    .arg(buf[11],2,16,QLatin1Char('0'))
                                    .arg(buf[12],2,16,QLatin1Char('0'))
                                    .arg(buf[13],2,16,QLatin1Char('0'))
                                    .arg(buf[14],2,16,QLatin1Char('0'))
                                    .arg(buf[15],2,16,QLatin1Char('0'))
                                    .arg(buf[16],2,16,QLatin1Char('0'))
                                    .arg(buf[17],2,16,QLatin1Char('0'))
                                    .arg(buf[18],2,16,QLatin1Char('0'))
                                    .arg(buf[19],2,16,QLatin1Char('0'))
                                    .arg(buf[20],2,16,QLatin1Char('0'))
                                    .arg(buf[21],2,16,QLatin1Char('0'))
                                    .arg(buf[22],2,16,QLatin1Char('0'))
                                    .arg(buf[23],2,16,QLatin1Char('0'))
                                    .arg(buf[24],2,16,QLatin1Char('0'))
                                    .arg(buf[25],2,16,QLatin1Char('0'))
                                    .arg(buf[26],2,16,QLatin1Char('0'))
                                    .arg(buf[27],2,16,QLatin1Char('0'))
                                    .arg(buf[28],2,16,QLatin1Char('0'))
                                    .arg(buf[29],2,16,QLatin1Char('0'))
                                    .arg(buf[30],2,16,QLatin1Char('0')));
            }
            // update real time
            if (buf[0] == 0x10 && buf[8] == 0x0d)
            {
                msg.maplist.insert("dateTimeEdit", (QString("%1-%2-%3 %4:%5:%6").arg((buf[10]<<8)+buf[11])
                        .arg(buf[12]).arg(buf[13]).arg(buf[14]).arg(buf[15]).arg(buf[16])));
                continue;
            }
            // update charging times
            if (buf[0] == 0x09 && buf[8] == 0x28)
            {
                msg.maplist.insert("label1_20", QString("%1").arg(buf[9]));
                continue;
            }
            // update device ID
            if (buf[0] == 0x10 && buf[8] == 0x2a)
            {
                msg.maplist.insert("label1_22", QString(array.mid(10,8)));
                continue;
            }
            // update Access switch status
            if (buf[0] == 0x0a && buf[8] == 0x21)
            {
                switch (buf[10]) {
                case 0:
                    msg.maplist.insert("label2_22", QString::fromUtf8("闭合"));
                    continue;
                case 1:
                    msg.maplist.insert("label2_22", QString::fromUtf8("弹开"));
                    continue;
                }
            }
            // update Emergency stop status
            if (buf[0] == 0x0a && buf[8] == 0x22)
            {
                switch (buf[10]) {
                case 0:
                    msg.maplist.insert("label2_21", QString::fromUtf8("解除"));
                    continue;
                case 1:
                    msg.maplist.insert("label2_21", QString::fromUtf8("按紧"));
                    continue;
                }
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
    while (rSocket->hasPendingDatagrams())
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
        size = int(rSocket->bytesAvailable());
        array.resize(size);    // or array.resize(Socket->pendingDatagramSize());
        rSocket->readDatagram(array.data(),size,&address,&port);
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
            // show eletronic locks
            if (arraycmp(eleLock, buf, sizeof (eleLock), sizeof (buf)) == 1)
            {
                switch (buf[size-4])
                {
                case 0:
                    msg.maplist.insert("label3_1",QString::fromUtf8("锁定"));
                    continue;
                case 1:
                    msg.maplist.insert("label3_1",QString::fromUtf8("解锁"));
                    continue;
                default:
                    msg.maplist.insert("label3_1", "F");
                }
            }
        }
    }
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
