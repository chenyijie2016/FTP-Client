#include "FtpClient.h"
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "common.h"
#include <QStringList>
#include <QDir>

QTextStream cin(stdin, QIODevice::ReadOnly);
QTextStream cout(stdout, QIODevice::WriteOnly);

FtpClient::FtpClient(QString hostString, QString portString) :
        clientMode(PassiveMode),
        status(DisConnected),
        client(-1) {
    if (hostString.isEmpty()) { ;// Do nothing
    } else {
        quint16 port;
        if (portString.isEmpty()) {
            open({"open", hostString});
        } else {
            open({"open", hostString, portString});
        }
    }
}

void FtpClient::run() {
    QString command;
    while (true) {
        cout << "ftp >";
        cout.flush();
        command = cin.readLine();
        auto res = parserCommand(command);
        processCommand(res);
    }
}


QStringList FtpClient::parserCommand(QString command) {
    QStringList parserdCommandList;
            foreach(QString cmd, command.trimmed().split(" ")) {
            if (!cmd.isEmpty()) {
                parserdCommandList << cmd;
            }
        }
    return parserdCommandList;
}

int FtpClient::processCommand(QStringList commandList) {
    //qDebug() << "Your Command List" << commandList;
    if (commandList.length() == 0) {
        return 0;
    }
    if (supportedClientCommandSet.find(commandList[0]) != supportedClientCommandSet.end()) {
        if (commandList[0] == "delete") {// delete与C++关键字冲突
            if (status == Connected)//delete需要已连接
                _delete(commandList);
            else
                cout << "Not connected." << endl;
            return 0;
        } else if (commandList[0] == "?" || commandList[0] == "help") {
            help(commandList);
        } else if (commandList[0] == "!" || commandList[0] == "exit") {
            exit(commandList);
        } else if (commandList[0] == "open") {
            if (status == DisConnected) {
                open(commandList);
            } else {
                cout << "Already connected to " << serverHost << ", use close first." << endl;
            }
        } else if (status == Connected) {
            int ret;
            bool bret = metaObject()->invokeMethod(this, commandList[0].toLatin1(), Q_RETURN_ARG(int, ret),
                                                   Q_ARG(QStringList, commandList));
            if (bret) {
                //;
            } else {
                cout << "Invoke Method " << commandList[0] << " failed" << endl;
            }
        } else {
            cout << "Not connected." << endl;
            return 0;
        }
    } else {
        cout << "?Invalid command" << endl;
        return -1;
    }
    return 0;
}

int FtpClient::parserResponse(QString line) {
    line = line.trimmed();
    QStringList cmdlist = line.split(" ");
    if (cmdlist.length() > 0) {
        bool ok = false;
        int code = cmdlist[0].toInt(&ok);
        if (ok) {
            //qDebug() << "code = " << code;
            return code;
        }
    }
    return -1;
}

int FtpClient::sendCommand(QString command) {
    //qDebug() << "SEND: " << command;
    const char *data = command.toLatin1().data();
    if (status == Connected) {
        ::write(client, data, strlen(data));
    }
}

int FtpClient::login() {
    QString localuser = qgetenv("USER");
    cout << QString("Name (%1:%2):").arg(serverHost).arg(localuser);
    cout.flush();
    QString username = cin.readLine();
    if (username.isEmpty()) {
        user({"user", "anonymous"});
    } else {
        user({"user", username});
    }
    return 0;
}

int FtpClient::ascii(QStringList) {
    sendCommand("TYPE I\r\n");
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    if (parserResponse(QString(buffer)) == 200) {
        cout << "Using ascii clientMode to transfer files." << endl;
        transferMode = Ascii;
    }
    return 0;
}

int FtpClient::binary(QStringList) {
    sendCommand("TYPE I\r\n");
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    if (parserResponse(QString(buffer)) == 200) {
        cout << "Using binary clientMode to transfer files." << endl;
        transferMode = Binary;
    }
    return 0;
}

int FtpClient::bye(QStringList) {
    return 0;
}

int FtpClient::cd(QStringList cmdlist) {
    if (cmdlist.length() < 2) {
        cout << "usage: cd remote-directory" << endl;
        return 0;
    }
    sendCommand(QString("CWD %1\r\n").arg(cmdlist[1]));
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    if (parserResponse(QString(buffer))) {
        cout << QString(buffer).trimmed() << endl;
    }
    return 0;
}

int FtpClient::close(QStringList) {
    return quit(QStringList());
}

int FtpClient::pass(QStringList cmdlist) {
    return passive(cmdlist);
}

int FtpClient::_delete(QStringList) {
    return 0;
}

int FtpClient::dir(QStringList) {
    return ls(QStringList());
}

