#include "rs232device.h"
#include "rs232deviceprivate.h"

#include <QDebug>
#include <QTimer>

#define TIMER_RICERCA 1000

Rs232Device * Rs232Device::m_Instance = NULL;

/*!
 * \brief Rs232Device::Instance
 * \param parent
 * \return Pointer all'unica istanza di Rs232Device
 */
Rs232Device * Rs232Device::Instance(QObject *parent)
{
    if (m_Instance == NULL)
        new Rs232Device(parent);

    return m_Instance;
}

/*!
 * \brief Rs232Device::Rs232Device - CTor
 * \param parent
 */
Rs232Device::Rs232Device(QObject *parent) : AbstractDevice (parent)
{
    m_Instance = this;
    m_devicePrivate = NULL;
    connect (&m_timer, SIGNAL(timeout()), this, SLOT(searchSlot()));
    searchSlot();
}

/*!
 * \brief Rs232Device::~Rs232Device - DTor
 */
Rs232Device::~Rs232Device ()
{
    m_Instance = NULL;
}

/*!
 * \brief Rs232Device::toDevice
 * \param buffer - messaggio da spedire verso il bus CAN
 */
void Rs232Device::toDevice (const QByteArray &buffer)
{
    if (m_devicePrivate)
        m_devicePrivate->sendMsgCan (buffer);
}

/*************************************************************
 *
 *                        GET/SET
 *
 *
 *************************************************************/
/*!
 * \brief Rs232Device::getComStatFromDevice
 * \return - ritorna il valore del bus CAN
 */
quint8 Rs232Device::getComStatFromDevice()
{
    quint8 comstat = 0;
    if (m_devicePrivate)
        m_devicePrivate->getComStat(comstat);

    return comstat;
}

/*!
 * \brief Rs232Device::getVersionFromDevice - Richiamata dal Client per sapere la versione del converter: se il converter non e' collegato, ritorna 0.0
 * \param major - Aggiornato dal converter se esiste
 * \param minor - Aggiornato dal converter se esiste
 */
void Rs232Device::getVersionFromDevice (quint8 & versioneMajor, quint8 & versioneMinor)
{
    versioneMajor = 0;
    versioneMinor = 0;
    if (m_devicePrivate)
        m_devicePrivate->getVersion(versioneMajor, versioneMinor);
}

/*************************************************************
 *
 *                        SLOTS
 *
 *
 *************************************************************/
/*!
 * \brief Rs232Device::startSearch - Crea tanti oggetti di tipo Rs232DevicePrivate, per ogni seriale
 *                                   che si possa aprire nel sistema.
 *                                   Gli oggetti a loro volta provano a comunicare con il converter: se
 *                                   lo trovano, emetteranno un signal ("fondItSignal()") per far sapere a questo
 *                                   oggetto di interrompere la ricerca. Se l'oggetto Rs232DevicePrivate
 *                                   non trovera' il converter, si autodistruggera'.
 */
void Rs232Device::searchSlot ()
{
    if (m_devicePrivate)
        m_devicePrivate = NULL;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);

        // Provo ad aprire il device per controllare che esista fisicamente
        if (serial.open(QIODevice::ReadWrite))
        {
            // Il device esiste
            serial.close();
            Rs232DevicePrivate * devicePrivate = new Rs232DevicePrivate(info, this);
            devicePrivate->setDebug(getDebug());
            // Se l'oggetto Rs232DevicePrivate trovera' il converter, emettera' "fondItSignal()"
            connect (devicePrivate, SIGNAL(fondItSignal()), this, SLOT(foundItSlot()));
        }
    }

    // Faccio partire un timer per far ripartire la ricerca fino a quando un converter mi rispondera'
    m_timer.start(TIMER_RICERCA);
}

/*!
 * \brief Rs232Device::foundItSlot - Slot collegato con il segnale "fondItSignal()" proveniente da Rs232DevicePrivate quando viene trovato il converter
 */
void Rs232Device::foundItSlot()
{
    // Controllo se esiste gia' un "vecchio" converter: ma e' possibile? Sono stati collegati 2 converter?
    if (m_devicePrivate)
        delete m_devicePrivate;

    // Recupero chi ha trovato il converter
    m_devicePrivate = (Rs232DevicePrivate *) sender();
    // Interrompo il timer per la ricerca dei converter
    m_timer.stop();
    // Collego alla distruzione del converter, la ricerca di un nuovo converter
    connect (m_devicePrivate, SIGNAL(destroyed()), this, SLOT(searchSlot()));
    connect (m_devicePrivate, SIGNAL(toClientsSignal(QByteArray)), this, SLOT(fromDeviceSlot(QByteArray)));

    // Non mi serve piu' saperlo
    disconnect (m_devicePrivate, SIGNAL(fondItSignal()), this, SLOT(foundItSlot()));    
    m_devicePrivate->setDebug(getDebug());
}

void Rs232Device::fromDeviceSlot(const QByteArray &buffer)
{
    fromDeviceToClients(buffer);
}

void Rs232Device::buildGetId(QByteArray & bufferForDevice)
{
    QDataStream stream(&bufferForDevice, QIODevice::WriteOnly);
    stream << (quint8) getComStatFromDevice();
    quint8 versione_device_major, versione_device_minor;
    getVersionFromDevice(versione_device_major, versione_device_minor);
    stream << (quint8) versione_device_major;
    stream << (quint8) versione_device_minor;
}
