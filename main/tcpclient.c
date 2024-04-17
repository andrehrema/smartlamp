#include <lwip/sockets.h>
#include "esp_log.h"
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include <stdint.h>
#include "tcpclient.h"
#include <sys/poll.h>

#define LOG_TAG "socket_client"

int32_t tcpCreateClientSocket(const char *ip, uint16_t port)
{
    int32_t sockfd;
    struct sockaddr_in serverAddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ESP_LOGE(LOG_TAG, "Error during socket instance creation");
        return -1;
    }

    sockfd = tcpConnect(sockfd, ip, port);

    return sockfd;
}

int32_t tcpConnect(int32_t sockfd, const char *ip, uint16_t port)
{
    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sockfd);
        ESP_LOGE(LOG_TAG, "Unsuccessful connection to the server");
        return -1;
    }

    return sockfd;
}

bool tcpSendData(int32_t sockfd, const char *data, uint32_t len)
{
    bool retVal = send(sockfd, data, len, 0) == len;
    if(!retVal)
    {
        ESP_LOGE(LOG_TAG, "Error sending packet");
    }

    return retVal;
}

int32_t tcpRecvData(int32_t sockfd, char *data, uint32_t len)
{
    struct pollfd pfds;
    pfds.fd = sockfd;
    pfds.events = POLLIN;

    nfds_t nfds = 1;
    uint32_t retVal = 0;

    int retPoll = poll(&pfds, nfds, 100);


    if (retPoll > 0) {
        if (pfds.revents & POLLIN) {
            retVal = recv(sockfd, data, len, 0);
        }
        else {
            ESP_LOGE(LOG_TAG, "Error during poll");
            retVal = -1;
        }
    }
    else if (retPoll == 0) {
        ESP_LOGE(LOG_TAG, "Timeout during poll");
        retVal = -1;
    }
    else if (retPoll == -1) {
        ESP_LOGE(LOG_TAG, "Error during poll");
        retVal = -1;
    }

    return retVal;
}