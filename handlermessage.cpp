#include "abstractdevice.h"
#include "handlermessage.h"
#include "tcpgateway.h"

HandlerMessage * HandlerMessage::m_Instance = NULL;

HandlerMessage *HandlerMessage::Instance (TcpGateway *clients, AbstractDevice * device, QObject *parent)
{
    if (m_Instance == NULL)
        new HandlerMessage(clients, device, parent);

    return m_Instance;
}

HandlerMessage::HandlerMessage(TcpGateway *clients, AbstractDevice * device, QObject *parent) :
    QObject(parent)
{
    m_debug = false;
    m_versioneMajor = m_versioneMinor = 0;

    m_clients = clients;
    m_device = device;

    if (clients == NULL)
        return;
    if (device == NULL)
        return;

    //    QObject::connect (clients, SIGNAL(toDeviceSignal(QByteArray, ClientOven*)),
    //                      device, SLOT(fromClientSlot(QByteArray, ClientOven*)));

    QObject::connect (m_clients, SIGNAL(toDeviceSignal(QByteArray, ClientOven*)),
                      this, SLOT(fromClientSlot(QByteArray, ClientOven*)));

    QObject::connect (this, SIGNAL(toClientsSignal(QByteArray, ClientOven*)),
                      m_clients, SLOT(fromDeviceSlot(QByteArray, ClientOven *)));
    QObject::connect (m_device, SIGNAL(toClientsSignal(QByteArray, ClientOven*)),
                      m_clients, SLOT(fromDeviceSlot(QByteArray, ClientOven *)));

    QObject::connect (m_device, SIGNAL(toOneClientOnlySignal(QByteArray, ClientOven*)),
                      m_clients, SLOT(toOneClientOnlySlot(QByteArray,ClientOven*)));
}

void HandlerMessage::setDebug(const bool &val)
{
    m_debug = val;

    if (m_device)
        m_device->setDebug(val);
    if (m_clients)
        m_clients->setDebug(val);
}

void HandlerMessage::debug (const QString &testo)
{
    if (m_debug)
        qDebug() << headDebug << qPrintable(testo);
}

void HandlerMessage::setVersioneSw (const quint8 &versioneMajor, const quint8 &versioneMinor)
{
    m_versioneMajor = versioneMajor;
    m_versioneMinor = versioneMinor;
}

/*!
 * \brief AbstractDevice::fromClientSlot
 * \param buffer - dati che mi arrivano dal Client
 */
const int lngHeadMsg = 5;
void HandlerMessage::fromClientSlot (const QByteArray &buffer, ClientOven*client)
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

            buildGetId (bufferForDevice);
            QDataStream stream(&bufferToClients, QIODevice::WriteOnly);
            stream << (quint8) getTipoIdFromDevice();
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
        {
            quint8 statoWd;
            ds >> statoWd;
        }
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
            QString testo = QString ("Messaggio sconosciuto: %1").arg(comando);
            debug (testo);
        }
        break;
    }
}
