#pragma once

#include "uh/models/Recording.hpp"

class QJsonObject;

namespace uh {

class SavedRecording : public Recording
{
public:
    static SavedRecording* load(const QString& fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    MappingInfo& mappingInfo() { return mappingInfo_; }

private:
    SavedRecording(MappingInfo&& mapping,
                   QVector<uint8_t>&& playerFighterIDs,
                   QVector<QString>&& playerTags,
                   uint16_t stageID);

    static SavedRecording* loadVersion_1_0(const QJsonObject& json);
    static SavedRecording* loadVersion_1_1(const QJsonObject& json);
    static SavedRecording* loadVersion_1_2(const QJsonObject& json);
};

}
