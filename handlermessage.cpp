#include <QTimer>

#include "abstractdevice.h"
#include "handlermessage.h"
#include "powermanager.h"
#include "tcpgateway.h"

static const char headDebug[] = "[HandlerMessage]";

HandlerMessageTcpIp * HandlerMessageTcpIp::m_Instance = NULL;

HandlerMessageTcpIp *HandlerMessageTcpIp::Instance (QObject *parent)
{
    if (m_Instance == NULL)
        new HandlerMessageTcpIp(parent);

    return m_Instance;
}

HandlerMessageTcpIp::HandlerMessageTcpIp(QObject *parent) :
    QObject(parent)
{
    m_Instance      = this;
    m_debug         = false;
    m_versioneMajor = 0;
    m_versioneMinor = 0;
    m_clients       = NULL;
    m_deviceCAN     = NULL;
    m_devicePower   = NULL;
}

void HandlerMessageTcpIp::setDevice (TcpGateway *clients, AbstractDevice * device)
{
    Q_ASSERT(clients);
    Q_ASSERT(device);

    m_clients = clients;
    m_deviceCAN = device;

    m_deviceCAN->setDebug(m_debug);
    m_clients->setDebug(m_debug);

    QObject::connect (clients, SIGNAL(toDeviceSignal(QByteArray, ClientOven*)),
                      this, SLOT(fromClientSlot(QByteArray, ClientOven*)));

    QObject::connect (this, SIGNAL(toClientsSignal(QByteArray, ClientOven*)),
                      clients, SLOT(fromDeviceSlot(QByteArray, ClientOven *)));

    QObject::connect (this, SIGNAL(toOneClientOnlySignal(QByteArray, ClientOven*)),
                      clients, SLOT(toOneClientOnlySlot(QByteArray,ClientOven*)));

    QObject::connect (device, SIGNAL(toClientsSignal(QByteArray, ClientOven*)),
                      clients, SLOT(fromDeviceSlot(QByteArray, ClientOven *)));
}

void HandlerMessageTcpIp::setDevice (TcpGateway *clients, PowerManager * device)
{
#ifdef Q_WS_QWS
    Q_ASSERT(clients);
    Q_ASSERT(device);

    m_clients = clients;
    m_devicePower = device;

    m_devicePower->setDebug(m_debug);

    QObject::connect (device, SIGNAL(toClientsSignal(QByteArray, ClientOven*)),
                      clients, SLOT(fromDeviceSlot(QByteArray, ClientOven *)));
#endif
}

void HandlerMessageTcpIp::setDebug(const bool &val)
{
    m_debug = val;

    if (m_deviceCAN)
        m_deviceCAN->setDebug(val);
    if (m_clients)
        m_clients->setDebug(val);
#ifdef Q_WS_QWS
    if (m_devicePower)
        m_devicePower->setDebug(val);
#endif
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
            m_deviceCAN->toDevice(bufferToDevice);
            emit toClientsSignal(buffer, client);
        }
        break;

        case TIPO_RX_TCPIP_GET_ID:
        {
            QByteArray bufferToClients;
            QByteArray bufferForDevice;

            m_deviceCAN->buildGetId (bufferForDevice);
            QDataStream stream(&bufferToClients, QIODevice::WriteOnly);
            stream << (quint8) m_deviceCAN->getTipoIdFromDevice();
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
#ifdef Q_WS_QWS
            if (m_devicePower)
            {
                QByteArray bufferToDevice = buffer.right(buffer.length() - lngHeadMsg);
                m_devicePower->toDevice (bufferToDevice);
            }
#endif
        }
        break;

        case TIPO_RX_TCPIP_WD:
        {
            quint8  val;
            ds >> val;
        }
        break;

        case TIPO_RX_TCPIP_POWER_OFF:
        {
#ifdef Q_WS_QWS
        quint8 comando;
        ds >> comando;
        if (comando == 0) {
            if (m_devicePower)
            {
                QByteArray bufferToDevice = "";
                bufferToDevice.append (TIPO_RX_UART_POWER_OFF);
                m_devicePower->toDevice (bufferToDevice);
                m_devicePower->toDevice (bufferToDevice);
                m_devicePower->toDevice (bufferToDevice);
            }
            system ("shutdown -h now");
        } else if (comando == 1) {
            system ("reboot");
        }
        debug ("Gateway: EXIT");
        exit(0xFF);
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
