#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <QObject>

class QSerialPort;

class ClientOven;

class PowerManager : public QObject
{
    Q_OBJECT
public:
    static PowerManager *Instance (QObject *parent= 0);
    void setDebug(const bool &val);
    void fromClientHandler (const QByteArray & buffer);
    bool setDevice (const QString &name);

#define TIPO_TX_TCPIP_POWER_MSG (0x00)

protected:
    explicit PowerManager(QObject *parent);
    void debug (const QString &testo);

signals:
    void toClientsSignal (const QByteArray &buffer, ClientOven *client);

protected slots:
    void fromDeviceSlot();

private:
    static  PowerManager        *m_Instance;
            bool                m_debug;
            QSerialPort         *m_device;
};

#endif // POWERMANAGER_H
