#include "rfcommon/init.h"
#include "rfcommon/crc32.h"
#include "rfcommon/Log.hpp"
#include "rfcommon/tcp_socket.h"

/* ------------------------------------------------------------------------- */
int rfcommon_init(const char* logPath)
{
    rfcommon::Log::init(logPath);

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
    rfcommon::Log::deinit();
}
