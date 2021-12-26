#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/DataSetProcessorListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/SessionListener.hpp"

namespace rfcommon {

class AnalysisResult;
class Session;
class DataSet;

extern template class RFCOMMON_TEMPLATE_API SmallVector<DataSetProcessorListener*, 4>;
extern template class RFCOMMON_TEMPLATE_API ListenerDispatcher<DataSetProcessorListener>;

class DataSetOut;
class RFCOMMON_PUBLIC_API DataSetIn
{
public:
    virtual void onDataSetProcessorSessionStarted(rfcommon::Session* session) = 0;
    virtual void onDataSetProcessorNewUniqueFrame(rfcommon::Session* session, const SmallVector<int, 8>& stateIdxs) = 0;
    virtual void onDataSetProcessorSessionEnded(rfcommon::Session* session) = 0;
};

class RFCOMMON_PUBLIC_API DataSetOut
{
public:
    ListenerDispatcher<DataSetIn> dispatcher;

protected:
    void notifySessionStarted(rfcommon::Session* session);
    void notifyNewUniqueFrame(rfcommon::Session* session, const SmallVector<int, 8>& stateIdxs);
    void notifySessionEnded(rfcommon::Session* session);
};

class RFCOMMON_PUBLIC_API AnalysisResultOut
{
public:
    ListenerDispatcher<DataSetIn> dispatcher;

protected:
    AnalysisResult* newAnalysis();
    void finishAnalysis(AnalysisResult* a);
};

class RFCOMMON_PUBLIC_API DataSetProcessor : public SessionListener
{
public:
    // RunningGameSession events
    virtual void onRunningGameSessionPlayerNameChanged(int playerIdx, const SmallString<15>& name) = 0;
    virtual void onRunningGameSessionSetNumberChanged(SetNumber number) = 0;
    virtual void onRunningGameSessionGameNumberChanged(GameNumber number) = 0;
    virtual void onRunningGameSessionFormatChanged(const SetFormat& format) = 0;
    virtual void onRunningGameSessionWinnerChanged(int winnerPlayerIdx) = 0;

    // RunningSession events
    virtual void onRunningSessionNewUniquePlayerState(int playerIdx, const PlayerState& state) = 0;
    virtual void onRunningSessionNewPlayerState(int playerIdx, const PlayerState& state) = 0;
    virtual void onRunningSessionNewUniqueFrame(const SmallVector<PlayerState, 8>& states) = 0;
    virtual void onRunningSessionNewFrame(const SmallVector<PlayerState, 8>& states) = 0;
};

class RFCOMMON_PUBLIC_API DataSetProcessorOld : public DataSetProcessorListener
{
public:
    ListenerDispatcher<DataSetProcessorListener> dispatcher;

protected:
    void notifyDataSetProcessing(float progress, const String& info);
    void notifyDataSetCancelled();
    void notifyDataSetComplete(const DataSet* dataSet);
};

}
