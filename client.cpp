#include <QtWidgets/QApplication>
#include <iostream>
#include <QString>
#include "FtpClient.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("client");
    QCoreApplication::setApplicationVersion("version 0.1");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption serverHostOption(
            QStringList() << "host",
            QCoreApplication::translate("main", "ftp server host"),
            QCoreApplication::translate("main", "host")
    );
    parser.addOption(serverHostOption);

    QCommandLineOption serverPortOption(
            QStringList() << "port",
            QCoreApplication::translate("main", "ftp server port"),
            QCoreApplication::translate("main", "port")
    );
    parser.addOption(serverPortOption);

    parser.process(app);
    const QString hostString = parser.value(serverHostOption);
    const QString portString = parser.value(serverPortOption);
    FtpClient Client(hostString, portString);
    Client.run();
    return app.exec();
}