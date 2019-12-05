#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include "ui_widget.h"
#include <QObject>
#include <QWidget>
#include <QUdpSocket>
#include <QDateTime>
#include <QThread>

typedef struct {
    QMultiMap<QString, QString> maplist;
    QMap<QString, int> lcdnum;
    QMap<QString,double> lcdisplay;
    QMap<QString,bool> Flag;
}Msg;

class DataProcessor : public QObject
{
    Q_OBJECT
public:
    explicit DataProcessor(QObject *parent = nullptr);
    QUdpSocket *lSocket;
    QUdpSocket *rSocket;

signals:
    void Send2UI(Msg msg);

public slots:
    void ReceiveDatagrams();
    void LprocessDatagrams();
    void RprocessDatagrams();
    bool arraycmp(unsigned char arrayA[], unsigned char arrayB[], unsigned long a, unsigned long b);

private:
    Msg msg;
    quint16 leftport = 2001;
    quint16 rightport =2002;
};

#endif // DATAPROCESSOR_H
