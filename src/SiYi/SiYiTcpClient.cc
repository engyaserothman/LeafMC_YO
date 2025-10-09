#include <QTimer>
#include <QtEndian>
#include <QDateTime>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimerEvent>

#include "SiYiCrcApi.h"
#include "SiYiTcpClient.h"



SiYiTcpClient::SiYiTcpClient(const QString ip, quint16 port, QObject *parent)
    : QThread(parent)
    , ip_(ip)
    , port_(port)
{
    sequence_ = quint16(QDateTime::currentMSecsSinceEpoch());
    sequence_2 = sequence_;
    // 自动重连
    connect(this, &SiYiTcpClient::finished, this, [=]() { start(); });
}

SiYiTcpClient::~SiYiTcpClient()
{
    if (isRunning()) {
        exit();
        wait();
    }
}

void SiYiTcpClient::setIpPort(const QString &ip, quint16 port)
{
    ip_ = ip;
    port_ = port;
}

void SiYiTcpClient::sendMessage(const QByteArray &msg)
{
    if (isRunning()) {
        txMessageVectorMutex_.lock();
        txMessageVector_.append(msg);
        txMessageVectorMutex_.unlock();
    }
}

void SiYiTcpClient::sendmessageUdp(const QByteArray &msg)
{
    if (isRunning()) {
        u_txMessageVectorMutex_.lock();
        u_txMessageVector_.append(msg);
        u_txMessageVectorMutex_.unlock();
    }
}

void SiYiTcpClient::analyzeIp(QString videoUrl)
{
}

quint16 SiYiTcpClient::sequence()
{
    quint16 seq = sequence_;
    sequence_ += 1;
    return seq;
}

quint16 SiYiTcpClient::sequenceV2()
{
    quint16 seq = sequence_2;
    sequence_2 += 1;
    return seq;
}

