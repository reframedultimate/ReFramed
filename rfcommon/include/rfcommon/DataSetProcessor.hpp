#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/DataSetProcessorListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/Reference.hpp"

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
    virtual void onDataSetProcessorStartSession(const String& name) = 0;
    virtual void onDataSetProcessorFrame(int frame, const SmallVector<PlayerState, 8>& states) = 0;
    virtual void onDataSetProcessorEndSession(const String& name) = 0;
};

class RFCOMMON_PUBLIC_API DataSetOut
{
public:
    ListenerDispatcher<DataSetIn> dispatcher;

protected:
    void notifyStartDataSetRange(const String& name);
    void notifyFrame(Session* session, const SmallVector<PlayerState, 8>& states);
    void notifyEndDataSetRange(const String& name);
};

class RFCOMMON_PUBLIC_API AnalysisResultOut
{
public:
    ListenerDispatcher<DataSetIn> dispatcher;

protected:
    void notifyStartAnalysis(const String& name);
    void notifyStartDataSetRange(const String& name);
    void notifyFrame(Session* session, const SmallVector<PlayerState, 8>& states);
    void notifyEndDataSetRange(const String& name);
    void notifyEndAnalysis(const String& name);
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
