#pragma once

#include "rfcommon/RealtimePlugin.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/Reference.hpp"
#include <memory>

namespace rfcommon {
    class Session;
}

class DamageTimePlotListener;
class DamageTimePlotCurveData;

class DamageTimePlotModel : public rfcommon::SessionListener
{
public:
    void setSession(rfcommon::Session* session);
    void clearSession(rfcommon::Session* session);

    int fighterCount() const;
    const rfcommon::SmallString<15>& name(int fighterIdx) const;
    int dataCount(int fighterIdx) const
        { return data_[fighterIdx].count(); }
    float secondsLeft(int fighterIdx, int i) const
        { return data_[fighterIdx][i].secondsLeft; }
    float damage(int fighterIdx, int i) const
        { return data_[fighterIdx][i].damage; }

    rfcommon::ListenerDispatcher<DamageTimePlotListener> dispatcher;

private:
    void appendDataPoint(int fighterIdx, rfcommon::FramesLeft frameNumber, float damage);

private:
    void onRunningGameSessionPlayerNameChanged(int playerIdx, const rfcommon::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) override {}
    void onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) override {}
    void onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) override {}
    void onRunningGameSessionWinnerChanged(int winnerIdx) override {}
    void onRunningSessionNewUniqueFrame(int frameIdx, const rfcommon::Frame& frame) override {}
    void onRunningSessionNewFrame(int frameIdx, const rfcommon::Frame& frame) override;

private:
    struct Point
    {
        float secondsLeft, damage;
    };

    rfcommon::Reference<rfcommon::Session> session_;
    rfcommon::SmallVector<rfcommon::Vector<Point>, 2> data_;
};
