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

/*!
 * \brief Global initialization of sockets. On Windows this calls WSAStartup().
 * \return Returns 0 on success, non-zero on failure.
 */
RFCOMMON_PUBLIC_API int tcp_socket_global_init(void);

/*!
 * \brief Global deinitialization of sockets. On Windows this calls WSACleanup().
 */
RFCOMMON_PUBLIC_API void tcp_socket_global_deinit(void);

/*!
 * \brief Attempts to connect to the host IP and port.
 *
 * If successful, a new socket handle is written to the sock parameter and
 * the socket is ready for communication. You must eventually call tcp_socket_close()
 * to clean up resources when you are done.
 *
 * If unsuccessful, the sock parameter will contain an error message instead.
 * Use tcp_socket_get_connect_error() to retrieve it. You should NOT call
 * tcp_socket_close() in this case.
 *
 * \return Returns 0 on success, non-zero on failure.
 */
RFCOMMON_PUBLIC_API int tcp_socket_connect_to_host(union tcp_socket* sock, const char* ip, uint16_t port);

/*!
 * \brief Get the error string after a failed connect attempt.
 * \note This function will crash the program if you try to use it on a valid socket.
 */
RFCOMMON_PUBLIC_API const char* tcp_socket_get_connect_error(union tcp_socket* sock);

/*!
 * \brief Shuts down reading and writing operations on the socket.
 */
RFCOMMON_PUBLIC_API void tcp_socket_shutdown(union tcp_socket* sock);

/*!
 * \brief Closes the socket.
 */
RFCOMMON_PUBLIC_API void tcp_socket_close(union tcp_socket* sock);

/*!
 * \brief Read data from the socket.
 * \note This doesn't necessarily read "len" number of bytes. It can return
 * early. If you want a function that reads the requested number of bytes
 * before returning, \see tcp_socket_read_exact().
 * \return Returns the number of bytes actually read. A value of 0 means EOF
 * (socket was shut down or closed on the other end). A negative value indicates
 * an error.
 */
RFCOMMON_PUBLIC_API int tcp_socket_read(union tcp_socket* sock, void* buf, int len);

/*!
 * \brief Write data to the socket.
 */
RFCOMMON_PUBLIC_API int tcp_socket_write(union tcp_socket* sock, const void* buf, int len);

/*!
 * \brief Converts the socket to a void*, making it easier to pass around.
 */
RFCOMMON_PUBLIC_API void* tcp_socket_to_handle(union tcp_socket* sock);

/*!
 * \brief Converts a void* back to a socket.
 */
RFCOMMON_PUBLIC_API union tcp_socket tcp_socket_from_handle(void* p);

/*!
 * \brief Blocks until the exact number of bytes requested has been read from
 * the socket, unlike tcp_socket_read().
 * \return Returns the number of bytes read. This function can return 0 to
 * indicate EOF (socket was shut down or closed on the other end) or a negative
 * value to indicate an error.
 */
RFCOMMON_PUBLIC_API int tcp_socket_read_exact(union tcp_socket* sock, void* buf, int len);

#ifdef __cplusplus
}
#endif
