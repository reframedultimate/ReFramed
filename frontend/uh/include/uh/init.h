#pragma once

#include "uh/config.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Initializes the uh (ultimate hindsight) library. Call this before
 * using any other function or class.
 * \return Returns 0 on success, non-zero on failure.
 */
UH_PUBLIC_API int uh_init(void);

/*!
 * \brief Cleans up any remaining global resources. Call this after a successful
 * call to uh_init().
 */
UH_PUBLIC_API void uh_deinit(void);

#ifdef __cplusplus
}
#endif
