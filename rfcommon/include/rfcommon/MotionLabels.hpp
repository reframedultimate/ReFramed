#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"

/*!
 * \brief Every user defined label is associated with one of the following
 * categories. This is primarily used in the UI for organization purposes.
 */
#define RFCOMMON_MOTION_LABEL_CATEGORIES_LIST \
    X(MOVEMENT,        "Movement")            \
    X(GROUND_ATTACKS,  "Ground Attacks")      \
    X(AERIAL_ATTACKS,  "Aerial Attacks")      \
    X(SPECIAL_ATTACKS, "Special Attacks")     \
    X(GRABS,           "Grabs")               \
    X(LEDGE,           "Ledge")               \
    X(DEFENSIVE,       "Defensive")           \
    X(DISADVANTAGE,    "Disadvantage")        \
    X(ITEMS,           "Items")               \
    X(MISC,            "Misc")                \
    X(UNLABELED,       "Unlabeled")

namespace rfcommon {

/*!
 * \brief This class handles everything to do with converting hash40 motion
 * values to strings and back, including managing user-defined labels.
 * 
 * Some notes on design decisions:
 *   - We use our own binary file format for the param labels instead of loading
 *     the official CSV file, as it cuts down on startup times by almost a second.
 *   - We categorize 
 */
class RFCOMMON_PUBLIC_API MotionLabels
{
public:
    enum Category
    {
#define X(name, str) name,
        RFCOMMON_MOTION_LABEL_CATEGORIES_LIST
#undef X
    };

    enum Usage
    {
        HASH40   = 0x01,
        READABLE = 0x02,
        NOTATION = 0x04,
        GROUP    = 0x08
    };

    MotionLabels();
    ~MotionLabels();

    /*!
     * \brief Load all hash40 strings from the specified file.
     * 
     * The official CSV file "ParamLabels.csv" is updated periodically
     * (https://github.com/ultimate-research/param-labels).
     * This function loads all labels from the CSV and then updates our
     * binary file "ParamLabels.bin" with the new data.
     * 
     * \param[in] fileName Path to the paramLabels.csv file.
     * \return Returns true if all labels were loaded successfully.
     */
    bool loadCSV(const char* fileNameUtf8);

    bool loadBinary(const char* fileNameUtf8);
    bool saveBinary(const char* fileNameUtf8);

    /*!
     * \brief Does a reverse-lookup for the original hash40 string.
     * \param[in] motion The motion value to convert.
     * \param[in] fallback The string to return if the lookup fails.
     * \return If the motion value is unknown, then this method will return
     * the fallback parameter, which defaults to nullptr.
     */
    const char* toHash40String(FighterMotion motion, const char* fallback=nullptr) const;

    /*!
     * \brief Converts a motion value into a string from a specific layer. You
     * can use layerIndex() to find e.g. a layer for a specific language.
     * \param[in] motion The motion value to convert.
     * \param[in] layerIdx The layer to search. If you specify a value of -1
     * then this method will always fail. This makes it possible to write:
     *     toString(motion, layerIndex("English"))
     * \param[in] fallback The string to return if the lookup fails.
     * \return Returns a string if found, or returns the fallback parameter,
     * which defaults to nullptr.
     */
    const char* toString(FighterMotion motion, int layerIdx, const char* fallback=nullptr) const;

    /*!
     * \brief Converts a motion value into a string from any layer marked as
     * NOTATION. You can use layerIndex() to find a layer for the preferred
     * notation.
     * \param[in] motion The motion value to convert.
     * \param[in] preferredLayerIdx The layer to search initially.If you specify
     * a value of -1 then all layers marked as NOTATION will be searched. This
     * makes it possible to write:  toNotation(motion, layerIndex("Pikacord"))
     * \param[in] fallback The string to return if the lookup fails.
     * \return Returns a string if found, or returns the fallback parameter,
     * which defaults to nullptr.
     */
    const char* toNotation(FighterMotion motion, int preferredLayerIdx, const char* fallback=nullptr) const;

    /*!
     * \brief Runs the string through @see hash40(), but only returns the
     * motion value if it exists in the list of known values. If you need
     * to hash unknown values, use @see hash40().
     * \return If not found, FighterMotion::makeInvalid() is returned. You
     * can check the validity with motion.isValid().
     */
    FighterMotion toMotion(const char* hash40Str) const;

    /*!
     * \brief Looks up all user labels matching the specified string for the
     * specified fighter. This is mostly used by the decision graph plugin
     * when compiling queries.
     * \param[in] fighterID The fighter to search the label for.
     * \param[in] label The label to search.
     * \return It's possible for a label to match multiple motion values. All
     * matching values are returned. If none are found the list will be empty.
     */
    SmallVector<FighterMotion, 4> toMotion(FighterID fighterID, const char* label) const;
};

}
