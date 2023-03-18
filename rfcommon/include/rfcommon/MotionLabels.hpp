#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/Vector.hpp"

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

#define RFCOMMON_MOTION_LABEL_USAGE_LIST      \
    X(READABLE,        "Readable")            \
    X(NOTATION,        "Notation")            \
    X(CATEGORIZATION,  "Categorization")

namespace rfcommon {

class MotionLabelsListener;

/*!
 * \brief This class handles everything to do with converting hash40 motion
 * values to strings and back, including managing user-defined labels.
 *
 * Some notes on design decisions:
 *   - We use our own binary file format for the param labels instead of loading
 *     the official CSV file, as it cuts down on startup times by almost a second.
 */
class RFCOMMON_PUBLIC_API MotionLabels : public RefCounted
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
#define X(name, str) name,
        RFCOMMON_MOTION_LABEL_USAGE_LIST
#undef X
    };

    MotionLabels();
    MotionLabels(const char* filePathUtf8);
    ~MotionLabels();

    bool load();
    bool load(const char* filePathUtf8);
    bool save() const;
    bool save(const char* filePathUtf8) const;

    /*!
     * \brief Load all hash40 strings from the specified CSV file.
     *
     * The official CSV file "ParamLabels.csv" is updated periodically
     * (https://github.com/ultimate-research/param-labels).
     * This function loads all labels from the CSV and then updates our
     * binary file with the new data.
     *
     * \param[in] fileName Path to the paramLabels.csv file.
     * \return Returns the number of values that were updated. Returns -1 if something went wrong.
     */
    int updateHash40FromCSV(const char* filePathUtf8);

    int importLayers(const char* filePathUtf8);
    bool exportLayers(SmallVector<int, 4> layers, const char* filePathUtf8) const;
    bool exportLayers(SmallVector<int, 4> layers, FighterID fighterID, const char* filePathUtf8) const;

    void setPreferredLayer(Usage usage, const char* layerName);
    const char* preferredLayer(Usage usage) const
            { return preferredLayers_[usage].cStr(); }

    const char* toPreferredNotation(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion, const char* fallback=nullptr) const;
    const char* toPreferredReadable(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion, const char* fallback=nullptr) const;
    const char* toPreferredCategory(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion, const char* fallback=nullptr) const;

    /*!
     * \brief Does a reverse-lookup for the original hash40 string.
     * \param[in] motion The motion value to convert.
     * \param[in] fallback The string to return if the lookup fails.
     * \return If the motion value is unknown, then this method will return
     * the fallback parameter, which defaults to nullptr.
     */
    const char* lookupHash40(FighterMotion motion, const char* fallback=nullptr) const;

    /*!
     * \brief Runs the string through @see hash40(), but only returns the
     * motion value if it exists in the list of known values. If you need
     * to hash unknown values, use @see hash40().
     * \return If not found, FighterMotion::makeInvalid() is returned. You
     * can check the validity with motion.isValid().
     */
    FighterMotion toMotion(const char* hash40Str) const;

    /*!
     * \brief Searches the table for the specified motion and returns the row
     * number where it was found, or -1 if not found.
     */
    int lookupRow(FighterID fighterID, FighterMotion motion) const;

    /*!
     * \brief Converts a motion value into a string from a specific layer. You
     * can use findLayer() to find e.g. a layer for a specific language.
     * \param[in] motion The motion value to convert.
     * \param[in] layerIdx The layer to search. If you specify a value of -1
     * then this method will always fail. This makes it possible to write:
     *      toString(motion, findLayer("English"))
     * \param[in] fallback The string to return if the lookup fails.
     * \return Returns a string if found, or returns the fallback parameter,
     * which defaults to nullptr.
     */
    const char* lookupLayer(FighterID fighterID, FighterMotion motion, int layerIdx, const char* fallback=nullptr) const;

    /*!
     * \brief Converts a motion value into a string from any layer matching the
     * specified name. If the a layer contains an empty string, then the search
     * continues to the next layer with a matching name, until a non-empty
     * entry is found. If all layers are empty, then the fallback parameter is
     * returned.
     * \param[in] motion The motion value to convert.
     * \param[in] preferredLayerIdx The layer to search initially. If you specify
     * a value of -1 then all layers marked with the specified usage will be
     * searched. This makes it possible to write:
     *      toString(motion, NOTATION, findLayer("Pikacord"))
     * \param[in] fallback The string to return if the lookup fails.
     * \return Returns a string if found, or returns the fallback parameter,
     * which defaults to nullptr.
     */
    const char* lookupGroup(FighterID fighterID, FighterMotion motion, Usage usage, const char* layerName, const char* fallback=nullptr) const;

    /*!
     * \brief Looks up all user labels matching the specified string for the
     * specified fighter. This is mostly used by the decision graph plugin
     * when compiling queries.
     * \param[in] fighterID The fighter to search the label for.
     * \param[in] label The label to search.
     * \return It's possible for a label to match multiple motion values. All
     * matching values are returned. If none are found the list will be empty.
     */
    SmallVector<FighterMotion, 4> toMotions(FighterID fighterID, const char* label) const;

    //! Returns the total number of layers
    int layerCount() const
        { return layerNames_.count(); }

    //! Returns the first layer index matching the specified name, or -1 if none is found
    int findLayer(const char* layerName) const;

    //! Returns the name of the specified layer
    const char* layerName(int layerIdx) const
        { return layerNames_[layerIdx].cStr(); }

    //! Returns the layer's usage
    Usage layerUsage(int layerIdx) const
        { return layerUsages_[layerIdx]; }

    /*!
     * \brief Creates a new layer with the specified name.
     * \param[in] nameUtf8 Name of the layer. The name doesn't have to be unique.
     * \param[in] usage Categorizes how the new layer will be used. This affects
     * where the layer will be inserted, and can possibly shift around other
     * existing layers.
     * \return Returns the index of the newly created layer. Operation will
     * never fail.
     */
    int newLayer(const char* nameUtf8, Usage usage);
    int newLayerNoNotify(const char* nameUtf8, Usage usage);

    //! Deletes the specified layer
    void deleteLayer(int layerIdx);
    void deleteLayers(rfcommon::SmallVector<int, 4> layerIdxs);

    //! Gives the specified layer a new name. The name doesn't have to be unique.
    void renameLayer(int layerIdx, const char* newNameUtf8);

    void changeUsage(int layerIdx, Usage newUsage);

    int moveLayer(int layerIdx, int insertIdx);

    /*!
     * \brief Attempts to merge all entries of the source layer into the target
     * layer. The source and target layers must share the same "Usage".
     *
     * Merge is implemented by first creating a third layer, then merging the
     * first and second layer values into the third. If any conflicts occur,
     * they will appear as "a|b" inside each row, indicating that merging
     * was not successful. The third layer's index is then returned from this
     * function. If merging is successful, then the target layer is replaced
     * with the third merge layer, and the source layer is deleted. A value of
     * "0" is returned to indicate success. NOTE: Even though "0" is a valid
     * layer index as well, it could never be the index of an unmerged layer
     * because there must be at least 2 layers in order to perform a merge in
     * the first place.
     * \param[in] targetLayerIdx
     * \param[in] sourceLayerIdx
     * \return Returns 0 if successful. Returns the layer index of the partially
     * merged layer if conflicts occurred. Returns -1 if the layers having
     * different usages.
     */
    int mergeLayers(int targetLayerIdx, int sourceLayerIdx);

    bool isLayerEmpty(int layerIdx) const;

    int rowCount(FighterID fighterID) const
        { return fighterID.value() < fighters_.count() ? fighters_[fighterID.value()].colMotionValue.count() : 0; }
    Category categoryAt(FighterID fighterID, int row) const
        { return fighters_[fighterID.value()].colCategory[row]; }
    FighterMotion motionAt(FighterID fighterID, int row) const
        { return fighters_[fighterID.value()].colMotionValue[row]; }
    const char* labelAt(FighterID fighterID, int layerIdx, int row) const
        { return fighters_[fighterID.value()].colLayer[layerIdx].labels[row].cStr(); }

    bool addUnknownMotion(FighterID fighterID, FighterMotion motion);
    bool addUnknownMotionNoNotify(FighterID fighterID, FighterMotion motion);
    int addNewLabel(FighterID fighterID, FighterMotion motion, Category category, int layerIdx, const char* label);
    int addNewLabelNoNotify(FighterID fighterID, FighterMotion motion, Category category, int layerIdx, const char* label);
    void changeLabel(FighterID fighterID, int row, int layerIdx, const char* newLabel);
    void changeCategory(FighterID fighterID, int row, Category newCategory);
    int propagateLabel(FighterID fighterID, int row, int layerIdx, bool replaceExisting=false, bool forceCreation=false);

    ListenerDispatcher<MotionLabelsListener> dispatcher;

