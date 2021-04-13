#pragma once

#include <cstddef>
#include <cstdint>

class QLayout;

namespace uh {

void clearLayout(QLayout* layout);

uint32_t crc32(const void* buf, size_t len, uint32_t crc=0);
uint32_t crc32(const char* str, uint32_t crc=0);
uint64_t hash40(const void* buf, size_t len);
uint64_t hash40(const char* str);

}
