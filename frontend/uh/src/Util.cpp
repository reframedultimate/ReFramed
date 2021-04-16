#include "uh/Util.hpp"
#include <QLayout>
#include <QLayoutItem>
#include <QWidget>
#include <QStackedWidget>
#include <QDir>
#include <QFileInfo>
#include <cstring>

// ----------------------------------------------------------------------------
qhash_result_t qHash(const QDir& c, qhash_result_t seed) noexcept
{
    return qHash(c.canonicalPath().constData(), seed);
}
qhash_result_t qHash(const QFileInfo& c, qhash_result_t seed) noexcept
{
    return qHash(c.absoluteFilePath().constData(), seed);
}

namespace uh {

// ----------------------------------------------------------------------------
void clearLayout(QLayout* layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->layout() != nullptr)
            item->layout()->deleteLater();
        if (item->widget() != nullptr)
            item->widget()->deleteLater();
    }
}

// ----------------------------------------------------------------------------
void clearStackedWidget(QStackedWidget* sw)
{
    while (sw->count())
    {
        QWidget* widget = sw->widget(0);
        sw->removeWidget(widget);
        widget->deleteLater();
    }
}

// ----------------------------------------------------------------------------
static uint32_t crc32_table[256];
static void calculate_crc32_table()
{
    for (int i = 0; i < 256; i++) {
        uint32_t rem = i;  /* remainder from polynomial division */
        for (int j = 0; j < 8; j++) {
            if (rem & 1) {
                rem >>= 1;
                rem ^= 0xedb88320;
            } else
                rem >>= 1;
        }
        crc32_table[i] = rem;
    }
}
uint32_t crc32(const void* buf, size_t len, uint32_t crc)
{
    static bool have_table = 0;
    if (have_table == false)
    {
        calculate_crc32_table();
        have_table = true;
    }

    crc = ~crc;
    const uint8_t* q = static_cast<const uint8_t*>(buf) + len;
    for (const uint8_t* p = static_cast<const uint8_t*>(buf); p < q; p++) {
        uint8_t octet = *p;  /* Cast to unsigned octet. */
        crc = (crc >> 8) ^ crc32_table[(crc & 0xff) ^ octet];
    }
    return ~crc;
}
uint32_t crc32(const char* str, uint32_t crc)
{
    return crc32(static_cast<const void*>(str), strlen(str), crc);
}

// ----------------------------------------------------------------------------
uint64_t hash40(const void* buf, size_t len)
{
    return static_cast<uint64_t>(crc32(buf, len, 0)) |
          (static_cast<uint64_t>(len) << 32);
}
uint64_t hash40(const char* str)
{
    return hash40(static_cast<const void*>(str), strlen(str));
}

}
