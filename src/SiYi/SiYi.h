#ifndef SIYI_H
#define SIYI_H

#include <QMutex>
#include <QObject>
#include <QVariant>

#include "SiYiCamera.h"
#include "SiYiTransmitter.h"

class SiYi : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant camera READ camera CONSTANT)
    Q_PROPERTY(QVariant transmitter READ transmitter CONSTANT)
    Q_PROPERTY(bool isAndroid READ isAndroid CONSTANT)
    Q_PROPERTY(bool hideWidgets READ hideWidgets WRITE setHideWidgets NOTIFY hideWidgetsChanged)
    Q_PROPERTY(int iconsHeight READ iconsHeight WRITE setIconsHeight NOTIFY iconsHeightChanged)
    Q_PROPERTY(QString gimbalIp READ gimbalIp WRITE setGimbalIp NOTIFY gimbalIpChanged)
    Q_PROPERTY(int cameraPort READ cameraPort WRITE setCameraPort NOTIFY cameraPortChanged)
    Q_PROPERTY(int transmitterPort READ transmitterPort WRITE setTransmitterPort NOTIFY transmitterPortChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)

public:
    explicit SiYi(QObject *parent = nullptr);
    static SiYi *instance();
    SiYiCamera *cameraInstance();
    SiYiTransmitter *transmitterInstance();

    QString gimbalIp() const { return _gimbalIp; }
    void setGimbalIp(const QString &ip);

    int cameraPort() const { return _cameraPort; }
    void setCameraPort(int port);

    int transmitterPort() const { return _transmitterPort; }
    void setTransmitterPort(int port);

    bool isConnected() const { return isTransmitterConnected_; }

    Q_INVOKABLE void connectLink();
    Q_INVOKABLE void disconnectLink();

signals:
    void hideWidgetsChanged();
    void iconsHeightChanged();
    void gimbalIpChanged();
    void cameraPortChanged();
    void transmitterPortChanged();
    void isConnectedChanged();

private slots:
    void _updateIsConnected(bool connected);

private:
    static SiYi *instance_;
    SiYiCamera *camera_;
    SiYiTransmitter *transmitter_;
    bool isTransmitterConnected_{false};
    QString _gimbalIp;
    int     _cameraPort;
    int     _transmitterPort;

private:
    QVariant camera(){return QVariant::fromValue(camera_);}
    QVariant transmitter(){return QVariant::fromValue(transmitter_);}
private:
    bool isAndroid_;
    bool isAndroid(){return isAndroid_;}

    bool hideWidgets_{false};
    bool hideWidgets() { return hideWidgets_; }
    void setHideWidgets(bool hideWidgets)
    {
        hideWidgets_ = hideWidgets;
        emit hideWidgetsChanged();
    }

    int iconsHeight_{54};
    int iconsHeight() { return iconsHeight_; }
    void setIconsHeight(int iconsHeight)
    {
        iconsHeight_ = iconsHeight;
        emit iconsHeightChanged();
    }
};

#endif // SIYI_H
