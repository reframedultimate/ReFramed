#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tcp_socket
{
    void* handle;
};

int tcp_socket_global_init(void);
void tcp_socket_global_deinit(void);

int tcp_socket_connect_to_host(struct tcp_socket* sock, const char* ip, uint16_t port);
void tcp_socket_shutdown(struct tcp_socket* sock);
void tcp_socket_close(struct tcp_socket* sock);
int tcp_socket_read(struct tcp_socket* sock, void* buf, int len);
int tcp_socket_write(struct tcp_socket* sock, const void* buf, int len);

#ifdef __cplusplus
}
#endif