int FtpClient::disconnect(QStringList) {
    return quit(QStringList());
}

int FtpClient::exit(QStringList) {
    quit(QStringList());
    ::exit(0);
}

int FtpClient::get(QStringList cmdlist) {
    if (cmdlist.length() < 2) {
        cout << "usage: get remote-file" << endl;
        return 0;
    }
    return retr(cmdlist[1]);


    return 0;
}

int FtpClient::help(QStringList) {
    cout << "Commands may be abbreviated.  Commands are:";
    for (auto i = 0; i < supportedClientCommandList.length(); i++) {
        if (i % 5 == 0)cout << endl;
        cout << QString("%1").arg(supportedClientCommandList[i], 16);
    }
    cout << endl;
    return 0;
}

int FtpClient::lcd(QStringList cmdlist) {
    if (cmdlist.length() == 1) {
        cout << QDir::currentPath() << endl;
        return 0;
    }
    if (QDir::setCurrent(cmdlist[1])) {
        cout << "Local directory now " << QDir::currentPath() << endl;
    } else {
        cout << QString("local: %1: No such file or directory").arg(cmdlist[1]) << endl;
    }
    return 0;
}

int FtpClient::ls(QStringList) {
    QByteArray data;
    receiveBinaryData("LIST\r\n", data);
    cout << QString(data);
    return 0;
}

int FtpClient::mkdir(QStringList cmdlist) {
    if (cmdlist.length() < 2) {
        cout << "usage: mkdir remote-directory" << endl;
    }
    sendCommand(QString("MKD %1\r\n").arg(cmdlist[1]));
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    cout << QString(buffer).trimmed() << endl;
}

int FtpClient::open(QStringList cmdlist) {
    if (status == DisConnected) {
        QString host;
        quint16 port;
        if (cmdlist.length() < 2) {
            cout << "Please input FTP server host." << endl;
            return -1;
        }
        host = cmdlist[1];
        if (cmdlist.length() >= 3) {
            port = cmdlist[2].toInt();
        } else {
            port = 21;
        }
        client = createClientSocket(host.toLatin1().data(), port);

        if (client > 0) {
            char buffer[BUFFERSIZE];
            memset(buffer, 0, BUFFERSIZE);
            receiveCommand(buffer, BUFFERSIZE);
            QString cmd = QString::fromLatin1(buffer);
            if (parserResponse(cmd) == 220) {
                cout << QString("Connected to %1.").arg(host) << endl;
                cout << cmd.trimmed() << endl;
                status = Connected;
                serverHost = host;
                login();
            } else {
                //TODO: Error handling
                qDebug() << "Error Response";
            }
        } else {
            cout << "ftp: connect: Invalid argument" << endl;
        }
    }
    return 0;
}

int FtpClient::passive(QStringList) {
    if (clientMode == ActiveMode) {
        clientMode = PassiveMode;
        cout << "Passive clientMode on." << endl;
    } else {
        clientMode = ActiveMode;
        cout << "Passive clientMode off." << endl;
    }
    return 0;
}

int FtpClient::put(QStringList cmdlist) {
    if (cmdlist.length() < 2) {
        cout << "usage: put local-filename" << endl;
        return 0;
    }
    return sendBinaryFile(cmdlist[1]);
}

int FtpClient::pwd(QStringList) {
    sendCommand(QString("PWD \r\n"));
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    if (parserResponse(QString(buffer))) {
        cout << QString(buffer).trimmed() << endl;
    }
    return 0;
}

int FtpClient::quit(QStringList) {
    sendCommand("QUIT \r\n");
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    if (parserResponse(QString(buffer)) == 221)
        cout << QString(buffer).trimmed() << endl;
    status = DisConnected;
    return 0;
}

int FtpClient::recv(QStringList cmdlist) {
    return get(cmdlist);
}

int FtpClient::rmdir(QStringList) {
    return 0;
}

int FtpClient::size(QStringList) {
    return 0;
}

int FtpClient::send(QStringList cmdlist) {
    return put(cmdlist);
}

int FtpClient::type(QStringList cmdlist) {
    if (cmdlist.length() == 1) {
        if (transferMode == Ascii) {
            cout << "Using ascii mode to transfer files." << endl;
        } else {
            cout << "Using binary mode to transfer files." << endl;
        }
    } else if (cmdlist[1] == "binary") {
        binary(QStringList());
    } else if (cmdlist[1] == "ascii") {
        ascii(QStringList());
    } else {
        cout << QString("%1 :unknown mode").arg(cmdlist[1]) << endl;
        return -1;
    }
    return 0;
}

