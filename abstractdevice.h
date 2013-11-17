#ifndef ABSTRACTDEVICE_H
#define ABSTRACTDEVICE_H

#include <QObject>
#include "clientoven.h"

/*
struct IdStruct {
    quint8 tipo;
    quint32 lunghezza;
    quint8 stato_interno;
    quint8 versione_major;
    quint8 versione_minor;
    quint8 com_stat;
    quint8 versione_device_major;
    quint8 versione_device_minor;
};
*/
class AbstractDevice : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDevice(QObject *parent = 0);

    void setDebug (const bool &val);
    inline bool getDebug () { return m_debug; }
    void setVersioneSw (const quint8 &versioneMajor, const quint8 &versioneMinor);

signals:
    void toClientsSignal (const QByteArray &buffer, ClientOven *client);
    void toOneClientOnlySignal (const QByteArray &buffer, ClientOven *client);

public slots:
    void fromClientSlot (const QByteArray &buffer, ClientOven *client);

protected:

#define TIPO_RX_TCPIP_CAN_MSG 0x00
#define TIPO_RX_TCPIP_GET_ID  0x0B

#define TIPO_TX_TCPIP_CAN_MSG 0x00

    virtual void toDevice (const QByteArray &buffer) = 0;
    virtual quint8 getTipoIdFromDevice() = 0;
    virtual quint8 getComStatFromDevice() = 0;
    virtual void debug (const QString &testo);

    void fromDeviceToClients (const QByteArray &msgCANfromDevice);
    virtual void buildGetId(QByteArray & bufferForDevice) = 0;

private:
    bool m_debug;

    quint8 m_versioneMajor;
    quint8 m_versioneMinor;
};

#endif // ABSTRACTDEVICE_H
