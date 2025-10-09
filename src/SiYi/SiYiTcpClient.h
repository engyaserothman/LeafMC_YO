#ifndef SIYITCPCLIENT_H
#define SIYITCPCLIENT_H

#include <QMutex>
#include <QThread>
#include <QVector>
#include <QTcpSocket>

#define PROTOCOL_STX 0x5566AABB
#define PROTOCOL_STX_V2 0x5566

class SiYiTcpClient : public QThread
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
public:
    SiYiTcpClient(const QString ip, quint16 port, QObject *parent = Q_NULLPTR);
    ~SiYiTcpClient();

    void setIpPort(const QString& ip, quint16 port);
    void sendMessage(const QByteArray &msg);
    void sendmessageUdp(const QByteArray &msg);
    Q_INVOKABLE virtual void analyzeIp(QString videoUrl);

protected:
    virtual void analyzeMessage() = 0;
    virtual void analyzeUDPMessage() = 0;
    virtual QByteArray heartbeatMessage() = 0;
    virtual QByteArray heartbeatMessageUdp() = 0;
protected:
    QVector<QByteArray> txMessageVector_, u_txMessageVector_;
    QMutex txMessageVectorMutex_, u_txMessageVectorMutex_;
    QByteArray rxBytes_, u_rxBytes_;
    QMutex rxBytesMutex_, u_rxBytesMutex_;
    int timeoutCount = 0, u_timeoutCount = 0;
    QMutex timeoutCountMutex, u_timeoutCountMutex;
    QString ip_;
    quint16 port_;
protected:
    quint16 sequence();
    quint16 sequenceV2();
    void run() override;
    quint32 checkSum32(const QByteArray &bytes);
    quint16 checkSum16(const QByteArray &bytes);
    void resetIp(const QString &ip);
private:
    quint16 sequence_;
    quint16 sequence_2;
signals:
    void connected();
    void disconnected();
    void udpConnected();
    void udpDisconnected();
    void ipChanged();
private:
    bool isConnected_{false}, isUdpConnected_{false};

    Q_SIGNAL void isConnectedChanged();
    Q_SIGNAL void isUdpConnectedChanged();
public:
    bool isConnected(){return isConnected_;}
    bool isUdpConnected(){return isUdpConnected_;}
};

#endif // SIYITCPCLIENT_H