int FtpClient::system(QStringList) {
    sendCommand("SYST \r\n");
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    if (parserResponse(QString(buffer)) == 215) {
        cout << QString(buffer).trimmed() << endl;
    } else {
        //TODO Error handling
    }
    return 0;
}

int FtpClient::user(QStringList cmdlist) {
    if (cmdlist.length() < 2) {
        cout << "usage: user username " << endl;
        return 0;
    }
    sendCommand(QString("USER %1\r\n").arg(cmdlist[1]));
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    if (parserResponse(QString(buffer)) == 331) {
        cout << QString(buffer).trimmed() << endl;
        cout << "Password:";
        cout.flush();
        QString password = cin.readLine();
        if (password.isEmpty()) {
            sendCommand(QString("PASS password\r\n"));
        } else {
            sendCommand(QString("PASS %1\r\n").arg(password));
        }
        receiveCommand(buffer, BUFFERSIZE);
        if (parserResponse(QString(buffer)) == 230) {
            cout << QString(buffer).trimmed() << endl;
            system(QStringList());
            binary(QStringList());
        } else {
            //TODO: Error handling
        }
    } else {
        // TODO: Error handling
    }
    return 0;
}

int FtpClient::receiveCommand(int fd, char *buffer, int buffersize) {
    int received_count;
    memset(buffer, 0, buffersize);
    int received_total = 0;
    do {
        received_count = read(fd, buffer + received_total, buffersize);
        if (received_count < 0) {
            return -1;
        } else if (received_count == 0) {
            break;
        } else if (received_count > 0) {
            received_total += received_count;
        }
        if (buffer[received_total - 1] == '\n') {
            break;
        }
    } while (received_count > 0 && received_count < buffersize);
}

int FtpClient::receiveCommand(char *buffer, int buffersize) {
    return receiveCommand(client, buffer, buffersize);
}

int FtpClient::pasv() {
    sendCommand("PASV \r\n");
    QRegExp pasvPattern(R"((\d+),(\d+),(\d+),(\d+),(\d+),(\d+))");
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    if (parserResponse(QString(buffer)) == 227) {
        if (pasvPattern.indexIn(QString(buffer)) != -1) {
            pasvServerHost = QString("%1.%2.%3.%4").
                    arg(pasvPattern.cap(1)).
                    arg(pasvPattern.cap(2)).
                    arg(pasvPattern.cap(3)).
                    arg(pasvPattern.cap(4));
            pasvServerPort = pasvPattern.cap(5).toInt() * 256
                             + pasvPattern.cap(6).toInt();
        } else {
            //TODO Error handling
            return -1;
        }
    } else {
        //TODO Error handling
        return -1;
    }
    return 0;
}

int FtpClient::port() {
    struct sockaddr_in inaddr;
    socklen_t connectedAddrLen = sizeof(inaddr);
    getsockname(client, (struct sockaddr *) &inaddr, &connectedAddrLen);
    int ip_long = inaddr.sin_addr.s_addr;
    int ip[4];
    for (int i = 0; i < 4; i++) {
        ip[i] = (ip_long >> (8 * i)) & 0xff;
    }

    uint16_t port;
    int fd;
    do {
        port = static_cast<uint16_t>(getRandomInt(20000, 30000));
        fd = createListenSocket(port);
    } while (fd < 0);


    sendCommand(QString("PORT %1,%2,%3,%4,%5,%6\r\n").arg(ip[0]).arg(ip[1]).arg(ip[2]).arg(ip[3])
                        .arg(port / 256).arg(port % 256));
    char buffer[BUFFERSIZE];
    receiveCommand(buffer, BUFFERSIZE);
    if (parserResponse(QString(buffer)) == 200) {
        cout << QString(buffer).trimmed() << endl;
    }
    return fd;
}


/*
 * 封装好的接收二进制数据的函数
 * 先根据客户端的不同连接属性自动发送PASV/PORT指令
 * 再发送要求的指令
 * 最后从数据端口读取数据到data中
 */
