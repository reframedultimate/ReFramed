#pragma once

#include "uh/config.hpp"
#include "uh/Session.hpp"

namespace uh {

class UH_PUBLIC_API SavedSession : virtual public Session
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
