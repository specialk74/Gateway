#ifndef HANDLERMESSAGE_H
#define HANDLERMESSAGE_H

#include <QObject>

class QTimer;

class AbstractDevice;
class ClientOven;
class TcpGateway;

class HandlerMessageTcpIp : public QObject
{
    Q_OBJECT
public:
    static HandlerMessageTcpIp *Instance (TcpGateway *clients = 0, AbstractDevice * device = 0, QObject *parent= 0);
    void setDebug(const bool &val);
    void setVersioneSw (const quint8 &versioneMajor, const quint8 &versioneMinor);

#define TIPO_RX_TCPIP_CAN_MSG       (0x00)
#define TIPO_RX_TCPIP_GET_ID        (0x0B)
#define TIPO_RX_TCPIP_SET_DEBUG     (0x14)
#define TIPO_RX_TCPIP_POWER         (0x30)
#define TIPO_RX_TCPIP_WD            (0x31)
#define TIPO_RX_TCPIP_POWER_OFF     (0xFF)

protected:
    explicit HandlerMessageTcpIp(TcpGateway *clients, AbstractDevice * device, QObject *parent);
    void debug (const QString &testo);

private:
    static HandlerMessageTcpIp     *m_Instance;
    bool                m_debug;
    quint8              m_versioneMajor;
    quint8              m_versioneMinor;
    TcpGateway         *m_clients;
    AbstractDevice     *m_device;
    quint8              m_statoWD;
    QTimer             *m_timerWD;

protected slots:
    void fromClientSlot (const QByteArray &buffer, ClientOven*client);
    void timeoutWd ();

signals:
    void toClientsSignal (const QByteArray &buffer, ClientOven *client);
    void toOneClientOnlySignal (const QByteArray &buffer, ClientOven *client);
};

#endif // HANDLERMESSAGE_H