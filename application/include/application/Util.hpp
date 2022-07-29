#pragma once

#include <cstddef>
#include <cstdint>
#include <QString>
#include <QHash>

class QDir;
class QFileInfo;
class QLayout;
class QStackedWidget;

namespace rfcommon {
    class GameSession;
}

using qhash_result_t = size_t;
qhash_result_t qHash(const QDir& c, qhash_result_t seed=0) noexcept;
qhash_result_t qHash(const QFileInfo& c, qhash_result_t seed=0) noexcept;

namespace rfapp {

/*!
 * \brief Allows QString to be used in rfcommon::HashMap
 */
template <typename H=uint32_t>
struct QStringHasher
{
    typedef H HashType;
    HashType operator()(const QString& s) const {
        return qHash(s);
    }
};

/*!
 * \brief Deletes all widgets/layouts from a layout
 */
void clearLayout(QLayout* layout);

/*!
 * \brief Deletes all widgets in a QStackedWidget
 */
void clearStackedWidget(QStackedWidget* sw);

QString composeFileName(const rfcommon::Session* session);

}
