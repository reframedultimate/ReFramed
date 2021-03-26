#pragma once

#include <QHash>

namespace uh {

class FighterIDMapping
{
public:
    const QString* map(uint8_t fighterID) const;
    void add(uint8_t fighterId, const QString& name);

private:
    QHash<uint8_t, QString> map_;
};

}
