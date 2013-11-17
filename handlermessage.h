#ifndef HANDLERMESSAGE_H
#define HANDLERMESSAGE_H

#include <QObject>

class AbstractDevice;
class ClientOven;
class TcpGateway;

class HandlerMessage : public QObject
{
    Q_OBJECT
public:
    static HandlerMessage *Instance (TcpGateway *clients = 0, AbstractDevice * device = 0, QObject *parent= 0);
    void setDebug(const bool &val);
    void setVersioneSw (const quint8 &versioneMajor, const quint8 &versioneMinor);

#define TIPO_RX_TCPIP_CAN_MSG       (0x00)
#define TIPO_RX_TCPIP_GET_ID        (0x0B)
#define TIPO_RX_TCPIP_SET_DEBUG     (0x14)
#define TIPO_RX_TCPIP_POWER         (0x30)
#define TIPO_RX_TCPIP_WD            (0x31)
#define TIPO_RX_TCPIP_POWER_OFF     (0xFF)

protected:
    explicit HandlerMessage(TcpGateway *clients, AbstractDevice * device, QObject *parent);
    void debug (const QString &testo);

private:
    HandlerMessage     *m_Instance;
    bool                m_debug;
    quint8              m_versioneMajor;
    quint8              m_versioneMinor;
    TcpGateway         *m_clients;
    AbstractDevice     *m_device;

protected slots:
    void fromClientSlot (const QByteArray &buffer, ClientOven*client);

signals:
    void toClientsSignal (const QByteArray &buffer, ClientOven *client);
};

#endif // HANDLERMESSAGE_H
