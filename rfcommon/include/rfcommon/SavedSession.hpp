#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Session.hpp"

namespace rfcommon {

class SavedGameSession;
class StreamBuffer;

class RFCOMMON_PUBLIC_API SavedSession : virtual public Session
{
protected:
    SavedSession();

public:
    static SavedSession* load(const String& fileName);

private:
    // Using void* here to avoid json objects leaking into the rest of the
    // program
    static SavedSession* loadLegacy_1_0(const void* jptr);
    static SavedSession* loadLegacy_1_1(const void* jptr);
    static SavedSession* loadLegacy_1_2(const void* jptr);
    static SavedSession* loadLegacy_1_3(const void* jptr);
    static SavedSession* loadLegacy_1_4(const void* jptr);

    static SavedSession* loadModern(FILE* fp);
    static SavedSession* loadJSON_1_5(const void* jptr);
    static Vector<Frame> loadFrameData_1_5(StreamBuffer* data);

public:
    int winner() const override
        { return winner_; }

private:
    int winner_;
};

}
