#include "utils.h"

#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "powermanager.h"

static const char headDebug[] = "[PowerManager]";

PowerManager * PowerManager::m_Instance = NULL;


PowerManager *PowerManager::Instance (QObject *parent)
{
    if (m_Instance == NULL)
        new PowerManager(parent);

    return m_Instance;
}

PowerManager::PowerManager(QObject *parent) :
    QObject(parent)
{
    m_Instance = this;

    m_device = new QSerialPort(this);
    Q_ASSERT(m_device);
    connect(m_device, SIGNAL(readyRead()), this, SLOT(fromDeviceSlot()));
}

void PowerManager::setDebug(const bool &val)
{
    m_debug = val;
}

void PowerManager::debug (const QString &testo)
{
    if (m_debug)
        qDebug() << headDebug << qPrintable(testo);
}

/*!
 * \brief AbstractDevice::toClients
 * \param buffer - Sono solo messaggi CAN
 */
void PowerManager::fromDeviceSlot()
{
    QByteArray bufferToClients;
    QByteArray msgfromDevice = m_device->readAll();
    {
        QDataStream stream (&bufferToClients, QIODevice::WriteOnly);
        stream << (quint8) TIPO_TX_TCPIP_POWER_MSG;
        stream << _htonl((quint32) msgfromDevice.length());
    }
    bufferToClients.append(msgfromDevice);

    emit toClientsSignal(bufferToClients, NULL);
}


/*!
 * \brief PowerManager::fromClientHandler
 * \param buffer - messaggio da spedire verso il Power Manager
 */
void PowerManager::fromClientHandler (const QByteArray & buffer)
{
    if (m_device)
    {
        m_device->write(buffer);
        m_device->flush();
    }
}

/*!
 * \brief Rs232DevicePrivate::configPort - Parametri per configurare la porta seriale
 * \return true se riesce a configurare correttamente la porta seriale
 */

bool PowerManager::setDevice (const QString &name)
{
//#ifdef Q_WS_QWS
    m_device->setPortName(name);

    if (!m_device->open(QIODevice::ReadWrite)) {
        QString testo = QString("Can't open %1, error code %2")
                    .arg(name).arg(m_device->error());
        debug(testo);
        return false;
    }

    if (!m_device->setBaudRate(QSerialPort::Baud115200)) {
        QString testo = QString("Can't set rate 115200 baud to port %1, error code %2")
                .arg(name).arg(m_device->error());
        debug(testo);
        return false;
    }

    if (!m_device->setDataBits(QSerialPort::Data8)) {
        QString testo = QString("Can't set 8 data bits to port %1, error code %2")
                .arg(name).arg(m_device->error());
        debug(testo);
        return false;
    }

    if (!m_device->setParity(QSerialPort::NoParity)) {
        QString testo = QString("Can't set no patity to port %1, error code %2")
                .arg(name).arg(m_device->error());
        debug(testo);
        return false;
    }

    if (!m_device->setStopBits(QSerialPort::OneStop)) {
        QString testo = QString("Can't set 1 stop bit to port %1, error code %2")
                .arg(name).arg(m_device->error());
        debug(testo);
        return false;
    }

    if (!m_device->setFlowControl(QSerialPort::NoFlowControl)) {
        QString testo = QString("Can't set no flow control to port %1, error code %2")
                .arg(name).arg(m_device->error());
        debug(testo);
        return false;
    }
//#endif
   return true;
}
