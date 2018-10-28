//
// Created by cyj on 10/27/18.
//

#ifndef FTPCLIENT_FTPCLIENT_H
#define FTPCLIENT_FTPCLIENT_H


#include <QtNetwork>
#include <QSet>
#include <QString>
#include <QObject>

typedef enum {
    Connected,
    DisConnected,
} ClientConnectStatus;
typedef enum {
    PassiveMode,
    ActiveMode,
} ClientMode;
typedef enum {
    Binary,
    Ascii
} TransferMode;
const QStringList supportedClientCommandList = {
        "ascii", "binary", "bye", "cd", "close",
        "delete", "dir", "disconnect", "exit",
        "get", "help", "lcd", "lls", "ls", "mkdir", "open", "pass",
        "passive", "put", "pwd", "quit", "recv", "rename",
        "rmdir", "size", "send", "type", "system", "user",
        "?", "!"
};

const QSet<QString> supportedClientCommandSet = QSet<QString>().fromList(supportedClientCommandList);

class FtpClient : QObject {
Q_OBJECT
public:
    ClientConnectStatus status;
    ClientMode clientMode;
    TransferMode transferMode;
    int client;
    QString serverHost;
    QString pasvServerHost;
    int pasvServerPort;

    FtpClient(QString hostString, QString portString);

    void run();

private:
    QStringList parserCommand(QString command);

    int parserResponse(QString);

    int login();

    int processCommand(QStringList commandList);

    int receiveCommand(int fd, char *buffer, int buffersize);//通用的接收命令数据
    int receiveCommand(char *buffer, int buffersize);//单纯从指令端口接收数据
    int sendCommand(QString command);

    int receiveBinaryData(QString command, QByteArray &data);

    int sendBinaryFile(QString localfilename);

    // 数据传输指令
    int pasv();

    int port();

    int retr(QString filename);

    int list();
    // 设置 Q_INVOKABLE 宏，通过元对象反射从命令名调用函数
    Q_INVOKABLE int ascii(QStringList);

    Q_INVOKABLE int binary(QStringList);

    Q_INVOKABLE int bye(QStringList);

    Q_INVOKABLE int cd(QStringList);

    Q_INVOKABLE int close(QStringList);

    Q_INVOKABLE int pass(QStringList);

    Q_INVOKABLE int _delete(QStringList);

    Q_INVOKABLE int dir(QStringList);

    Q_INVOKABLE int disconnect(QStringList);

    Q_INVOKABLE int exit(QStringList);

    Q_INVOKABLE int get(QStringList);

    Q_INVOKABLE int help(QStringList);

    Q_INVOKABLE int lcd(QStringList);

    Q_INVOKABLE int lls(QStringList);

    Q_INVOKABLE int ls(QStringList);

    Q_INVOKABLE int mkdir(QStringList);

    Q_INVOKABLE int open(QStringList);

    Q_INVOKABLE int passive(QStringList);

    Q_INVOKABLE int put(QStringList);

    Q_INVOKABLE int pwd(QStringList);

    Q_INVOKABLE int quit(QStringList);

    Q_INVOKABLE int recv(QStringList);

    Q_INVOKABLE int rmdir(QStringList);

    Q_INVOKABLE int size(QStringList);

    Q_INVOKABLE int send(QStringList);

    Q_INVOKABLE int type(QStringList);

    Q_INVOKABLE int system(QStringList);

    Q_INVOKABLE int user(QStringList);


};


#endif //FTPCLIENT_FTPCLIENT_H
