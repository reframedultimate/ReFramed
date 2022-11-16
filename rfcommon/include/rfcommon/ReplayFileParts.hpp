#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/BracketType.hpp"
#include "rfcommon/Round.hpp"
#include "rfcommon/ScoreCount.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/String.hpp"

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
            BracketType event,
            Round round,
            SetFormat format,
            ScoreCount score,
            uint8_t loserSide);
    ~ReplayFileParts();

    static ReplayFileParts fromFileName(const char* fileName);
    static ReplayFileParts fromMetaData(const rfcommon::MappingInfo* map, const rfcommon::MetaData* mdata);
    void updateFromMetaData(const rfcommon::MappingInfo* map, const rfcommon::MetaData* mdata);
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
    BracketType event() const { return event_; }
    Round round() const { return round_; }
    SetFormat setFormat() const { return format_; }
    ScoreCount score() const { return score_; }
    String stage() const { return stage_; }
    int playerCount() const { return playerNames_.count(); }
    String playerName(int idx) const { return playerNames_[idx]; }
    String fighterName(int idx) const { return fighterNames_[idx]; }
    bool isLoserSide(int idx) const { return loserSide_ & (1<<idx); }

private:
    String originalFileName_;
    SmallVector<String, 2> playerNames_;
    SmallVector<String, 2> fighterNames_;
    String date_;
    String time_;
    String stage_;
    BracketType event_;
    ScoreCount score_;
    Round round_;
    SetFormat format_;
    uint8_t loserSide_;
};

}
