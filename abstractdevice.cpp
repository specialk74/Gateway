#include <QDebug>
#include <QString>
#include "abstractdevice.h"
#include "utils.h"

static const char headDebug[] = "[Device]";

AbstractDevice::AbstractDevice(QObject *parent) :
    QObject(parent)
{
    m_debug = false;
    m_versioneMajor = m_versioneMinor = 0;
}

void AbstractDevice::setDebug(const bool &val)
{
    m_debug = val;
}

void AbstractDevice::debug (const QString &testo)
{
    if (m_debug)
        qDebug() << headDebug << qPrintable(testo);
}

void AbstractDevice::setVersioneSw (const quint8 &versioneMajor, const quint8 &versioneMinor)
{
    m_versioneMajor = versioneMajor;
    m_versioneMinor = versioneMinor;
}

#if 0
union lunghezza {
    quint32 u32;
    quint8 dato[4];
} ;
#endif

/*!
 * \brief AbstractDevice::toClients
 * \param buffer - Sono solo messaggi CAN
 */
void AbstractDevice::fromDeviceToClients (const QByteArray &msgCANfromDevice)
{
    QByteArray bufferToClients;
    {
        QDataStream stream (&bufferToClients, QIODevice::WriteOnly);
        stream << (quint8) TIPO_TX_TCPIP_CAN_MSG;
        stream << _htonl((quint32) 17);
    }
    bufferToClients.append(msgCANfromDevice);

    emit toClientsSignal(bufferToClients, NULL);
}

/*!
 * \brief AbstractDevice::fromClientSlot
 * \param buffer - dati che mi arrivano dal Client
 */
const int lngHeadMsg = 5;
void AbstractDevice::fromClientSlot (const QByteArray &buffer, ClientOven*client)
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
    quint8 comando;
    quint32 temp;
    ds >> comando;
    ds >> temp;
    quint32 lunghezza = _ntohl(temp);
    if (lunghezza != (quint32) buffer.length())
    {
        QString testo = QString ("Lunghezza Messaggio errata %1 - %2").arg(lunghezza).arg(buffer.length());
        debug (testo);
        return;
    }

    switch (comando)
    {
    case TIPO_RX_TCPIP_CAN_MSG:
    {
        QByteArray bufferToDevice = buffer.right(buffer.length() - lngHeadMsg);
        toDevice (bufferToDevice);
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

    default:
    {
        QString testo = QString ("Messaggio sconosciuto: %1").arg(comando);
        debug (testo);
    }
        break;
    }
}
