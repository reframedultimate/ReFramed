#pragma once

#include "uh/config.hpp"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tcp_socket
{
    void* handle;
};

UH_PUBLIC_API int tcp_socket_global_init(void);
UH_PUBLIC_API void tcp_socket_global_deinit(void);

UH_PUBLIC_API int tcp_socket_connect_to_host(struct tcp_socket* sock, const char* ip, uint16_t port);
UH_PUBLIC_API void tcp_socket_shutdown(struct tcp_socket* sock);
UH_PUBLIC_API void tcp_socket_close(struct tcp_socket* sock);
UH_PUBLIC_API int tcp_socket_read(struct tcp_socket* sock, void* buf, int len);
UH_PUBLIC_API int tcp_socket_write(struct tcp_socket* sock, const void* buf, int len);

#ifdef __cplusplus
}
#endif
