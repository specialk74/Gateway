#include <QCoreApplication>
#include <QDebug>
#include <QRegExp>
#include <QStringList>

#ifdef Q_WS_QWS
    #include "candevice.h"
#endif // #ifdef Q_WS_QWS

#include "handlermessage.h"
#include "powermanager.h"
#include "rs232device.h"
#include "tcpgateway.h"

        const   char    projectName[]   = "ComOven2";
        const   int     portServer      = 6800;

static  const   int     versioneMajor   = 0;
static  const   int     versioneMinor   = 2;

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

#if 0
AbstractDevice * connectToSerialDevice (const bool & debug)
{
    AbstractDevice * device = Rs232Device::Instance();
    Rs232Device::Instance()->setVersioneSw(versioneMajor, versioneMinor);
    Rs232Device::Instance()->setDebug(debug);

    return device;
}
#endif

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

    HandlerMessageTcpIp::Instance()->setDebug(debug);
    HandlerMessageTcpIp::Instance()->setVersioneSw(versioneMajor, versioneMinor);

    TcpGateway::Instance()->setPort(port);
    TcpGateway::Instance()->startListen();

    AbstractDevice * deviceCAN = NULL;

    PowerManager::Instance()->setDevice ("/dev/ttyAM0");
    HandlerMessageTcpIp::Instance()->setDevice(TcpGateway::Instance(), PowerManager::Instance());


#ifdef Q_WS_QWS
    device = CanDevice::Instance();
    if (!CanDevice::Instance()->exist())
    {
       delete CanDevice::Instance();
       //device  =  connectToSerialDevice(debug);
        device = Rs232Device::Instance();
    }
#else // #ifdef Q_WS_QWS
    //device = connectToSerialDevice(debug);
    deviceCAN = Rs232Device::Instance();
#endif // #ifdef Q_WS_QWS

    HandlerMessageTcpIp::Instance()->setDevice(TcpGateway::Instance(), deviceCAN);

    return app.exec();
}
