#include "application/platform/tcp_socket.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <WinSock2.h>
#   include <ws2tcpip.h>
#else
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <errno.h>
#endif

/* ------------------------------------------------------------------------- */
int tcp_socket_global_init(void)
{
#if defined(WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
        return 0;
    return -1;
#else
    return 0;
#endif
}

/* ------------------------------------------------------------------------- */
void tcp_socket_global_deinit(void)
{
#if defined(WIN32)
    WSACleanup();
#endif
}

/* ------------------------------------------------------------------------- */
int tcp_socket_connect_to_host(struct tcp_socket* sock, const char* ip, uint16_t port)
{
#if defined(WIN32)
    SOCKET sockfd;
    int result;
    char port_str[6];
    struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    sprintf(port_str, "%d", (int)port);
    if ((result = getaddrinfo(ip, port_str, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "Error resolving host address: %s\n", gai_strerror(result));
        goto getaddrinfo_failed;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET)
            continue;

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR)
        {
            closesocket(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "Failed to connect: %s\n", strerror(errno));
        goto connect_failed;
    }

    freeaddrinfo(servinfo);
    sock->handle = (void*)(uintptr_t)sockfd;
    return 0;

connect_failed: freeaddrinfo(servinfo);
getaddrinfo_failed: return -1;
#else
    int sockfd;
    int result;
    char port_str[6];
    struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    sprintf(port_str, "%d", (int)port);
    if ((result = getaddrinfo(ip, port_str, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "Error resolving host address: %s\n", gai_strerror(result));
        goto getaddrinfo_failed;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "Failed to connect: %s\n", strerror(errno));
        goto connect_failed;
    }

    freeaddrinfo(servinfo);
    sock->handle = (void*)(uintptr_t)sockfd;
    return 0;

    connect_failed     : freeaddrinfo(servinfo);
    getaddrinfo_failed : return -1;
#endif
}

/* ------------------------------------------------------------------------- */
void tcp_socket_shutdown(struct tcp_socket* sock)
{
#if defined(WIN32)
    SOCKET sockfd = (SOCKET)(uintptr_t)sock->handle;
    shutdown(sockfd, SD_RECEIVE);
#else
    int fd = (int)(uintptr_t)sock->handle;
    shutdown(fd, SHUT_RD);
#endif
}

/* ------------------------------------------------------------------------- */
void tcp_socket_close(struct tcp_socket* sock)
{
#if defined(WIN32)
    SOCKET sockfd = (SOCKET)(uintptr_t)sock->handle;
    closesocket(sockfd);
#else
    int fd = (int)(uintptr_t)sock->handle;
    close(fd);
#endif
}

/* ------------------------------------------------------------------------- */
int tcp_socket_read(struct tcp_socket* sock, void* buf, int len)
{
#if defined(WIN32)
    SOCKET sockfd = (SOCKET)(uintptr_t)sock->handle;
    return recv(sockfd, buf, len, 0);
#else
    int fd = (int)(uintptr_t)sock->handle;
    return recv(fd, buf, len, 0);
#endif
}

/* ------------------------------------------------------------------------- */
int tcp_socket_write(struct tcp_socket* sock, const void* buf, int len)
{
#if defined(WIN32)
    SOCKET sockfd = (SOCKET)(uintptr_t)sock->handle;
    return send(sockfd, buf, len, 0);
#else
    int fd = (int)(uintptr_t)sock->handle;
    return send(fd, buf, len, 0);
#endif
}
