#pragma once

#include <QHash>

namespace uh {

class FighterIDMapping
{
public:
    const QString* map(uint8_t fighterID) const;
    void add(uint8_t fighterId, const QString& name);

    const QHash<uint8_t, QString>& get() const { return map_; }

private:
    QHash<uint8_t, QString> map_;
};

}
