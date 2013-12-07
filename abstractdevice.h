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
    virtual void buildGetId(QByteArray & bufferForDevice) = 0;
    virtual quint8 getTipoIdFromDevice() = 0;
    virtual void toDevice (const QByteArray &buffer) = 0;

signals:
    void toClientsSignal (const QByteArray &buffer, ClientOven *client);

protected:

#define TIPO_TX_TCPIP_CAN_MSG 0x00

    virtual quint8 getComStatFromDevice() = 0;
    virtual void debug (const QString &testo);

    void fromDeviceToClients (const QByteArray &msgCANfromDevice);

private:
    bool m_debug;
};

#endif // ABSTRACTDEVICE_H
