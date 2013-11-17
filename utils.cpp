#include "utils.h"

#include <QDataStream>
#include <QtEndian>

/*!
 * \brief decode
 * \param bufferIn : buffer di dati in ingresso
 * \param bufferOut : buffer di dati in uscita da cui vengon tolti i DLE duplicati nei dati
 * \param idx: punto di partenza da cui leggere il buffer in ingresso
 * \param stato: ultimo stato in cui era rimasto il buffer precedente
 * \return: true - se il buffer contiene un messaggio valido come protocollo
 *          false - se il buffer non contiene un messaggio valido completo
 */

bool decodeTcpIpMsg (const QByteArray &bufferIn, QByteArray &bufferOut, int & idx, STATO_DECODER_TCPIP_MSG & stato)
{
    int end = bufferIn.length();
    char dato;
    for (;idx < end; idx++)
    {
        dato = bufferIn[idx];
        switch (stato)
        {
        // Cerco il primo DLE della sequenza DLE-STX
        case STATO_TCPIP_DLE_STX:
            if (dato == DLE)
                stato = STATO_TCPIP_STX;
            break;
        // Cerco l'STX subito dopo il DLE iniziale
        case STATO_TCPIP_STX:
            if (dato == STX)
                stato = STATO_TCPIP_DATO; // Trovato
            else
                stato = STATO_TCPIP_DLE_STX; // Non trovato: ricomincio dall'inizio
            break;
        // Sono nella parte dati
        case STATO_TCPIP_DATO:
            if (dato == DLE)
                stato = STATO_TCPIP_ETX; // Se trovo un DLE controllo il carattere che viene dopo
            else
                bufferOut.append(dato); // Nulla di strano: lo inserisco nel buffer
            break;
        // Arrivo dallo stato precendente
        case STATO_TCPIP_ETX:
            if (dato == DLE)
            {
                // Ho trovato un altro DLE: lo inserisco nel buffer e mi riporto allo stato
                // precedente
                bufferOut.append(dato);
                stato = STATO_TCPIP_DATO;
            }
            else if (dato == ETX)
            {
                // Ho trovato ETX: ho finito riportando lo stato a quello iniziale
                stato = STATO_TCPIP_DLE_STX;
                // Inserisco questo per convalidare che ho gestito questo ultimo carattere
                idx++;
                return true;
            }
            else
            {
                // Errore: questo carattere non doveva essere qua. Pilusco il buffer
                // e ricomincio dall'inizio
                bufferOut.clear();
                stato = STATO_TCPIP_DLE_STX;
            }
            break;
        }
    }

    // Non ho trovato un buffer valido completo
    return false;
}


bool decodeRs232Msg (const QByteArray &bufferIn, QByteArray &bufferOut, int & idx, STATO_DECODER_RS232_MSG & stato, quint8 &checksum)
{
    int end = bufferIn.length();
    char dato;
printf ("dentro decodeRs232Msg idx: %d end: %d\n", idx, end);
    for (;idx < end; idx++)
    {
        dato = bufferIn[idx];
	printf (" - %x - ", dato);
        switch (stato)
        {
        // Cerco il primo DLE della sequenza DLE-STX
        case STATO_RS232_DLE_STX:
            if (dato == DLE)
                stato = STATO_RS232_STX;
            break;
        // Cerco l'STX subito dopo il DLE iniziale
        case STATO_RS232_STX:
            if (dato == STX)
            {
                stato = STATO_RS232_DATO; // Trovato
                checksum = 0;
            }
            else
                stato = STATO_RS232_DLE_STX; // Non trovato: ricomincio dall'inizio
            break;
        // Sono nella parte dati
        case STATO_RS232_DATO:
            if (dato == DLE)
                stato = STATO_RS232_ETX; // Se trovo un DLE controllo il carattere che viene dopo
            else
            {
                bufferOut.append(dato); // Nulla di strano: lo inserisco nel buffer
                checksum += dato;
            }
            break;
        // Arrivo dallo stato precendente
        case STATO_RS232_ETX:
            if (dato == DLE)
            {
                // Ho trovato un altro DLE: lo inserisco nel buffer e mi riporto allo stato
                // precedente
                bufferOut.append(dato);
                stato = STATO_RS232_DATO;
            }
            else if (dato == ETX)
            {
                stato = STATO_RS232_CHECKSUM;
            }
            else
            {
                // Errore: questo carattere non doveva essere qua. Pilusco il buffer
                // e ricomincio dall'inizio
                bufferOut.clear();
                stato = STATO_RS232_DLE_STX;
            }
            break;

        case STATO_RS232_CHECKSUM:
            stato = STATO_RS232_DLE_STX;
            if (dato == ~checksum)
            {
                checksum = 0;
                idx++;
                return true;
            }
            break;
        }
    }

    // Non ho trovato un buffer valido completo
printf ("\n");
    return false;
}

void insertHead (QByteArray &bufferOut)
{
    bufferOut.append(DLE);
    bufferOut.append(STX);
}

void insertTail (QByteArray &bufferOut)
{
    bufferOut.append(DLE);
    bufferOut.append(ETX);
}

quint8 cchecksum (const QByteArray & bufferIn)
{
    quint8 checksum = 0;
    int end = bufferIn.length();
    for (int idx = 0; idx < end; idx++)
        checksum += (unsigned char)bufferIn[idx];

    return checksum;
}

/*!
 * \brief encode
 * \param bufferIn : buffer di dati in ingresso
 * \param bufferOut : buffer di dati in uscita dove vengono duplicati i DLE ed inoltre
 *                    vengono aggiunti i DLE-STX in testa al buffer e DLE-ETX in coda
 *                    al buffer
 */
void encode (const QByteArray &bufferIn, QByteArray &bufferOut)
{
    insertHead(bufferOut);

    int end = bufferIn.length();
    char dato;
    for (int idx = 0; idx < end; idx++)
    {
        dato = bufferIn[idx];
        bufferOut.append(dato);
        // Se il dato da inserire e' il DLE allora lo duplico
        if (dato == DLE)
            bufferOut.append(DLE);
    }

    insertTail(bufferOut);
}

quint32 fromBufferToNumber (const QByteArray &buffer)
{
    QDataStream ds (buffer);
    quint32 temp;
    ds >> temp;
    return (temp);
}

QByteArray fromNumberToBuffer (quint32 number)
{
    QByteArray temp;
    QDataStream  ds (temp);
    ds << _htonl(number);

    return (temp);
}

#define BYTE_SWAP4(x) \
    (((x & 0xFF000000) >> 24) | \
     ((x & 0x00FF0000) >> 8)  | \
     ((x & 0x0000FF00) << 8)  | \
     ((x & 0x000000FF) << 24))

#define BYTE_SWAP2(x) \
    (((x & 0xFF00) >> 8) | \
     ((x & 0x00FF) << 8))

quint16 _htons(quint16 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP2(x);
    }
}

quint16 _ntohs(quint16 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP2(x);
    }
}

quint32 _htonl(quint32 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP4(x);
    }
}

quint32 _ntohl(quint32 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP4(x);
    }
}
