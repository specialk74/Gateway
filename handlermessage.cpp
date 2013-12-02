#include <QTimer>

#include "abstractdevice.h"
#include "handlermessage.h"
#include "tcpgateway.h"

static const char headDebug[] = "[HandlerMessage]";

HandlerMessageTcpIp * HandlerMessageTcpIp::m_Instance = NULL;

HandlerMessageTcpIp *HandlerMessageTcpIp::Instance (TcpGateway *clients, AbstractDevice * device, QObject *parent)
{
    if (m_Instance == NULL)
        new HandlerMessageTcpIp(clients, device, parent);

    return m_Instance;
}

HandlerMessageTcpIp::HandlerMessageTcpIp(TcpGateway *clients, AbstractDevice * device, QObject *parent) :
    QObject(parent)
{
    m_Instance = this;
    m_debug = false;
    m_versioneMajor = m_versioneMinor = 0;

    m_clients = clients;
    m_device = device;

    if (clients == NULL)
        return;
    if (device == NULL)
        return;

    QObject::connect (m_clients, SIGNAL(toDeviceSignal(QByteArray, ClientOven*)),
                      this, SLOT(fromClientSlot(QByteArray, ClientOven*)));

    QObject::connect (this, SIGNAL(toClientsSignal(QByteArray, ClientOven*)),
                      m_clients, SLOT(fromDeviceSlot(QByteArray, ClientOven *)));

    m_timerWD = new QTimer (this);
    Q_ASSERT(m_timerWD);
    connect (m_timerWD, SIGNAL(timeout()), this, SLOT(timeoutWd()));
}

void HandlerMessageTcpIp::setDebug(const bool &val)
{
    m_debug = val;

    if (m_device)
        m_device->setDebug(val);
    if (m_clients)
        m_clients->setDebug(val);
}

void HandlerMessageTcpIp::debug (const QString &testo)
{
    if (m_debug)
        qDebug() << headDebug << qPrintable(testo);
}

void HandlerMessageTcpIp::setVersioneSw (const quint8 &versioneMajor, const quint8 &versioneMinor)
{
    m_versioneMajor = versioneMajor;
    m_versioneMinor = versioneMinor;
}

/*!
 * \brief AbstractDevice::fromClientSlot
 * \param buffer - dati che mi arrivano dal Client
 */
const int lngHeadMsg = 5;
void HandlerMessageTcpIp::fromClientSlot (const QByteArray &buffer, ClientOven*client)
{
    // Controllo che la lunghezza minima sia 5 (un byte per il tipo e 4 byte di lunghezza)
    if (m_debug)
    {
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug;
        int var;
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }

    if (buffer.length() < lngHeadMsg)
    {
        debug ("Lunghezza Messaggio corta");
        return;
    }

    // Recupero la lunghezza del messaggio dai dati
    QDataStream ds(buffer);
    quint8 tipo;
    quint32 temp;
    ds >> tipo;
    ds >> temp;
    quint32 lunghezza = _ntohl(temp);
    if (lunghezza != (quint32) buffer.length())
    {
        QString testo = QString ("Lunghezza Messaggio errata %1 - %2").arg(lunghezza).arg(buffer.length());
        debug (testo);
        return;
    }

    switch (tipo)
    {
        case TIPO_RX_TCPIP_CAN_MSG:
        {
            QByteArray bufferToDevice = buffer.right(buffer.length() - lngHeadMsg);

            m_device->fromClientHandler (bufferToDevice);
            //toDevice (bufferToDevice);
            emit toClientsSignal(buffer, client);
        }
        break;

        case TIPO_RX_TCPIP_GET_ID:
        {
            QByteArray bufferToClients;
            QByteArray bufferForDevice;

            m_device->buildGetId (bufferForDevice);
            QDataStream stream(&bufferToClients, QIODevice::WriteOnly);
            stream << (quint8) m_device->getTipoIdFromDevice();
            lunghezza = _htonl(8 + bufferForDevice.length());
            stream << (quint32) lunghezza; // Lunghezza
            stream << (quint8) 0; // Stato Interno
            stream << (quint8) m_versioneMajor;
            stream << (quint8) m_versioneMinor;
            quint8 var;
            foreach (var, bufferForDevice)
                stream << var;

            emit toOneClientOnlySignal(bufferToClients, client);
        }
        break;

        case TIPO_RX_TCPIP_SET_DEBUG:
        {
            quint8 debug;
            ds >> debug;
            setDebug (debug);
        }
        break;

        case TIPO_RX_TCPIP_POWER:
        {
        }
        break;

        case TIPO_RX_TCPIP_WD:
            ds >> m_statoWD;
            if (m_statoWD)
                m_timerWD->start(60*1000);
            else
                m_timerWD->stop();
        break;

        case TIPO_RX_TCPIP_POWER_OFF:
        {
#ifdef Q_WS_QWS
            system ("shutdown -h now");
#endif
        }
        break;

        default:
        {
            QString testo = QString ("Messaggio Tcp/Ip sconosciuto: %1").arg(tipo);
            debug (testo);
        }
        break;
    }
}

void HandlerMessageTcpIp::timeoutWd()
{

}
