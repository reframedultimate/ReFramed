#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/HashMap.hpp"

namespace rfcommon {

/*!
 * \brief Contains a dictionary of hash40 hash values that map back to strings.
 * Essentially, this is the reverse operation of the hash40() function. These
 * strings were derived through brute force.
 * https://github.com/ultimate-research/param-labels
 *
 * Smash uses these to refer to animation states, resources, and other things.
 */
class RFCOMMON_PUBLIC_API Hash40Strings : public RefCounted
{
    Hash40Strings();
public:
    ~Hash40Strings();

    /*!
     * \brief Load all hash40 strings from the specified file.
     * \param fileName Path to the paramLabels.csv file.
     * \return Returns true if all labels were loaded successfully.
     */
    static Hash40Strings* loadCSV(const char* fileName);

    /*!
     * \brief Load all hash40 strings from the specified file.
     * \param fileName Path to the paramLabels.dat file.
     * \return Returns true if all labels were loaded successfully.
     */
    static Hash40Strings* loadBinary(const char* fileName);

    static Hash40Strings* makeEmpty();

    /*!
     * \brief Reverse-lookup the original string for the given motion value.
     * \return If the motion value is unknown, then this method will return
     * nullptr. Otherwise, a valid string is returned.
     */
    const char* toString(FighterMotion motion) const;
    const char* toString(FighterMotion motion, const char* fallback) const;

    /*!
     * \brief Runs the string through @see hash40(), but only returns the
     * motion value if it exists in the list of known values. If you need
     * to hash unknown values, use @see hash40().
     */
    FighterMotion toMotion(const char* str) const;

    const HashMap<FighterMotion, SmallString<31>, FighterMotion::Hasher>& entries() const { return entries_; }

private:
    HashMap<FighterMotion, SmallString<31>, FighterMotion::Hasher> entries_;
};

}
