#pragma once

#include <QHash>

namespace uh {

/*!
 * \brief Character animation states are all described with a single u16 integer
 * value. This class stores information for mapping a state value to a name.
 *
 * Mappings are not unique among all fighters. There are a base set of enum
 * values that match the pattern
 *
 *     FIGHTER_STATUS_KIND_xxx
 *
 * and then there is a character specific set that matches the pattern
 *
 *     FIGHTER_CHARNAME_STATUS_KIND_xxx
 *
 * The character specific sets share values among each other, so it's not
 * possible to simply map int <-> name.
 *
 * We also have multiple layers of names to go through. The first mapping is
 * integer to the enum name. The second layer is a mapping from enum name to
 * a more readable name such as "utilt" or "fsmash". The final layer is a
 * mapping to user defined names, as this application lets the user give their
 * own names to states.
 */
class FighterStatusMapping
{
public:
    const QString* statusToBaseEnumName(uint16_t status) const;
    const QString* statusToFighterSpecificEnumName(uint16_t status, uint8_t fighterID) const;

    void addBaseEnumName(uint16_t status, const QString& name);
    void addFighterSpecificEnumName(uint16_t status, uint8_t fighterID, const QString& name);

    const QHash<uint16_t, QString>& baseEnumNames() const { return baseEnumNames_; }
    const QHash<uint8_t, QHash<uint16_t, QString>>& fighterSpecificEnumNames() const { return fighterSpecificEnumNames_; }

private:
    QHash<uint16_t, QString> baseEnumNames_;
    QHash<uint8_t, QHash<uint16_t, QString>> fighterSpecificEnumNames_;
};

}
