#include "utils.h"

#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>

#include "powermanager.h"

static const char headDebug[] = "[PowerManager]";

static const char wdtMessageStr[] = {0x01};
static QByteArray wdtMessage (wdtMessageStr);

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
    m_statoWD = 0;
    m_device = NULL;

    m_device = new QSerialPort(this);
    Q_ASSERT(m_device);
    connect(m_device, SIGNAL(readyRead()), this, SLOT(fromDeviceSlot()));

    m_timerWD = new QTimer (this);
    Q_ASSERT(m_timerWD);
    connect (m_timerWD, SIGNAL(timeout()), this, SLOT(timeoutWd()));
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

    if (m_debug)
    {
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug;
        debugBuffer << " Rx ";
        int var;
        foreach (var, msgfromDevice) {
            debugBuffer << hex << var;
        }
    }

    if (msgfromDevice.length() != 2)
    {
        debug("Messaggio con lunghezza diversa da 2");
        return;
    }

    if ((msgfromDevice.at(0) ^ 0xFF) != msgfromDevice.at(1))
    {
        QString testo = QString ("CRC errato %1").arg(msgfromDevice.at(0) ^ 0xFF, 16, 0);
        debug(testo);
        return;
    }

    {
        QDataStream stream (&bufferToClients, QIODevice::WriteOnly);
        stream << (quint8) TIPO_TX_TCPIP_POWER_MSG;
        stream << _htonl((quint32) 7);
        stream << m_lastCmdRx;
        stream << msgfromDevice.at(0);
    }

    emit toClientsSignal(bufferToClients, NULL);
}


/*!
 * \brief PowerManager::toDevice
 * \param buffer - messaggio da spedire verso il Power Manager
 */
void PowerManager::toDevice (const QByteArray & buffer)
{    
    if (m_device && m_device->isOpen())
    {        
        QByteArray bufferToDevice = buffer;
        m_lastCmdRx = buffer.at(0);
        bufferToDevice.append(buffer.at(0) ^ 0xFF);

        if (m_debug)
        {
            QDebug debugBuffer = qDebug();
            debugBuffer << headDebug;
            debugBuffer << " Tx ";
            int var;
            foreach (var, bufferToDevice) {
                debugBuffer << hex << var;
            }
        }

        m_device->write(bufferToDevice);
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

    if (!m_device->setBaudRate(QSerialPort::Baud9600)) {
        QString testo = QString("Can't set rate 9600 baud to port %1, error code %2")
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
#if 0
    for (quint8 comando = 0; comando != 0xFF; comando++)
    {
        QByteArray array;
        for (quint8 comando2 = 0; comando2 != 0xFF; comando2++)
        {
            array.clear();
            array.append (comando);
            array.append (comando2);
            toDevice (array);
        }
    }
#endif
   return true;
}


void PowerManager::setWatchDog (const quint8 &val)
{
    m_statoWD = val;
    if (m_statoWD)
        m_timerWD->start(10*1000);
    else
        m_timerWD->stop();
}

void PowerManager::timeoutWd()
{
    toDevice(wdtMessage);
}
