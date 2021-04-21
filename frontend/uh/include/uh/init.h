#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Initializes the uh (ultimate hindsight) library. Call this before
 * using any other function or class.
 * \return Returns 0 on success, non-zero on failure.
 */
int uh_init(void);

/*!
 * \brief Cleans up any remaining global resources. Call this after a successful
 * call to uh_init().
 */
void uh_deinit(void);

#ifdef __cplusplus
}
#endif
