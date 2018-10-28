#include "common.h"
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>
#include <QtGlobal>
#include <QtCore/QTime>

static const char *CommandList[] =
        {"ABOR", "AUTH", "CWD", "DELE", "FEAT", "LIST", "MDTM", "MKD", "NLST", "PASS",
         "PASV", "PORT", "PWD", "QUIT", "RETR", "RMD", "RNFR", "RNTO", "SITE",
         "SIZE", "STOR", "SYST", "TYPE", "USER"};

int getRandomInt(int min, int max) {
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    return qrand() % (max - min) + min;
}

int createListenSocket(uint16_t port) {
    int listenfd = -1; //监听socket和连接socket不一样，后者用于数据传输
    struct sockaddr_in addr;
    //创建socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    //设置本机的ip和port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY; //监听"0.0.0.0"

    //将本机的ip和port与socket绑定
    if (bind(listenfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    //开始监听socket
    if (listen(listenfd, 10) == -1) {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    return listenfd;
}

// 创建与远程主机的连接
int createClientSocket(char *remote_addr, int remote_port) {
    int sockfd;
    struct sockaddr_in addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    //设置目标主机的ip和port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote_port);
    if (inet_pton(AF_INET, remote_addr, &addr.sin_addr) <= 0) { //转换ip地址:点分十进制-->二进制
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    //连接上目标主机（将socket和目标主机连接）-- 阻塞函数
    if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    return sockfd;
}
