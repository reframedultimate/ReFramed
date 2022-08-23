#include "rfcommon/tcp_socket.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

/* ------------------------------------------------------------------------- */
int tcp_socket_global_init(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
void tcp_socket_global_deinit(void)
{
}

/* ------------------------------------------------------------------------- */
int tcp_socket_connect_to_host(union tcp_socket* sock, const char* ip, uint16_t port)
{
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
        sock->error_msg = gai_strerror(result);
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
        sock->error_msg = strerror(errno);
        goto connect_failed;
    }

    freeaddrinfo(servinfo);
    sock->handle = (void*)(uintptr_t)sockfd;
    return 0;

    connect_failed     : freeaddrinfo(servinfo);
    getaddrinfo_failed : return -1;
}

/* ------------------------------------------------------------------------- */
const char* tcp_socket_get_connect_error(union tcp_socket* sock)
{
    return sock->error_msg;
}

/* ------------------------------------------------------------------------- */
void tcp_socket_shutdown(union tcp_socket* sock)
{
    int fd = (int)(uintptr_t)sock->handle;
    shutdown(fd, SHUT_RD);
}

/* ------------------------------------------------------------------------- */
void tcp_socket_close(union tcp_socket* sock)
{
    int fd = (int)(uintptr_t)sock->handle;
    close(fd);
}

/* ------------------------------------------------------------------------- */
int tcp_socket_read(union tcp_socket* sock, void* buf, int len)
{
    int fd = (int)(uintptr_t)sock->handle;
    return recv(fd, buf, len, 0);
}

/* ------------------------------------------------------------------------- */
int tcp_socket_write(union tcp_socket* sock, const void* buf, int len)
{
    int fd = (int)(uintptr_t)sock->handle;
    return send(fd, buf, len, 0);
}

/* ------------------------------------------------------------------------- */
void* tcp_socket_to_handle(union tcp_socket* sock)
{
    void* p = sock->handle;
    return p;
}

/* ------------------------------------------------------------------------- */
union tcp_socket tcp_socket_from_handle(void* p)
{
    union tcp_socket sock;
    sock.handle = p;
    return sock;
}

/* ------------------------------------------------------------------------- */
int tcp_socket_read_exact(union tcp_socket* sock, void* buf, int len)
{
    uint8_t* ptr = (uint8_t*)buf;
    while (len > 0)
    {
        int bytes_read = tcp_socket_read(sock, ptr, len);
        if (bytes_read <= 0)
            return bytes_read;
        ptr += bytes_read;
        len -= bytes_read;
    }

    return (int)(ptr - (uint8_t*)buf);
}
