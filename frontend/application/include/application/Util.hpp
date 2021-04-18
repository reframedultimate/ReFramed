#pragma once

#include <cstddef>
#include <cstdint>

class QDir;
class QFileInfo;
class QLayout;
class QStackedWidget;

using qhash_result_t = size_t;
qhash_result_t qHash(const QDir& c, qhash_result_t seed=0) noexcept;
qhash_result_t qHash(const QFileInfo& c, qhash_result_t seed=0) noexcept;

namespace uh {

/*!
 * \brief Deletes all widgets/layouts from a layout
 */
void clearLayout(QLayout* layout);

/*!
 * \brief Deletes all widgets in a QStackedWidget
 */
void clearStackedWidget(QStackedWidget* sw);

uint32_t crc32(const void* buf, size_t len, uint32_t crc=0);
uint32_t crc32(const char* str, uint32_t crc=0);
uint64_t hash40(const void* buf, size_t len);
uint64_t hash40(const char* str);

}
