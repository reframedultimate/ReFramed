#pragma once

#include <QHash>

namespace uh {

class StageIDMapping
{
public:
    const QString* map(uint16_t stageID) const;
    void add(uint16_t stageID, const QString& name);

    const QHash<uint16_t, QString>& get() const { return map_; }

private:
    QHash<uint16_t, QString> map_;
};

}
