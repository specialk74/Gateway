#ifndef RS232DEVICEPRIVATE_H
#define RS232DEVICEPRIVATE_H

#include "utils.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>

class Rs232DevicePrivate : public QSerialPort
{
    Q_OBJECT
public:
    explicit Rs232DevicePrivate(const QSerialPortInfo &info, QObject *parent = 0);
    ~Rs232DevicePrivate();

    void getVersion (quint8 & versioneMajor, quint8 & versioneMinor);
    void getComStat (quint8 &comstat);
    void sendMsgCan (const QByteArray &msgCAN);
    void setDebug (const bool &val);

signals:
    void fondItSignal();
    void toClientsSignal(const QByteArray &);

protected slots:
    void errorSlot(QSerialPort::SerialPortError serialPortError);
    void fromDeviceSlot();
    void sendMsgGetId();

protected:
    void handleMsgRxFromDevice (const QByteArray & buffer);
    bool configPort ();
    void sendMsg(const QByteArray &bufferIn);
    void debug (const QString &testo);

private:

#define TIPO_TX_RS232_CAN_MSG 0x00
#define TIPO_TX_RS232_CAN_GET_ID 0x0B

#define TIPO_RX_RS232_CAN_MSG 0x00
#define TIPO_RX_RS232_CAN_ID 0x0C

    quint8 m_checksum;
    QTimer m_timerAutodelete;
    QTimer m_timerSendGetId;
    QByteArray m_buffer;
    STATO_DECODER_RS232_MSG m_statoParser;

    quint8 m_versioneMajor;
    quint8 m_versioneMinor;
    quint8 m_comstat;
    quint8 m_statoInterno;
    bool m_debug;
};

#endif // RS232DEVICEPRIVATE_H
