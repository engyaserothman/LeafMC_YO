#include <QCoreApplication>

#include "SiYi.h"
#include <QSettings>
#include <QFile>
#include <QStandardPaths>

SiYi *SiYi::instance_ = Q_NULLPTR;
SiYi::SiYi(QObject *parent)
    : QObject{parent}
{
    const QString defaultIp = QStringLiteral("192.168.144.25");
    const int defaultTransmitterPort = 5864;
    const int defaultCameraPort = 37256;

    QSettings settings;
    settings.beginGroup("SiYi");

    _gimbalIp = settings.value("gimbalIp", defaultIp).toString();
    _transmitterPort = settings.value("transmitterPort", defaultTransmitterPort).toInt();
    _cameraPort = settings.value("cameraPort", defaultCameraPort).toInt();

    settings.endGroup();

    camera_ = new SiYiCamera(_gimbalIp, (quint16)_cameraPort, this);
    transmitter_ = new SiYiTransmitter(_gimbalIp, (quint16)_transmitterPort, this);

    connect(transmitter_, &SiYiTransmitter::connected, this, [this](){ _updateIsConnected(true); });
    connect(transmitter_, &SiYiTransmitter::disconnected, this, [this](){ _updateIsConnected(false); });
    connect(camera_, &SiYiCamera::connected, this, [this](){ _updateIsConnected(true); });
    connect(camera_, &SiYiCamera::disconnected, this, [this](){ _updateIsConnected(false); });

#ifdef Q_OS_ANDROID
    isAndroid_ = true;
#else
    isAndroid_ = false;
#endif
}

void SiYi::setGimbalIp(const QString &ip)
{
    if (_gimbalIp != ip) {
        _gimbalIp = ip;
        QSettings settings;
        settings.beginGroup("SiYi");
        settings.setValue("gimbalIp", _gimbalIp);
        settings.endGroup();
        emit gimbalIpChanged();
    }
}

void SiYi::setCameraPort(int port)
{
    if (_cameraPort != port) {
        _cameraPort = port;
        QSettings settings;
        settings.beginGroup("SiYi");
        settings.setValue("cameraPort", _cameraPort);
        settings.endGroup();
        emit cameraPortChanged();
    }
}

void SiYi::setTransmitterPort(int port)
{
    if (_transmitterPort != port) {
        _transmitterPort = port;
        QSettings settings;
        settings.beginGroup("SiYi");
        settings.setValue("transmitterPort", _transmitterPort);
        settings.endGroup();
        emit transmitterPortChanged();
    }
}

void SiYi::connectLink()
{
    disconnectLink();
    camera_->setIpPort(_gimbalIp, (quint16)_cameraPort);
    transmitter_->setIpPort(_gimbalIp, (quint16)_transmitterPort);
    transmitter_->start();
    camera_->start();
}

void SiYi::disconnectLink()
{
    if (transmitter_->isRunning()) {
        transmitter_->exit();
        transmitter_->wait();
    }
    if (camera_->isRunning()) {
        camera_->exit();
        camera_->wait();
    }
}

void SiYi::_updateIsConnected(bool connected)
{
    if (isTransmitterConnected_ != connected) {
        isTransmitterConnected_ = connected;
        emit isConnectedChanged();
    }
}

SiYi *SiYi::instance()
{
    if (!instance_) {
        instance_ = new SiYi(qApp);
    }

    Q_ASSERT_X(instance_, __FUNCTION__,
               "Can not allocate memory for SiYi instance!");
    return instance_;
}

SiYiCamera *SiYi::cameraInstance()
{
    return camera_;
}

SiYiTransmitter *SiYi::transmitterInstance()
{
    return transmitter_;
}
