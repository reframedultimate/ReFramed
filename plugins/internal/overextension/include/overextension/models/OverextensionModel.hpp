#pragma once

#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class Metadata;
}

class OverextensionListener;

class OverextensionModel
{
public:
    void startNewSession(const rfcommon::MappingInfo* map, const rfcommon::Metadata* mdata);
    void addFrame(int frameIdx, const rfcommon::FrameData* fdata);
    void addAllFrames(const rfcommon::FrameData* fdata);
    void clearAll();

    int fighterCount() const;
    int currentFighter() const;
    void setCurrentFighter(int fighterIdx) const;
    const char* playerName(int fighterIdx) const;
    const char* fighterName(int fighterIdx) const;

    int numTotal(int fighterIdx) const;
    int numTrueCombos(int fighterIdx) const;
    int numPositiveOverextensions(int fighterIdx) const;
    int numNegativeOverextensions(int fighterIdx) const;
    int numFrameTraps(int fighterIdx) const;

    rfcommon::ListenerDispatcher<OverextensionListener> dispatcher;

private:
};
