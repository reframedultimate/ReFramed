#pragma once

#include <cstddef>
#include <cstdint>
#include <QString>

class QDir;
class QFileInfo;
class QLayout;
class QStackedWidget;

namespace uh {
    class Recording;
}

using qhash_result_t = size_t;
qhash_result_t qHash(const QDir& c, qhash_result_t seed=0) noexcept;
qhash_result_t qHash(const QFileInfo& c, qhash_result_t seed=0) noexcept;

namespace uhapp {

/*!
 * \brief Deletes all widgets/layouts from a layout
 */
void clearLayout(QLayout* layout);

/*!
 * \brief Deletes all widgets in a QStackedWidget
 */
void clearStackedWidget(QStackedWidget* sw);

QString composeFileName(const uh::Recording* recording);

}
