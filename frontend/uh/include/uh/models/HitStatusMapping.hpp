#pragma once

#include <QString>
#include <QHash>

namespace uh {

class HitStatusMapping
{
public:
    const QString* map(uint8_t status) const;
    void add(uint8_t stageID, const QString& name);

    const QHash<uint8_t, QString>& get() const { return map_; }

private:
    QHash<uint8_t, QString> map_;
};

}
