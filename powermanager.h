#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <QObject>

class QSerialPort;
class QTimer;

class ClientOven;

class PowerManager : public QObject
{
    Q_OBJECT
public:
    static PowerManager *Instance (QObject *parent= 0);
    void setDebug(const bool &val);
    void toDevice (const QByteArray & buffer);
    bool setDevice (const QString &name);
    void setWatchDog (const quint8 &val);

#define TIPO_TX_TCPIP_POWER_MSG (0x30)

protected:
    explicit PowerManager(QObject *parent);
    void debug (const QString &testo);

signals:
    void toClientsSignal (const QByteArray &buffer, ClientOven *client);

protected slots:
    void fromDeviceSlot();
    void timeoutWd ();

private:
    static  PowerManager        *m_Instance;
            bool                m_debug;
            QSerialPort         *m_device;
            quint8              m_statoWD;
            QTimer              *m_timerWD;
            quint8              m_lastCmd;
};

#endif // POWERMANAGER_H
