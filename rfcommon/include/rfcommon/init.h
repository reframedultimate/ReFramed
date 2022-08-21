#pragma once

#include "rfcommon/config.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Initializes the rfcommon library. Call this before
 * using any other function or class.
 * \param logPath This should be an absolute path to a writeable directory
 * where log files can be created. Make sure to NOT add a trailing slash.
 * \return Returns 0 on success, non-zero on failure.
 */
RFCOMMON_PUBLIC_API int rfcommon_init(const char* logPath);

/*!
 * \brief Cleans up any remaining global resources. Call this after a successful
 * call to rfcommon_init().
 */
RFCOMMON_PUBLIC_API void rfcommon_deinit(void);

#ifdef __cplusplus
}
#endif
