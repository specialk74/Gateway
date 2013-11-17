#include <QDebug>
#include <QStringList>
#include <QRegExp>
#include <QCoreApplication>

#include "tcpgateway.h"
#include "rs232device.h"

#ifdef Q_WS_QWS
  #include "candevice.h"
#endif // #ifdef Q_WS_QWS

const char projectName[] = "ComOven2";
const int portServer = 6800;

static const int versioneMajor = 0;
static const int versioneMinor = 1;

QString getVersion ()
{
    return QString ("Version: %1.%2").arg(versioneMajor).arg(versioneMinor);
}

void usage (void)
{
    qDebug() << "Usage: " << projectName << "[OPTION...]";
    qDebug() << "  " << getVersion();
    qDebug() << "      -d, --debug         emette messaggi di debug";
    qDebug() << "      -p, --port=PORT     porta in ascolto del server (default" << portServer << ")";
    qDebug() << "      -V, --version       visualizza la versione";
    qDebug() << "      -h, --help          visualizza questo help";
}

AbstractDevice * connectToSerialDevice (const bool & debug)
{
    AbstractDevice * device = Rs232Device::Instance();
    Rs232Device::Instance()->setVersioneSw(versioneMajor, versioneMinor);
    Rs232Device::Instance()->setDebug(debug);

//    QObject::connect (TcpGateway::Instance(), SIGNAL(toDeviceSignal(QByteArray)), Rs232Device::Instance(), SLOT(fromClientSlot(QByteArray)));
//    QObject::connect (Rs232Device::Instance(), SIGNAL(toClientsSignal(QByteArray)), TcpGateway::Instance(), SLOT(fromDeviceSlot(QByteArray)));

    return device;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    quint16 port = portServer;
    bool debug = false;
    bool printUsage = false;

    QStringList args = app.arguments();
    QRegExp rxArgDebug("-d");
    QRegExp rxArgDebugLong("--debug");
    QRegExp rxArgHelp("-h");
    QRegExp rxArgHelpLong("--help");
    QRegExp rxArgVersion("-V");
    QRegExp rxArgVersionLong("--version");
    QRegExp rxArgPort("-p([0-9]{1,})");
    QRegExp rxArgPortLong("--port=([0-9]{1,})");
    QRegExp rxArgQws("-qws");


    for (int i = 1; i < args.size(); ++i) {
        if ((rxArgPort.indexIn(args.at(i)) != -1 )) {
            port =  rxArgPort.cap(1).toInt();
        }
        else if ((rxArgPortLong.indexIn(args.at(i)) != -1 )) {
            port =  rxArgPortLong.cap(1).toInt();
        }
        else if ((rxArgDebug.indexIn(args.at(i)) != -1) || (rxArgDebugLong.indexIn(args.at(i)) != -1)) {
            debug = true;
        }
        else if ((rxArgVersion.indexIn(args.at(i)) != -1) || (rxArgVersionLong.indexIn(args.at(i)) != -1)) {
            qDebug() << getVersion();
            return 0;
        }
        else if ((rxArgHelp.indexIn(args.at(i)) != -1) || (rxArgHelpLong.indexIn(args.at(i)) != -1)) {
            usage();
            return 0;
        }
        else if (rxArgQws.indexIn(args.at(i)) != -1)
        {
            // Non faccio nulla
        }
        else {
            qDebug() << "Uknown arg:" << args.at(i);
            printUsage = true;
        }
    }

    if (printUsage)
        usage();

    TcpGateway::Instance()->setDebug (debug);
    TcpGateway::Instance()->setPort(port);
    TcpGateway::Instance()->startListen();

    AbstractDevice * device = NULL;

#ifdef Q_WS_QWS
    device = CanDevice::Instance();
    if (!CanDevice::Instance()->exist())
    {
       delete CanDevice::Instance();
       device  =  connectToSerialDevice(debug);
    }
#else // #ifdef Q_WS_QWS
    device = connectToSerialDevice(debug);
#endif // #ifdef Q_WS_QWS

    if (device)
    {
        device->setVersioneSw(versioneMajor, versioneMinor);
        device->setDebug(debug);
        QObject::connect (TcpGateway::Instance(), SIGNAL(toDeviceSignal(QByteArray, ClientOven*)),
                          device, SLOT(fromClientSlot(QByteArray, ClientOven*)));
        QObject::connect (device, SIGNAL(toClientsSignal(QByteArray, ClientOven*)),
                          TcpGateway::Instance(), SLOT(fromDeviceSlot(QByteArray, ClientOven *)));
        QObject::connect (device, SIGNAL(toOneClientOnlySignal(QByteArray, ClientOven*)),
                          TcpGateway::Instance(), SLOT(toOneClientOnlySlot(QByteArray,ClientOven*)));
    }

    return app.exec();
}
