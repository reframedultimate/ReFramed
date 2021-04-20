#pragma once

#include "uh/config.hpp"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t time_milli_seconds_since_epoch(void);
uint64_t time_qt_to_milli_seconds_since_epoch(const char* str);

#ifdef __cplusplus
}
#endif
