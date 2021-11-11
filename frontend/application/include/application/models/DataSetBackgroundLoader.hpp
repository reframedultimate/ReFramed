#pragma once

#include "application/listeners/DataSetBackgroundLoaderListener.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/Reference.hpp"
#include "uh/DataSet.hpp"  // required by MOC
#include "uh/Recording.hpp"  // required by MOC, required by explicit template instantiation of Reference<Recording>
#include <QThread>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QWaitCondition>
#include <QFileInfo>
#include <unordered_map>

namespace uh {
    class DataSet;
    class GameSession;
}

namespace uhapp {

class RecordingGroup;

class DataSetBackgroundLoader : public QThread
{
    Q_OBJECT

public:
    struct OutData {
        QFileInfo recordingFile;
        RecordingGroup* group;
        uint32_t taskID;
    };

    struct InData {
        uh::Reference<uh::GameSession> recording;
        RecordingGroup* group;
        uint32_t taskID;
    };

    explicit DataSetBackgroundLoader(QObject* parent=nullptr);
    ~DataSetBackgroundLoader();

    void loadGroup(RecordingGroup* group);
    void cancelGroup(RecordingGroup* group);
    void cancelAll();

    uh::ListenerDispatcher<DataSetBackgroundLoaderListener> dispatcher;

signals:
    // NOTE: For internal use only
    void _dataSetLoaded(quint32 taskID, uh::DataSet* dataSet, RecordingGroup* group);

private slots:
    void onDataSetLoaded(quint32 taskID, uh::DataSet* dataSet, RecordingGroup* group);

private:
    void run() override;

private:
    QMutex mutex_;
    QQueue<OutData> out_;
    QQueue<InData> in_;
    QWaitCondition cond_;
    std::unordered_map<uint32_t, RecordingGroup*> pendingTasks_;
    int activeWorkers_ = 0;
    static uint32_t taskIDCounter_;
    bool requestShutdown_ = false;
};

}
