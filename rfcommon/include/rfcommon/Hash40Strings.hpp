#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
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
class RFCOMMON_PUBLIC_API Hash40Strings
{
public:
    /*!
     * \brief Load all hash40 strings from the specified file.
     * \param fileName Path to the paramLabels.csv file.
     * \return Returns true if all labels were loaded successfully.
     */
    bool loadCSV(const char* fileName);

    /*!
     * \brief Reverse-lookup the original string for the given motion value.
     * \return If the motion value is unknown, then this method will return
     * nullptr. Otherwise, a valid string is returned.
     */
    const char* stringOf(FighterMotion motion) const;

    /*!
     * \brief Does the exact same thing as @see hash40().
     */
    FighterMotion motionOf(const char* str) const;

private:
    // Motion values are actually 40 bits in length, where the lower 32 bits are
    // a crc32 checksum of the original string, and the upper byte is the length
    // of that original string. For our purposes, it should be enough to use the
    // lower 32 bits for our hashmap
    struct FighterMotionHasher
    {
        typedef uint32_t HashType;
        HashType operator()(FighterMotion motion) const {
            return motion.lower();
        }
    };

    HashMap<FighterMotion, SmallString<31>, FighterMotionHasher> entries_;
};

}