private:
    void populateMissingFighters(FighterID fighterID);

private:
    rfcommon::String filePath_;

    // The structures below hold the entire table of each fighter. The table
    // consists of rows and columns. If a row is added or deleted, then it is
    // added/removed from all columns, guaranteeing that "colMotionValue",
    // "colCategory", "colLayer[x].labels" and "colLayer[x].usages" vectors
    // will always have the same size as each other.
    //
    // The hashmap "motionToRow" maps a given motion value to a row index
    // in the table. This is simple enough, since all motion values are unique
    // and map 1:1.
    //
    // Each hashmap in "layerMaps" corresponds to one layer. Each "labelToRow"
    // hashmap maps a user label to a set of rows in the table. Since user labels
    // don't have to be unique, the same label can return more than 1 row.
    //
    // Note that layers actually span over all fighters, and are not specific
    // to any particular fighter. This is why layer names are stored in an
    // outside vector "layerNames_". The UI can choose to hide layers who's
    // columns are all empty, if needed.
    struct Layer
    {
        Vector<SmallString<15, int8_t>> labels;  // Contains all rows of this layer column's labels in the table
    };

    struct Fighter
    {
        Vector<FighterMotion> colMotionValue;    // Contains all rows of the "motion value" column in the table
        Vector<Category>      colCategory;       // Contains all rows of the "category" column in the table
        SmallVector<Layer, 8> colLayer;          // Contains all rows of each layer in the table

        struct LayerMap
        {
            HashMap<SmallString<15, int8_t>, SmallVector<int, 4>> labelToRows;
        };

        HashMap<FighterMotion, int, FighterMotion::Hasher> motionToRow;
        SmallVector<LayerMap, 8> layerMaps;
    };

    String preferredLayers_[3];
    Vector<Fighter, int16_t> fighters_;
    Vector<String, int16_t> layerNames_;
    Vector<Usage, int16_t> layerUsages_;
    HashMap<FighterMotion, SmallString<31>, FighterMotion::Hasher> hash40s_;
};

}