int FtpClient::receiveBinaryData(QString command, QByteArray &data) {
    char buffer[BUFFERSIZE];
    char dataBuffer[BUFFERSIZE];
    int n;
    int clientFd;
    int fd = -1;
    switch (clientMode) {
        case PassiveMode:
            if (pasv() == 0) { // 必须先发送 PASV
                fd = createClientSocket(pasvServerHost.toLatin1().data(), pasvServerPort);
            } else {
                //TODO Error handling
            }
            break;
        case ActiveMode:
            if ((fd = port()) == -1) {
                //TODO Error handling
                return -1;
            }
            break;
        default:
            break;
    }
    sendCommand(command);
    receiveCommand(buffer, BUFFERSIZE);
    switch (parserResponse(QString(buffer))) {
        case 150:
            cout << QString(buffer).trimmed() << endl;
            if (clientMode == PassiveMode) {
                clientFd = fd;
            } else if (clientMode == ActiveMode) {
                if ((clientFd = accept(fd, NULL, NULL)) == -1) {//等待服务器连接
                    cout << "Error Occur!" << endl;
                    return -1;
                }
            }
            n = 0;
            do {
                memset(dataBuffer, 0, BUFFERSIZE);
                n = read(clientFd, dataBuffer, BUFFERSIZE);
                if (n <= 0) {
                    // Error
                    break;
                }
                data.append(dataBuffer, n);
            } while (n > 0);
            receiveCommand(buffer, BUFFERSIZE);
            if (parserResponse(QString(buffer)) == 226) {
                cout << QString(buffer).trimmed() << endl;
            } else if (parserResponse(QString(buffer)) == 426) {
                cout << QString(buffer).trimmed() << endl;
            }
            ::close(clientFd);
            ::close(fd);
            break;
        case 426:
            cout << QString(buffer).trimmed() << endl;
            break;
        default:
            cout << QString(buffer).trimmed() << endl;
            break;
    }
    return 0;
}

/*
 * STOR 传输数据
 */
int FtpClient::sendBinaryFile(QString localfilename) {
    char buffer[BUFFERSIZE];
    int n;
    int clientFd = -1;
    QFile file(localfilename);
    if (!file.open(QIODevice::ReadOnly)) {
        cout << QString("No such file %1").arg(localfilename) << endl;
        return -1;
    }
    QByteArray filedatabuffer;
    int fd = -1;
    switch (clientMode) {
        case PassiveMode:
            if (pasv() == 0) { // 必须先发送 PASV
                fd = createClientSocket(pasvServerHost.toLatin1().data(), pasvServerPort);
            } else {
                //TODO Error handling
            }
            break;
        case ActiveMode:
            if ((fd = port()) == -1) {
                //TODO Error handling
                return -1;
            }
            break;
        default:
            break;
    }

    sendCommand(QString("STOR %1\r\n").arg(localfilename));
    receiveCommand(buffer, BUFFERSIZE);
    switch (parserResponse(QString(buffer))) {
        case 150:
            cout << QString(buffer).trimmed() << endl;
            if (clientMode == PassiveMode) {
                clientFd = fd;
            } else if (clientMode == ActiveMode) {
                if ((clientFd = accept(fd, NULL, NULL)) == -1) {//等待服务器连接
                    cout << "Error Occur!" << endl;
                    return -1;
                }
            }
            n = 0;
            do {
                filedatabuffer = file.read(BUFFERSIZE);
                write(clientFd, filedatabuffer.data(), filedatabuffer.length());
                n += filedatabuffer.length();
            } while (!file.atEnd());
            ::close(clientFd);
            ::close(fd);
            receiveCommand(buffer, BUFFERSIZE);
            if (parserResponse(QString(buffer)) == 226) {
                cout << QString(buffer).trimmed() << endl;
            } else if (parserResponse(QString(buffer)) == 426) {
                cout << QString(buffer).trimmed() << endl;
            }
            cout << QString("send %1 bytes.").arg(n) << endl;
            break;
        case 426:
            cout << QString(buffer).trimmed() << endl;
            break;
        default:
            cout << QString(buffer).trimmed() << endl;
            break;
    }
    return 0;
}

int FtpClient::retr(QString filename) {
    QFile file(filename);

//    if (file.exists()) {
//        cout << "There is a local file of the same name, overwrite?(N/y):" << endl;
//        QString chioce = cin.readLine();
//        if (chioce.isEmpty() or chioce == "N" or chioce == "n") {
//            return 0;
//        } else if (chioce == "y" or chioce == "Y") { ;
//        } else {
//            cout << "Invalid Choice" << endl;
//            return 0;
//        }
//    }

    if (!file.open(QIODevice::WriteOnly)) {
        cout << "Create Local File Error" << endl;
        return -1;
    };
    QByteArray fileData;

    if (receiveBinaryData(QString("RETR %1\r\n").arg(filename), fileData) == 0) {
        cout << QString("%1 bytes received").arg(fileData.length()) << endl;
        file.write(fileData);
        file.close();
        return 0;
    }

    return 0;
}

int FtpClient::lls(QStringList) {
    QDir dir(QDir::currentPath());
    QFileInfoList fileInfoList = dir.entryInfoList();
    for (auto &fileinfo :fileInfoList) {
        if (fileinfo.isDir())
            cout << "\033[34m" << fileinfo.fileName() << "\033[0m ";
        else {
            cout << fileinfo.fileName() << " ";
        }
    }
    cout << endl;
    return 0;
}





