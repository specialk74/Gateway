#ifndef RS232DEVICE_H
#define RS232DEVICE_H

#include <abstractdevice.h>
#include <QTimer>
class Rs232DevicePrivate;

class Rs232Device : public AbstractDevice
{
    Q_OBJECT

public:
    static Rs232Device * Instance(QObject *parent = 0);
    ~Rs232Device ();

protected:
    enum {
        TIPO_RX_TCPIP_ID  = 0x0D
    };

    static Rs232Device * m_Instance;
    explicit Rs232Device(QObject *parent);

    virtual void toDevice (const QByteArray &buffer);
    virtual quint8 getTipoIdFromDevice() { return TIPO_RX_TCPIP_ID; }
    virtual quint8 getComStatFromDevice();
    virtual void buildGetId(QByteArray & bufferForDevice);

    void getVersionFromDevice (quint8 & versioneMajor, quint8 & versioneMinor);

protected slots:
    void searchSlot ();
    void foundItSlot();
    void fromDeviceSlot(const QByteArray &buffer);

private:
    Rs232DevicePrivate * m_devicePrivate;
    QTimer m_timer;
};

#endif // RS232DEVICE_H
