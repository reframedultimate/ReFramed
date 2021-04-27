#include "application/models/DataSetBackgroundLoader.hpp"
#include "application/models/RecordingGroup.hpp"
#include "uh/SavedRecording.hpp"
#include <QRunnable>
#include <QThreadPool>

namespace uhapp {

uint32_t DataSetBackgroundLoader::taskIDCounter_ = 0;

class DataSetBackgroundLoaderWorker : public QRunnable
{
public:
    DataSetBackgroundLoaderWorker(QMutex* mutex,
                                  QWaitCondition* cond,
                                  QQueue<DataSetBackgroundLoader::OutData>* in,
                                  QQueue<DataSetBackgroundLoader::InData>* out,
                                  int* activeWorkers)
        : mutex_(mutex)
        , cond_(cond)
        , in_(in)
        , out_(out)
        , activeWorkers_(activeWorkers)
    {
    }

signals:
    void recordingLoaded(uh::Recording* recording);

private:
    void run() override
    {
        mutex_->lock();
        while (true)
        {
            if (in_->isEmpty())
                break;

            auto item = in_->dequeue();
            mutex_->unlock();
                uh::Recording* recording = uh::SavedRecording::load(item.recordingFile.absoluteFilePath().toStdString());
            mutex_->lock();

            if (recording)
            {
                out_->enqueue({recording, item.group, item.taskID});
                cond_->wakeOne();
#ifndef NDEBUG
                for (int i = 0; i < recording->playerCount(); ++i)
                    assert(recording->playerStateCount(i) > 0);
#endif
            }
        }

        // The reading thread (which dequeues our output queue) will want to know
        // when all recordings were loaded, which will be the case when the
        // worker count drops to 0
        (*activeWorkers_)--;
        cond_->wakeOne();

        mutex_->unlock();
    }

private:
    QMutex* mutex_;
    QWaitCondition* cond_;
    QQueue<DataSetBackgroundLoader::OutData>* in_;
    QQueue<DataSetBackgroundLoader::InData>* out_;
    int* activeWorkers_;
};

// ----------------------------------------------------------------------------
DataSetBackgroundLoader::DataSetBackgroundLoader(QObject* parent)
    : QThread(parent)
{
    connect(this, &DataSetBackgroundLoader::_dataSetLoaded,
            this, &DataSetBackgroundLoader::onDataSetLoaded);

    start();
}

// ----------------------------------------------------------------------------
DataSetBackgroundLoader::~DataSetBackgroundLoader()
{
    cancelAll();
    mutex_.lock();
        requestShutdown_ = true;
        cond_.wakeOne();
    mutex_.unlock();

    wait();
    QThreadPool::globalInstance()->waitForDone();
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::loadGroup(RecordingGroup* group)
{
    printf("%d: loadGroup()\n", taskIDCounter_);
    mutex_.lock();
        for (const auto& fileInfo : group->absFilePathList())
            out_.enqueue({fileInfo, group, taskIDCounter_});

        while (activeWorkers_ < QThread::idealThreadCount())
        {
            DataSetBackgroundLoaderWorker* worker = new DataSetBackgroundLoaderWorker(
                &mutex_, &cond_, &out_, &in_, &activeWorkers_
            );

            QThreadPool::globalInstance()->start(worker);
            activeWorkers_++;
        }

        // Associate a new task ID with this group. If the same group was
        // cancelled, then loaded again, and the threads are still stuck on
        // processing the previous workload (of the same group), then the task
        // ID lets the threads know which results can be discarded and which
        // ones not.
        pendingTasks_.emplace(taskIDCounter_, group);
    mutex_.unlock();

    taskIDCounter_++;
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::cancelGroup(RecordingGroup* group)
{
    printf("cancelGroup()\n");
    mutex_.lock();
        // Delete all pending recording file names in the output queue that
        // originated from the specified recording group so the workers don't
        // load any more of them
        auto it = out_.begin();
        while (it != out_.end())
        {
            if (it->group == group)
                it = out_.erase(it);
            else
                ++it;
        }

        // Remove the group and the task ID from the set of pending tasks
        for (auto it = pendingTasks_.begin(); it != pendingTasks_.end(); )
        {
            if (it->second == group)
            {
                printf("%d: Cancelled group\n", it->first);
                it = pendingTasks_.erase(it);
            }
            else
                ++it;
        }
    mutex_.unlock();
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::cancelAll()
{
    mutex_.lock();
        out_.clear();
        pendingTasks_.clear();
    mutex_.unlock();
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::onDataSetLoaded(quint32 taskID, uh::DataSet* dataSet, RecordingGroup* group)
{
    uh::Reference<uh::DataSet> ds = dataSet;

    // Some sanity checks. There was a case where recordings had 0 player states
#ifndef NDEBUG
    assert(ds->dataPointCount() > 0);
#endif

    // Discard any data sets that are from cancelled tasks
    mutex_.lock();
        auto it = pendingTasks_.find(taskID);
        if (it == pendingTasks_.end())
        {
            printf("%d: Data set was cancelled, discarding...\n", taskID);
            mutex_.unlock();
            return;
        }

        // Task is complete
        pendingTasks_.erase(it);
    mutex_.unlock();

    printf("%d: Data set loaded\n", taskID);
    dispatcher.dispatch(&DataSetBackgroundLoaderListener::onDataSetBackgroundLoaderDataSetLoaded, group, ds);
}

// ----------------------------------------------------------------------------
void DataSetBackgroundLoader::run()
{
    struct PendingDataSet
    {
        uh::Reference<uh::DataSet> dataSet;
        RecordingGroup* group;
    };

    uh::SmallLinearMap<uint32_t, PendingDataSet, 4> pendingDataSets;

    mutex_.lock();
    while (true)
    {
        if (in_.isEmpty())
            cond_.wait(&mutex_);

        if (requestShutdown_)
            break;

        // There will usually be quite a few items in the queue (maybe 1000?)
        // so they should be transferred to a local container before merging
        // as to not block the main thread. Merging takes a bit of time as it
        // turns out.
        std::vector<InData> items;
        while (!in_.isEmpty())
            items.emplace_back(in_.dequeue());

        // The only way to know right now if all of the recordings are done
        // loading is by checking the active worker count
        bool completeDataSets = (activeWorkers_ == 0);

        mutex_.unlock();

            for (const auto& item : items)
            {
                // Ensure that we've created a target dataset for each task
                auto it = pendingDataSets.find(item.taskID);
                if (it == pendingDataSets.end())
                    it = pendingDataSets.insertOrGet(item.taskID, PendingDataSet{new uh::DataSet, item.group});

                it->value().dataSet->addRecordingNoSort(item.recording);
            }

            if (completeDataSets)
            {
                for (auto it : pendingDataSets)
                {
                    printf("%d: sorting\n", it->key());
                    it->value().dataSet->sort();
                    emit _dataSetLoaded(it->key(), it->value().dataSet.detach(), it->value().group);
                }
                pendingDataSets.clear();
            }

        mutex_.lock();
    }
    mutex_.unlock();
}

}
