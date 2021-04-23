#pragma once

#include "application/listeners/DataSetBackgroundLoaderListener.hpp"
#include "uh/ListenerDispatcher.hpp"
#include "uh/Reference.hpp"
#include "uh/DataSet.hpp"  // required by MOC
#include <QThread>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QWaitCondition>
#include <QFileInfo>

namespace uh {
    class DataSet;
    class Recording;
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
    };

    struct InData {
        uh::Reference<uh::Recording> recording;
        RecordingGroup* group;
    };

    explicit DataSetBackgroundLoader(QObject* parent=nullptr);
    ~DataSetBackgroundLoader();

    void loadGroup(RecordingGroup* group);
    void cancelGroup(RecordingGroup* group);
    void cancelAll();

    uh::ListenerDispatcher<DataSetBackgroundLoaderListener> dispatcher;

signals:
    // NOTE: For internal use only
    void _dataSetLoaded(RecordingGroup* group);

private slots:
    void onDataSetLoaded(RecordingGroup* group);

private:
    void run() override;

private:
    QMutex mutex_;
    QQueue<OutData> out_;
    QQueue<InData> in_;
    QWaitCondition cond_;
    std::unordered_map<RecordingGroup*, uh::Reference<uh::DataSet>> dataSets_;
    int activeWorkers_ = 0;
    bool requestShutdown_ = false;
};

}
