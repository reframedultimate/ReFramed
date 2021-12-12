#pragma once

#include "rfcommon/config.hpp"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

union tcp_socket
{
    void* handle;
    const char* error_msg;
};

RFCOMMON_PUBLIC_API int tcp_socket_global_init(void);
RFCOMMON_PUBLIC_API void tcp_socket_global_deinit(void);

RFCOMMON_PUBLIC_API int tcp_socket_connect_to_host(union tcp_socket* sock, const char* ip, uint16_t port);
RFCOMMON_PUBLIC_API const char* tcp_socket_get_last_error(union tcp_socket* sock);
RFCOMMON_PUBLIC_API void tcp_socket_shutdown(union tcp_socket* sock);
RFCOMMON_PUBLIC_API void tcp_socket_close(union tcp_socket* sock);
RFCOMMON_PUBLIC_API int tcp_socket_read(union tcp_socket* sock, void* buf, int len);
RFCOMMON_PUBLIC_API int tcp_socket_write(union tcp_socket* sock, const void* buf, int len);

RFCOMMON_PUBLIC_API void* tcp_socket_to_handle(union tcp_socket* sock);
RFCOMMON_PUBLIC_API union tcp_socket tcp_socket_from_handle(void* p);

RFCOMMON_PUBLIC_API int tcp_socket_read_exact(union tcp_socket* sock, void* buf, int len);

#ifdef __cplusplus
}
#endif
