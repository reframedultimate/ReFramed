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

/*
class DataSetOut;
class RFCOMMON_PUBLIC_API DataSetProcessorListener
{
public:
    virtual void onDataSetProcessorStartSession(const String& name) = 0;
    virtual void onDataSetProcessorFrame(int frame, const SmallVector<FighterFrame, 8>& states) = 0;
    virtual void onDataSetProcessorEndSession(const String& name) = 0;
};

class RFCOMMON_PUBLIC_API AnalysisProcessorListener
{
public:
    virtual void onAnalysisProcessorStartSession(const String& name) = 0;
    virtual void onAnalysisProcessorStartSessionRange(const String& name) = 0;
    virtual void onAnalysisProcessorFrame(const SmallVector<FighterFrame, 8>& states) = 0;
    virtual void onAnalysisProcessorEndSessionRange(const String& name) = 0;
    virtual void onAnalysisProcessorEndSession(const String& name) = 0;
};

class RFCOMMON_PUBLIC_API DataSetProcessor : public DataSetProcessorListener
{
public:
    ListenerDispatcher<DataSetProcessorListener> dispatcher;

protected:
    void notifyDataSetStartSession(const String& name);
    void notifyDataSetFrame(Session* session, const SmallVector<FighterFrame, 8>& states);
    void notifyDataSetEndSession(const String& name);
};

class RFCOMMON_PUBLIC_API AnalysisProcessor : public DataSetProcessorListener
{
public:
    ListenerDispatcher<AnalysisProcessorListener> dispatcher;

protected:
    void notifyAnalysisProcessorStartSession(const String& name);
    void notifyAnalysisProcessorStartSessionRange(const String& name);
    void notifyAnalysisProcessorFrame(const SmallVector<FighterFrame, 8>& states);
    void notifyAnalysisProcessorEndSessionRange(const String& name);
    void notifyAnalysisProcessorEndSession(const String& name);
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
    virtual void onRunningSessionNewUniquePlayerState(int playerIdx, const FighterFrame& state) = 0;
    virtual void onRunningSessionNewPlayerState(int playerIdx, const FighterFrame& state) = 0;
    virtual void onRunningSessionNewUniqueFrame(const SmallVector<FighterFrame, 8>& states) = 0;
    virtual void onRunningSessionNewFrame(const SmallVector<FighterFrame, 8>& states) = 0;
};*/

class RFCOMMON_PUBLIC_API DataSetProcessor : public DataSetProcessorListener
{
public:
    ListenerDispatcher<DataSetProcessorListener> dispatcher;

protected:
    void notifyDataSetProcessing(float progress, const String& info);
    void notifyDataSetCancelled();
    void notifyDataSetComplete(const DataSet* dataSet);
};

}
