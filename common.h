#ifndef COMMON_H
#define COMMON_H

#include <stdint-gcc.h>

#define BUFFERSIZE 4096

int getRandomInt(int min, int max);

int createListenSocket(uint16_t port);

int createClientSocket(char *remote_addr, int remote_port);

#endif