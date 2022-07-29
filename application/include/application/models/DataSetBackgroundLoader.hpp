#pragma once

#include "application/listeners/DataSetBackgroundLoaderListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/DataSet.hpp"  // required by MOC
#include <QThread>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QWaitCondition>
#include <QFileInfo>
#include <unordered_map>

namespace rfcommon {
    class DataSet;
    class GameSession;
}

namespace rfapp {

class ReplayGroup;

class DataSetBackgroundLoader : public QThread
{
    Q_OBJECT

public:
    struct OutData {
        QFileInfo recordingFile;
        ReplayGroup* group;
        uint32_t taskID;
    };

    struct InData {
        rfcommon::Reference<rfcommon::SavedGameSession> session;
        ReplayGroup* group;
        uint32_t taskID;
    };

    explicit DataSetBackgroundLoader(QObject* parent=nullptr);
    ~DataSetBackgroundLoader();

    void loadGroup(ReplayGroup* group);
    void cancelGroup(ReplayGroup* group);
    void cancelAll();

    rfcommon::ListenerDispatcher<DataSetBackgroundLoaderListener> dispatcher;

signals:
    // NOTE: For internal use only
    void _dataSetLoaded(quint32 taskID, rfcommon::DataSet* dataSet, ReplayGroup* group);

private slots:
    void onDataSetLoaded(quint32 taskID, rfcommon::DataSet* dataSet, ReplayGroup* group);

private:
    void run() override;

private:
    QMutex mutex_;
    QQueue<OutData> out_;
    QQueue<InData> in_;
    QWaitCondition cond_;
    std::unordered_map<uint32_t, ReplayGroup*> pendingTasks_;
    int activeWorkers_ = 0;
    static uint32_t taskIDCounter_;
    bool requestShutdown_ = false;
};

}
