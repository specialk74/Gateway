#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>

#define DLE 0x10
#define STX 0x02
#define ETX 0x03

enum STATO_DECODER_TCPIP_MSG {
    STATO_TCPIP_DLE_STX,
    STATO_TCPIP_STX,
    STATO_TCPIP_DATO,
    STATO_TCPIP_ETX
};

enum STATO_DECODER_RS232_MSG {
    STATO_RS232_DLE_STX,
    STATO_RS232_STX,
    STATO_RS232_DATO,
    STATO_RS232_ETX,
    STATO_RS232_CHECKSUM
};

bool decodeTcpIpMsg (const QByteArray &bufferIn, QByteArray &bufferOut, int & idx, STATO_DECODER_TCPIP_MSG & stato);
bool decodeRs232Msg (const QByteArray &bufferIn, QByteArray &bufferOut, int & idx, STATO_DECODER_RS232_MSG & stato, quint8 &checksum);
void encode(const QByteArray &bufferIn, QByteArray &bufferOut);
quint8 cchecksum (const QByteArray & bufferIn);


quint32 fromBufferToNumber (const QByteArray &buffer);
QByteArray fromNumberToBuffer (quint32 number);
quint16 _htons(quint16 x);
quint16 _ntohs(quint16 x);
quint32 _htonl(quint32 x);
quint32 _ntohl(quint32 x);

#endif // UTILS_H
