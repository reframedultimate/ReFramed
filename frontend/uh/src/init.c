#include "uh/init.h"
#include "uh/tcp_socket.h"
#include "uh/crc32.h"

/* ------------------------------------------------------------------------- */
int uh_init(void)
{
    if (tcp_socket_global_init() != 0)
        goto init_tcp_failed;

    crc32_init();

    return 0;

    init_tcp_failed : return -1;
}

/* ------------------------------------------------------------------------- */
void uh_deinit(void)
{
    tcp_socket_global_deinit();
}
