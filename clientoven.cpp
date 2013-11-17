#include "clientoven.h"

static const char headDebug[] = "[ClientOven]";

ClientOven::ClientOven(QObject *parent) :
    QObject(parent)
{
    m_debug = NULL;
    m_socket = NULL;
    // Gestione dei dati quando arrivano da un Client
    m_statoParser = STATO_TCPIP_DLE_STX;
}


/*!
 * \brief ClientOven::fromClients - Slot per gestire i dati che mi arrivano dal client
 */
void ClientOven::fromClientsSlot()
{
    QByteArray buffer = m_socket->readAll();
    if (m_debug)
    {
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Rx ";
        quint8 var;
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }
    int start = 0;
    int end = buffer.length();
    // Fin tanto che non sono arrivato al fondo del buffer che il client mi ha spedisco, decodifico!
    while (start < end)
    {
        if (decodeTcpIpMsg (buffer, m_buffer, start, m_statoParser))
        {
            // E' stato trovato un messaggio valido completo: mando un segnale per gestirlo
            emit toDeviceSignal(m_buffer, this);
            // Ripulisco il buffer perche' non serve piu'
            m_buffer.clear();
        }
    }
}

/*!
 * \brief ClientOven::toClient
 * \param buffer - buffer da spedire al Client: non devo fare nulla perche' e' gia' stato codificato da TcpGateway
 */
void ClientOven::toClientSlot (const QByteArray &buffer, ClientOven *client)
{
    if (m_socket && (client != this))
        send(buffer);
}

void ClientOven::toOneClientOnlySlot (const QByteArray &buffer, ClientOven *client)
{
    if (m_socket && (client == this))
        send(buffer);
}

void ClientOven::send (const QByteArray &buffer)
{
    if (m_debug)
    {
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Tx ";
        quint8 var;
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }
    m_socket->write(buffer);
}


void ClientOven::setSocket (QTcpSocket *socket)
{
    m_socket = socket;
    connect (m_socket, SIGNAL(readyRead()), this, SLOT(fromClientsSlot()));
}
