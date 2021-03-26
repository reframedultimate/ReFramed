#pragma once

#include <QHash>

namespace uh {

class StageIDMapping
{
public:
    const QString* map(uint16_t stageID) const;
    void add(uint16_t stageID, const QString& name);

private:
    QHash<uint16_t, QString> map_;
};

}
