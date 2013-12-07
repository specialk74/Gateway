#include <QDebug>
#include <QString>
#include "abstractdevice.h"
#include "utils.h"

static const char headDebug[] = "[Device]";

AbstractDevice::AbstractDevice(QObject *parent) :
    QObject(parent)
{
    m_debug = false;
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
