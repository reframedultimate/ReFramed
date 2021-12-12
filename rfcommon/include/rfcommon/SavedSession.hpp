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
    static SavedGameSession* load(const String& fileName);
    // NOTE: Since training session saving currently isn't supported, this
    // returns a SavedGameSession instead of just a SavedSession. If we
    // decide to support saving training sessions in the future then this
    // should be changed back to SavedSession*

private:
    // Using void* here to avoid json objects leaking into the rest of the
    // program
    static SavedGameSession* loadVersion_1_0(const void* jptr);
    static SavedGameSession* loadVersion_1_1(const void* jptr);
    static SavedGameSession* loadVersion_1_2(const void* jptr);
    static SavedGameSession* loadVersion_1_3(const void* jptr);
    static SavedGameSession* loadVersion_1_4(const void* jptr);

public:
    /*!
     * \brief Gets the absolute time of when the session ended in unix time
     * (milli-seconds since Jan 1 1970). May be slightly off by 1 second or so
     * depending on latency.
     *
     * In the case of a game session, this marks the last frame of gameplay.
     *
     * In the case of a training session, this marks the last frame of training
     * mode.
     */
    TimeStampMS timeStampEndedMs() const;

    DeltaTimeMS lengthMs() const
        { return timeStampStartedMs() - timeStampEndedMs(); }

    int winner() const override
        { return winner_; }

private:
    int winner_;
};

}
