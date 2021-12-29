#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Session.hpp"

namespace rfcommon {

class SavedGameSession;

class RFCOMMON_PUBLIC_API SavedSession : virtual public Session
{
protected:
    SavedSession();

public:
    static SavedSession* load(const String& fileName);

private:
    // Using void* here to avoid json objects leaking into the rest of the
    // program
    static SavedSession* loadVersion_1_0(const void* jptr);
    static SavedSession* loadVersion_1_1(const void* jptr);
    static SavedSession* loadVersion_1_2(const void* jptr);
    static SavedSession* loadVersion_1_3(const void* jptr);
    static SavedSession* loadVersion_1_4(const void* jptr);

public:
    int winner() const override
        { return winner_; }

private:
    int winner_;
};

}