void SiYiTcpClient::run()
{
    QTcpSocket *tcpClient = new QTcpSocket();
    QUdpSocket *udpClient = new QUdpSocket();
    QTimer *txTimer = new QTimer();
    QTimer *u_txTimer = new QTimer();
    QTimer *rxTimer = new QTimer();
    QTimer *u_rxTimer = new QTimer();
    QTimer *heartbeatTimer = new QTimer();
    QTimer *u_heartbeatTimer = new QTimer();
    const QString info = QString("[%1:%2]:").arg(ip_, QString::number(port_));

    connect(tcpClient, &QTcpSocket::connected, tcpClient, [=](){
        qInfo() << info << "Connect to server successfully!";

        heartbeatTimer->start();
        txTimer->start();

        this->isConnected_ = true;

        emit connected();
        emit isConnectedChanged();
    });

    connect(udpClient, &QUdpSocket::connected, udpClient, [=](){
        qInfo() << info << "Connect to UDP server successfully!";

        u_heartbeatTimer->start();
        u_txTimer->start();

        this->isUdpConnected_ = true;

        emit udpConnected();
        emit isUdpConnectedChanged();
    });

    connect(tcpClient, &QTcpSocket::disconnected, tcpClient, [=]() {
        qInfo() << info << "Disconnect from server:" << tcpClient->errorString();

        this->isConnected_ = false;
        this->txMessageVectorMutex_.lock();
        this->txMessageVector_.clear();
        this->txMessageVectorMutex_.unlock();

        emit isConnectedChanged();

        heartbeatTimer->stop();
        exit();
    });

    connect(udpClient, &QUdpSocket::disconnected, udpClient, [=]() {
        qInfo() << info << "Disconnect from UDP server:" << udpClient->errorString();

        this->isUdpConnected_ = false;
        this->u_txMessageVectorMutex_.lock();
        this->u_txMessageVector_.clear();
        this->u_txMessageVectorMutex_.unlock();

        emit isUdpConnectedChanged();

        u_heartbeatTimer->stop();
        exit();
    });

    connect(tcpClient, &QTcpSocket::errorOccurred, tcpClient, [=](){
        heartbeatTimer->stop();
        exit();
        qInfo() << info << tcpClient->errorString();
    });

    connect(udpClient, &QUdpSocket::errorOccurred, udpClient, [=](){
        u_heartbeatTimer->stop();
        exit();
        qInfo() << info << udpClient->errorString();
    });

    // 定时发送
    txTimer->setInterval(10);
    txTimer->setSingleShot(true);
    connect(txTimer, &QTimer::timeout, txTimer, [=](){
        this->txMessageVectorMutex_.lock();
        QByteArray msg = this->txMessageVector_.isEmpty()
                             ? QByteArray()
                             : this->txMessageVector_.takeFirst();
        this->txMessageVectorMutex_.unlock();

        if ((!msg.isEmpty())) {
            if ((tcpClient->state() == QTcpSocket::ConnectedState)) {
                if (tcpClient->write(msg) != -1) {
                    //qInfo() << info << "Tx:" << msg.toHex(' ');
                } else {
                    qInfo() << info << tcpClient->errorString();
                }
            } else {
                qInfo() << info << "Not connected state, the state is:" << tcpClient->state();
                qInfo() << info << tcpClient->errorString();
                exit();
            }
        }

        txTimer->start();
    });
    u_txTimer->setInterval(10);
    u_txTimer->setSingleShot(true);
    connect(u_txTimer, &QTimer::timeout, u_txTimer, [=](){
        this->u_txMessageVectorMutex_.lock();
        QByteArray msg = this->u_txMessageVector_.isEmpty()
                             ? QByteArray()
                             : this->u_txMessageVector_.takeFirst();
        this->u_txMessageVectorMutex_.unlock();

        if ((!msg.isEmpty())) {
            if ((udpClient->state() == QUdpSocket::ConnectedState)) {
                if (udpClient->writeDatagram(msg, QHostAddress(ip_), 37260) != -1) {
                    //qInfo() << info << "Tx:" << msg.toHex(' ');
                } else {
                    qInfo() << info << udpClient->errorString();
                }
            } else {
                qInfo() << info << "Not connected state, the state is:" << udpClient->state();
                qInfo() << info << udpClient->errorString();
                exit();
            }
        }

        u_txTimer->start();
    });


    // 定时处理接收数据
    rxTimer->setInterval(1);
    rxTimer->setSingleShot(true);
    connect(rxTimer, &QTimer::timeout, rxTimer, [=](){
        this->rxBytesMutex_.lock();

        QByteArray bytes = tcpClient->readAll();
        this->rxBytes_.append(bytes);

        analyzeMessage();

        this->rxBytesMutex_.unlock();

        rxTimer->start();
    });
    u_rxTimer->setInterval(1);
    u_rxTimer->setSingleShot(true);
    connect(u_rxTimer, &QTimer::timeout, u_rxTimer, [=](){
        this->u_rxBytesMutex_.lock();

        QByteArray bytes = udpClient->readAll();
        this->u_rxBytes_.append(bytes);

        analyzeUDPMessage();

        this->u_rxBytesMutex_.unlock();

        u_rxTimer->start();
    });

    // 心跳
    heartbeatTimer->setInterval(1500);
    heartbeatTimer->setSingleShot(true);
    connect(heartbeatTimer, &QTimer::timeout, heartbeatTimer, [=](){
        // 心跳超时后退出线程
        this->timeoutCountMutex.lock();
        int count = this->timeoutCount;
        this->timeoutCountMutex.unlock();

        if (count > 3) {
            this->timeoutCountMutex.lock();
            this->timeoutCount = 0;
            this->timeoutCountMutex.unlock();

            qWarning() << "Heartbeat timeout, the client will be restart soon!";
            this->exit();
        }

        this->timeoutCountMutex.lock();
        this->timeoutCount += 1;
        this->timeoutCountMutex.unlock();

        QByteArray msg = heartbeatMessage();
        sendMessage(msg);
        heartbeatTimer->start();
    });
    u_heartbeatTimer->setInterval(1500);
    u_heartbeatTimer->setSingleShot(true);
    connect(u_heartbeatTimer, &QTimer::timeout, u_heartbeatTimer, [=](){
        // 心跳超时后退出线程
        this->u_timeoutCountMutex.lock();
        int count = this->u_timeoutCount;
        this->u_timeoutCountMutex.unlock();

        if (count > 3) {
            this->u_timeoutCountMutex.lock();
            this->u_timeoutCount = 0;
            this->u_timeoutCountMutex.unlock();

            qWarning() << "Heartbeat timeout, the UDP client will be restart soon!";
            this->exit();
        }

        this->u_timeoutCountMutex.lock();
        this->u_timeoutCount += 1;
        this->u_timeoutCountMutex.unlock();

        QByteArray msg = heartbeatMessageUdp();
        sendmessageUdp(msg);
        u_heartbeatTimer->start();
    });

    tcpClient->connectToHost(ip_, port_);
    udpClient->connectToHost(ip_, 37260);

    //txTimer->start();
    rxTimer->start();
    u_rxTimer->start();
    exec();
    txTimer->deleteLater();
    u_txTimer->deleteLater();
    tcpClient->deleteLater();
    udpClient->deleteLater();
}

quint32 SiYiTcpClient::checkSum32(const QByteArray &bytes)
{
    return SiYiCrcApi::calculateCrc32(bytes);
}

quint16 SiYiTcpClient::checkSum16(const QByteArray &bytes)
{
    return SiYiCrcApi::calculateCrc16(bytes);
}

void SiYiTcpClient::resetIp(const QString &ip)
{
    if (ip_ != ip) {
        ip_ = ip;
        exit();
        wait();
        emit ipChanged();
    }
}
