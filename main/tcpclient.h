#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "esp_log.h"
#include <arpa/inet.h>
#include <errno.h>
#include <lwip/sockets.h>
#include <netdb.h> // struct addrinfo
#include <stdint.h>

#define LOG_TAG "socket_client"

int32_t tcpCreateClientSocket(const char* ip, uint16_t port);

int32_t tcpConnect(int32_t sockfd, const char* ip, uint16_t port);

/**
 * @brief Send data via socket passed as argument
 *
 * @param sockfd socket file descriptor
 * @param data data to be sent
 * @param len size of data to be sent
 * @return true when correctly sent
 * @return false when incorrectly sent
 */
bool tcpSendData(int32_t* sockfd, const char* data, uint32_t len);
/**
 * @brief
 *
 * @param sockfd
 * @param data
 * @param len
 * @return int32_t
 */
int32_t tcpRecvData(int32_t* sockfd, char* data, uint32_t len);

#endif // TCPCLIENT_H