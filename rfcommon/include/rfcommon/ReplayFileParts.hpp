#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/SetNumber.hpp"
#include "rfcommon/GameNumber.hpp"

namespace rfcommon {

class MappingInfo;
class MetaData;

class RFCOMMON_PUBLIC_API ReplayFileParts
{
public:
    ReplayFileParts(
            String originalFileName,
            SmallVector<String, 2>&& playerNames,
            SmallVector<String, 2>&& characterNames,
            String date,
            String time,
            String stage,
            SetNumber setNumber,
            GameNumber gameNumber,
            SetFormat format);
    ~ReplayFileParts();

    static ReplayFileParts fromFileName(const char* fileName);
    static ReplayFileParts fromMetaData(const rfcommon::MappingInfo* map, const rfcommon::MetaData* mdata);
    void updateMetaData(const rfcommon::MappingInfo* map, const rfcommon::MetaData* mdata);
    String toFileName() const;

    /*! 
     * \brief Returns the filename that was passed to ReplayFileParts::fromFileName()
     * to construct this object. If the object was constructed from metadat instead,
     * then this string will be empty.
     */
    const String& originalFileName() const { return originalFileName_; }

    bool hasMissingInfo() const;

    String date() const { return date_; }
    String time() const { return time_; }
    SetFormat setFormat() const { return format_; }
    SetNumber setNumber() const { return setNumber_; }
    GameNumber gameNumber() const { return gameNumber_; }
    String stage() const { return stage_; }
    int playerCount() const { return playerNames_.count(); }
    String playerName(int idx) const { return playerNames_[idx]; }
    String characterName(int idx) const { return characterNames_[idx]; }

    inline bool operator==(const ReplayFileParts& other) const
    {
        if (playerNames_.count() != other.playerNames_.count())
            return false;

        for (int i = 0; i != playerNames_.count(); ++i)
            if (playerNames_[i] != other.playerNames_[i])
                return false;

        for (int i = 0; i != characterNames_.count(); ++i)
            if (characterNames_[i] != other.characterNames_[i])
                return false;

        if (date_ != other.date_)
            return false;
        if (setNumber_ != other.setNumber_)
            return false;
        if (stage_ != other.stage_)
            return false;
        if (format_ != other.format_)
            return false;

        return true;
    }

private:
    String originalFileName_;
    SmallVector<String, 2> playerNames_;
    SmallVector<String, 2> characterNames_;
    String date_;
    String time_;
    String stage_;
    SetNumber setNumber_;
    GameNumber gameNumber_;
    SetFormat format_;
};

}
