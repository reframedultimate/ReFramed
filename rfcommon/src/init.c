#include "rfcommon/init.h"
#include "rfcommon/tcp_socket.h"
#include "rfcommon/crc32.h"

/* ------------------------------------------------------------------------- */
int rfcommon_init(void)
{
    if (tcp_socket_global_init() != 0)
        goto init_tcp_failed;

    crc32_init();

    return 0;

    init_tcp_failed : return -1;
}

/* ------------------------------------------------------------------------- */
void rfcommon_deinit(void)
{
    tcp_socket_global_deinit();
}
