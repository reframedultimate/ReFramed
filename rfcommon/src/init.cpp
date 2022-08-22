#include "rfcommon/init.h"
#include "rfcommon/crc32.h"
#include "rfcommon/Log.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/tcp_socket.h"

/* ------------------------------------------------------------------------- */
int rfcommon_init(const char* logPath)
{
    NOPROFILE();

    rfcommon::Log::init(logPath);
#if defined(RFCOMMON_PROFILER)
    rfcommon::Profiler::init();
#endif

    if (tcp_socket_global_init() != 0)
        goto init_tcp_failed;

    crc32_init();

    return 0;

    init_tcp_failed : return -1;
}

/* ------------------------------------------------------------------------- */
void rfcommon_deinit(void)
{
    NOPROFILE();

    tcp_socket_global_deinit();
#if defined(RFCOMMON_PROFILER)
    rfcommon::Profiler::init();
#endif
    rfcommon::Log::deinit();
}
