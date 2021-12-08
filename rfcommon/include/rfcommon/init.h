#pragma once

#include "rfcommon/config.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Initializes the rfcommon (ultimate hindsight) library. Call this before
 * using any other function or class.
 * \return Returns 0 on success, non-zero on failure.
 */
RFCOMMON_PUBLIC_API int rfcommon_init(void);

/*!
 * \brief Cleans up any remaining global resources. Call this after a successful
 * call to rfcommon_init().
 */
RFCOMMON_PUBLIC_API void rfcommon_deinit(void);

#ifdef __cplusplus
}
#endif
